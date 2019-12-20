#include "detector.h"
#include <cassert>
#include <fstream>
#include "utilities/json.hpp"

// #include <cuda_runtime_api.h>
#include "NvInfer.h"


constexpr long long int operator"" _MiB(long long unsigned int val)
{
    return val * (1 << 20);
}

void object_detection::Detector::loadModel(const char* modelPath, const std::string& boxConfigPath) {
  std::cout << "Loading model for object detection..." << std::endl;

  // Get prior box setup
  std::ifstream ifs(boxConfigPath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open prior box config file: " + boxConfigPath);
  }

  nlohmann::json priorBoxConfig = nlohmann::json::parse(ifs);
  for(const float it: priorBoxConfig) {
    _priorBoxes.push_back(it);
  }
  // TODO: put this into the prior box json file
  _numClasses = 7;

  // Load model
  auto builder = TRTUniquePtr<nvinfer1::IBuilder>(nvinfer1::createInferBuilder(_logger.getTRTLogger()));
  if(!builder) {
    throw std::runtime_error("TensorRT: creating builder failed!");
  }

  const auto explicitBatch = 1U << static_cast<uint32_t>(nvinfer1::NetworkDefinitionCreationFlag::kEXPLICIT_BATCH);

  auto network = TRTUniquePtr<nvinfer1::INetworkDefinition>(builder->createNetworkV2(explicitBatch));
  if(!network) {
    throw std::runtime_error("TensorRT: creating network failed!");
  }

  auto config = TRTUniquePtr<nvinfer1::IBuilderConfig>(builder->createBuilderConfig());
  if(!config) {
    throw std::runtime_error("TensorRT: creating config failed!");
  }

  auto parser = TRTUniquePtr<nvonnxparser::IParser>(nvonnxparser::createParser(*network, _logger.getTRTLogger()));
  if(!parser) {
    throw std::runtime_error("TensorRT: creating parser failed!");
  }

  std::cout << NV_ONNX_PARSER_VERSION << std::endl;

  // Construct the Network with an ONNX model
  const int verbosityLevel = static_cast<int>(nvinfer1::ILogger::Severity::kWARNING);
  bool parsed = parser->parseFromFile(modelPath, verbosityLevel);
  if(!parsed) {
    throw std::runtime_error("TensorRT: failed to parse onnx model!");
  }
  std::cout << "** Parsing done **" << std::endl;

  builder->setMaxBatchSize(1);
  builder->createOptimizationProfile();

  config->setMaxWorkspaceSize(1024_MiB);
  config->setFlag(nvinfer1::BuilderFlag::kFP16);
  // I think kINT8 is a bit faster but not as precise?
  // config->setFlag(nvinfer1::BuilderFlag::kINT8);
  // samplesCommon::setAllTensorScales(network.get(), 127.0f, 127.0f);

  // config->setEngineCapability(nvinfer1::EngineCapability::kSAFE_GPU);

  // Checking if Input is okay and optimize for fixed input
  assert(network->getNbInputs() == 1);
  _inputDim = network->getInput(0)->getDimensions();
  assert(_inputDim.nbDims == 4); // (batch_size, height, width, channels)

  // We want to optimize for a batch size of 1. Docu for optimization profiles: 
  // https://docs.nvidia.com/deeplearning/sdk/tensorrt-developer-guide/index.html#opt_profiles
  nvinfer1::IOptimizationProfile* profile = builder->createOptimizationProfile();
  const nvinfer1::Dims4 profileDims(1, _inputDim.d[1], _inputDim.d[2], _inputDim.d[3]);
  profile->setDimensions(network->getInput(0)->getName(), nvinfer1::OptProfileSelector::kMIN, profileDims);
  profile->setDimensions(network->getInput(0)->getName(), nvinfer1::OptProfileSelector::kOPT, profileDims);
  profile->setDimensions(network->getInput(0)->getName(), nvinfer1::OptProfileSelector::kMAX, profileDims);
  config->addOptimizationProfile(profile);

  // config->setAvgTimingIterations(1);
  // config->setMinTimingIterations(0);

  assert(network->getNbOutputs() == 1);
  _outputDim = network->getOutput(0)->getDimensions();
  assert(_outputDim.nbDims == 2); // (batch_size, classes + boxes)

  _engine = std::shared_ptr<nvinfer1::ICudaEngine>(builder->buildEngineWithConfig(*network, *config), InferDeleter());
  if(!_engine) {
   throw std::runtime_error("TensorRT: failed to build engine!");
  }
  else {
    std::cout << "** Built Engine **" << std::endl;
  }
}

void object_detection::Detector::detect(const cv::Mat& img) {
  // Fill input buffer with image
  // TODO: resize img according to the input_width and input_channels
  const int inputHeight = _inputDim.d[1];
  const int inputWidth = _inputDim.d[2];
  // const int inputChannels = _inputDim.d[3];
  // assert(img.size[0] == inputHeight && 
  //        img.size[1] == inputWidth && 
  //        img.channels() == inputChannels && 
  //        "Input image does not fit input layer size");

  for(int i = 0; i < inputHeight; i++)
  {
    for(int j = 0; j < inputWidth; j++)
    {
      cv::Vec3b vec = img.at<cv::Vec3b>(i, j);
      float b = static_cast<float>(vec[0]);
      float g = static_cast<float>(vec[1]);
      float r = static_cast<float>(vec[2]);
    }
  }

  // Run inference

  // Read result data and convert to image boxes
  int test = _priorBoxes.size();
}

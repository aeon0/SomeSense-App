#include "detector.h"
#include <cassert>
#include <fstream>
#include "utilities/json.hpp"

#include <cuda_runtime_api.h>
#include "NvInfer.h"

// Enables to write values such as 16_MiB
constexpr long long int operator"" _MiB(long long unsigned int val) {
    return val * (1 << 20);
}

void object_detection::Detector::loadModel(const std::string& modelPath, const std::string& boxConfigPath) {
  // Get prior box info
  std::ifstream ifs(boxConfigPath);
  if (!ifs.good()) {
    throw std::runtime_error("Could not open prior box config file: " + boxConfigPath);
  }

  nlohmann::json priorBoxConfig = nlohmann::json::parse(ifs);
  for(const float it: priorBoxConfig) {
    _priorBoxes.push_back(it);
  }
  // TODO: Put this into the prior box json file including class names for a cleaner interface
  // TODO: Also add input layer name to the config file (maybe rename it to model_settings.json or sth like that)
  _numClasses = 7;
  ifs.close();

  // Model path is in the form of /path/to/model.onnx, remove the .onnx part and change it to .engine
  const std::string enginePath = modelPath.substr(0, modelPath.size() - 5) + ".engine";

  std::ifstream file(enginePath.c_str());
  if(file.good()) {
    std::cout << "** Found Engine on Disk, loading... **" << std::endl;

    file.seekg(0, file.end);
    auto size = static_cast<size_t>(file.tellg());
    file.seekg(0, file.beg);
    std::vector<char> engineStream;
    engineStream.resize(size);
    file.read(engineStream.data(), size);
    file.close();

    nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(_logger.getTRTLogger());
    _engine = std::shared_ptr<nvinfer1::ICudaEngine>(runtime->deserializeCudaEngine(engineStream.data(), size), InferDeleter());
    runtime->destroy();
  }
  else {
    std::cout << "** No Engine found on Disk, creating... **" << std::endl;
    loadModelFromOnnx(modelPath);

    std::cout << "** Save Engine to Disk **" << std::endl;
    // Save the model to disk to skip the engine creation the next time
    nvinfer1::IHostMemory* serializedModel =_engine->serialize();
    std::ofstream p(enginePath, std::ios::binary);
    p.write((const char*)serializedModel->data(),serializedModel->size());
    p.close();
    serializedModel->destroy();
  }
}

void object_detection::Detector::loadModelFromOnnx(const std::string& modelPath) {
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

  // Construct the Network with an ONNX model
  const int verbosityLevel = static_cast<int>(nvinfer1::ILogger::Severity::kWARNING);
  bool parsed = parser->parseFromFile(modelPath.c_str(), verbosityLevel);
  if(!parsed) {
    throw std::runtime_error("TensorRT: failed to parse onnx model!");
  }
  std::cout << "** Parsing done **" << std::endl;

  config->setMaxWorkspaceSize(16_MiB);
  config->setFlag(nvinfer1::BuilderFlag::kFP16);
  // I think kINT8 is a bit faster but not as precise?
  // config->setFlag(nvinfer1::BuilderFlag::kINT8);
  // samplesCommon::setAllTensorScales(network.get(), 127.0f, 127.0f);

  // config->setEngineCapability(nvinfer1::EngineCapability::kSAFE_GPU);

  // Checking if Input is okay and optimize for fixed input
  assert(network->getNbInputs() == 1);
  nvinfer1::Dims inputDim = network->getInput(0)->getDimensions();
  assert(inputDim.nbDims == 4); // (batch_size, height, width, channels)

  // We want to optimize for a batch size of 1. Docu for optimization profiles: 
  // https://docs.nvidia.com/deeplearning/sdk/tensorrt-developer-guide/index.html#opt_profiles
  nvinfer1::IOptimizationProfile* profile = builder->createOptimizationProfile();
  const nvinfer1::Dims4 profileDims(1, inputDim.d[1], inputDim.d[2], inputDim.d[3]);
  network->getInput(0)->setDimensions(profileDims);
  profile->setDimensions(network->getInput(0)->getName(), nvinfer1::OptProfileSelector::kMIN, profileDims);
  profile->setDimensions(network->getInput(0)->getName(), nvinfer1::OptProfileSelector::kOPT, profileDims);
  profile->setDimensions(network->getInput(0)->getName(), nvinfer1::OptProfileSelector::kMAX, profileDims);
  config->addOptimizationProfile(profile);

  assert(network->getNbOutputs() == 1);
  nvinfer1::Dims outputDim = network->getOutput(0)->getDimensions();
  assert(outputDim.nbDims == 2); // (batch_size, classes + boxes)

  _engine = std::shared_ptr<nvinfer1::ICudaEngine>(builder->buildEngineWithConfig(*network, *config), InferDeleter());
  if(!_engine) {
    throw std::runtime_error("TensorRT: failed to build engine!");
  }
}

void object_detection::Detector::detect(const cv::Mat& img) {
  int inputIndex = _engine->getBindingIndex("input_1");
  if(inputIndex == -1) {
    throw std::runtime_error("Name for Input Layer is not found in model!");
  }

  nvinfer1::Dims inputDim = _engine->getBindingDimensions(inputIndex);
  assert(inputDim.nbDims == 4); // (batch_size, height, width, channels)
  const int batchSize = inputDim.d[0]; // should be 1
  const int inputHeight = inputDim.d[1];
  const int inputWidth = inputDim.d[2];
  const int inputChannels = inputDim.d[3];

  size_t inputSize = batchSize * inputHeight * inputWidth * inputChannels * sizeof(float);


  int* cpuPtr;
  int* gpuPtr;
  cudaHostAlloc((void**)&cpuPtr, inputSize, cudaHostAllocMapped);

	// if( !cpuPtr || !gpuPtr || size == 0 )
	// 	return false;

	// //CUDA(cudaSetDeviceFlags(cudaDeviceMapHost));

	// if( CUDA_FAILED(cudaHostAlloc(cpuPtr, size, cudaHostAllocMapped)) )
	// 	return false;

	// if( CUDA_FAILED(cudaHostGetDevicePointer(gpuPtr, *cpuPtr, 0)) )
	// 	return false;

	// memset(*cpuPtr, 0, size);
	// //printf(LOG_CUDA "cudaAllocMapped %zu bytes, CPU %p GPU %p\n", size, *cpuPtr, *gpuPtr);
	// return true;

  // Alloc cuda memory
// 	if( !cudaAllocMapped((void**)&mInputCPU, (void**)&mInputCUDA, inputSize) )
// 	{
// 		printf("failed to alloc CUDA mapped memory for tensorNet input, %zu bytes\n", inputSize);
// 		return false;
// 	}

  auto context = TRTUniquePtr<nvinfer1::IExecutionContext>(_engine->createExecutionContext());
  if (!context) {
    throw std::runtime_error("TensorRT: Creating inference context failed!");
  }



  // Fill input buffer with image
  // TODO: resize img according to the input_width and input_channels
  // const int inputChannels = _inputDim.d[3];
  // assert(img.size[0] == inputHeight && 
  //        img.size[1] == inputWidth && 
  //        img.channels() == inputChannels && 
  //        "Input image does not fit input layer size");

  // for(int i = 0; i < inputHeight; i++)
  // {
  //   for(int j = 0; j < inputWidth; j++)
  //   {
  //     cv::Vec3b vec = img.at<cv::Vec3b>(i, j);
  //     float b = static_cast<float>(vec[0]);
  //     float g = static_cast<float>(vec[1]);
  //     float r = static_cast<float>(vec[2]);
  //   }
  // }

  // Run inference

  // Read result data and convert to image boxes
  int test = _priorBoxes.size();
}

#include "detector.h"
#include <cassert>
#include <fstream>
#include "utilities/json.hpp"

// #include <cuda_runtime_api.h>
#include "NvInfer.h"
#include "NvUffParser.h"


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
}

void object_detection::Detector::detect(const cv::Mat& img) {
  // Fill input buffer with image

  // TODO: resize img according to the input_width and input_channels

  // assert(img.size[0] == input_height && 
  //        img.size[1] == input_width && 
  //        img.channels() == input_channels && 
  //        "Input image does not fit input layer size");

  // for(int i = 0; i < input_height; i++)
  // {
  //   for(int j = 0; j < input_width; j++)
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

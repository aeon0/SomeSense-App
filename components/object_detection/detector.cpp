#include "detector.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"

#include "tensorflow/lite/context.h"
#include <iostream>
#include <cassert>


void object_detection::Detector::loadModel(const char* path) {
  std::cout << "Loading model for object detection..." << std::endl;
  _isWarmedUp = false;

  // Load model
  _model = tflite::FlatBufferModel::BuildFromFile(path);
  if(_model == nullptr) {
    throw std::runtime_error("Tensorflow Lite model not found");
  }

  // Build the interpreter
  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder builder(*_model, resolver);
  builder(&_interpreter);

  // Add GPU support
  // Not working yet... not sure how to get gpu support for tf lite
  // auto* delegate = TfLiteGpuDelegateV2Create(nullptr);
  // if(_interpreter->ModifyGraphWithDelegate(delegate) != kTfLiteOk) {
  //   throw std::runtime_error("Error on adding GPU support");
  // };

  // Allocate tensor buffers.
  if(_interpreter->AllocateTensors() != kTfLiteOk) {
    throw std::runtime_error("Failed to allocate tensors");
  }
}


void object_detection::Detector::detect(const cv::Mat& img) {
  // Apperently the c++ implementation needs a "warm up phase"
  // where it is just executed a few times in order to reach its best performance
  if(!_isWarmedUp) {
    std::cout << "Warming up detector" << std::endl;
    _isWarmedUp = true;
    // Warm up by calling inference for some time
    for(int i = 0; i < 12; ++i) {
      detect(img);
    }
  }

  // Fill input buffer with image
  assert(_interpreter->inputs().size() == 1 && "Only single inputs are supported");
  const int inputIndex = 0;
  int inputTensorId = _interpreter->inputs()[inputIndex];
  TfLiteIntArray* dims = _interpreter->tensor(inputTensorId)->dims;
  int input_height = dims->data[1];
  int input_width = dims->data[2];
  int input_channels = dims->data[3];

  // TODO: resize img according to the input_width and input_channels

  // assert(img.size[0] == input_height && 
  //        img.size[1] == input_width && 
  //        img.channels() == input_channels && 
  //        "Input image does not fit input layer size");

  for(int i = 0; i < input_height; i++)
  {
    for(int j = 0; j < input_width; j++)
    {
      // You can now access the pixel value with cv::Vec3b
      cv::Vec3b vec = img.at<cv::Vec3b>(i, j);
      float b = static_cast<float>(vec[0]);
      float g = static_cast<float>(vec[1]);
      float r = static_cast<float>(vec[2]);

      _interpreter->typed_input_tensor<float>(inputIndex)[3*(i+j) + 0] = b;
      _interpreter->typed_input_tensor<float>(inputIndex)[3*(i+j) + 1] = g;
      _interpreter->typed_input_tensor<float>(inputIndex)[3*(i+j) + 2] = r;
    }
  }

  // Run inference
  if(_interpreter->Invoke() != kTfLiteOk) {
    throw std::runtime_error("Invoke failed on Tensorflow Lite model");
  }

  // Read result data
  float* output = _interpreter->typed_output_tensor<float>(0.0f);
  std::cout << *output << std::endl;
}

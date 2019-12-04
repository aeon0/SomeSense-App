#include "detector.h"
#include "tensorflow/lite/delegates/gpu/delegate.h"
#include "tensorflow/lite/delegates/nnapi/nnapi_delegate.h"

#include "tensorflow/lite/context.h"
#include <iostream>


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
  // Not working yet... probably have to rebuild tflite with gpu support or something like that
  // auto* delegate = TfLiteGpuDelegateV2Create(nullptr);
  // if(_interpreter->ModifyGraphWithDelegate(delegate) != kTfLiteOk) {
  //   throw std::runtime_error("Error on adding GPU support");
  // };
}

void object_detection::Detector::detect(const cv::Mat& img) {
  std::cout << "Do some object detection!" << std::endl;

  if(!_isWarmedUp) {
    std::cout << "Warming up detector" << std::endl;
    _isWarmedUp = true;
    // Warm up by calling inference for some time
    for(int i = 0; i < 12; ++i) {
      detect(img);
    }
  }

  // Allocate tensor buffers.
  if(_interpreter->AllocateTensors() != kTfLiteOk) {
    throw std::runtime_error("Failed to allocate tensors");
  }

  // Fill input buffer with image
  float* input = _interpreter->typed_input_tensor<float>(0.0f);

  // Run inference
  if(_interpreter->Invoke() != kTfLiteOk) {
    throw std::runtime_error("Invoke failed on Tensorflow Lite model");
  }

  // Read result data
  float* output = _interpreter->typed_output_tensor<float>(0.0f);
}

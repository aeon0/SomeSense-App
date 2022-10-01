#include "inference.h"
#include <iostream>
#include <cmath>
#include "util/proto.h"


algo::Inference::Inference(util::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  // Check if edge tpu is available
  const auto& availableTpus = edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu();
  std::cout << "Found Edge TPUs: " << availableTpus.size() << std::endl;
  _edgeTpuAvailable = availableTpus.size() > 0;

  // Load model
  if (_edgeTpuAvailable) {
    std::cout << "TPU Type: " << availableTpus[0].type << ", TPU Path: " << availableTpus[0].path << std::endl;
    std::cout << "Load Model from: " << PATH_EDGETPU_MODEL << std::endl;
    _model = tflite::FlatBufferModel::BuildFromFile(PATH_EDGETPU_MODEL.c_str());
    _edgeTpuContext = edgetpu::EdgeTpuManager::GetSingleton()->OpenDevice(availableTpus[0].type, availableTpus[0].path);
    _resolver.AddCustom(edgetpu::kCustomOp, edgetpu::RegisterCustomOp());
  }
  else {
    _model = tflite::FlatBufferModel::BuildFromFile(PATH_TFLITE_MODEL.c_str());
  }
  assert(_model != nullptr);

  // Create interpreter and allocate input memory
  TfLiteStatus status;
  status = tflite::InterpreterBuilder(*_model, _resolver)(&_interpreter);
  if (_edgeTpuAvailable) {
    _interpreter->SetExternalContext(kTfLiteEdgeTpuContext, _edgeTpuContext.get());
    _interpreter->SetNumThreads(1);
  }
  assert(status == kTfLiteOk && _interpreter != nullptr);
  // Allocate tensor buffers.
  status = _interpreter->AllocateTensors();
  assert(status == kTfLiteOk);
}

void algo::Inference::reset() {
  _semsegOut.setTo(cv::Scalar::all(0));
  _depthOut.setTo(cv::Scalar::all(0));
}

void algo::Inference::processImg(const cv::Mat &img) {
  _runtimeMeasService.startMeas("inference/input");
  // Get input size and resize img to input size
  const int inputHeight = _interpreter->input_tensor(0)->dims->data[1];
  const int inputWidth = _interpreter->input_tensor(0)->dims->data[2];
  cv::Mat inputImg;
  _roi = util::img::cropAndResize(img, inputImg, inputHeight, inputWidth, OFFSET_BOTTOM);

  // Set data to model input
  size_t sizeOfInputInBytes = _interpreter->input_tensor(0)->bytes;
  size_t sizeOfMatInBytes = inputImg.total() * inputImg.elemSize();
  assert(sizeOfInputInBytes == sizeOfMatInBytes);
  uint8_t* input = _interpreter->typed_input_tensor<uint8_t>(0);
  memcpy(input, (uint8_t*)inputImg.data, sizeOfInputInBytes);
  _runtimeMeasService.endMeas("inference/input");

  // Run inference
  _runtimeMeasService.startMeas("inference/run");
  auto status = _interpreter->Invoke();
  assert(status == kTfLiteOk);
  _runtimeMeasService.endMeas("inference/run");

  _runtimeMeasService.startMeas("inference/post-process");
  const uint8_t* outputIt = _interpreter->typed_output_tensor<uint8_t>(0);
  // Multitask output concatentes the outputs to [SEMSEG, DEPTH] with the same size of height and width
  const int outHeight = _interpreter->output_tensor(0)->dims->data[1];
  const int outWidth = _interpreter->output_tensor(0)->dims->data[2];
  // In case the output is also scaled down, adjust the _roi, currently does not support new aspect ratios!
  _roi.scale *= (double(outWidth) / double(inputWidth));
  assert(int((inputHeight / double(inputWidth)) * 100.0) == int(((outHeight) / double(outWidth)) * 100.0) && "Aspect ratio of output different from input, currently not supported!");
  const int outChannels = _interpreter->output_tensor(0)->dims->data[3];
  const int nbPixels = outHeight * outWidth;
  // Allocated data and clear previous data
  _semsegOut = cv::Mat(outHeight, outWidth, CV_8UC1);
  _semsegImg = cv::Mat(outHeight, outWidth, CV_8UC3);
  _depthOut = cv::Mat(outHeight, outWidth, CV_32FC1);
  _depthImg = cv::Mat(outHeight, outWidth, CV_8UC1);

  for (int col = 0; col < outWidth; ++col) {
    for (int row = (outHeight - 1); row >= 0; --row) {
      const int iterOffset = ((row * outWidth) + col) * outChannels;
      const uint8_t* startEle = outputIt + iterOffset;
      const uint8_t* endEle = outputIt + iterOffset + outChannels;

      // Fill semseg mask
      const uint8_t* startSemseg = startEle + SEMSEG_START_IDX;
      int idx = 0;
      int semsegMax = *startSemseg;
      for (int i = 1; i < NUM_SEMSEG_CLS; ++i) {
        const uint8_t newSemsegVal = *(startSemseg + i);
        if (newSemsegVal > semsegMax) {
          idx = i;
          semsegMax = newSemsegVal;
        }
      }
      _semsegOut.at<uint8_t>(row, col) = idx;
      _semsegImg.at<cv::Vec3b>(row, col) = COLORS_SEMSEG[idx];

      // Fill depth map
      const uint8_t rawDepthVal = *(startEle + DEPTH_IDX);
      const float depthVal = pow((static_cast<float>(rawDepthVal) * QUANT_SCALE * 255.0 * (1.0/22.0)), 2.0) + 3.0;
      _depthOut.at<float>(row, col) = depthVal; // Adding a bit of a bias as depth always seems to be on the shorter side! Yes, its hacky.
      _depthImg.at<uint8_t>(row, col) = static_cast<uint8_t>(std::clamp((float)(rawDepthVal) * 1.6F, 0.0F, 253.0F));
    }
  }
  _runtimeMeasService.endMeas("inference/post-process");

  // _runtimeMeasService.printToConsole();
  // cv::imshow("InputImg", inputImg);
  // cv::imshow("Depth", _depthImg);
  // cv::imshow("Semseg", _semsegImg);
  // cv::waitKey(1);
}

void algo::Inference::serialize(proto::CamSensor& camSensor) {
  // Depth Output
  auto depthRaw = camSensor.mutable_depthraw();
  util::fillProtoImg<float>(depthRaw, _depthOut, _roi);
  // Semseg Output
  auto semsegRaw = camSensor.mutable_semsegraw();
  util::fillProtoImg<uchar>(semsegRaw, _semsegOut, _roi);

  // Semseg Img
  auto semseg = camSensor.mutable_semsegimg();
  util::fillProtoImg<uchar>(semseg, _semsegImg, _roi);
  // Depth Img
  auto depth = camSensor.mutable_depthimg();
  util::fillProtoImg<uchar>(depth, _depthImg, _roi);
}

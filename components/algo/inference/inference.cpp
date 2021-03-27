#include "inference.h"
#include <iostream>
#include <cmath>
#include "params.h"


inference::Inference::Inference(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  // Check if edge tpu is available
  const auto& availableTpus = edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu();
  std::cout << "Found Edge TPUs: " << availableTpus.size() << std::endl;
  _edgeTpuAvailable = availableTpus.size() > 0;

  // Load model
  if (_edgeTpuAvailable) {
    std::cout << "Load Multitask Model from: " << PATH_EDGETPU_MODEL << std::endl;
    _model = tflite::FlatBufferModel::BuildFromFile(PATH_EDGETPU_MODEL.c_str());
    _edgeTpuContext = edgetpu::EdgeTpuManager::GetSingleton()->OpenDevice(availableTpus[0].type, availableTpus[0].path);
    _resolver.AddCustom(edgetpu::kCustomOp, edgetpu::RegisterCustomOp());
  }
  else {
    _model = tflite::FlatBufferModel::BuildFromFile(PATH_TFLITE_MODEL.c_str());
  }
  assert(_model != nullptr);
}

void inference::Inference::reset() {
  _semsegOut.setTo(cv::Scalar::all(0));
  _depthOut.setTo(cv::Scalar::all(0));
}

void inference::Inference::processImg(const cv::Mat &img) {
  _runtimeMeasService.startMeas("inference/input");
  // Create interpreter and allocate input memory
  std::unique_ptr<tflite::Interpreter> interpreter;
  TfLiteStatus status;
  status = tflite::InterpreterBuilder(*_model, _resolver)(&interpreter); // != kTfLiteOk) {
  if (_edgeTpuAvailable) {
    interpreter->SetExternalContext(kTfLiteEdgeTpuContext, _edgeTpuContext.get());
    interpreter->SetNumThreads(1);
  }
  assert(status == kTfLiteOk && interpreter != nullptr);
  // Allocate tensor buffers.
  status = interpreter->AllocateTensors();
  assert(status == kTfLiteOk);

  // Get input size and resize img to input size
  const int inputHeight = interpreter->input_tensor(0)->dims->data[1];
  const int inputWidth = interpreter->input_tensor(0)->dims->data[2];
  cv::Mat inputImg;
  _roi = util::img::cropAndResize(img, inputImg, inputHeight, inputWidth, OFFSET_BOTTOM);
  _roi.scale = 0.5; // Network output is also scaled down by /2

  // Set data to model input
  size_t sizeOfInputInBytes = interpreter->input_tensor(0)->bytes;
  size_t sizeOfMatInBytes = inputImg.total() * inputImg.elemSize();
  assert(sizeOfInputInBytes == sizeOfMatInBytes);
  uint8_t* input = interpreter->typed_input_tensor<uint8_t>(0);
  memcpy(input, (uint8_t*)inputImg.data, sizeOfInputInBytes);
  _runtimeMeasService.endMeas("inference/input");

  // Run inference
  _runtimeMeasService.startMeas("inference/run");
  status = interpreter->Invoke();
  assert(status == kTfLiteOk);
  const uint8_t* outputIt = interpreter->typed_output_tensor<uint8_t>(0);
  _runtimeMeasService.endMeas("inference/run");

  _runtimeMeasService.startMeas("inference/post-process");
  // Multitask output concatentes the outputs to [CENTERNET, SEMSEG, DEPTH] with the same size of height and width
  const int outHeight = interpreter->output_tensor(0)->dims->data[1];
  const int outWidth = interpreter->output_tensor(0)->dims->data[2];
  const int outChannels = interpreter->output_tensor(0)->dims->data[3];
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
      const float depthVal = pow(((static_cast<float>(rawDepthVal) * QUANT_SCALE * 255.0) / 22.0), 2.0) + 3.0;
      _depthOut.at<float>(row, col) = depthVal * 1.05F + 1.0F; // Adding a bit of a bias as depth always seems to be on the shorter side! Yes, its hacky.
      _depthImg.at<uint8_t>(row, col) = static_cast<uint8_t>(std::clamp((float)(rawDepthVal) * 1.6F, 0.0F, 253.0F));
    }
  }

  _runtimeMeasService.endMeas("inference/post-process");

  // _runtimeMeasService.printToConsole();
  // cv::imshow("Output", inputImg);
  // cv::imshow("Depth", _depthImg);
  // cv::imshow("Semseg", _semsegImg);
  // cv::waitKey(1);
}

void inference::Inference::serialize(CapnpOutput::CamSensor::Builder& builder) {
  builder.getSemsegImg().setWidth(_semsegImg.size().width);
  builder.getSemsegImg().setHeight(_semsegImg.size().height);
  builder.getSemsegImg().setChannels(_semsegImg.channels());
  builder.getSemsegImg().setOffsetLeft(_roi.offsetLeft);
  builder.getSemsegImg().setOffsetTop(_roi.offsetTop);
  builder.getSemsegImg().setScale(_roi.scale);
  builder.getSemsegImg().setData(
    kj::arrayPtr(_semsegImg.data, _semsegImg.size().width * _semsegImg.size().height * _semsegImg.channels() * sizeof(uchar))
  );
  builder.getDepthImg().setWidth(_depthImg.size().width);
  builder.getDepthImg().setHeight(_depthImg.size().height);
  builder.getDepthImg().setChannels(_depthImg.channels());
  builder.getDepthImg().setOffsetLeft(_roi.offsetLeft);
  builder.getDepthImg().setOffsetTop(_roi.offsetTop);
  builder.getDepthImg().setScale(_roi.scale);
  builder.getDepthImg().setData(
    kj::arrayPtr(_depthImg.data, _depthImg.size().width * _depthImg.size().height * _depthImg.channels() * sizeof(uchar))
  );
}

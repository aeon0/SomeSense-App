#include "inference.h"
#include <iostream>
#include <cmath>

#include "edgetpu_c.h"
#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"


inference::Inference::Inference(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  std::unique_ptr<tflite::FlatBufferModel> model;
  model = tflite::FlatBufferModel::BuildFromFile(PATH_EDGETPU_MODEL.c_str());
  assert(model != nullptr);

  tflite::ops::builtin::BuiltinOpResolver resolver;
  tflite::InterpreterBuilder(*model, resolver)(&_interpreter);

  size_t num_devices;
  std::unique_ptr<edgetpu_device, decltype(&edgetpu_free_devices)> devices(edgetpu_list_devices(&num_devices), &edgetpu_free_devices);
  std::cout << "Found EdgeTPU devices: " << num_devices << std::endl;

  const auto& device = devices.get()[0];
  std::cout << "Type: " << device.type << ", Path: " << device.path << std::endl;
  auto* delegate = edgetpu_create_delegate(device.type, device.path, nullptr, 0);
  _interpreter->ModifyGraphWithDelegate({delegate, edgetpu_free_delegate});
  _interpreter->SetNumThreads(1);
  assert(_interpreter->AllocateTensors() == kTfLiteOk);
}

void inference::Inference::reset() {
  _semsegOut.setTo(cv::Scalar::all(0));
  _depthOut.setTo(cv::Scalar::all(0));
  _objects2D.clear();
}

void inference::Inference::processImg(const cv::Mat &img) {
  _runtimeMeasService.startMeas("inference/input");
  // Get input size and resize img to input size
  const int inputHeight = _interpreter->input_tensor(0)->dims->data[1];
  const int inputWidth = _interpreter->input_tensor(0)->dims->data[2];
  cv::Mat inputImg;
  _roi = util::img::cropAndResize(img, inputImg, inputHeight, inputWidth, OFFSET_BOTTOM);
  _roi.scale = 0.5; // Network output is also scaled down by /2

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
  // Multitask output concatentes the outputs to [CENTERNET, SEMSEG, DEPTH] with the same size of height and width
  const int outHeight = _interpreter->output_tensor(0)->dims->data[1];
  const int outWidth = _interpreter->output_tensor(0)->dims->data[2];
  const int outChannels = _interpreter->output_tensor(0)->dims->data[3];
  const int nbPixels = outHeight * outWidth;
  // Allocated data and clear previous data
  _semsegOut = cv::Mat(outHeight, outWidth, CV_8UC1);
  _semsegImg = cv::Mat(outHeight, outWidth, CV_8UC3);
  _depthOut = cv::Mat(outHeight, outWidth, CV_32FC1);
  _depthImg = cv::Mat(outHeight, outWidth, CV_8UC1);
  const int NUM_OD_IDX = SEMSEG_START_IDX - OD_HEATMAP_IDX;
  auto odOut = cv::Mat(outHeight, outWidth, CV_8UC(NUM_OD_IDX));

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

      // Fill od map
      memcpy(&odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(row, col), startEle, NUM_OD_IDX);
    }
  }

  // Check with Kernel for 2D Objects
  _objects2D.clear();
  for (int x = 1; x < odOut.size().width - 1; ++x) {
    for (int y = 1; y < odOut.size().height - 1; ++y) {
      bool isPeak = false;
      auto data = odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y, x);
      float heatmapVal = static_cast<float>(data[0]) * QUANT_SCALE;
      if (heatmapVal > HEATMAP_THRESHOLD) {
        uint8_t maxVal = std::max({
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y - 1, x - 1)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y - 1, x + 0)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y - 1, x + 1)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y + 1, x - 1)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y + 1, x + 0)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y + 1, x + 1)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y + 0, x - 1)[0],
          odOut.at<cv::Vec<uint8_t, NUM_OD_IDX>>(y + 0, x + 1)[0],
        });
        isPeak = data[0] > maxVal;
      }

      if (isPeak) {
        // Fill 2D Object
        auto center = util::img::convertToRoi(_roi, cv::Point2f(x, y));
        float dist = pow(((static_cast<float>(data[9]) * QUANT_SCALE * 255.0) / 22.0), 2.0) + 3.0;
        // int idx = 0;
        // int clsMax = data[1];
        // for (int i = 1; i < NUM_OD_CLS; ++i) {
        //   const uint8_t newClsMax = data[i+1];
        //   if (newClsMax > clsMax) {
        //     idx = i;
        //     clsMax = newClsMax;
        //   }
        // }
        _objects2D.push_back({center.x, center.y, 0, dist});
      }
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
  // Semseg
  builder.getSemsegImg().setWidth(_semsegImg.size().width);
  builder.getSemsegImg().setHeight(_semsegImg.size().height);
  builder.getSemsegImg().setChannels(_semsegImg.channels());
  builder.getSemsegImg().setOffsetLeft(_roi.offsetLeft);
  builder.getSemsegImg().setOffsetTop(_roi.offsetTop);
  builder.getSemsegImg().setScale(_roi.scale);
  builder.getSemsegImg().setData(
    kj::arrayPtr(_semsegImg.data, _semsegImg.size().width * _semsegImg.size().height * _semsegImg.channels() * sizeof(uchar))
  );
  // Depth
  builder.getDepthImg().setWidth(_depthImg.size().width);
  builder.getDepthImg().setHeight(_depthImg.size().height);
  builder.getDepthImg().setChannels(_depthImg.channels());
  builder.getDepthImg().setOffsetLeft(_roi.offsetLeft);
  builder.getDepthImg().setOffsetTop(_roi.offsetTop);
  builder.getDepthImg().setScale(_roi.scale);
  builder.getDepthImg().setData(
    kj::arrayPtr(_depthImg.data, _depthImg.size().width * _depthImg.size().height * _depthImg.channels() * sizeof(uchar))
  );
  // Objects
  auto objects2D = builder.initObjects2D(_objects2D.size());
  for (int i = 0; i < _objects2D.size(); ++i) {
    objects2D[i].setCx(_objects2D[i].cx);
    objects2D[i].setCy(_objects2D[i].cy);
    objects2D[i].setRadialDist(_objects2D[i].radialDist);
    objects2D[i].setObjClass(_objects2D[i].clsIdx);
    objects2D[i].setWidth(0.0F);
    objects2D[i].setHeight(0.0F);
  }
}

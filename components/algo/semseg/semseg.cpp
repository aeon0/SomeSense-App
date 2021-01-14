#include "semseg.h"
#include <iostream>
// #include "opencv2/imgproc.hpp"
#include "params.h"


semseg::Semseg::Semseg(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  // Check if edge tpu is available
  const auto& availableTpus = edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu();
  std::cout << "Found Edge TPUs: " << availableTpus.size() << std::endl;
  _edgeTpuAvailable = availableTpus.size() > 0;

  // Load model
  if (_edgeTpuAvailable) {
    std::cout << "Load Semseg Model from: " << PATH_EDGETPU_MODEL << std::endl;
    _model = tflite::FlatBufferModel::BuildFromFile(PATH_EDGETPU_MODEL.c_str());
    _edgeTpuContext = edgetpu::EdgeTpuManager::GetSingleton()->OpenDevice(availableTpus[0].type, availableTpus[0].path);
    _resolver.AddCustom(edgetpu::kCustomOp, edgetpu::RegisterCustomOp());
  }
  else {
    _model = tflite::FlatBufferModel::BuildFromFile(PATH_TFLITE_MODEL.c_str());
  }
  assert(_model != nullptr);
}

void semseg::Semseg::reset() {
  _semsegMask.setTo(cv::Scalar::all(0));
  _obstacles.clear();
  _laneMarkings.clear();
}

void semseg::Semseg::processImg(const cv::Mat &img, const data_reader::ICam &cam) {
  _runtimeMeasService.startMeas("semseg prepare");
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
  _maskRoi = util::img::cropAndResize(img, inputImg, inputHeight, inputWidth, OFFSET_BOTTOM);

  // Set data to model input
  cv::Mat inputImgFloat;
  inputImg.convertTo(inputImgFloat, CV_32FC3);
  size_t sizeOfInputInBytes = interpreter->input_tensor(0)->bytes;
  size_t sizeOfMatInBytes = inputImgFloat.total() * inputImgFloat.elemSize();
  assert(sizeOfInputInBytes == sizeOfMatInBytes);
  float* input = interpreter->typed_input_tensor<float>(0);
  memcpy(input, (float*)inputImgFloat.data, sizeOfInputInBytes);
  _runtimeMeasService.endMeas("semseg prepare");

  // Run inference
  _runtimeMeasService.startMeas("semseg inference");
  status = interpreter->Invoke();
  assert(status == kTfLiteOk);
  const float* outputIt = interpreter->typed_output_tensor<float>(0);
  _runtimeMeasService.endMeas("semseg inference");

  _runtimeMeasService.startMeas("semseg output proccess");
  // Get output mask image and find the max values -> convert index to class -> color output img
  const int maskHeight = interpreter->output_tensor(0)->dims->data[1];
  const int maskWidth = interpreter->output_tensor(0)->dims->data[2];
  const int nbClasses = CLASS_MAPPING_COLORS.size();
  const int nbPixels = maskHeight * maskWidth;
  // Allocated data and clear previous data
  if (_semsegMask.size().width != maskWidth || _semsegMask.size().height != maskHeight) {
    _semsegMask = cv::Mat(maskHeight, maskWidth, CV_8UC3);
  }
  _obstacles.clear();
  _laneMarkings.clear();
  // Checking each column from bottom to top for drivable path (or lane marking) until a non-drivable or moving class is hit
  // Note: The image is stored in row major, this has to be accounted for when increasing the img iterator
  for (int col = 0; col < maskWidth; ++col) {
    bool foundBarrier = false;
    bool foundDriveBinStart = false;
    float driveBinEndX = 120;
    cv::Point3f driveBinStart(-1, -1, -1);
    for (int row = (maskHeight - 1); row >= 0; --row) {
      // Find index of max class, outputIt is stored row major
      const int iterOffset = ((row * maskWidth) + col) * nbClasses;
      const auto startEle = outputIt + iterOffset;
      const auto endEle = outputIt + iterOffset + nbClasses;
      const auto idx = std::distance(startEle, std::max_element(startEle, endEle));

      // Fill semseg mask with color of best scoring class
      _semsegMask.at<cv::Vec3b>(row, col) = CLASS_MAPPING_COLORS[idx];

      // Fill the point clouds and drive bins
      if (!foundBarrier) {
        foundBarrier = (idx == semseg::UNDRIVEABLE || idx == semseg::MOVABLE);
        // Create 3D point with flatworld assumption
        const cv::Point2f converted = util::img::convertToRoi(_maskRoi, cv::Point2f(col, row));
        const cv::Point3f point3d = cam.imageToWorldKnownZ(converted, 0);
        if (point3d.x < 120.0 && point3d.x > 0.2) {
          // Fill point clouds (lane markings and obstacles)
          if (idx == semseg::MOVABLE) {
            _obstacles.push_back(point3d);
            _obstacles.push_back({point3d.x, point3d.y, 0.45});
            _obstacles.push_back({point3d.x, point3d.y, 0.9});
          }
          if (idx == semseg::LANE_MARKINGS) {
            _laneMarkings.push_back(point3d);
          }
        }
      }
    }
  }

  // Erode & Dilate
  // cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));
  // cv::erode(outputImg, outputImg, kernel);
  // cv::dilate(outputImg, outputImg, kernel);
  _runtimeMeasService.endMeas("semseg output proccess");

  // cv::imshow("Output", _semsegMask);
  // cv::imshow("Img", inputImg);
  // cv::waitKey(1);
}

void semseg::Semseg::serialize(CapnpOutput::CamSensor::Semseg::Builder& builder) {
  // Fill img
  builder.getMask().setWidth(_semsegMask.size().width);
  builder.getMask().setHeight(_semsegMask.size().height);
  builder.getMask().setChannels(_semsegMask.channels());
  builder.getMask().setData(
    kj::arrayPtr(_semsegMask.data, _semsegMask.size().width * _semsegMask.size().height * _semsegMask.channels() * sizeof(uchar))
  );
  builder.setOffsetLeft(_maskRoi.offsetLeft);
  builder.setOffsetTop(_maskRoi.offsetTop);
  builder.setScale(_maskRoi.scale);
  // Fill obstacles
  auto obstacles = builder.initObstacles(_obstacles.size());
  for (int i = 0; i < _obstacles.size(); ++i)
  {
    obstacles[i].setX(_obstacles[i].x);
    obstacles[i].setY(_obstacles[i].y);
    obstacles[i].setZ(_obstacles[i].z);
  }
  // Fill lane markings
  auto laneMarkings = builder.initLaneMarkings(_laneMarkings.size());
  for (int i = 0; i < _laneMarkings.size(); ++i)
  {
    laneMarkings[i].setX(_laneMarkings[i].x);
    laneMarkings[i].setY(_laneMarkings[i].y);
    laneMarkings[i].setZ(_laneMarkings[i].z);
  }
}

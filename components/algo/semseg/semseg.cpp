#include "semseg.h"
#include <iostream>


semseg::Semseg::Semseg(frame::RuntimeMeasService& runtimeMeasService) :
  _runtimeMeasService(runtimeMeasService)
{
  // Check if edge tpu is available
  const auto& available_tpus = edgetpu::EdgeTpuManager::GetSingleton()->EnumerateEdgeTpu();
  std::cout << "Found Edge TPUs: " << available_tpus.size() << std::endl;
  useTpu = available_tpus.size() > 0;

  // Load model
  std::string FILENAME = "/home/jodo/trained_models/semseg_12-08-2020-12-08-16/good_tf_model_15/model_quant.tflite";
  if (useTpu) {
    FILENAME = "/home/jodo/trained_models/semseg_12-08-2020-12-08-16/good_tf_model_15/model_quant_edgetpu.tflite";
  }
  model = tflite::FlatBufferModel::BuildFromFile(FILENAME.c_str());
  assert(model != nullptr);

  // Set up resolver
  if (useTpu) {
    edgeTpuContext = edgetpu::EdgeTpuManager::GetSingleton()->OpenDevice(available_tpus[0].type, available_tpus[0].path);
    resolver.AddCustom(edgetpu::kCustomOp, edgetpu::RegisterCustomOp());
  }
}

void semseg::Semseg::reset() {

}

void semseg::Semseg::processImg(const cv::Mat &img) {
  _runtimeMeasService.startMeas("semseg prepare");
  // Create interpreter
  std::unique_ptr<tflite::Interpreter> interpreter;
  if (tflite::InterpreterBuilder(*model, resolver)(&interpreter) != kTfLiteOk) {
    std::cout << "WARNING: Failed to build interpreter for Semseg" << std::endl;
    return;
  }
  if (useTpu) {
    interpreter->SetExternalContext(kTfLiteEdgeTpuContext, edgeTpuContext.get());
    interpreter->SetNumThreads(1);
  }
  assert(interpreter != nullptr);
  // Allocate tensor buffers.
  if (interpreter->AllocateTensors() != kTfLiteOk) {
    std::cout << "ERROR: could not allocate tensor for semseg" << std::endl;
    return;
  }
  // Resize Img, convert to float, set as input
  double curr_ratio = img.size().width / static_cast<double>(img.size().height);
  double target_ratio = INPUT_WIDTH / static_cast<double>(INPUT_HEIGHT);
  int delta_height = img.size().height - (img.size().width / target_ratio);
  cv::Rect roi;
  roi.x = 0;
  roi.y = OFFSET_TOP;
  roi.width = img.size().width;
  roi.height = img.size().height - delta_height;
  cv::Mat croppedImg = img(roi);
  cv::Mat resizedImg;
  cv::resize(croppedImg, resizedImg, cv::Size(INPUT_WIDTH, INPUT_HEIGHT));
  // Set data to model input
  cv::Mat inputFloatImg;
  resizedImg.convertTo(inputFloatImg, CV_32FC3);
  size_t sizeOfInputInBytes = interpreter->input_tensor(0)->bytes;
  size_t sizeOfMatInBytes = inputFloatImg.total() * inputFloatImg.elemSize();
  assert(sizeOfInputInBytes == sizeOfMatInBytes);
  float* input = interpreter->typed_input_tensor<float>(0);
  memcpy(input, (float*)inputFloatImg.data, sizeOfInputInBytes);
  _runtimeMeasService.endMeas("semseg prepare");

  // Test input img
  cv::Mat testImg = cv::Mat(INPUT_HEIGHT, INPUT_WIDTH, CV_32FC3, interpreter->typed_input_tensor<float>(0));
  testImg.convertTo(testImg, CV_8UC3);
  cv::imshow("input data test", testImg);
  cv::waitKey(0);

  // Run inference
  _runtimeMeasService.startMeas("semseg inf");
  if(interpreter->Invoke() != kTfLiteOk) {
    std::cout << "ERROR: could not run semseg inference" << std::endl;
    return;
  }
  float* output = interpreter->typed_output_tensor<float>(0);
  size_t sizeOfOutputInBytes = interpreter->output_tensor(0)->bytes;
  // std::cout << "Size of of output [bytes]: " << sizeOfOutputInBytes << std::endl;
  _runtimeMeasService.endMeas("semseg inf");

  _runtimeMeasService.startMeas("semseg output proccess");
  // Get output mask image and find the max values -> convert index to class -> color output img
  const int classSize = CLASS_MAPPING.size();
  const int nbPixels = MASK_HEIGHT * MASK_WIDTH;
  cv::Mat outputImg = cv::Mat(MASK_HEIGHT, MASK_WIDTH, CV_8UC3);
  outputImg.setTo(cv::Scalar::all(0));
  int pixlIdx = 0;
  for (int _ = 0; _ < nbPixels; ++_) {
    // float x[5];
    // memcpy(&x, output, sizeof(x));
    // Find index of max class
    auto startEle = output;
    auto endEle = output + classSize;
    auto idx = std::distance(startEle, std::max_element(startEle, endEle));
    output = endEle;
    // Fill output image
    int j = pixlIdx % MASK_WIDTH;
    int i = (pixlIdx - j) / MASK_WIDTH;
    outputImg.at<cv::Vec3b>(i, j) = CLASS_MAPPING[idx];
    pixlIdx++;
  }
  _runtimeMeasService.endMeas("semseg output proccess");

  // Test output img
  cv::imshow("Output", outputImg);
  cv::waitKey(0);

  _runtimeMeasService.printToConsole();
}

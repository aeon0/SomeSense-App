#include "detector.h"
#include <iostream>

#include <cstdio>
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"


void object_detection::Detector::test() {
  std::cout << "Hello from Detector" << std::endl;
}
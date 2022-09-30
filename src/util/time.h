#pragma once

#include <chrono>


namespace util {
  typedef std::chrono::time_point<std::chrono::high_resolution_clock> TS;

  int64_t timepointToInt64(TS timePoint);
  int64_t calcDurationInInt64(TS end, TS start);
  TS int64ToTimepoint(int64_t tsMicro);
}

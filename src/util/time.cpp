#include "time.h"

int64_t util::timepointToInt64(util::TS timePoint) {
  return std::chrono::time_point_cast<std::chrono::microseconds>(timePoint).time_since_epoch().count();
}

int64_t util::calcDurationInInt64(util::TS end, util::TS start) {
  return static_cast<int64_t>(std::chrono::duration<double, std::micro>(end - start).count());
}

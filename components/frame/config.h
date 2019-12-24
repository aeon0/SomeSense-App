#pragma once

class Config {
public:
  static const double goalFrameLength; // Desired frame length in [ms] when working with a real sensor
                                       // Note 1: in case runtime is too big the frame length might increase and drop fps
                                       // Note 2: do not use this for any algo specific stuff, but instead use the delta times from the actual frames
};

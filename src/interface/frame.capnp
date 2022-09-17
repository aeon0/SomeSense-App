@0xbe4036c999076850;

const interfaceVersionMajor :Int32 = 1;
const interfaceVersionMinor :Int32 = 0;

using import "types.capnp".Img;
using import "types.capnp".Point;
using import "types.capnp".CamCalibration;
using import "algo/scheduler.capnp".RuntimeMeas;
using import "algo/inference.capnp".ObjectImg;


struct CapnpOutput {
  struct CamSensor {
    idx @0 :Int32;
    key @1 :Text;
    timestamp @2 :Int64; # timestamp in [us]
    img @3 :Img;
    calib @4 :CamCalibration;
  }

  struct Frame {
    versionMajor @0 :Int32 = .interfaceVersionMajor; # major interface version, should always be increased once breaking changes happen
    versionMinor @1: Int32 = .interfaceVersionMinor; # minor interface version, should always be increase for non breaking changes
    timestamp @2 :Int64; # from the start of the app in [us]
    plannedFrameLength @3 :Float64; # planned length of the frame in [ms]
    frameCount @4 :Int64;
    runtimeMeas @5 :List(RuntimeMeas);
    camSensors @6 :List(CamSensor);
  }
}

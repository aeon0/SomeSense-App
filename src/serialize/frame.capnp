@0x95121194d6c74461;

const interfaceVersionMajor :Int32 = 1;
const interfaceVersionMinor :Int32 = 0;

struct CapnpOutput {
  struct Point {
    x @0 :Float32;
    y @1 :Float32;
    z @2 :Float32;
  }

  struct CamSensor {
    idx @0 :Int32;
    key @1 :Text;
    timestamp @2 :Int64; # timestamp in [us]
    focalLengthX @3 :Float32; # in [px]
    focalLengthY @4 :Float32; # in [px]
    principalPointX @5 :Float32; # in [px]
    principalPointY @6 :Float32; # in [px]
    horizon @7 :Float32; # in [px] (y-axis)
    # position (autosar) in bumper coordinates in [m]
    x @8 :Float32;
    y @9 :Float32;
    z @10 :Float32;
    yaw @11 :Float32; # in [rad]
    roll @12 :Float32; # in [rad]
    pitch @13 :Float32; # in [rad]
    fovHorizontal @14 :Float32; # in [rad]
    fovVertical @15 :Float32; # in [rad]
    img @16 :Img;
    opticalFlow @17 :OpticalFlow;
    semsegImg @18 :Img;
    depthImg @19 :Img;
    objects2D @20 :List(Object2D);

    struct Img {
      width @0 :Int32; # in [px]
      height @1 :Int32; # in [px]
      channels @2 :Int16;
      data @3 :Data;
      scale @4 :Float32 = 1.0; # scale with respect to input img
      offsetLeft @5 :Float32 = 0.0; # Offset from left edge in [px]
      offsetTop @6 :Float32 = 0.0; # Offset from top edge in [px]
    }

    struct Object2D {
      cx @0 :Int32;
      cy @1 :Int32;
      objClass @2 :Int32;
      width @3 :Int32;
      height @4 :Int32;
      radialDist @5 :Float32;
    }

    # [algos] per image
    struct OpticalFlow {
      struct FlowTrack {
        # in normalized coordinates
        startX @0 :Float32;
        startY @1 :Float32;
        endX @2 :Float32;
        endY @3 :Float32;
      }
      endTs @0 :Int64; # in [us]
      deltaTime @1 :Float64; # in [ms]
      flowTracks @2 :List(FlowTrack);
    }
  }

  struct RuntimeMeas {
    name @0 :Text;
    start @1 :Int64; # start timestamp in [us]
    duration @2 :Float64; # in [ms]
  }

  # [algos]
  struct Track {
    trackId @0 :Text;
    # position (autosar) in bumper coordinates
    x @1 :Float32;
    y @2 :Float32;
    z @3 :Float32;
    # rotation (origin bottom center of object)
    yaw @4 :Float32;
    roll @5 :Float32;
    pitch @6 :Float32;
    height @7 :Float32;
    width @8 :Float32;
    length @9 :Float32;
    velocity @10 :Float32;
    objClass @11 :Int32;
  }

  struct Frame {
    versionMajor @0 :Int32 = .interfaceVersionMajor; # major interface version, should always be increased once breaking changes happen
    versionMinor @1: Int32 = .interfaceVersionMinor; # minor interface version, should always be increase for non breaking changes
    timestamp @2 :Int64; # from the start of the app in [us] (is the latest of sensor input)
    frameStart @3 :Int64; # time of current frame start in [us]
                          # difference between frameStart and timestamp is the latency we have
    plannedFrameLength @4 :Float64; # planned length of the frame in [ms]
    frameCount @5 :Int64;
    camSensors @6 :List(CamSensor);
    runtimeMeas @7 :List(RuntimeMeas);

    # [algos]
    tracks @8 :List(Track);
    obstacles @9 :List(Point);
    laneMarkings @10 :List(Point);
  }
}

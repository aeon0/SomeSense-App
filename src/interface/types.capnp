@0xc09cee77412a9911;

struct Point {
  x @0 :Float32;
  y @1 :Float32;
  z @2 :Float32;
}

struct CamCalibration {
  # Extrinsics
  x @0 :Float32; # in [m] autosar, bumper coordinates
  y @1 :Float32; # in [m] autosar, bumper coordinates
  z @2 :Float32; # in [m] autosar, bumper coordinates
  yaw @3 :Float32; # in [rad]
  roll @4 :Float32; # in [rad]
  pitch @5 :Float32; # in [rad]
  # Intrinsics
  focalLengthX @6 :Float32; # in [px]
  focalLengthY @7 :Float32; # in [px]
  principalPointX @8 :Float32; # in [px]
  principalPointY @9 :Float32; # in [px]
  # Pre computed helpers
  horizon @10 :Float32; # in [px] (y-axis), 0 means invalid / not filled
}

struct Img {
  width @0 :Int32; # in [px]
  height @1 :Int32; # in [px]
  channels @2 :Int16;
  data @3 :Data;
  scale @4 :Float32 = 1.0; # scale with respect to input img
  offsetLeft @5 :Float32 = 0.0; # Offset from left edge in [px]
  offsetTop @6 :Float32 = 0.0; # Offset from top edge in [px]
}

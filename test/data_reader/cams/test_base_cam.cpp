#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "data_reader/cams/base_cam.h"


class DummyCam : public data_reader::BaseCam {
public:
  DummyCam(const std::string name) : BaseCam(name, std::chrono::high_resolution_clock::now()) {
    setIntrinsics(600, 400, 1.4);
  }
  void start() { }
};


TEST(data_reader, BaseCam)
{
  DummyCam cam("test");
  const cv::Mat& poseMat = cam.getPoseMat();

  std::cout << "pose = " << std::endl << cv::format(poseMat, cv::Formatter::FMT_PYTHON) << std::endl << std::endl;
}

TEST(data_reader, worldtoImage)
{
  DummyCam cam("test");

  cv::Point3f worldCoord(10, 5, 0.5);
  cv::Point2f imgCoords = cam.worldtoImage(worldCoord);
  std::cout << imgCoords << std::endl;
}

TEST(data_reader, imageToWorld)
{
  DummyCam cam("test");

  cv::Point2f imgCoord(144.834, 172.682);
  cv::Point3f worldCoord = cam.imageToWorldKnownZ(imgCoord, 0.5);
  std::cout << worldCoord << std::endl;
}

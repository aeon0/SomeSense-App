#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "data_reader/cams/base_cam.h"


class DummyCam : public data_reader::BaseCam {
public:
  DummyCam(const std::string name) : BaseCam(name, std::chrono::high_resolution_clock::now()) {
    setIntrinsics(640, 480, 1.5);
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
  cv::Point2f imgCoords = cam.worldToImage(worldCoord);
  std::cout << imgCoords << std::endl;
}

TEST(data_reader, imageToWorld)
{
  DummyCam cam("test");

  cv::Point2f imgCoord1(320.0, 300.0);
  cv::Point3f worldCoord1 = cam.imageToWorldKnownZ(imgCoord1, 0.0);
  std::cout << worldCoord1 << std::endl;

  cv::Point2f imgCoord2(320.0, 238.0);
  cv::Point3f worldCoord2 = cam.imageToWorldKnownZ(imgCoord2, 0.0);
  std::cout << worldCoord2 << std::endl;
}

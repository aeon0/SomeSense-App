#pragma once

#include <carla/client/Sensor.h>
#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/World.h>
#include <carla/client/Waypoint.h>
#include <carla/geom/Transform.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/sensor/data/Image.h>


namespace data_reader {
  namespace sim {
    class IScene {
    public:
      // Setup the scene
      virtual void setup(carla::client::Client client) = 0;
      virtual boost::shared_ptr<carla::client::Sensor> getRgbCam() const = 0;
    };
  }
}

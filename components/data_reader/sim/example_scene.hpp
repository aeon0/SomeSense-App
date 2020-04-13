#include "iscene.h"


namespace data_reader {
  namespace sim {
    class ExampleScene : public IScene {
    public:
      ExampleScene() : _width(1280), _height(720), _fov(120.0f), _fps(50) {}

      ~ExampleScene() {
        std::cout << "Destroy stuff" << std::endl;
        _rgbCamera->Destroy();
        _egoVehicle->Destroy();
        _stationaryVehicle->Destroy();
      }

      // Setup the scene
      void setup(carla::client::Client client) override {
         // Load world
        std::cout << "Loading world: Town03" << std::endl;
        auto world = client.LoadWorld("Town03");
        auto map = world.GetMap();

        // Create ego vehicle
        auto egoVehicleTransform = carla::geom::Transform {
          carla::geom::Location{110.0f, -198.2f, 2.5f}, // x, y, z
          carla::geom::Rotation{0.0f, -178.4f, 0.0f} // pitch, yaw, roll
        };
        auto blueprintLib = world.GetBlueprintLibrary();
        auto vehicleBpPtr = blueprintLib->Find("vehicle.tesla.model3");
        assert(vehicleBpPtr != nullptr && "Carla: Can not find vehicle blueprint");
        auto vehicleBp = static_cast<carla::client::ActorBlueprint>(*vehicleBpPtr);
        auto egoVehicleActor = world.SpawnActor(vehicleBp, egoVehicleTransform);
        _egoVehicle = boost::static_pointer_cast<carla::client::Vehicle>(egoVehicleActor);

        // Create stationary vehicle in same lane
        auto vehicleTransform = carla::geom::Transform {
          carla::geom::Location{50.0f, -199.875f, 2.5f}, // x, y, z
          carla::geom::Rotation{0.0f, -178.4f, 0.0f} // pitch, yaw, roll
        };
        auto vehicleActor = world.SpawnActor(vehicleBp, vehicleTransform);
        _stationaryVehicle = boost::static_pointer_cast<carla::client::Vehicle>(vehicleActor);
        _stationaryVehicle->SetVelocity({0.0f, 0.0f, 0.0f});

        // Set up RGB camera and attach to vehicle's windshield
        auto cameraBpPtr = blueprintLib->Find("sensor.camera.rgb");
        assert(cameraBpPtr != nullptr && "Carla: Can not find sensor blueprint");
        auto cameraBp = static_cast<carla::client::ActorBlueprint>(*cameraBpPtr);
        cameraBp.SetAttribute("fov", std::to_string(_fov));
        cameraBp.SetAttribute("image_size_x", std::to_string(_width));
        cameraBp.SetAttribute("image_size_y", std::to_string(_height));
        cameraBp.SetAttribute("sensor_tick", std::to_string(1.0f/static_cast<float>(_fps))); // 50 fps
        auto cameraTransform = carla::geom::Transform {
            carla::geom::Location{0.15f, 0.0f, 1.5f},   // x, y, z.
            carla::geom::Rotation{0.0f, 0.0f, 0.0f}}; // pitch, yaw, roll.
        auto camActor = world.SpawnActor(cameraBp, cameraTransform, egoVehicleActor.get());
        _rgbCamera = boost::static_pointer_cast<carla::client::Sensor>(camActor);

        // Move spectator so we can see the vehicle from the simulator window.
        auto spectator = world.GetSpectator();
        auto spectatorTransform = carla::geom::Transform{
          _egoVehicle->GetLocation(),
          {-70.0f, -90.0f, 0.0f}
        };
        spectatorTransform.location.z += 55.0f;
        spectatorTransform.location.x += -40.0f;
        spectatorTransform.location.y += 20.0f;
        spectator->SetTransform(spectatorTransform);

        // Apply control to vehicle
        carla::client::Vehicle::Control control;
        control.throttle = 0.35f;
        _egoVehicle->ApplyControl(control);
      };

      const boost::shared_ptr<carla::client::Sensor> getRgbCam() const override { return _rgbCamera; }

    private:
      boost::shared_ptr<carla::client::Sensor> _rgbCamera;
      boost::shared_ptr<carla::client::Vehicle> _egoVehicle;
      boost::shared_ptr<carla::client::Vehicle> _stationaryVehicle;

      int _width;
      int _height;
      float _fov; // field of view in degree
      int _fps;
    };
  }
}

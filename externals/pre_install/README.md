# Pre-Built EdgeTpu & TFLite Libraries


### EdgeTpu
To rebuild clone: https://github.com/google-coral/libedgetpu and run the Docker command specified in the Readme `DOCKER_CPUS="k8 armv7a aarch64" DOCKER_TARGETS=libedgetpu make docker-build`.

Copy the contents of the out folder in here as well as the edgetpu_c.h and edgetpu.h.

### TFLite
Clone the tensorflow repo and checkout this commit: b36436b087bd8e8701ef51718179037cccdfc26e.
Might have to install the aarch64 cross compiler first: `sudo apt-get install g++-aarch64-linux-gnu`.

```bash
cd tensorflow/lite/tools/make
bash download_dependencies.sh
bash build_lib.sh
bash build_aarch64.sh
# You can find the .a files in tensorflow/tensorflow/lite/tools/make/gen/linux_x86_64/lib/libtensorflow-lite.a
cd ./SomeSense-App/externals/pre_install/tensorflow_lite/include
find ~/git/tensorflow/tensorflow/lite -name "*.h" | xargs cp --parents -t .
```

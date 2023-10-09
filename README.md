# TS-7970 Grid-EYE Demo

This repository hosts the software package for our trade show demo of our TS-7970 interfacing with a Panasonic AMG88 Grid-EYE contactless thermal sensor array. Sensor data is drawn on an attached HDMI display using a simple Qt project to show color data on a large 8x8 grid of pixels, scaling the color from blue to red in each pixel based on the temperature reported by each array member.

This repository is dual purpose, it is both the Qt application code as well as a Buildroot br2-external tree nested with our existing [Buildroot br2-external](https://github.com/embeddedTS/buildroot-ts) project. This nesting allows this project to add necessary packages and configurations, but still take advantages of packages provided by our existing Buildroot br2-external repository, which itself takes advantage of the mainline Buildroot project.


## About Buildroot and Runtime

The final generated runtime uses the `linux-5.10.y` branch of our [linux-lts](https://github.com/embeddedTS/linux-lts/) project. The kernel supports full GPU acceleration using the open-source Etnaviv driver for the i.MX6 Vivante GC2000 which integrates with OpenGL 2.0 ES shaders.

The main UI interface is provided by X11 with DRI2 to interface with Mesa3D and OpenGL.

The Qt runtime is able to take advantage of the graphics framework to be fully hardware accelerated. Even under operation, the main application only consumes a few % of CPU time. This allows much more intense background processing to take place while keeping the UI responsive and keeping up with data from the airflow sensor.

All of this is build and packaged by Buildroot.


## Building the Distribution

The following instructions can be used to build the whole application and Buildroot distribution:

```
git clone --recurse-submodules https://github.com/embeddedTS/ts7970-grid-eye-demo
cd ts7970-grid-eye-demo/buildroot
./buildroot-ts/scripts/run_docker_buildroot.sh make ts7970_grideye_defconfig clean all
```

The last command above configures and builds the final output image. It does this by using a Docker container to ensure a known good build environment. If it is preferred to not use docker, the whole distribution can be built natively instead:

```
# From the ts7970-grid-eye-demo/buildroot/ directory
make ts7970_grideye_defconfig clean all
```


This will output a `rootfs.tar.xz` tarball in `ts7970-grid-eye-demo/buildroot/buildroot-ts/buildroot/output/images/`.

The rootfs can be directly unpacked to the eMMC of the TS-7970. This can be accomplished manually or by utilizing our [Image Relicator tool for the TS-7970](https://docs.embeddedts.com/TS-7970#Image_Replicator) by copying the tarball to `/emmcimage.tar.xz` of a bootable Image Replicator USB disk.

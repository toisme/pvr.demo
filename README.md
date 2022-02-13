[![License: GPL-2.0-or-later](https://img.shields.io/badge/License-GPL%20v2+-blue.svg)](LICENSE.md)

# Demo PVR
Demo PVR client addon for [Kodi](https://kodi.tv)

## Build instructions

### Linux

1. `git clone --branch Matrix https://github.com/xbmc/xbmc.git`
2. `git clone --branch Matrix https://github.com/toisme/pvr.demo.json.git`
3. `mkdir -p xbmc/cmake/addons/addons/pvr.demo.json/`
4. `echo "pvr.demo.json https://github.com/toisme/pvr.demo.json Matrix" > xbmc/cmake/addons/addons/pvr.demo.json/pvr.demo.json.txt`
5. `cd pvr.demo.json && mkdir build && cd build`
6. `cmake -DADDONS_TO_BUILD=pvr.demo.json -DADDON_SRC_PREFIX=../.. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../xbmc/addons -DPACKAGE_ZIP=1 ../../xbmc/cmake/addons`
7. `make`

### Android

Prior to build the addon for Android, Kodi build environmento for Android should be prepared.
Please read **[Android build guide](https://github.com/xbmc/xbmc/blob/Matrix/docs/README.Android.md)** first.

#### Build tools and initial addon build

1. Get the repos
 * `cd $HOME`
 * `git clone --branch Matrix https://github.com/xbmc/xbmc.git`
 * `git clone --branch Matrix https://github.com/toisme/pvr.demo.json.git`
2. Build the Kodi tools
 * `cd $HOME/xbmc/tools/depends`
 * `./bootstrap`
 * `./configure --with-tarballs=$HOME/android-tools/xbmc-tarballs --host=aarch64-linux-android --with-sdk-path=$HOME/android-tools --with-ndk-path=$HOME/android-tools/android-ndk-r20b --prefix=$HOME/android-tools/xbmc-depends`
 * `make -j$(getconf _NPROCESSORS_ONLN)`
3. Build the addon
 * `cd $HOME`
 * `patch -d xbmc -p1 < pvr.demo.json/local_addon_definitions_repository.patch`
 * ``echo "binary-addons file://`pwd`/repo-binary-addons" > xbmc/cmake/addons/bootstrap/repositories/binary-addons.txt``
 * `mkdir -p repo-binary-addons/pvr.demo.json/`
 * ``echo "pvr.demo.json file://`pwd`/pvr.demo.json" > repo-binary-addons/pvr.demo.json/pvr.demo.json.txt``
 * `cd $HOME/xbmc`
 * `make -j$(getconf _NPROCESSORS_ONLN) -C tools/depends/target/binary-addons ADDONS="pvr.demo.json"`

##### Useful links

* [Kodi's PVR user support](https://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support](https://forum.kodi.tv/forumdisplay.php?fid=136)

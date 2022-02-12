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

##### Useful links

* [Kodi's PVR user support](https://forum.kodi.tv/forumdisplay.php?fid=167)
* [Kodi's PVR development support](https://forum.kodi.tv/forumdisplay.php?fid=136)

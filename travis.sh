#!/usr/bin/env sh
# This is called by `.travis.yml` via Travis CI.
# Travis supplies $TRAVIS_OS_NAME.
#  http://docs.travis-ci.com/user/multi-os/
# Our .travis.yml also defines:
#   - SHARED_LIB=ON/OFF
#   - STATIC_LIB=ON/OFF
#   - CMAKE_PKG=ON/OFF
#   - BUILD_TYPE=release/debug
#   - VERBOSE_MAKE=false/true
#   - VERBOSE (set or not)

# -e: fail on error
# -v: show commands
# -x: show expanded commands
set -vex

env | sort

echo ${CXX}
${CXX} --version
python3 --version
meson --version
ninja --version
meson --buildtype ${BUILD_TYPE} --default-library ${LIB_TYPE} . build-${LIB_TYPE}
ninja -v -C build-${LIB_TYPE}
#ninja -v -C build-${LIB_TYPE} test
cd build-${LIB_TYPE}
meson test --no-rebuild --print-errorlogs
cd -
rm -r build-${LIB_TYPE}

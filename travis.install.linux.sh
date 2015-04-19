set -vex

wget https://github.com/ninja-build/ninja/releases/download/v1.7.2/ninja-linux.zip
unzip -q ninja-linux.zip -d build

pip3 install meson
# /usr/bin/gcc is 4.6 always, but gcc-X.Y is available.
if [[ $CXX = g++ ]]; then export CXX="g++-4.9" CC="gcc-4.9"; fi
# /usr/bin/clang has a conflict with gcc, so use clang-X.Y.
if [[ $CXX = clang++ ]]; then export CXX="clang++-3.5" CC="clang-3.5"; fi
echo ${PATH}
ls /usr/local
ls /usr/local/bin
export PATH="${PWD}"/build:/usr/local/bin:/usr/bin:${PATH}

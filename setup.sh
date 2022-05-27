#!/bin/bash
set -e

sudo apt -y install cmake
sudo apt -y install qtbase5-dev libqt5svg5-dev libqt5websockets5-dev libqt5opengl5-dev libqt5x11extras5-dev libprotoc-dev
sudo apt -y install libzmq3-dev

mkdir -p build
cd build
cmake ..
make -j8
sudo make install

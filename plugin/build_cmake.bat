@echo off

cd build
cmake ../ -DBUILD_SHARED_LIBS=ON -DSC_ANIMATE_IMAGE_DEBUG=ON
pause
@echo off

cd build
cmake ../ -DBUILD_SHARED_LIBS=ON -DSC_ANIMATE_IMAGE_DEBUG=OFF -DLIBNEST2D_HEADER_ONLY=ON -DRP_BUILD_SHARED_LIBS=OFF
pause
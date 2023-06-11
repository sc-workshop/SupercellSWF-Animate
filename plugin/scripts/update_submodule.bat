@echo off
cd ../../
call git submodule update --recursive --remote
popd
pause

@echo off
cd ../../
call git submodule update --recursive --remote --force --init
popd
pause

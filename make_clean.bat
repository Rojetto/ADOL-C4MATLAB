@echo off

rmdir /S /Q madDrivers\windows\vc2015\temp32
rmdir /S /Q madDrivers\windows\vc2015\temp64
rmdir /S /Q madDrivers\windows\vc2015\ipch
del /S /Q madDrivers\windows\vc2015\*.user
del /S /Q madDrivers\windows\vc2015\*.ncb
del /S /Q madDrivers\windows\vc2015\*.sdf




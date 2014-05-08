@echo off
rmdir /S /Q madDrivers\windows\vc2008\temp32
rmdir /S /Q madDrivers\windows\vc2008\temp64
rmdir /S /Q madDrivers\windows\vc2008\ipch
del /S /Q madDrivers\windows\vc2008\*.user
del /S /Q madDrivers\windows\vc2008\*.ncb
del /S /Q madDrivers\windows\vc2008\*.sdf

rmdir /S /Q madDrivers\windows\vc2010\temp32
rmdir /S /Q madDrivers\windows\vc2010\temp64
rmdir /S /Q madDrivers\windows\vc2010\ipch
del /S /Q madDrivers\windows\vc2010\*.user
del /S /Q madDrivers\windows\vc2010\*.ncb
del /S /Q madDrivers\windows\vc2010\*.sdf

rmdir /S /Q madDrivers\windows\vc2013\temp32
rmdir /S /Q madDrivers\windows\vc2013\temp64
rmdir /S /Q madDrivers\windows\vc2013\ipch
del /S /Q madDrivers\windows\vc2013\*.user
del /S /Q madDrivers\windows\vc2013\*.ncb
del /S /Q madDrivers\windows\vc2013\*.sdf

del /Q "madDrivers\build32"
del /Q "madDrivers\build64"




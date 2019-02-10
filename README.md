RTMP Live
============
This is a program that show the position and orientation of cube in 3D scene.  

![screenshot](https://raw.github.com/jasonblog/RTMPLive/master/screenshot/RTMP_Live.png)  
![screenshot](https://raw.github.com/jasonblog/RTMPLive/master/screenshot/9334191885812.jpg)  

## Features
- push-stream / pull-stream

## Third party
- libfaac
- libfaad2
- ffmpeg / openh264
- libyuv

## build & install
```
cd RTMPLive
./gradlew assembleRelease
adb install ./app/build/outputs/apk/app-release.apk
```

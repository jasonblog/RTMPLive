RTMP Live
============
![screenshot](https://raw.github.com/jasonblog/RTMPLive/master/screenshot/RTMP_Live.png)  

## Features
- push-stream / pull-stream

## Media Server
- Node-Media-Server

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

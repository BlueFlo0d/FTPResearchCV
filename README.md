# Important Note

This is one of my early project aims to implement high performance structural light reconstruction algorithm on mobile platforms. The performance turns out to be very good. However, I've decided to stop the development, and make all of the code open source.

All code in this repo is released under AGPL v2.0, even the license isn't added to every file.

# FTPResearchCV
Project Moon 3d Reconstruction

Currently the code is for **research** purpose.
The code currently assumed a 1024x1024 image is provided, and the API exposes implemention detail.
It's nessecary to wrap it before using in production environment.
## Current Usage
### Setup
* Setup OpenCV Library 3
* include QTFTP.hpp
```C++
#include "QTFTP.hpp"
```
### API
Since currently Phase Parsing Algrithm and Phase Unwrapping Algrithm is separated, you need to do as what's in TestUtils.cpp:
```C++
//put your structure light image into Cv::Mat input
Mat output = Mat(SIZE_Y,SIZE_X,CV_32F,Scalar(0));
Mat R = Mat(SIZE_Y,SIZE_X,CV_32F);
input = depthMap(input,R);
unwrap_phase(input, output,R);
```
Here the output is raw Phase data. In order to convert to absolute distance data a ToF measured point is required. (from proximity sensor)

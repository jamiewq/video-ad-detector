# video-ad-detector
## Overview

This is a Class that can detect the ads in a video(raw rgb form currently) by first cutting the video into **shots** and then analyzing the length of shots to detect ads( usually rapid changed shots ). 

It can be cool if combined with other method of detecting Ad( may be Machine Learning considering Audio and Content )

The shot cut algorithm idea of using tomograph is from the paper:

Jina Varghese and K. N. Ramachandran Nair. 2016. Detecting Video Shot Boundaries by Modified Tomography. In Proceedings of the Third International Symposium on Computer Vision and the Internet (VisionNet'16). ACM, New York, NY, USA, 131-135. DOI: http://dx.doi.org/10.1145/2983402.2983441


This project is currently limited:

1. Input format is RGB stream (0-255 rgb value each pixel, frame by frame data).
2. Detection correctness depends on the parameters.

## Features
* Shot boundary detection using tomograph
* K-means clustering of length of shots
* Prior knowledge as parameters


## Compile
Make sure you have openCV ready.

~~~
$ clang++ `pkg-config --cflags --libs opencv` main.cpp Image.cpp Detector.cpp -o out
~~~

## Usage
Check out the demo video: [Link to Dataset](https://drive.google.com/drive/folders/0B2jNhQHbeb2dbEhrZ3JwbDBTUE0?usp=sharing)

For more detailed information, checkout Detector.h and main.cpp

~~~
  // Initialization with target rgb file and resolution of each frame
  ShotBoundaryDetector detector("./dataset/Ads/Subway_Ad_15s.rgb", 480, 270);
  detector.display_cut(true); // Display first frame of each shot
  detector.set_similar_threshold(6); // Threshold of how to define similar of two pixel
  detector.set_cut_threshold(10); // Threshold of cut detecting (value of second-order derivative)
  detector.set_min_ad_length(10); // Minimal ad length (frames)
  detector.set_max_ad_length(3600); // Maximal ad length (frames)
  detector.set_max_ad_shot(600); // Priori knowledge, boundary between ad and main shot. No worry to much about this, as we consider great changes first
  detector.set_max_delta(30); //  Maximal delta, Delta = length[i]/length[i-1] or length[i-1]/length[i] whichever >= 1
  
  detector.StartDetection(); // Start Detecting
  vector<pair<long, long> > ads = detector.get_ad_list(); // Contain the frame ID of beginning and ending of each ads detected
~~~

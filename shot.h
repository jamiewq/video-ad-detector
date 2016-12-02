#ifndef SHOT_H_
#define SHOT_H_

#include <iostream>

using namespace std;

class Shot {
public:
  long start_frame_id;
  long end_frame_id;
  long length;
  bool isAd;
  Shot(long s, long e): start_frame_id(s), end_frame_id(e) { length = e - s; isAd = false;}
  Shot(){isAd = false;}
  ~Shot(){}
};

#endif //SHOT_H_

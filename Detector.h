#ifndef DETECTOR_H_
#define DETECTOR_H_

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include "Image.h"

using namespace std;

class ShotBoundaryDetector {
public:

  bool StartDetection();

  vector<MyImage> get_boundary_frames() {
    return boundary_frame_list;
  }

  vector<long> get_boundary_ids() {
    return boundary_id_list;
  }

  void set_file_name(const string& file_name) {
    video_file_name = file_name;
  }

  string get_file_name() {
    return video_file_name;
  }

  void	set_width( const int w) { Width = w; };
	void	set_height( const int h) { Height = h; };

  ShotBoundaryDetector(const string& file_name): video_file_name(file_name){ Width = 0; Height = 0;}
  ShotBoundaryDetector(const string& file_name, int w, int h): video_file_name(file_name), Width(w), Height(h){}
  ShotBoundaryDetector(){video_file_name = ""; Width = 0; Height = 0;}
  ~ShotBoundaryDetector(){}

private:
  int		Width;
	int		Height;
  string video_file_name;
  vector<MyImage> boundary_frame_list;
  vector<long> boundary_id_list;
  bool ReadNextFrame(MyImage& output_image, FILE* fp);
  bool GetAvgRow(MyImage& input_image, char* output_row);
};

#endif //DETECTOR_H_

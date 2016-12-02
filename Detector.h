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

  vector<long> get_boundary_ids() {
    return boundary_id_list;
  }

  void set_file_name(const string& file_name) {
    video_file_name = file_name;
  }

  string get_file_name() {
    return video_file_name;
  }

  vector<pair<long, long> > get_ad_list() {
      return ad_list;
  }

  void set_width( const int w) { Width = w; };
  void set_height( const int h) { Height = h; };
  void set_similar_threshold(const int t) {similarThreshold = t;}
  void display_cut(bool d) {display_each_cut = d;}

  ShotBoundaryDetector(const string& file_name): video_file_name(file_name){ Width = 0; Height = 0; similarThreshold = 12; display_each_cut = false;}
  ShotBoundaryDetector(const string& file_name, int w, int h): video_file_name(file_name), Width(w), Height(h){similarThreshold = 12; display_each_cut = false;}
  ShotBoundaryDetector(){video_file_name = ""; Width = 0; Height = 0; similarThreshold = 12; display_each_cut = false;}
  ~ShotBoundaryDetector(){}

private:
  int		Width;
  int		Height;
  string    video_file_name;
  vector<long>  boundary_id_list;
  vector<pair<long, long> >     ad_list;
  bool  display_each_cut;
  int   similarThreshold;
  bool  ReadNextFrame(MyImage& output_image, FILE* fp);
  bool  similar(char* row1, int pos1, char* row2, int pos2);
  bool  GetAvgRow(MyImage& input_image, char* output_row);
};

#endif //DETECTOR_H_

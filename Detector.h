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
  ofstream log;


  int max_ad_shot_length;
  float max_delta;
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
  void set_min_ad_length(const long t) { min_ad_length = t; }
  void set_max_ad_length(const long t) { max_ad_length = t; }
  void set_cut_threshold(const float t) { cutThreshold = t; }
  void set_max_delta(const float t) { max_delta = t; }
  void set_max_ad_shot( const int t) { max_ad_shot_length = t; };
  void set_width( const int w) { Width = w; };
  void set_height( const int h) { Height = h; };
  void set_similar_threshold(const int t) { similarThreshold = t; }
  void display_cut(bool d) { display_each_cut = d; }

  ShotBoundaryDetector(const string& file_name): video_file_name(file_name){
      max_ad_shot_length = INT_MAX;
      min_ad_length = 0;
      max_ad_length = INT_MAX;
      max_delta = INT_MAX;
      cutThreshold = 20;
      log.open("log.txt");
      Width = 0;
      Height = 0;
      similarThreshold = 12;
      display_each_cut = false;
  }
  ShotBoundaryDetector(const string& file_name, int w, int h): video_file_name(file_name), Width(w), Height(h){
      max_ad_shot_length = INT_MAX;
      min_ad_length = 0;
      max_ad_length = INT_MAX;
      max_delta = INT_MAX;
      cutThreshold = 20;
      log.open("log.txt");
      similarThreshold = 12;
      display_each_cut = false;
  }
  ShotBoundaryDetector(){
      max_ad_shot_length = INT_MAX;
      min_ad_length = 0;
      max_ad_length = INT_MAX;
      max_delta = INT_MAX;
      cutThreshold = 20;
      log.open("log.txt");
      video_file_name = "";
      Width = 0; Height = 0;
      similarThreshold = 12;
      display_each_cut = false;
  }
  ~ShotBoundaryDetector(){}

private:
  int		Width;
  int		Height;
  string    video_file_name;
  long      min_ad_length;
  long      max_ad_length;
  bool      display_each_cut;
  int       similarThreshold;
  float     cutThreshold;
  bool      ReadNextFrame(MyImage& output_image, FILE* fp);
  bool      similar(char* row1, int pos1, char* row2, int pos2);
  bool      GetAvgRow(MyImage& input_image, char* output_row);
  vector<long>  boundary_id_list;
  vector<pair<long, long> >     ad_list;
};

#endif //DETECTOR_H_

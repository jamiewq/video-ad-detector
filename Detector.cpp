#include "Detector.h"
#include "Shot.h"
#include <cv.h>
#include <highgui.h>

using namespace cv;

// Compile : clang++ `pkg-config --cflags --libs opencv` main.cpp Image.cpp Detector.cpp -o out
bool display(MyImage& img, int index){
  static int count = 1;
  char* imgData = img.getImageData();
  Mat image(img.getHeight(), img.getWidth(),CV_8UC3, Scalar(0, 0, 0));

  for (int i = 0; i < image.cols; i++) {
      for (int j = 0; j < image.rows; j++) {
          Vec3b &intensity = image.at<Vec3b>(j, i);
          // calculate pixValue
          int position = j * img.getWidth() * 3 + i * 3;
          intensity.val[0] = imgData[position];
          intensity.val[1] = imgData[position + 1];
          intensity.val[2] = imgData[position + 2];
       }
  }
  stringstream ss;
  ss << "Display Frame" << index << " No."<<count;
  namedWindow(ss.str() , CV_WINDOW_AUTOSIZE );

  imshow( ss.str(), image );

  waitKey(100);
  count++;
  return true;
}

bool ShotBoundaryDetector::similar(char* row1, int pos1, char* row2, int pos2) {
  if( (abs(row1[pos1]-row2[pos2]) +
      abs(row1[pos1 + 1]-row2[pos2 + 1]) +
      abs(row1[pos1 + 2]-row2[pos2 + 2]) ) < similarThreshold ) {
    return true;
  }
  else return false;
}

float GetFloatPrecision(float value, float precision)
{
    return (floor((value * pow(10, precision) + 0.5)) / pow(10, precision));
}

bool ShotBoundaryDetector::StartDetection() {

  if (video_file_name.size() == 0 || Width == 0 || Height == 0)
  {
    fprintf(stderr, "Video location not defined");
    return false;
  }

  FILE *IN_FILE;
  IN_FILE = fopen(video_file_name.c_str(), "rb");
  if ( IN_FILE == NULL )
  {
    fprintf(stderr, "Error Opening File for Reading");
    return false;
  }

  vector<vector<char> > tomoData_r;
  vector<vector<char> > tomoData_g;
  vector<vector<char> > tomoData_b;

  char* avg_row_prev = new char[Width*3];
  char* avg_row_curr = new char[Width*3];

  vector<int> counts;
  vector<float> dc;
  vector<float> ddc;
  float fade_begin_dc = -1;

  int i = 0;
  // Include first frame
  boundary_id_list.push_back(0);
  while ( !feof(IN_FILE) )
  {
    counts.push_back(1);

    MyImage frame;
    frame.setWidth(Width);
    frame.setHeight(Height);

    stringstream ss;
    ss << "frame_" << i<<".rgb";
    frame.setImagePath(ss.str().c_str());

    if(!ReadNextFrame(frame, IN_FILE)) return false;

    if(!GetAvgRow(frame,avg_row_curr)) return false;

    tomoData_r.push_back(vector<char>(0));
    tomoData_g.push_back(vector<char>(0));
    tomoData_b.push_back(vector<char>(0));

    for(int col = 0; col < Width; col++) {
      int row_position = col * 3;
      tomoData_r[i].push_back( avg_row_curr[row_position] );
      tomoData_g[i].push_back( avg_row_curr[row_position + 1] );
      tomoData_b[i].push_back( avg_row_curr[row_position + 2] );
		}

    char* t = avg_row_prev;
    avg_row_prev = avg_row_curr;
    avg_row_curr = t;

    for(int col = 0; col < Width; col++) {
      int pos = col * 3;
      if(similar(avg_row_curr, pos, avg_row_prev, pos)) {
        counts[i] ++;
      }
      else if( pos - 3 >= 0 && similar(avg_row_curr, pos, avg_row_prev, pos - 3)) {
        counts[i] ++;
      }
      else if(pos + 3 < Width && similar(avg_row_curr, pos, avg_row_prev, pos + 3) ) {
        counts[i] ++;
      }
    }

    if(i >= 1) dc.push_back(1.0 * counts[i] / counts[i-1]);
    if(i >= 2) {
      ddc.push_back(dc.back() / dc[dc.size()-2]);

      if(fade_begin_dc < 0) {
        if(ddc.back() > 20) {
            //find a hard cut
            frame.WriteImage();
            if(display_each_cut) display(frame, i);
            boundary_id_list.push_back(i);
        }
        else if(ddc.back() > 5) {
          fade_begin_dc = dc[dc.size()-2];
        }
      }
      else {
        if( dc.back() / fade_begin_dc > 20) {
          fade_begin_dc = -1;
          //find a fade cut
          frame.WriteImage();
          if(display_each_cut) display(frame, i);
          boundary_id_list.push_back(i);
        }
      }

    }
    i++;
  }
  // Include last frame
  boundary_id_list.push_back(i-1);

  vector<Shot> shot_list;

  for(int i = 0; i < boundary_id_list.size() - 1; i++) {
    Shot new_shot = Shot(boundary_id_list[i], boundary_id_list[i+1]);
    // Eliminate those extremly short shot, which might be wrong cut
    if(new_shot.length <= 10) continue;
    shot_list.push_back(new_shot);
  }

  Mat dataset(shot_list.size(),1, CV_32F);
  Mat lables(shot_list.size(),1, CV_32F);
  Mat centers;

  for(int i = 0; i < shot_list.size(); i++) {
    // cout<< "Shot"<< i << ".length = " << shot_list[i].length << " start : " << shot_list[i].start_frame_id << " end : " << shot_list[i].end_frame_id << endl;
    if(i == 0) {
        dataset.at<float>(i) =  1;
        continue;
    }
    float l1 = max(shot_list[i].length, shot_list[i-1].length);
    float l2 = min(shot_list[i].length, shot_list[i-1].length);
    dataset.at<float>(i) = l1/l2;
  }

  kmeans( dataset, 2, lables, TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0), 3, KMEANS_RANDOM_CENTERS, centers);

  cout << "Ad detection result : " <<endl;
  float lable_1_sample = -1;
  float lable_0_sample = -1;
  for(int i = 0; i < shot_list.size(); i++) {
      cout <<"shot "<< i << "\tlength : "<< shot_list[i].length << "\t delta: "<< dataset.at<float>(i)<< "\tfrom : "<< shot_list[i].start_frame_id << "\tto : "<< shot_list[i].end_frame_id << "\tlabel: "<< lables.at<int>(i) <<endl;
      if(lables.at<int>(i) == 0) {
          lable_0_sample = dataset.at<float>(i);
      }
      else {
          lable_1_sample = dataset.at<float>(i);
      }
  }

  vector<pair<long, long> > ad_list_frameid_frameid;

  long ad_start_frame = 0;
  long ad_end_frame = 0;
  // lable 0 is Ad change frame
  if(lable_0_sample > lable_1_sample) {

      for(int i = 0; i < shot_list.size(); i++) {
          if(lables.at<int>(i) == 0) {
              // from main content to Ad
              if(shot_list[i].length < shot_list[i-1].length) {
                  if(!(i-1 >=0 && lables.at<int>(i-1) == 0 && shot_list[i-1].length < shot_list[i-2].length))
                    ad_start_frame = shot_list[i].start_frame_id;
              }
              // from Ad to main content
              else if(shot_list[i].length > shot_list[i-1].length){
                  if(!(i+1 < shot_list.size() && lables.at<int>(i+1) == 0 && shot_list[i+1].length > shot_list[i].length)) {
                      ad_end_frame = shot_list[i-1].end_frame_id;
                      ad_list_frameid_frameid.push_back( make_pair(ad_start_frame, ad_end_frame) );
                  }
              }
          }
      }
  }
  // lable 1 is Ad change frame
  else {
      for(int i = 0; i < shot_list.size(); i++) {
          if(lables.at<int>(i) == 1) {
              // from main content to Ad
              if(shot_list[i].length < shot_list[i-1].length) {
                  if(!(i-1 >=0 && lables.at<int>(i-1) == 1 && shot_list[i-1].length < shot_list[i-2].length))
                    ad_start_frame = shot_list[i].start_frame_id;
              }
              // from Ad to main content
              else if(shot_list[i].length > shot_list[i-1].length){
                  if(!(i+1 < shot_list.size() && lables.at<int>(i+1) == 1 && shot_list[i+1].length > shot_list[i].length)) {
                      ad_end_frame = shot_list[i-1].end_frame_id;
                      ad_list_frameid_frameid.push_back( make_pair(ad_start_frame, ad_end_frame) );
                  }
              }
          }
      }
  }

  for(int i = 0 ; i < ad_list_frameid_frameid.size(); i++) {
      cout <<"ad" << i <<":\t";
      cout << "start frame : " << ad_list_frameid_frameid[i].first << " end frame : " << ad_list_frameid_frameid[i].second<<endl;
  }

  ad_list = ad_list_frameid_frameid;

  int frame_num = i;
  cout << "The Video contains" << frame_num << " frames" <<endl;

  // Generate tomograph for whole video
  MyImage tomograph;
  tomograph.setWidth(Width);
  tomograph.setHeight(frame_num);
  tomograph.setImagePath("tomograph.rgb");

  char * tomoData = new char[Width*i*3];

  for(int row = 0; row < frame_num; row ++) {
    for(int col = 0; col < Width; col ++) {
      int pos = row * Width * 3 + col * 3;
      tomoData[pos] = tomoData_r[row][col];
      tomoData[pos + 1] = tomoData_g[row][col];
      tomoData[pos + 2] = tomoData_b[row][col];
    }
  }

  tomograph.setImageData(tomoData);
  tomograph.WriteImage();

  fclose(IN_FILE);
  if(display_each_cut) waitKey(0);
  return true;
}

bool ShotBoundaryDetector::GetAvgRow(MyImage& input_image, char* output_row) {
  if(input_image.getWidth() == 0 || input_image.getHeight() == 0 || input_image.getWidth() != Width || input_image.getHeight() != Height) {
    fprintf(stderr, "Error getting avg row, size invalid");
    return false;
  }

  char *Data = input_image.getImageData();
  for(int col = 0; col < Width; col++) {
    double sum_r = 0;
    double sum_g = 0;
    double sum_b = 0;
    int rows = 0;
    for(int row = 0; row < Height; row++) {
      rows ++;
      int position = row * Width * 3 + col * 3;
      sum_r += Data[position];
      sum_g += Data[position + 1];
      sum_b += Data[position + 2];
		}
    output_row[col * 3] = (char)( min<double>(255,max<double>(sum_r / rows, 0)) );
    output_row[col * 3 + 1] = (char)( min<double>(255,max<double>(sum_g / rows, 0)) );
    output_row[col * 3 + 2] = (char)( min<double>(255,max<double>(sum_b / rows, 0)) );
  }

  return true;
}

bool ShotBoundaryDetector::ReadNextFrame(MyImage& output_image, FILE* fp) {

      char *Data   = new char[Width*Height*3];

      // BGR for testing on windows
    	for (int i = 0; i < Width*Height; i ++)
    	{
    			Data[3*i+2] = fgetc(fp);
    	}
    	for (int i = 0; i < Width*Height; i ++)
    	{
    		  Data[3*i+1]	= fgetc(fp);
    	}
    	for (int i = 0; i < Width*Height; i ++)
    	{
    		  Data[3*i] = fgetc(fp);
    	}

      output_image.setImageData(Data);

    	return true;
}

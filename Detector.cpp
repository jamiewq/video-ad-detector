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
    log << i << "\t" << counts[i] << "\t";

    if(i >= 1) dc.push_back(1.0 * counts[i] / counts[i-1]);
    if(i >= 2) {
      ddc.push_back(dc.back() / dc[dc.size()-2]);
      log << i << "\t" << ddc.back() << endl;
      if(fade_begin_dc < 0) {
        if(ddc.back() > cutThreshold) {
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
        if( dc.back() / fade_begin_dc > cutThreshold) {
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
  int frame_num = i;
  // Include last frame
  boundary_id_list.push_back(i-1);

  vector<Shot> origin_shot_list;

  for(int i = 0; i < boundary_id_list.size() - 1; i++) {
    Shot new_shot = Shot(boundary_id_list[i], boundary_id_list[i+1]);
    // Eliminate those extremly short shot, which might be wrong cut
    while(i + 1 < boundary_id_list.size() && (boundary_id_list[i+1]-boundary_id_list[i]) <= 10) i++;
    if(i + 1 < boundary_id_list.size()) {
        new_shot.end_frame_id = boundary_id_list[i+1];
        new_shot.length = new_shot.end_frame_id - new_shot.start_frame_id;
    }
    origin_shot_list.push_back(new_shot);
  }

  // Preprocessing of shot lengths
  // If we have a sudden drop of length then sudden up again, it is a fault. No ad will only contain a single short shot
  vector<Shot> shot_list = origin_shot_list;
  for(int i = 0 ; i < origin_shot_list.size(); i++) {
      if(i == 0 && (float)(origin_shot_list[i+1].length) / origin_shot_list[i].length > 10 ) {
          shot_list[0].length = origin_shot_list[i+1].length;
      }
      else if(i == (origin_shot_list.size() - 1) && i != 0 && (float)(origin_shot_list[i-1].length)/ origin_shot_list[i].length > 10){
          shot_list[i].length = origin_shot_list[i-1].length;
      }
      else if(i > 0 && i + 1 < origin_shot_list.size() && (float)(origin_shot_list[i-1].length) / origin_shot_list[i].length > 3 && (float)(origin_shot_list[i+1].length) / origin_shot_list[i].length > 3){
          shot_list[i].length = min(origin_shot_list[i-1].length, origin_shot_list[i+1].length);
      }
      if (shot_list[i].length > max_ad_shot_length) shot_list[i].length *= 1.5;
      else if (shot_list[i].length > max_ad_shot_length / 2) shot_list[i].length *= 1;
      else if (shot_list[i].length < max_ad_shot_length / 2) shot_list[i].length *= 0.7;
  }

  vector<float> dataset_in_vector(shot_list.size());
  Mat dataset_change(shot_list.size(),1, CV_32F);
  Mat lables_change(shot_list.size(),1, CV_32F);
  Mat centers;

  for(int i = 0; i < shot_list.size(); i++) {
    // cout<< "Shot"<< i << ".length = " << shot_list[i].length << " start : " << shot_list[i].start_frame_id << " end : " << shot_list[i].end_frame_id << endl;
    if(i == 0) {
        dataset_change.at<float>(i) =  1;
        dataset_in_vector[i] = 1;
        continue;
    }
    float l1 = max(shot_list[i].length, shot_list[i-1].length);
    float l2 = min(shot_list[i].length, shot_list[i-1].length);
    dataset_change.at<float>(i) = min(l1/l2, max_delta);
    dataset_in_vector[i] = dataset_change.at<float>(i);
  }

  kmeans( dataset_change, 2, lables_change, TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 10, 1.0), 3, KMEANS_RANDOM_CENTERS, centers);

  cout << "Ad detection result : " <<endl;
  float lable_1_sample = -1;
  float lable_0_sample = -1;
  float lable_1_sample_lengths = -1;
  float lable_0_sample_lengths = -1;
  for(int i = 0; i < shot_list.size(); i++) {
      cout <<"shot "<< i <<  "\tlength : "<< origin_shot_list[i].length << "\tweight : "<< shot_list[i].length << "\t delta: "<< dataset_change.at<float>(i)<< "\tfrom : "<< shot_list[i].start_frame_id << "\tto : "<< shot_list[i].end_frame_id << "\tlabel: "<< lables_change.at<int>(i)<<endl;
      if(lables_change.at<int>(i) == 0) {
          lable_0_sample = dataset_change.at<float>(i);
      }
      else {
          lable_1_sample = dataset_change.at<float>(i);
      }
  }

  vector<pair<long, long> > ad_list_frameid_frameid;
  vector<long> ad_list_shot_ids;

  long ad_start_frame = -1;
  long ad_end_frame = -1;
  long start_id = -1;
  // lable 0 is Ad change frame
  if(lable_0_sample > lable_1_sample) {
    // Deal with gradually change, they all labled with 0(no great change), but 29-138-291 is a gruadual change:
    // shot 22	length : 51	weight : 35	 delta: 1.59091	from : 6337	to : 6388	label: 0
    // shot 23	length : 24	weight : 16	 delta: 2.1875	from : 6388	to : 6412	label: 0
    // shot 24	length : 39	weight : 27	 delta: 1.6875	from : 6412	to : 6451	label: 0
    // shot 25	length : 138	weight : 96	 delta: 3.55556	from : 6451	to : 6589	label: 0
    // shot 26	length : 291	weight : 436	 delta: 4.54167	from : 6589	to : 6880	label: 0
    // shot 27	length : 668	weight : 1002	 delta: 2.29817	from : 6880	to : 7548	label: 0
    // shot 28	length : 392	weight : 588	 delta: 1.70408	from : 7548	to : 7940	label: 0
      for(int i = 0 ; i < shot_list.size()-1; i++) {
          if(lables_change.at<int>(i) == 1
              && lables_change.at<int>(i+1) == 1
              && dataset_change.at<float>(i) > 3
              && dataset_change.at<float>(i+1) > 3
          ) {
                cout <<" find a fucking gradually change"<<endl;
                float avg_prev = 0;
                float avg_post = 0;
                int count_prev = 0;
                int count_post = 0;
                for(int j = 1 ; j <= 2; j++) {
                    if(i - j >= 0) {
                        avg_prev += shot_list[i - j].length;
                        count_prev ++;
                    }
                    if(i + j < shot_list.size()) {
                        avg_post += shot_list[i + j].length;
                        count_post ++;
                    }
                    cout <<"avg_prev: " << avg_prev << endl;
                    cout <<"avg_post: " << avg_post << endl;
                    cout <<"count_prev: " << count_prev << endl;
                    cout <<"count_post: " << count_post << endl;
                }
                if(count_prev) avg_prev /= count_prev;
                if(count_post) avg_post /= count_post;
                cout <<"avg_prev: " << avg_prev << endl;
                cout <<"avg_post: " << avg_post << endl;
                if( avg_prev && avg_post && ((avg_post / avg_prev) > 5 || (avg_prev / avg_post) > 5 )) {
                    cout <<"get a fucking shit" <<endl;
                    lables_change.at<int>(i) = 0;
                }
            }
      }

      cout<<"0 is big change"<<endl;
      for(int i = 0; i < shot_list.size(); i++) {
          if(lables_change.at<int>(i) == 0) {
              // from main content to Ad
              if(ad_start_frame == -1 && shot_list[i].length < shot_list[i-1].length) {
                  ad_start_frame = shot_list[i].start_frame_id;
                  start_id = i;
                  cout << "ad_start_frame" << ad_start_frame<<endl;
              }
              // from Ad to main content
              else if(shot_list[i].length > shot_list[i-1].length ){
                  if(!(i+1 < shot_list.size() && lables_change.at<int>(i+1) == 0 && shot_list[i+1].length > shot_list[i].length)) {
                      for(int j = start_id; j <= i-1; j++) ad_list_shot_ids.push_back(j);
                      start_id = -1;
                      ad_end_frame = shot_list[i-1].end_frame_id;
                      if( ad_end_frame - std::max(ad_start_frame, 0l) >= min_ad_length && ad_end_frame - std::max(ad_start_frame, 0l) <= max_ad_length)
                        ad_list_frameid_frameid.push_back( make_pair(std::max(ad_start_frame, 0l), ad_end_frame) );
                      ad_start_frame = -1;
                  }
              }
          }
          else if(i == shot_list.size()-1 && ad_start_frame != -1) {
              for(int j = start_id; j <= i; j++) ad_list_shot_ids.push_back(j);
              start_id = -1;
              ad_end_frame = shot_list[i].end_frame_id;
              if( ad_end_frame - std::max(ad_start_frame, 0l) >= min_ad_length && ad_end_frame - std::max(ad_start_frame, 0l) <= max_ad_length)
                ad_list_frameid_frameid.push_back( make_pair(std::max(ad_start_frame, 0l), ad_end_frame) );
              ad_start_frame = -1;
          }
      }
  }
  // lable 1 is Ad change frame
  else {
      // Deal with gradually change, they all labled with 0(no great change), but 29-138-291 is a gruadual change:
      // shot 22	length : 51	weight : 35	 delta: 1.59091	from : 6337	to : 6388	label: 0
      // shot 23	length : 24	weight : 16	 delta: 2.1875	from : 6388	to : 6412	label: 0
      // shot 24	length : 39	weight : 27	 delta: 1.6875	from : 6412	to : 6451	label: 0
      // shot 25	length : 138	weight : 96	 delta: 3.55556	from : 6451	to : 6589	label: 0
      // shot 26	length : 291	weight : 436	 delta: 4.54167	from : 6589	to : 6880	label: 0
      // shot 27	length : 668	weight : 1002	 delta: 2.29817	from : 6880	to : 7548	label: 0
      // shot 28	length : 392	weight : 588	 delta: 1.70408	from : 7548	to : 7940	label: 0
        for(int i = 0 ; i < shot_list.size()-1; i++) {
            if(lables_change.at<int>(i) == 0
                && lables_change.at<int>(i+1) == 0
                && dataset_change.at<float>(i) > 3
                && dataset_change.at<float>(i+1) > 3
            ) {
                cout <<" find a fucking gradually change"<<endl;
                  long avg_prev = 0;
                  long avg_post = 0;
                  int count_prev = 0;
                  int count_post = 0;
                  for(int j = 1 ; j <= 2; j++) {
                      if(i - j >= 0) {
                          avg_prev += shot_list[i - j].length;
                          count_prev ++;
                      }
                      if(i + j < shot_list.size()) {
                          avg_post += shot_list[i + j].length;
                          count_post ++;
                      }
                      cout <<"avg_prev: " << avg_prev << endl;
                      cout <<"avg_post: " << avg_post << endl;
                      cout <<"count_prev: " << count_prev << endl;
                      cout <<"count_post: " << count_post << endl;
                  }
                  if(count_prev) avg_prev /= count_prev;
                  if(count_post) avg_post /= count_post;
                  cout <<"avg_prev: " << avg_prev << endl;
                  cout <<"avg_post: " << avg_post << endl;
                  if( avg_prev && avg_post && ((avg_post / avg_prev) > 5 || (avg_prev / avg_post) > 5 )) {
                      cout <<"get a fucking shit" <<endl;
                      lables_change.at<int>(i) = 1;
                  }
              }
        }

      cout<<"1 is big change"<<endl;
      for(int i = 0; i < shot_list.size(); i++) {
          if(lables_change.at<int>(i) == 1) {
              // from main content to Ad
              if(ad_start_frame == -1 && shot_list[i].length < shot_list[i-1].length) {
                  ad_start_frame = shot_list[i].start_frame_id;
                  start_id = i;
                  cout << "ad_start_frame" << ad_start_frame<<endl;
              }
              // from Ad to main content
              else if(shot_list[i].length > shot_list[i-1].length){
                  if(!(i+1 < shot_list.size() && lables_change.at<int>(i+1) == 1 && shot_list[i+1].length > shot_list[i].length)) {
                      for(int j = start_id; j <= i-1; j++) ad_list_shot_ids.push_back(j);
                      start_id = -1;
                      ad_end_frame = shot_list[i-1].end_frame_id;
                      if( ad_end_frame - std::max(ad_start_frame, 0l) >= min_ad_length && ad_end_frame - std::max(ad_start_frame, 0l) <= max_ad_length)
                        ad_list_frameid_frameid.push_back( make_pair(std::max(ad_start_frame, 0l), ad_end_frame) );
                      ad_start_frame = -1;
                  }
              }
          }
          else if(i == shot_list.size()-1 && ad_start_frame != -1) {
              for(int j = start_id; j <= i; j++) ad_list_shot_ids.push_back(j);
              start_id = -1;
              ad_end_frame = shot_list[i].end_frame_id;
              if( ad_end_frame - std::max(ad_start_frame, 0l) >= min_ad_length && ad_end_frame - std::max(ad_start_frame, 0l) <= max_ad_length)
                ad_list_frameid_frameid.push_back( make_pair(std::max(ad_start_frame, 0l), ad_end_frame) );
              ad_start_frame = -1;
          }
      }
  }

  cout << "shot of ADs : " << endl;
  for(int i = 0; i < ad_list_shot_ids.size(); i++) {
      cout<< i << "\t" << ad_list_shot_ids[i] <<endl;
  }

  for(int i = 0 ; i < ad_list_frameid_frameid.size(); i++) {
      cout <<"ad" << i <<":\t";
      cout << "start frame : " << ad_list_frameid_frameid[i].first << " end frame : " << ad_list_frameid_frameid[i].second<<endl;
  }

  ad_list = ad_list_frameid_frameid;

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

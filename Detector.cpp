#include "Detector.h"


bool similar(char* row1, int pos1, char* row2, int pos2) {
  if( abs(row1[pos1]-row2[pos2]) +
      abs(row1[pos1 + 1]-row2[pos2 + 1]) +
      abs(row1[pos1 + 2]-row2[pos2 + 2]) < 12 ) {
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

  ofstream myfile1;
  ofstream myfile2;
  myfile1.open ("ddc-y.txt");
  myfile2.open ("ddc-x.txt");

  int i = 0;
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
      myfile1<< GetFloatPrecision(ddc.back(),3) <<endl; // ddc val
      myfile2<<( (i%3==0)? 1.0*i/30 : i * 0.0333)<<endl; // time

      if(fade_begin_dc < 0) {
        if(ddc.back() > 20) {
            //find a hard cut
            frame.WriteImage();
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
          boundary_id_list.push_back(i);
        }
      }

    }
    i++;
  }

  int frame_num = i;
  cout << "Video contains" << frame_num << " frames" <<endl;
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

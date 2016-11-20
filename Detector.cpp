#include "Detector.h"


bool similar(char* row1, int pos1, char* row2, int pos2) {
  if( abs(row1[pos1]-row2[pos2]) +  abs(row1[pos1 + 1]-row2[pos2 + 1]) + abs(row1[pos1 + 2]-row2[pos2 + 2]) < 12 ) {
    return true;
  }
  else return false;
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

  int num_frame = 450;

  MyImage tomograph;
  tomograph.setWidth(Width);
  tomograph.setHeight(num_frame);
  tomograph.setImagePath("tomograph.rgb");
  char* tomoData = new char[tomograph.getWidth()*tomograph.getHeight()*3];

  long frame_id = 1;
  // while(!feof(IN_FILE))

  char* avg_row_prev = new char[Width*3];
  char* avg_row_curr = new char[Width*3];

  vector<int> counts(num_frame, 0);

  for(int i = 0; i < num_frame; i++)
  {
    MyImage frame;
    frame.setWidth(Width);
    frame.setHeight(Height);
    stringstream ss;
    ss << "frame_" << frame_id<<".rgb";
    frame.setImagePath(ss.str().c_str());
    if(!ReadNextFrame(frame, IN_FILE)) return false;
    //if(frame_id % 90 == 0) frame.WriteImage();
    frame_id ++;

    if(!GetAvgRow(frame,avg_row_curr)) return false;

    for(int col = 0; col < Width; col++) {
      int pic_position = i * Width * 3 + col * 3;
      int row_position = col * 3;
      tomoData[pic_position] = avg_row_curr[row_position];
      tomoData[pic_position + 1] = avg_row_curr[row_position + 1];
      tomoData[pic_position + 2] = avg_row_curr[row_position + 2];
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

  }

  vector<float> dc(num_frame, 0);
  for(int i = 1; i < num_frame; i++) {
    dc[i] = counts[i-1] !=0 ? 1.0* counts[i] / counts[i-1]: 100;
  }

  vector<float> ddc(num_frame, 0);
  ofstream myfile1;
  ofstream myfile2;
  myfile1.open ("result-y.txt");
  myfile2.open ("result-x.txt");

  for(int i = 2; i < num_frame; i++) {
    ddc[i] = dc[i-1] !=0? dc[i] / dc[i-1] : 100;
    myfile1<<ddc[i]<<endl;
    myfile2<<i * 0.033<<endl;
  }
  myfile1.close();
  myfile2.close();

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
    for(int row = 0; row < Height; row++) {
      int position = row * Width * 3 + col * 3;
      sum_r += Data[position];
      sum_g += Data[position + 1];
      sum_b += Data[position + 2];
		}
    output_row[col * 3] = (char)( min<double>(255,max<double>(sum_r / Height, 0)) );
    output_row[col * 3 + 1] = (char)( min<double>(255,max<double>(sum_g / Height, 0)) );
    output_row[col * 3 + 2] = (char)( min<double>(255,max<double>(sum_b / Height, 0)) );
  }

  return true;
}

bool ShotBoundaryDetector::ReadNextFrame(MyImage& output_image, FILE* fp) {
    	// Create and populate RGB buffers
    	int i;
    	char *Rbuf = new char[Height*Width];
    	char *Gbuf = new char[Height*Width];
    	char *Bbuf = new char[Height*Width];

      char *Data   = new char[Width*Height*3];

    	for (i = 0; i < Width*Height; i ++)
    	{
    		Rbuf[i] = fgetc(fp);
    	}
    	for (i = 0; i < Width*Height; i ++)
    	{
    		Gbuf[i] = fgetc(fp);
    	}
    	for (i = 0; i < Width*Height; i ++)
    	{
    		Bbuf[i] = fgetc(fp);
    	}

    	// Allocate Data structure and copy
    	Data = new char[Width*Height*3];
    	for (i = 0; i < Height*Width; i++)
    	{
    		Data[3*i]	= Bbuf[i];
    		Data[3*i+1]	= Gbuf[i];
    		Data[3*i+2]	= Rbuf[i];
    	}

    	// Clean up and return
    	delete[] Rbuf;
    	delete[] Gbuf;
    	delete[] Bbuf;

      output_image.setImageData(Data);

    	return true;
}

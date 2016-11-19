#include "Detector.h"

bool ShotBoundaryDetector::StartDetection() {
  if (video_file_name.size() == 0 && Width > 0 && Height > 0)
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

  long frame_id = 1;
  stringstream ss;
  while(!feof(IN_FILE))
  {
    MyImage frame;
    frame.setWidth(Width);
    frame.setHeight(Height);
    ss << "frame_" << frame_id;
    frame.setImagePath(ss.str().c_str());
    if(!ReadNextFrame(frame, IN_FILE)) return false;

  }

  fclose(IN_FILE);

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

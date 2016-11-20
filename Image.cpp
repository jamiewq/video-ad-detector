#include "Image.h"

// Constructor and Desctructors
MyImage::MyImage()
{
	Data = NULL;
	Width = -1;
	Height = -1;
	ImagePath[0] = 0;
}

MyImage::~MyImage()
{
	if ( Data )
		delete[] Data;
}

// Copy constructor
MyImage::MyImage( MyImage *otherImage)
{
	Height = otherImage->Height;
	Width  = otherImage->Width;
	Data   = new char[Width*Height*3];
	strcpy(otherImage->ImagePath, ImagePath );

	for ( int i=0; i<(Height*Width*3); i++ )
	{
		Data[i]	= otherImage->Data[i];
	}
}

// = operator overload
MyImage & MyImage::operator= (const MyImage &otherImage)
{
	Height = otherImage.Height;
	Width  = otherImage.Width;
	Data   = new char[Width*Height*3];
	strcpy( (char *)otherImage.ImagePath, ImagePath );

	for ( int i=0; i<(Height*Width*3); i++ )
	{
		Data[i]	= otherImage.Data[i];
	}

	return *this;

}

// MyImage::CreatImageCanv
// Function to create white image with two dots connected
bool MyImage::CreatImageCanv()
{
	// Allocate Data structure and copy
	Data = new char[Width*Height*3];
	for (int i = 0; i < Height*Width; i++)
	{
		Data[3*i]	= 255;
		Data[3*i+1]	= 255;
		Data[3*i+2]	= 255;
	}

	// two coordinates to connect a line, x is for height, y is for row
    int x1 = 200, y1 = 400;
    int x2 = 10, y2 = 200;

    int start_x = x1, start_y = y1, end_x = x2, end_y = y2;
    int dx = x2 - x1;
    int dy = y2 - y1;
    double slope = 0;

    bool anchor_x;
    if(dx != 0) slope = dy/(double)dx;

	if(std::abs(slope) <= 1 && dx !=0) {
    	anchor_x = true;
    	if(dx < 0) {
    		start_x = x2;
    		start_y = y2;
    		end_x = x1;
    		end_y = y1;
    	}
    	slope = (end_y - start_y)/(double)(end_x - start_x);
    }
    else {
    	anchor_x = false;
    	if(dy < 0) {
    		start_x = x2;
    		start_y = y2;
    		end_x = x1;
    		end_y = y1;
    	}
    	if(dx == 0) {slope = 0;}
    	else {slope = (end_x - start_x)/(double)(end_y - start_y);}
    }

	// set starting point to zero first
	Data[start_y*Width*3 + start_x*3] = 0;
	Data[start_y*Width*3 + start_x*3 + 1] = 0;
	Data[start_y*Width*3 + start_x*3 + 2] = 0;

	if(anchor_x) {
		double y = start_y + 0.5;
		for(int x = start_x + 1; x <= end_x; x++) {
			y = y + slope;
			int int_y = (int)floor(y);
			Data[int_y*Width*3 + x*3] = 0;
			Data[int_y*Width*3 + x*3 + 1] = 0;
			Data[int_y*Width*3 + x*3 + 2] = 0;
		}
	}
	else {
		double x = start_x + 0.5;
		for(int y = start_y + 1; y <= end_y; y++) {
			x = x + slope;
			int int_x = (int)floor(x);
			Data[y*Width*3 + int_x*3] = 0;
			Data[y*Width*3 + int_x*3 + 1] = 0;
			Data[y*Width*3 + int_x*3 + 2] = 0;
		}
	}
	return true;
}

// MyImage::ReadImage
// Function to read the image given a path
bool MyImage::ReadImage()
{

	// Verify ImagePath
	if (ImagePath[0] == 0 || Width < 0 || Height < 0 )
	{
		fprintf(stderr, "Image or Image properties not defined");
		fprintf(stderr, "Usage is `Image.exe Imagefile w h`");
		return false;
	}

	// Create a valid output file pointer
	FILE *IN_FILE;
	IN_FILE = fopen(ImagePath, "rb");
	if ( IN_FILE == NULL )
	{
		fprintf(stderr, "Error Opening File for Reading");
		return false;
	}

	// Create and populate RGB buffers
	int i;
	char *Rbuf = new char[Height*Width];
	char *Gbuf = new char[Height*Width];
	char *Bbuf = new char[Height*Width];

	for (i = 0; i < Width*Height; i ++)
	{
		Rbuf[i] = fgetc(IN_FILE);
	}
	for (i = 0; i < Width*Height; i ++)
	{
		Gbuf[i] = fgetc(IN_FILE);
	}
	for (i = 0; i < Width*Height; i ++)
	{
		Bbuf[i] = fgetc(IN_FILE);
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
	fclose(IN_FILE);

	return true;

}

// MyImage functions defined here
bool MyImage::WriteImage()
{
	// Verify ImagePath
	// Verify ImagePath
	if (ImagePath[0] == 0 || Width < 0 || Height < 0 )
	{
		fprintf(stderr, "Image or Image properties not defined");
		return false;
	}

	// Create a valid output file pointer
	FILE *OUT_FILE;
	OUT_FILE = fopen(ImagePath, "wb");
	if ( OUT_FILE == NULL )
	{
		fprintf(stderr, "Error Opening File for Writing");
		return false;
	}

	// Create and populate RGB buffers
	int i;
	char *Rbuf = new char[Height*Width];
	char *Gbuf = new char[Height*Width];
	char *Bbuf = new char[Height*Width];

	for (i = 0; i < Height*Width; i++)
	{
		Bbuf[i] = Data[3*i];
		Gbuf[i] = Data[3*i+1];
		Rbuf[i] = Data[3*i+2];
	}


	// Write data to file
	for (i = 0; i < Width*Height; i ++)
	{
		fputc(Rbuf[i], OUT_FILE);
	}
	for (i = 0; i < Width*Height; i ++)
	{
		fputc(Gbuf[i], OUT_FILE);
	}
	for (i = 0; i < Width*Height; i ++)
	{
		fputc(Bbuf[i], OUT_FILE);
	}

	// Clean up and return
	delete[] Rbuf;
	delete[] Gbuf;
	delete[] Bbuf;
	fclose(OUT_FILE);

	return true;

}

// Here is where you would place your code to modify an image
// eg Filtering, Transformation, Cropping, etc.
bool MyImage::Modify()
{

	// TO DO by student

	// sample operation
	for ( int i=0; i<Width*Height; i++ )
	{
		Data[3*i] = 0;
		Data[3*i+1] = 0;
	}

	return false;
}

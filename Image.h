#ifndef IMAGE_DISPLAY
#define IMAGE_DISPLAY


// C RunTime Header Files
#include <stdlib.h>
#include <string>
#include <cmath>

// Class structure of Image
// Use to encapsulate an RGB image
class MyImage
{

private:
	int		Width;					// Width of Image
	int		Height;					// Height of Image
	char	ImagePath[4096];	// Image location
	char*	Data;					// RGB data of the image

public:
	// Constructor
	MyImage();
	// Copy Constructor
	MyImage( MyImage *otherImage);
	// Destructor
	~MyImage();

	// operator overload
	MyImage & operator= (const MyImage & otherImage);

	// Reader & Writer functions
	void	setWidth( const int w)  { Width = w; };
	void	setHeight(const int h) { Height = h; };
	void	setImageData( const char *img ) { Data = (char *)img; };
	void	setImagePath( const char *path) { strcpy(ImagePath, path); }
	int		getWidth() { return Width; };
	int		getHeight() { return Height; };
	char*	getImageData() { return Data; };
	char*	getImagePath() { return ImagePath; }

	// Input Output operations
	bool	ReadImage();
	bool	WriteImage();
	bool  CreatImageCanv();
	// Modifications
	bool	Modify();

};

#endif //IMAGE_DISPLAY

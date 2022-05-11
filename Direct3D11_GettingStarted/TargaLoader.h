#pragma once
#include <memory>

class TargaImage;

struct TargaHeader {
	unsigned char miscData[ 12 ];
	unsigned short width;
	unsigned short height;
	unsigned char bitsPerPixel;
	unsigned char descriptor;
};

bool LoadTargaFromFile( const char* filePath, TargaImage* image );

class TargaImage
{
public:
	std::shared_ptr<char[]> getData();
	unsigned short getWidth();
	unsigned short getHeight();
	unsigned short getBitDepth();

	bool isBottomToTop();
	bool isLeftToRight();

protected:
	TargaHeader tHeader;
	std::shared_ptr<char[]> data;

	friend bool LoadTargaFromFile( const char* filePath, TargaImage* image );
};
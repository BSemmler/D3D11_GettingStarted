#include "TargaLoader.h"
#include <fstream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

bool LoadTargaFromFile( const char* filePath, TargaImage* image )
{
	if (!filePath || filePath == "" || !image)
	{
		OutputDebugStringA( "Invalid Path\n" );
		return false;
	}
		
	std::ifstream file( filePath, std::ios::binary );

	if (!file.is_open())
	{
		OutputDebugStringA( "File not found!\n" );
		return false;
	}

	// Get the file header.
	char header[ 18 ];
	file.read( ( char* ) &image->tHeader, sizeof( TargaHeader ) );
	// Write it into our image object.
	if (!header)
	{
		OutputDebugStringA( "Invalid Header\n" );
		return false;
	}

	if (image->tHeader.descriptor & 0x10)
	{
		OutputDebugStringA( "Top to bottom" );
	}

	if (image->tHeader.descriptor & 0x08)
	{
		OutputDebugStringA( "Right to left" );
	}

	int length = image->tHeader.width * image->tHeader.height * 4; // width * height * bytes per pixel
	auto data = std::make_shared<char[]>( length );
	image->data = std::make_shared<char[]>(length);
	file.read( data.get(), length);

	// We now need to reverse the image since targa stores images in an upside down format.
	//file.read( image->data.get(), length );
	
	image->data = data;
	//for 

	return true;
}

std::shared_ptr<char[]> TargaImage::getData()
{
	return data;
}

unsigned short TargaImage::getWidth()
{
	return tHeader.width;
}

unsigned short TargaImage::getHeight()
{
	return tHeader.height;
}

unsigned short TargaImage::getBitDepth()
{
	return tHeader.bitsPerPixel;
}

bool TargaImage::isBottomToTop()
{
	return false;
}

bool TargaImage::isLeftToRight()
{
	return false;
}

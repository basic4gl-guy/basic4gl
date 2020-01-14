//---------------------------------------------------------------------------
/*  Created 13-Mar-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
    Copyright (C) Thomas Mulgrew
    Load and return image. (Uses Corona, and embedded files mechanism.)
*/

#ifndef LoadImageH
#define LoadImageH

#include "Corona.h"
#include "EmbeddedFiles.h"
#include <string>
#include <vector>
#include <assert.h>
#include <windows.h>
#include "glheaders.h"
//---------------------------------------------------------------------------

// InitLoadImage must be called at least once, so that routines can find the
// loader object.
void InitLoadImage (FileOpener *_files);

// LoadImage loads an image and converts it into a useful format
corona::Image *LoadImage (std::string filename);

// Return the image format as an OpenGL constant
inline GLuint ImageFormat (corona::Image *image) {
    assert (image != NULL);
    return (image->getFormat () & 0xffff) == corona::PF_R8G8B8 ? GL_RGB : GL_RGBA;
}

// Scale the image so that the sizes are a power of 2.
corona::Image *ResizeImageForOpenGL (corona::Image *src);

// Break an image up into frames.
// Frame size specified in "frameWidth" and "frameHeight". Frames are considered
// to adjacent and arranged left-to-right, then top-to-bottom.
//
// If frame size is not specified (i.e. 0), frames are assumed to be square, and
// equal to the minimum of the image width and height.
std::vector<corona::Image*> SplitUpImageStrip(
    corona::Image* image,
    int frameWidth = 0,
    int frameHeight = 0);

// Return true if image is completely transparent
bool ImageIsBlank(corona::Image* image);

// Construct a new transparent image by copying the input image and replacing
// the specified colour with transparency.
// Note: The source image is not deleted.
corona::Image* ApplyTransparentColour(corona::Image* image, unsigned long int col);

#endif

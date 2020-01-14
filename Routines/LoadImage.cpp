//---------------------------------------------------------------------------
/*  Created 13-Mar-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
    Copyright (C) Thomas Mulgrew
    Load and return image. (Uses Corona, and embedded files mechanism.)
*/
#pragma hdrstop

#include "LoadImage.h"
#include <assert.h>

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

using namespace corona;
using namespace std;

// File opener
static FileOpener *files = NULL;

void InitLoadImage (FileOpener *_files) {
    assert (_files != NULL);
    files = _files;
}

corona::Image *LoadImage (std::string filename) {
    assert (files != NULL);

    // Load and return an image.
    // Returns NULL if image fails to load.
    // The Corona image lib supports:
    //  BMP
    //  GIF
    //  JPEG
    //  PCX
    //  PNG
    //  TGA

    // We convert all images to either RGB (24 bit) or RGBA (32 bit)

    // Load image
    corona::Image *image = corona::OpenImage (files->FilenameForRead (filename, false).c_str ());
    if (image != NULL) {

        // Flipping appears to be necessary..
        corona::FlipImage (image, corona::CA_X);

        // Convert to either RGB or RGBA
        corona::PixelFormat format = (corona::PixelFormat) ((int) image->getFormat () & 0xffff);
        corona::PixelFormat newFormat;
        switch (format) {
            case corona::PF_R8G8B8:
            case corona::PF_R8G8B8A8:
                newFormat = corona::PF_DONTCARE;
                break;
            case corona::PF_B8G8R8:
            case corona::PF_I8:
                newFormat = corona::PF_R8G8B8;
                break;
            case corona::PF_B8G8R8A8:
                newFormat = corona::PF_R8G8B8A8;
                break;
        }
        if (newFormat != corona::PF_DONTCARE)
            image = corona::ConvertImage (image, newFormat);        // (Note: Convert image destroys the old image...)
    }
    return image;
}

corona::Image *ResizeImageForOpenGL (corona::Image *src) {

    // Find first power of 2 greater than or equal to the current width and height
    int swidth = src->getWidth (), sheight = src->getHeight ();
    char *spixels = (char *) src->getPixels ();
    int width = 1, height = 1;
    while (width < swidth)
        width <<= 1;
    while (height < sheight)
        height <<= 1;

    // If image already has useable dimensions, return it
    if (width == swidth && height == sheight)
        return src;
    int bpp = ImageFormat (src) == GL_RGB ? 3 : 4;        // (BYTES per pixel)

    // Would dimension/2 be closer to the original size?
    if (abs (width - swidth) > abs ((width >> 1) - swidth))
        width >>= 1;
    if (abs (height - sheight) > abs ((height >> 1) - sheight))
        height >>= 1;

    // Allocate destination pixels
    corona::Image *dst = corona::CreateImage (width, height, src->getFormat ());
    char *pixels = (char *) dst->getPixels ();

#ifdef _CAN_INLINE_ASM
    int xdInt, xdFrac, ydInt, ydFrac;
    int xUnit, yUnit;
    __asm {

        pushad

        mov     eax,        [bpp]
        mov     [xUnit],    eax
        mov     eax,        [bpp]
        mul     [swidth]
        mov     [yUnit],    eax

        // Calculate xd
        xor     edx,        edx             // Calculate integer bit
        mov     eax,        [swidth]
        div     [width]
        push    edx                         // Save fraction
        mul     [xUnit]
        mov     [xdInt],    eax
        pop     edx

        xor     eax,        eax             // edx:eax = remainder x 10000h
        div     [width]						// Calculate fraction
        mov     [xdFrac],   eax

        // Calculate yd
        xor     edx,        edx             // Calculate integer bit
        mov     eax,        [sheight]
        div     [height]
        push    edx                         // Save fraction part
        mul     [yUnit]
        mov     [ydInt],    eax
        pop     edx

        xor     eax,        eax             // edx:eax = remainder x 10000h
        div     [height]					// Calculate fraction
        mov     [ydFrac],   eax

        // Setup registers for loop
        mov     esi,        [spixels]       // esi = source
        mov     edi,        [pixels]        // edi = dest
        xor     ebx,        ebx             // ebx = x fraction
        xor     edx,        edx             // edx = y fraction
        mov     ecx,        [height]        // ecx = loop counter
        jecxz   YLoopDone
YLoop:

        // Loop through columns
        push    ecx
        push    esi

        mov     ecx,        [width]
        jecxz   XLoopDone

        // 2 separate inner loops for speed.
        // * 4 bpp version
        // * 3 bpp version
        mov     eax,        3
        cmp     eax,        [bpp]
        jne     XLoop4

XLoop3:
        mov     ax,         [esi]           // Copy 3 bytes from source to dest
        mov     [edi],      ax
        mov     al,         [esi + 2]
        mov     [edi + 2],  al
        add     edi,        3

        // Move across
        add     ebx,        [xdFrac]
        jnc     NoXCarry3
        add     esi,        [xUnit]
NoXCarry3:
        add     esi,        [xdInt]

        // Loop back
        dec     ecx
        jnz     XLoop3
        jmp     XLoopDone

XLoop4:
        mov     eax,        [esi]           // Copy 4 bytes from source to dest
        mov     [edi],      eax
        add     edi,        4

        // Move across
        add     ebx,        [xdFrac]
        jnc     NoXCarry4
        add     esi,        [xUnit]
NoXCarry4:
        add     esi,        [xdInt]

        // Loop back
        dec     ecx
        jnz     XLoop4

XLoopDone:
        pop     esi
        pop     ecx

        // Move down
        add     edx,        [ydFrac]
        jnc     NoYCarry
        add     esi,        [yUnit]
NoYCarry:
        add     esi,        [ydInt]

        // Loop back
        dec     ecx
        jnz     YLoop
YLoopDone:

        popad
    }
#else
    float sx, sy, xd, yd;
    int x, y, offset, soffset;
    xd = (float) swidth / width;
    yd = (float) sheight / height;

    sy = 0;
    offset = 0;
    for (y = 0; y < height; y++) {
        sx = 0;
        soffset = ((int) sy) * swidth * bpp;
        for (x = 0; x < width; x++) {
            int sofs = soffset + ((int) sx) * bpp;
            pixels [offset    ] = spixels [sofs    ];
            pixels [offset + 1] = spixels [sofs + 1];
            pixels [offset + 2] = spixels [sofs + 2];
            if (bpp == 4)
                pixels [offset + 3] = spixels [sofs + 3];
            offset += bpp;
            sx += xd;
        }
        sy += yd;
    }
#endif

    // Return new image
    delete src;
    return dst;
}

vector<Image*> SplitUpImageStrip(
        Image* image,
        int frameWidth,
        int frameHeight) {
    assert(image != NULL);

    if (frameWidth <= 0 || frameHeight <= 0) {

        // Default to square image.
        if (image->getWidth() < image->getHeight())
            frameWidth = image->getWidth();
        else
            frameWidth = image->getHeight();
        frameHeight = frameWidth;
    }

    if (frameWidth > image->getWidth())
        frameWidth = image->getWidth();
    if (frameHeight > image->getHeight())
        frameHeight = image->getHeight();

    // Extract images
    vector<Image*> images;
    int bpp = GetPixelSize(image->getFormat());
    for (int y = image->getHeight() - frameHeight; y >= 0; y -= frameHeight) {
        for (int x = 0; x + frameWidth <= image->getWidth(); x += frameWidth) {

            // Create frame image
            Image* dst = CreateImage(frameWidth, frameHeight, image->getFormat());
            images.push_back(dst);

            // Copy pixels row by row
            char* srcPixels = (char*)image->getPixels();
            char* dstPixels = (char*)dst->getPixels();
            for (int dy = 0; dy < frameHeight; dy++)
                memcpy(
                    dstPixels + dy * frameWidth * bpp,
                    srcPixels + ((y + dy) * image->getWidth() + x) * bpp,
                    frameWidth * bpp);
        }
    }

    return images;
}

bool ImageIsBlank(Image* image) {
    assert(image != NULL);

    // Format must include an alpha channel
    switch (image->getFormat() & 0xffff) {
        case PF_R8G8B8A8:
        case PF_B8G8R8A8:
            break;
        default:
            return false;
    }

    // Search for non-transparent pixel
    char* pixels = (char*) image->getPixels();
    for (int i = 0; i < image->getWidth() * image->getHeight(); i++)
        if (pixels[i * 4 + 3] != 0)
            return false;

    return true;
}

Image* ApplyTransparentColour(Image* image, unsigned long int col) {
    assert(image != NULL);

    // Clone image and convert to RGBA
    Image* dst = CloneImage(image, PF_R8G8B8A8);
    if (dst == NULL)
        return image;           // Unable to convert. Just return original image

    // Convert pixels to transparent
    unsigned long int* pixels = (unsigned long int*)dst->getPixels();
    unsigned int count = dst->getWidth() * dst->getHeight();
    col = col & 0x00ffffff;                     // Mask out alpha channel
    for (int i = 0; i < count; i++)
        if ((pixels[i] & 0x00ffffff) == col)    // We mask out the alpha before comparing
            pixels[i] = 0;

    delete image;
    return dst;
}


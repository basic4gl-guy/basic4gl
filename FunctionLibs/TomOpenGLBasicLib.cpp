//---------------------------------------------------------------------------
// Created 27-Sep-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
#pragma hdrstop

#include "TomOpenGLBasicLib.h"
#include "Corona.h"
#include "LoadImage.h"

//---------------------------------------------------------------------------
#ifndef _MSC_VER
#pragma package(smart_init)
#endif
#include "vmData.h"
#include "TomLibRoutines.h"
#include "Basic4GLOpenGLObjects.h"
#include "glheaders.h"

using namespace corona;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Resources

// TextureResourceStore
//
// Stores OpenGL texture handles
class TextureResourceStore : public vmIntHandleResources {
protected:
    virtual void DeleteHandle (int handle);
};

void TextureResourceStore::DeleteHandle (int handle) {
    GLuint texture = (GLuint) handle;
    glDeleteTextures (1, &texture);
}

// ImageResourceStore
//
// Stores pointers to Corona image objects
typedef vmPointerResourceStore<corona::Image> ImageResourceStore;

// DisplayListResourceStore
//
// Stores OpenGL display lists handles
class DisplayListResourceStore : public vmIntHandleResources {
private:
    std::map<int,int> m_countMap;                               // Maps base to count
protected:
    virtual void DeleteHandle (int handle);
public:
    virtual void Clear ();
    void Store (int handle, int count)  {
        if (!Valid (handle) || m_countMap [handle] < count) {   // Not already stored, or new value covers a bigger range
            vmIntHandleResources::Store (handle);
            m_countMap [handle] = count;
        }
    }
    int GetCount (int base) {
        assert (Valid (base));
        return m_countMap [base];
    }
};

void DisplayListResourceStore::Clear () {
    vmIntHandleResources::Clear ();
    m_countMap.clear ();
}

void DisplayListResourceStore::DeleteHandle (int handle) {
    glDeleteLists (handle, m_countMap [handle]);
}

// Globals
OpenGLWindowManager*		windowManager;
TextureResourceStore        textures;
ImageResourceStore          images;
DisplayListResourceStore    displayLists;

// Global state
bool truncateBlankFrames;           // Truncate blank frames from image strips
bool usingTransparentCol;           // Use transparent colour when loading images
unsigned long int transparentCol;   // Transparent colour as RGB triplet
bool doMipmap;                      // Build mipmap textures when loading images
bool doLinearFilter;                // Use linear filtering on textures (otherwise use nearest)

////////////////////////////////////////////////////////////////////////////////
// Pre-run initialisation
static void Init (TomVM& vm) {
    //appWindow->ClearKeyBuffers ();
    textures.Clear ();

    // Set texture loading state behaviour defaults
    truncateBlankFrames = true;
    usingTransparentCol = false;
    doMipmap = true;
    doLinearFilter = true;
}

////////////////////////////////////////////////////////////////////////////////
// Helper functions

GLuint UploadTexture(Image* image) {
    assert(image != NULL);

    // Generate texture
    GLuint texture;
    glGenTextures (1, &texture);
    glBindTexture (GL_TEXTURE_2D, texture);

    int width = image->getWidth();
    int height = image->getHeight();
    int pixelSize = GetPixelSize(image->getFormat());
    int format = ImageFormat(image);
    char* pixels = (char*)image->getPixels();

    if (doMipmap) {
        // Set filtering for mipmap
        if (doLinearFilter) {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
        else {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        }

        // Build mipmaps
        gluBuild2DMipmaps ( GL_TEXTURE_2D,
                            pixelSize,
                            width,
                            height,
                            format,
                            GL_UNSIGNED_BYTE,
                            pixels);
    }
    else {
        // Set filtering for texture
        if (doLinearFilter) {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        else {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }

        // Upload texture
        glTexImage2D (      GL_TEXTURE_2D,
                            0,
                            pixelSize,
                            width,
                            height,
                            0,
                            format,
                            GL_UNSIGNED_BYTE,
                            pixels);
    }

    return texture;
}

GLuint LoadTex(string filename) {

    // Load image
    Image* image = LoadImage(filename);
    if (image != NULL) {

        // Process image
        if (usingTransparentCol)
            image = ApplyTransparentColour(image, transparentCol);
        image = ResizeImageForOpenGL(image);

        // Upload into texture
        GLuint texture = UploadTexture(image);
        delete image;

        return texture;
    }
    else
        return 0;
}

vector<Image*> LoadTexStripImages(string filename, int frameXSize, int frameYSize) {

    // Load main image
    Image* image = LoadImage(filename);
    if (image != NULL) {

        // Split into frames
        vector<Image*> images = SplitUpImageStrip(image, frameXSize, frameYSize);

        // Process images
        if (usingTransparentCol)
            for (   vector<Image*>::iterator i = images.begin();
                    i != images.end();
                    i++)
                *i = ApplyTransparentColour(*i, transparentCol);

        if (truncateBlankFrames) {
            while(images.size() > 1 && ImageIsBlank(images.back())) {
                delete images.back();
                images.pop_back();
            }
        }

        delete image;
        return images;
    }
    else
        return vector<Image*>();
}

void DeleteImages(vector<Image*> images) {
    for (   vector<Image*>::iterator i = images.begin();
            i != images.end();
            i++)
        delete *i;
}

int TexStripFrames(string filename, int frameXSize = 0, int frameYSize = 0) {

    // Load image
    vector<Image*> images = LoadTexStripImages(filename, frameXSize, frameYSize);

    // Count frames
    int result = images.size();
    
    DeleteImages(images);
    return result;
}

vector<GLuint> LoadTexStrip(string filename, int frameXSize = 0, int frameYSize = 0) {

    // Load images
    vector<Image*> images = LoadTexStripImages(filename, frameXSize, frameYSize);

    // Upload into textures
    vector<GLuint> textures;
    for (   vector<Image*>::iterator i = images.begin();
            i != images.end();
            i++) {
        *i = ResizeImageForOpenGL(*i);
        textures.push_back(UploadTexture(*i));
    }

    DeleteImages(images);
    return textures;
}

////////////////////////////////////////////////////////////////////////////////
// Function wrappers

void WrapLoadTex(TomVM& vm) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
        GLuint texture = LoadTex(vm.GetStringParam(1));
        textures.Store(texture);
        vm.Reg().IntVal() = texture;
    glPopAttrib();
}

void WrapLoadTexStrip(TomVM& vm) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
        vector<GLuint> texs = LoadTexStrip(vm.GetStringParam(1));

        // Convert to array and return
        if (texs.size() > 0) {
            int* array = new int[texs.size()];
            for (int i = 0; i < texs.size(); i++) {
                array[i] = texs[i];
                textures.Store(texs[i]);
            }
            vm.Reg().IntVal() = FillTempIntArray(vm.Data(), vm.DataTypes(), texs.size(), array);
            delete[] array;
        }
        else {
            int array[1];
            array[0] = 0;
            vm.Reg().IntVal() = FillTempIntArray(vm.Data(), vm.DataTypes(), 1, array);
        }
    glPopAttrib();
}

void WrapLoadTexStrip2(TomVM& vm) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
        vector<GLuint> texs = LoadTexStrip(vm.GetStringParam(3), vm.GetIntParam(2), vm.GetIntParam(1));

        // Convert to array and return
        if (texs.size() > 0) {
            int* array = new int[texs.size()];
            for (int i = 0; i < texs.size(); i++) {
                array[i] = texs[i];
                textures.Store(texs[i]);
            }
            vm.Reg().IntVal() = FillTempIntArray(vm.Data(), vm.DataTypes(), texs.size(), array);
            delete[] array;
        }
        else {
            int array[1];
            array[0] = 0;
            vm.Reg().IntVal() = FillTempIntArray(vm.Data(), vm.DataTypes(), 1, array);
        }
    glPopAttrib();
}

void WrapTexStripFrames(TomVM& vm) {
    vm.Reg().IntVal() = TexStripFrames(vm.GetStringParam(1));
}

void WrapTexStripFrames2(TomVM& vm) {
    vm.Reg().IntVal() = TexStripFrames(vm.GetStringParam(3), vm.GetIntParam(2), vm.GetIntParam(1));
}

void WrapSetTexIgnoreBlankFrames(TomVM& vm) {
    truncateBlankFrames = vm.GetIntParam(1) != 0;
}

void WrapSetTexTransparentCol(TomVM& vm) {
    transparentCol = vm.GetIntParam(1);
    usingTransparentCol = true;
}

void WrapSetTexTransparentCol2(TomVM& vm) {
    transparentCol =
        (vm.GetIntParam(3) & 0xff) |
        ((vm.GetIntParam(2) & 0xff) << 8) |
        ((vm.GetIntParam(1) & 0xff) << 16); 
    usingTransparentCol = true;
}

void WrapSetTexNoTransparentCol(TomVM& vm) {
    usingTransparentCol = false;
}

void WrapSetTexMipmap(TomVM& vm) {
    doMipmap = vm.GetIntParam(1) != 0;
}

void WrapSetTexLinearFilter(TomVM& vm) {
    doLinearFilter = vm.GetIntParam(1) != 0;
}

void WrapglGenTexture(TomVM& vm) {
    GLuint texture;
    glGenTextures (1, &texture);
    textures.Store (texture);
    vm.Reg ().IntVal () = texture;
}
void WrapglDeleteTexture(TomVM& vm) {
    textures.Free (vm.GetIntParam (1));
}
GLuint LoadTexture (std::string filename, bool mipmap) {

    // Load texture
    GLuint result = 0;
    glPushAttrib (GL_ALL_ATTRIB_BITS);

    // Generate and load image
    corona::Image *image = LoadImage (filename);
    if (image != NULL) {
        image = ResizeImageForOpenGL (image);

        // Generate texture
        GLuint texture;
        glGenTextures (1, &texture);
        glBindTexture (GL_TEXTURE_2D, texture);

        // Build mipmaps
        if (mipmap) {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            gluBuild2DMipmaps ( GL_TEXTURE_2D,
                                (image->getFormat () & 0xffff) == corona::PF_R8G8B8 ? 3 : 4,
                                image->getWidth (),
                                image->getHeight (),
                                ImageFormat (image),
                                GL_UNSIGNED_BYTE,
                                image->getPixels ());
        }
        else {
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D (      GL_TEXTURE_2D,
                                0,
                                (image->getFormat () & 0xffff) == corona::PF_R8G8B8 ? 3 : 4,
                                image->getWidth (),
                                image->getHeight (),
                                0,
                                ImageFormat (image),
                                GL_UNSIGNED_BYTE,
                                image->getPixels ());
        }

        // Store and return texture
        textures.Store (texture);
        result = texture;

        // Clean up image
        delete image;
    }

    // Clean up OpenGL state
    glPopAttrib ();
    return result;
}
void WrapLoadTexture(TomVM& vm) {

    // Load and return non-mipmapped texture
    vm.Reg ().IntVal () = LoadTexture (vm.GetStringParam (1), false);
}
void WrapLoadMipmapTexture(TomVM& vm) {

    // Load and return mipmapped texture
    vm.Reg ().IntVal () = LoadTexture (vm.GetStringParam (1), true);
}
void WrapLoadImage(TomVM& vm) {

    // Attempt to load image
    corona::Image *image = LoadImage (vm.GetStringParam (1));

    // If successful, store it and return handle
    vm.Reg ().IntVal () = (image != NULL) ? images.Alloc (image) : 0;
}
void WrapDeleteImage(TomVM& vm) {
    images.Free (vm.GetIntParam (1));
}
void WrapglTexImage2D(TomVM& vm) {

    // Find image data
    int index = vm.GetIntParam (1);
    if (images.IndexStored (index)) {
        void *pixels = images.Value (index)->getPixels ();

        // Generate image
        glTexImage2D (  vm.GetIntParam (9),         // target
                        vm.GetIntParam (8),         // level
                        vm.GetIntParam (7),         // components
                        vm.GetIntParam (6),         // width
                        vm.GetIntParam (5),         // height
                        vm.GetIntParam (4),         // border
                        vm.GetIntParam (3),         // format
                        vm.GetIntParam (2),         // type
                        pixels);                    // pixels
    }
}

#define MAXIMAGESIZE (2048*2048)
void InternalWrapglTexImage2D(TomVM& vm, vmBasicValType elementType, int dimensions, bool mipmap) {
    // Find array param, and extract dimensions
    int arrayOffset = vm.GetIntParam (1);
    int maxSize;
    if (dimensions == 2) {
        int xSize = ArrayDimensionSize (vm.Data (), arrayOffset, 0),
            ySize = ArrayDimensionSize (vm.Data (), arrayOffset, 1);

        // Verify size
        if (xSize <= 0 || ySize <= 0) {
            vm.FunctionError ("Bad array size");
            return;
        }
        if (ySize > (MAXIMAGESIZE / xSize)) {
            vm.FunctionError ("Cannot upload arrays greater than 2048 x 2048 into images");
            return;
        }
        maxSize = xSize * ySize;
    }
    else {
        assert (dimensions == 1);
        int size = ArrayDimensionSize (vm.Data (), arrayOffset, 0);
        if (size <= 0) {
            vm.FunctionError ("Bad array size");
            return;
        }
        if (size > MAXIMAGESIZE) {
            vm.FunctionError ("Cannot upload arrays greater than 2048 x 2048 into images");
            return;
        }
        maxSize = size;
    }

    // Read data, converted requested type
    int type = vm.GetIntParam (2);
    unsigned char *data = new unsigned char [maxSize * 4];
    ReadArrayDynamic (vm, 1, vmValType(elementType,dimensions,1,true), type, data, maxSize);

    // Generate image
    if (mipmap) {
        gluBuild2DMipmaps (     vm.GetIntParam (7),
                                vm.GetIntParam (6),
                                vm.GetIntParam (5),
                                vm.GetIntParam (4),
                                vm.GetIntParam (3),
                                type,
                                data);
    }
    else {
        glTexImage2D (  vm.GetIntParam (9),
                        vm.GetIntParam (8),
                        vm.GetIntParam (7),
                        vm.GetIntParam (6),
                        vm.GetIntParam (5),
                        vm.GetIntParam (4),
                        vm.GetIntParam (3),
                        type,
                        data);
    }

    // Free temporary buffer
    delete[] data;
}
void WrapglTexImage2D_2(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_INT, 1, false);
}
void WrapglTexImage2D_3(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_REAL, 1, false);
}
void WrapglTexImage2D_4(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_INT, 2, false);
}
void WrapglTexImage2D_5(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_REAL, 2, false);
}
void WrapgluBuild2DMipmaps_2(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_INT, 1, true);
}
void WrapgluBuild2DMipmaps_3(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_REAL, 1, true);
}
void WrapgluBuild2DMipmaps_4(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_INT, 2, true);
}
void WrapgluBuild2DMipmaps_5(TomVM& vm) {
    InternalWrapglTexImage2D (vm, VTP_REAL, 2, true);
}
void WrapglTexSubImage2D(TomVM& vm) {

    // Find image data
    int index = vm.GetIntParam (1);
    if (images.IndexStored (index)) {
        void *pixels = images.Value (index)->getPixels ();

        // Generate image
        glTexSubImage2D (   vm.GetIntParam (9),     // target
                            vm.GetIntParam (8),     // level
                            vm.GetIntParam (7),     // xoffset
                            vm.GetIntParam (6),     // y offset
                            vm.GetIntParam (5),     // width
                            vm.GetIntParam (4),     // height
                            vm.GetIntParam (3),     // format
                            vm.GetIntParam (2),     // type
                            pixels);                // pixels
    }
}
void WrapImageWidth(TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () = images.IndexStored (index) ? images.Value (index)->getWidth () : 0;
}
void WrapImageHeight(TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () = images.IndexStored (index) ? images.Value (index)->getHeight () : 0;
}
void WrapImageFormat(TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () = images.IndexStored (index) ? ImageFormat (images.Value (index)) : 0;
}
void WrapImageDataType(TomVM& vm) {
    vm.Reg ().IntVal () = GL_UNSIGNED_BYTE;             // Images always stored as unsigned bytes
}
void WrapgluBuild2DMipmaps(TomVM& vm) {

    // Find image data
    int index = vm.GetIntParam (1);
    if (images.IndexStored (index)) {
        void *pixels = images.Value (index)->getPixels ();

        // Build 2D mipmaps
        gluBuild2DMipmaps ( vm.GetIntParam (7),         // target
                            vm.GetIntParam (6),         // components
                            vm.GetIntParam (5),         // width
                            vm.GetIntParam (4),         // height
                            vm.GetIntParam (3),         // format
                            vm.GetIntParam (2),         // type
                            pixels);                    // pixels
    }
}
void WrapglMultiTexCoord2f(TomVM& vm) {
	if (glMultiTexCoord2fARB != NULL)
		glMultiTexCoord2fARB(vm.GetIntParam(3), vm.GetRealParam(2), vm.GetRealParam(1));
}
void WrapglMultiTexCoord2d(TomVM& vm) {
	if (glMultiTexCoord2dARB != NULL)
		glMultiTexCoord2dARB(vm.GetIntParam(3), vm.GetRealParam(2), vm.GetRealParam(1));
}
void WrapglActiveTexture(TomVM& vm) {
	if (glActiveTextureARB != NULL)
		glActiveTextureARB(vm.GetIntParam(1));
}
void WrapglGetString(TomVM& vm) {
    vm.RegString () = (char *) glGetString (vm.GetIntParam (1));
}
bool IsExtensionSupported(std::string ext)
{
	// Get all extensions
	auto allExtsBuf = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
	if (allExtsBuf == nullptr) return false;				// 0 indicates OpenGL error
	auto allExts = std::string(allExtsBuf);

	// Search for extension in list.
	// Use basic trick of adding a space before and after each.
	ext = " " + ext + " ";
	allExts = " " + allExts + " ";
	return ext.find(ext) != std::string::npos;
}
void WrapExtensionSupported(TomVM& vm) {
	auto ext = vm.GetStringParam(1);
	vm.Reg().IntVal() = IsExtensionSupported(ext) ? -1 : 0;
}
void WrapMaxTextureUnits(TomVM& vm) {
    GLint units;
    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &units);
    vm.Reg ().IntVal () = units;
}
void WrapWindowWidth(TomVM& vm) {
    vm.Reg().IntVal() = windowManager->GetPendingWindowWidth();
}
void WrapWindowHeight(TomVM& vm) {
	vm.Reg().IntVal() = windowManager->GetPendingWindowHeight();
}
void WrapglGenTextures (TomVM& vm) {
    int count = vm.GetIntParam (2);
    if (count > 65536) {
        vm.FunctionError ((std::string) "Count must be 0 - 65536 (Basic4GL restriction)");
        return;
    }
    if (count <= 0)
        return;

    // Generate some texture handles
    GLuint handles [65536];            // 64K should be enough for anybody ;)
    glGenTextures (count, handles);

    // Store textures in resouce store (so Basic4GL can track them and ensure they have been deallocated)
    for (int i = 0; i < count; i++)
        textures.Store (handles [i]);

    // Store handles in Basic4GL array
    WriteArray (vm.Data (), vm.GetIntParam (1), vmValType (VTP_INT, 1), handles, count);
}
void WrapglDeleteTextures (TomVM& vm) {
    int count = vm.GetIntParam (2);
    if (count > 65536) {
        vm.FunctionError ((std::string) "Count must be 0 - 65536 (Basic4GL restriction)");
        return;
    }
    if (count <= 0)
        return;

    // Read texture handles
    GLuint handles [65536];            // 64K should be enough for anybody ;)
    ReadAndZero (vm.Data (), vm.GetIntParam (1), vmValType (VTP_INT, 1), handles, count);

    // Delete texture handles
    glDeleteTextures (count, handles);
}

void WrapglLoadMatrixd(TomVM& vm) {
    GLdouble a1[16];
    ReadAndZero(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
    glLoadMatrixd(a1);WriteArray(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
}
void WrapglLoadMatrixf(TomVM& vm) {
    GLfloat a1[16];
    ReadAndZero(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
    glLoadMatrixf(a1);WriteArray(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
}
void WrapglMultMatrixd(TomVM& vm) {
    GLdouble a1[16];
    ReadAndZero(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
    glMultMatrixd(a1);WriteArray(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
}
void WrapglMultMatrixf(TomVM& vm) {
    GLfloat a1[16];
    ReadAndZero(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
    glMultMatrixf(a1);WriteArray(vm.Data(),vm.GetIntParam(1),vmValType(VTP_REAL,2,1,true),a1, 16);
}
void WrapglGetPolygonStipple(TomVM& vm) {
    GLubyte mask [128];
    glGetPolygonStipple (mask);
    WriteArray (vm.Data (), vm.GetIntParam(1), vmValType(VTP_INT,1,1,true), mask, 128);
}
void WrapglPolygonStipple(TomVM& vm) {
    GLubyte mask [128];
    ReadAndZero (vm.Data (), vm.GetIntParam(1), vmValType(VTP_INT,1,1,true), mask, 128);
    glPolygonStipple (mask);
}
void WrapglGenLists(TomVM& vm) {

    // Validate params
    int count = vm.GetIntParam (1);

    // Get display lists
    int base = glGenLists (count);

    // Track display lists, so Basic4GL can delete them if necessary
    if (base != GL_INVALID_VALUE || base != GL_INVALID_OPERATION)
        displayLists.Store (base, count);

    // Return result
    vm.Reg ().IntVal () = base;
}
void WrapglDeleteLists(TomVM& vm) {

    // Get params
    int base    = vm.GetIntParam (2),
        count   = vm.GetIntParam (1);

    // Delete display lists
    glDeleteLists (base, count);

    // Remove display lists entry (if the range was correctly deleted)
    if (displayLists.Valid (base) && displayLists.GetCount (base) <= count)
        displayLists.Remove (base);
}
void WrapglCallLists(TomVM& vm) {

    // VTP_REAL array version

    // Get size and type params
    int n = vm.GetIntParam (3), type = vm.GetIntParam (2);
    if (n > 65536) {
        vm.FunctionError ((std::string) "Count must be 0 - 65536 (Basic4GL restriction)");
        return;
    }
    if (n <= 0)
        return;

    // Get array parameter
    char array [65536 * 4];                     
    if (ReadArrayDynamic (vm, 1, vmValType(VTP_REAL,1,1,true), type, (void *) array, n) == 0)
        return;

    // Call glCallLists
    glCallLists (n, type, (void *) array);
}

void WrapglCallLists_2(TomVM& vm) {

    // VTP_INT array version

    // Get size and type params
    int n = vm.GetIntParam (3), type = vm.GetIntParam (2);
    if (n > 65536) {
        vm.FunctionError ((std::string) "Count must be 0 - 65536 (Basic4GL restriction)");
        return;
    }
    if (n <= 0)
        return;

    // Get array parameter
    char array [65536 * 4];
    if (ReadArrayDynamic (vm, 1, vmValType(VTP_INT,1,1,true), type, (void *) array, n) == 0)
        return;

    // Call glCallLists
    glCallLists (n, type, (void *) array);
}
void WrapglBegin(TomVM& vm) {
    //appWindow->SetDontPaint (true);             // Dont paint on WM_PAINT messages when between a glBegin() and a glEnd ()
    glBegin(vm.GetIntParam (1));                // This doesn't effect running code, but helps when stepping through calls in the debugger
}
void WrapglEnd(TomVM& vm) {
    glEnd();
    //appWindow->SetDontPaint (false);
}

std::string FileExt (std::string filename) {

    // Find last dot
    int extPos = filename.find_last_of ('.');
    if (extPos != std::string::npos) {

        // Extract extension characters
        std::string ext = filename.substr (extPos + 1, 100);

        // Convert to lower case
        for (int i = 0; i < ext.length(); i++)
            ext [i] = tolower (ext [i]);

        return ext;
    }
    else
        return "";
}
bool IsPowerOf2 (int value) {
    if (value <= 0)
        return false;
    while ((value & 1) == 0)
        value >>= 1;
    return value == 1;
}
void CopyPixels (   char *src,                      // Source image
                    unsigned int srcWidth,          // Image size
                    unsigned int srcHeight,
                    unsigned int srcX,              // Offset in image
                    unsigned int srcY,
                    char *dst,                      // Destination image
                    unsigned int dstWidth,          // And size
                    unsigned int dstHeight,
                    unsigned int bytesPerPixel) {   // (Source and dest)
    assert (src != NULL);
    assert (dst != NULL);
    assert (srcWidth >= srcX + dstWidth);
    assert (srcHeight >= srcY + dstHeight);

    // Copy image data line by line
    unsigned int y;
    for (y = 0; y < dstHeight; y++)
        memcpy (    dst + y * dstWidth * bytesPerPixel,
                    src + ((y + srcY) * srcWidth + srcX) * bytesPerPixel,
                    dstWidth * bytesPerPixel);
}
void WrapglGetFloatv_2D (TomVM& vm) {
    GLfloat data [16];
    memset (data, 0, 16 * sizeof (data [0]));
    glGetFloatv (vm.GetIntParam (2), data);
    WriteArray (vm.Data (), vm.GetIntParam (1), vmValType (VTP_REAL, 2, 1, true), data, 16);
}
void WrapglGetDoublev_2D (TomVM& vm) {
    GLdouble data [16];
    memset (data, 0, 16 * sizeof (data [0]));
    glGetDoublev (vm.GetIntParam (2), data);
    WriteArray (vm.Data (), vm.GetIntParam (1), vmValType (VTP_REAL, 2, 1, true), data, 16);
}
void WrapglGetIntegerv_2D (TomVM& vm) {
    GLint data [16];
    memset (data, 0, 16 * sizeof (data [0]));
    glGetIntegerv (vm.GetIntParam (2), data);
    WriteArray (vm.Data (), vm.GetIntParam (1), vmValType (VTP_INT, 2, 1, true), data, 16);
}
void WrapglGetBooleanv_2D (TomVM& vm) {
    GLboolean data [16];
    memset (data, 0, 16 * sizeof (data [0]));
    glGetBooleanv (vm.GetIntParam (2), data);
    WriteArray (vm.Data (), vm.GetIntParam (1), vmValType (VTP_INT, 2, 1, true), data, 16);
}

void WrapSetWindowWidth(TomVM& vm) {
    int value = vm.GetIntParam(1);
    if (value >= 1 && value <= 4096)
        windowManager->pendingParams.width = value;
}

void WrapSetWindowHeight(TomVM& vm) {
    int value = vm.GetIntParam(1);
    if (value >= 1 && value <= 4096)
        windowManager->pendingParams.height = value;
}

void WrapSetWindowFullscreen(TomVM& vm) {
	windowManager->pendingParams.isFullscreen = vm.GetIntParam(1) != 0;
}

void WrapSetWindowBorder(TomVM& vm) {
	windowManager->pendingParams.isBordered = vm.GetIntParam(1) != 0;
}

void WrapSetWindowBpp(TomVM& vm) {
    int value = vm.GetIntParam(1);
    if (value >= 1 && value <= 32)
        windowManager->pendingParams.bpp = value;
}

void WrapSetWindowStencil(TomVM& vm) {
    windowManager->pendingParams.isStencilBufferRequired = vm.GetIntParam(1) != 0;
}

void WrapSetWindowTitle(TomVM& vm) {
    windowManager->pendingParams.title = vm.GetStringParam(1);
}

void WrapSetWindowResizable(TomVM& vm) {
	windowManager->pendingParams.isResizable = vm.GetIntParam(1) != 0;
}

void WrapWindowFullscreen(TomVM& vm) {
    vm.Reg().IntVal() = windowManager->pendingParams.isFullscreen ? -1 : 0;
}

void WrapWindowBorder(TomVM& vm) {
	vm.Reg().IntVal() = windowManager->pendingParams.isBordered ? -1 : 0;
}

void WrapWindowBpp(TomVM& vm) {
	vm.Reg().IntVal() = windowManager->pendingParams.bpp;
}

void WrapWindowStencil(TomVM& vm) {
	vm.Reg().IntVal() = windowManager->pendingParams.isStencilBufferRequired ? -1 : 0;
}

void WrapWindowTitle(TomVM& vm) {
	vm.RegString() = windowManager->pendingParams.title;
}

void WrapWindowResizable(TomVM& vm) {
	vm.Reg().IntVal() = windowManager->pendingParams.isResizable ? -1 : 0;
}

/// Apply changes from SetWindowWidth/-Height/Visible etc calls.
void WrapUpdateWindow(TomVM& vm) {
	windowManager->RecreateWindow();
	vm.Reg().IntVal() = windowManager->Error() ? 0 : -1;
}

void WrapSwapBuffers(TomVM& vm)    {
	windowManager->SwapBuffers();
}

////////////////////////////////////////////////////////////////////////////////
// Interface for DLLs

class WindowAdapter : public IB4GLOpenGLWindow {
public:
    virtual int DLLFUNC Width();
    virtual int DLLFUNC Height();
    virtual int DLLFUNC BPP();
    virtual bool DLLFUNC Fullscreen();
    virtual void DLLFUNC SwapBuffers();
    virtual const char *DLLFUNC Title();
} windowAdapter;

int DLLFUNC WindowAdapter::Width() {
	return windowManager->GetWindowWidth();
}
int DLLFUNC WindowAdapter::Height() {
	return windowManager->GetWindowHeight();
}
int DLLFUNC WindowAdapter::BPP() {
	return windowManager->GetActiveParams().bpp;
}
bool DLLFUNC WindowAdapter::Fullscreen() {
	return windowManager->GetActiveParams().isFullscreen;
}
void DLLFUNC WindowAdapter::SwapBuffers() {
	windowManager->SwapBuffers();
}
const char *DLLFUNC WindowAdapter::Title() {
	return windowManager->GetActiveParams().title.c_str();
}

//------------------------------------------------------------------------------
// New image strip loading routines
void CalcImageStripFrames(
        corona::Image *image,
        int frameWidth,
        int frameHeight,
        int& frames,
        int& width,
        int& height) {

    // Return the # of frames in the image
    assert(image != NULL);
    
    width = image->getWidth();
    height = image->getHeight();
    frames = (width / frameWidth) * (height / frameHeight);
}

bool CheckFrameSize(int frameSize) {
   return frameSize >= 1 && frameSize <= 1024 && IsPowerOf2 (frameSize);
}

int ImageStripFrames(
        TomVM& vm,
        std::string filename,
        int frameWidth,
        int frameHeight) {

    if (!CheckFrameSize(frameWidth)) {
        vm.FunctionError ("Frame width must be a power of 2 from 1-1024");
        return 0;
    }
    if (!CheckFrameSize(frameHeight)) {
        vm.FunctionError ("Frame height must be a power of 2 from 1-1024");
        return 0;
    }

    int result = 0, width = 0, height = 0;
    corona::Image *image = LoadImage (filename);
    if (image != NULL) {
        CalcImageStripFrames(image, frameWidth, frameHeight, result, width, height);
        delete image;
    }
    return result;
}
void LoadImageStrip (
        TomVM& vm,
        std::string filename,
        int frameWidth,
        int frameHeight,
        bool mipmap) {

    if (!CheckFrameSize(frameWidth)) {
        vm.FunctionError ("Frame width must be a power of 2 from 1-1024");
        return;
    }
    if (!CheckFrameSize(frameHeight)) {
        vm.FunctionError ("Frame height must be a power of 2 from 1-1024");
        return;
    }

    // Load image strip
    corona::Image *image = LoadImage (filename);
    if (image != NULL) {
        int frameCount = 0, width = 0, height = 0;
        CalcImageStripFrames (image, frameWidth, frameHeight, frameCount, width, height);
        if (frameCount > 65536) {
            vm.FunctionError ("Cannot load more than 65536 images in an image strip");
            return;
        }
        if (frameCount > 0) {

            // Generate some OpenGL textures
            GLuint tex [65536];
            glGenTextures (frameCount, tex);

            // Store texture handles in texture store object so Basic4GL can track them
            for (int i = 0; i < frameCount; i++)
                textures.Store (tex [i]);

            // Iterate over image in grid pattern, extracting each frame
            int frame = 0;
            char    *buffer         = new char [frameWidth * frameHeight * 4];
            int     bytesPerPixel   = (image->getFormat () & 0xffff) == corona::PF_R8G8B8 ? 3 : 4;
            GLuint  format          = ImageFormat (image);
            for (int y = 0; y < height / frameHeight; y++) {
                for (int x = 0; x < width / frameWidth; x++) {

                    // Extract block of pixels
                    CopyPixels (    (char *) image->getPixels (),
                                    image->getWidth (),
                                    image->getHeight (),
                                    x * frameWidth,
                                    height - (y + 1) * frameHeight,
                                    buffer,
                                    frameWidth,
                                    frameHeight,
                                    bytesPerPixel);

                    // Upload texture
                    glBindTexture (GL_TEXTURE_2D, tex [frame]);
                    if (mipmap) {
                        gluBuild2DMipmaps ( GL_TEXTURE_2D,
                                            bytesPerPixel,
                                            frameWidth,
                                            frameHeight,
                                            format,
                                            GL_UNSIGNED_BYTE,
                                            buffer);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                    }
                    else {
                        glTexImage2D (      GL_TEXTURE_2D,
                                            0,
                                            bytesPerPixel,
                                            frameWidth,
                                            frameHeight,
                                            0,
                                            format,
                                            GL_UNSIGNED_BYTE,
                                            buffer);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                    // Increase frame counter
                    frame++;
                }
            }
            delete[] buffer;

            // Return array of textures
            delete image;
            vm.Reg ().IntVal () = FillTempIntArray (vm.Data (), vm.DataTypes (), frameCount, tex);
            return;
        }
        delete image;
    }

    // Load failed.
    // Return 1 element array containing a 0.
    GLuint blankFrame = 0;
    vm.Reg ().IntVal () = FillTempIntArray (vm.Data (), vm.DataTypes (), 1, &blankFrame);
}
void WrapLoadImageStrip(TomVM& vm) {
    LoadImageStrip(vm, vm.GetStringParam(3), vm.GetIntParam(2), vm.GetIntParam(1), false);
}
void WrapLoadMipmapImageStrip(TomVM& vm) {
    LoadImageStrip(vm, vm.GetStringParam(3), vm.GetIntParam(2), vm.GetIntParam(1), true);
}
void WrapImageStripFrames(TomVM& vm) {
    vm.Reg().IntVal() = ImageStripFrames(vm, vm.GetStringParam(3), vm.GetIntParam(2), vm.GetIntParam(1));
}

//------------------------------------------------------------------------------
// Old square image strip routines.
// These ones take only a single frame size param and assume the frame is square.
// I'm not too sure of the logic behind these original versions, but I'm keeping
// them for backwards compatibility.
void OldSquare_CalcImageStripFrames (
        corona::Image *image,
        int& frameSize,         // Input/Output
        int& frames,            // Output
        int& width,             // "
        int& height) {          // "

    // Return the # of frames in the currently bound image
    // Also adjusts the framesize if appropriate
    assert (image != NULL);

    // Get image dimensions
    width  = image->getWidth ();
    height = image->getHeight ();

    // Calculate frame size (each frame is square, so this represents the width
    // AND height of each frame.)
    while (frameSize > width)
        frameSize >>= 1;
    while (frameSize > height)
        frameSize >>= 1;

    frames = (width / frameSize) * (height / frameSize);
}
int OldSquare_ImageStripFrames (TomVM& vm, std::string filename, int frameSize) {

    // Image size must be power of 2
    if (frameSize < 1 || frameSize > 1024 || !IsPowerOf2 (frameSize)) {
        vm.FunctionError ("Frame size must be a power of 2 from 1-1024");
        return 0;
    }

    // Calculate and return # of frames in image strip
    int result = 0, width = 0, height = 0;

    corona::Image *image = LoadImage (filename);
    if (image != NULL) {
        OldSquare_CalcImageStripFrames (image, frameSize, result, width, height);
        delete image;
    }
    return result;
}
void OldSquare_LoadImageStrip (TomVM& vm, std::string filename, int frameSize, bool mipmap) {

    // Image size must be power of 2
    if (frameSize < 1 || frameSize > 1024 || !IsPowerOf2 (frameSize)) {
        vm.FunctionError ("Frame size must be a power of 2 from 1-1024");
        return;
    }

    // Load image strip
    corona::Image *image = LoadImage (filename);
    if (image != NULL) {
        int frameCount = 0, width = 0, height = 0;
        OldSquare_CalcImageStripFrames (image, frameSize, frameCount, width, height);
        if (frameCount > 65536) {
            vm.FunctionError ("Cannot load more than 65536 images in an image strip");
            return;
        }
        if (frameCount > 0) {

            // Generate some OpenGL textures
            GLuint tex [65536];
            glGenTextures (frameCount, tex);

            // Store texture handles in texture store object so Basic4GL can track them
            for (int i = 0; i < frameCount; i++)
                textures.Store (tex [i]);

            // Iterate over image in grid pattern, extracting each frame
            int frame = 0;
            char    *buffer         = new char [frameSize * frameSize * 4];
            int     bytesPerPixel   = (image->getFormat () & 0xffff) == corona::PF_R8G8B8 ? 3 : 4;
            GLuint  format          = ImageFormat (image);
            for (int y = 0; y < height / frameSize; y++) {
                for (int x = 0; x < width / frameSize; x++) {

                    // Extract block of pixels
                    CopyPixels (    (char *) image->getPixels (),
                                    image->getWidth (),
                                    image->getHeight (),
                                    x * frameSize,
                                    height - (y + 1) * frameSize,
                                    buffer,
                                    frameSize,
                                    frameSize,
                                    bytesPerPixel);

                    // Upload texture
                    glBindTexture (GL_TEXTURE_2D, tex [frame]);
                    if (mipmap) {
                        gluBuild2DMipmaps ( GL_TEXTURE_2D,
                                            bytesPerPixel,
                                            frameSize,
                                            frameSize,
                                            format,
                                            GL_UNSIGNED_BYTE,
                                            buffer);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                    }
                    else {
                        glTexImage2D (      GL_TEXTURE_2D,
                                            0,
                                            bytesPerPixel,
                                            frameSize,
                                            frameSize,
                                            0,
                                            format,
                                            GL_UNSIGNED_BYTE,
                                            buffer);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

                    // Increase frame counter
                    frame++;
                }
            }
            delete[] buffer;

            // Return array of textures
            delete image;
            vm.Reg ().IntVal () = FillTempIntArray (vm.Data (), vm.DataTypes (), frameCount, tex);
            return;
        }
        delete image;
    }

    // Load failed.
    // Return 1 element array containing a 0.
    GLuint blankFrame = 0;
    vm.Reg ().IntVal () = FillTempIntArray (vm.Data (), vm.DataTypes (), 1, &blankFrame);
}
void OldSquare_WrapLoadImageStrip (TomVM& vm) {
    OldSquare_LoadImageStrip (vm, vm.GetStringParam (1), 1024, false);
}
void OldSquare_WrapLoadImageStrip_2 (TomVM& vm) {
    OldSquare_LoadImageStrip (vm, vm.GetStringParam (2), vm.GetIntParam (1), false);
}
void OldSquare_WrapLoadMipmapImageStrip (TomVM& vm) {
    OldSquare_LoadImageStrip (vm, vm.GetStringParam (1), 1024, true);
}
void OldSquare_WrapLoadMipmapImageStrip_2 (TomVM& vm) {
    OldSquare_LoadImageStrip (vm, vm.GetStringParam (2), vm.GetIntParam (1), true);
}
void OldSquare_WrapImageStripFrames (TomVM& vm) {
    vm.Reg ().IntVal () = OldSquare_ImageStripFrames (vm, vm.GetStringParam (1), 1024);
}
void OldSquare_WrapImageStripFrames_2 (TomVM& vm) {
    vm.Reg ().IntVal () = OldSquare_ImageStripFrames (vm, vm.GetStringParam (2), vm.GetIntParam (1));
}
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Initialisation

void InitTomOpenGLBasicLib (TomBasicCompiler& comp, FileOpener *_files, OpenGLWindowManager* windowManager) {

    //assert (window != NULL);
    assert (_files != NULL);
    InitLoadImage (_files);
	::windowManager = windowManager;
	windowManager->SubscribeBeforeDestroyWindow([]()
	{
		// Free loaded textures before window is destroyed.
		// (Doesn't always indicate end of program. Program could be recreating the OpenGL window)
		textures.Clear();
		displayLists.Clear();
	});

    // Register interfaces
    comp.Plugins().RegisterInterface((IB4GLOpenGLWindow *) &windowAdapter, "IB4GLOpenGLWindow", 1, 0, NULL);

    // Register resources
    comp.VM().AddResources (textures);
    comp.VM().AddResources (images);
    comp.VM().AddResources (displayLists);

    // Register initialisation func
    comp.VM().AddInitFunc (Init);

    // Register constants
    comp.AddConstant ("GL_ACTIVE_TEXTURE",          GL_ACTIVE_TEXTURE_ARB);
    comp.AddConstant ("GL_CLIENT_ACTIVE_TEXTURE",   GL_CLIENT_ACTIVE_TEXTURE_ARB);
    comp.AddConstant ("GL_MAX_TEXTURE_UNITS",       GL_MAX_TEXTURE_UNITS_ARB);
    comp.AddConstant ("GL_TEXTURE0",                GL_TEXTURE0_ARB);
    comp.AddConstant ("GL_TEXTURE1",                GL_TEXTURE1_ARB);
    comp.AddConstant ("GL_TEXTURE2",                GL_TEXTURE2_ARB);
    comp.AddConstant ("GL_TEXTURE3",                GL_TEXTURE3_ARB);
    comp.AddConstant ("GL_TEXTURE4",                GL_TEXTURE4_ARB);
    comp.AddConstant ("GL_TEXTURE5",                GL_TEXTURE5_ARB);
    comp.AddConstant ("GL_TEXTURE6",                GL_TEXTURE6_ARB);
    comp.AddConstant ("GL_TEXTURE7",                GL_TEXTURE7_ARB);
    comp.AddConstant ("GL_TEXTURE8",                GL_TEXTURE8_ARB);
    comp.AddConstant ("GL_TEXTURE9",                GL_TEXTURE9_ARB);
    comp.AddConstant ("GL_TEXTURE10",               GL_TEXTURE10_ARB);
    comp.AddConstant ("GL_TEXTURE11",               GL_TEXTURE11_ARB);
    comp.AddConstant ("GL_TEXTURE12",               GL_TEXTURE12_ARB);
    comp.AddConstant ("GL_TEXTURE13",               GL_TEXTURE13_ARB);
    comp.AddConstant ("GL_TEXTURE14",               GL_TEXTURE14_ARB);
    comp.AddConstant ("GL_TEXTURE15",               GL_TEXTURE15_ARB);
    comp.AddConstant ("GL_TEXTURE16",               GL_TEXTURE16_ARB);
    comp.AddConstant ("GL_TEXTURE17",               GL_TEXTURE17_ARB);
    comp.AddConstant ("GL_TEXTURE18",               GL_TEXTURE18_ARB);
    comp.AddConstant ("GL_TEXTURE19",               GL_TEXTURE19_ARB);
    comp.AddConstant ("GL_TEXTURE20",               GL_TEXTURE20_ARB);
    comp.AddConstant ("GL_TEXTURE21",               GL_TEXTURE21_ARB);
    comp.AddConstant ("GL_TEXTURE22",               GL_TEXTURE22_ARB);
    comp.AddConstant ("GL_TEXTURE23",               GL_TEXTURE23_ARB);
    comp.AddConstant ("GL_TEXTURE24",               GL_TEXTURE24_ARB);
    comp.AddConstant ("GL_TEXTURE25",               GL_TEXTURE25_ARB);
    comp.AddConstant ("GL_TEXTURE26",               GL_TEXTURE26_ARB);
    comp.AddConstant ("GL_TEXTURE27",               GL_TEXTURE27_ARB);
    comp.AddConstant ("GL_TEXTURE28",               GL_TEXTURE28_ARB);
    comp.AddConstant ("GL_TEXTURE29",               GL_TEXTURE29_ARB);
    comp.AddConstant ("GL_TEXTURE30",               GL_TEXTURE30_ARB);
    comp.AddConstant ("GL_TEXTURE31",               GL_TEXTURE31_ARB);
    comp.AddConstant ("GL_ACTIVE_TEXTURE_ARB",          GL_ACTIVE_TEXTURE_ARB);
    comp.AddConstant ("GL_CLIENT_ACTIVE_TEXTURE_ARB",   GL_CLIENT_ACTIVE_TEXTURE_ARB);
    comp.AddConstant ("GL_MAX_TEXTURE_UNITS_ARB",       GL_MAX_TEXTURE_UNITS_ARB);
    comp.AddConstant ("GL_TEXTURE0_ARB",                GL_TEXTURE0_ARB);
    comp.AddConstant ("GL_TEXTURE1_ARB",                GL_TEXTURE1_ARB);
    comp.AddConstant ("GL_TEXTURE2_ARB",                GL_TEXTURE2_ARB);
    comp.AddConstant ("GL_TEXTURE3_ARB",                GL_TEXTURE3_ARB);
    comp.AddConstant ("GL_TEXTURE4_ARB",                GL_TEXTURE4_ARB);
    comp.AddConstant ("GL_TEXTURE5_ARB",                GL_TEXTURE5_ARB);
    comp.AddConstant ("GL_TEXTURE6_ARB",                GL_TEXTURE6_ARB);
    comp.AddConstant ("GL_TEXTURE7_ARB",                GL_TEXTURE7_ARB);
    comp.AddConstant ("GL_TEXTURE8_ARB",                GL_TEXTURE8_ARB);
    comp.AddConstant ("GL_TEXTURE9_ARB",                GL_TEXTURE9_ARB);
    comp.AddConstant ("GL_TEXTURE10_ARB",               GL_TEXTURE10_ARB);
    comp.AddConstant ("GL_TEXTURE11_ARB",               GL_TEXTURE11_ARB);
    comp.AddConstant ("GL_TEXTURE12_ARB",               GL_TEXTURE12_ARB);
    comp.AddConstant ("GL_TEXTURE13_ARB",               GL_TEXTURE13_ARB);
    comp.AddConstant ("GL_TEXTURE14_ARB",               GL_TEXTURE14_ARB);
    comp.AddConstant ("GL_TEXTURE15_ARB",               GL_TEXTURE15_ARB);
    comp.AddConstant ("GL_TEXTURE16_ARB",               GL_TEXTURE16_ARB);
    comp.AddConstant ("GL_TEXTURE17_ARB",               GL_TEXTURE17_ARB);
    comp.AddConstant ("GL_TEXTURE18_ARB",               GL_TEXTURE18_ARB);
    comp.AddConstant ("GL_TEXTURE19_ARB",               GL_TEXTURE19_ARB);
    comp.AddConstant ("GL_TEXTURE20_ARB",               GL_TEXTURE20_ARB);
    comp.AddConstant ("GL_TEXTURE21_ARB",               GL_TEXTURE21_ARB);
    comp.AddConstant ("GL_TEXTURE22_ARB",               GL_TEXTURE22_ARB);
    comp.AddConstant ("GL_TEXTURE23_ARB",               GL_TEXTURE23_ARB);
    comp.AddConstant ("GL_TEXTURE24_ARB",               GL_TEXTURE24_ARB);
    comp.AddConstant ("GL_TEXTURE25_ARB",               GL_TEXTURE25_ARB);
    comp.AddConstant ("GL_TEXTURE26_ARB",               GL_TEXTURE26_ARB);
    comp.AddConstant ("GL_TEXTURE27_ARB",               GL_TEXTURE27_ARB);
    comp.AddConstant ("GL_TEXTURE28_ARB",               GL_TEXTURE28_ARB);
    comp.AddConstant ("GL_TEXTURE29_ARB",               GL_TEXTURE29_ARB);
    comp.AddConstant ("GL_TEXTURE30_ARB",               GL_TEXTURE30_ARB);
    comp.AddConstant ("GL_TEXTURE31_ARB",               GL_TEXTURE31_ARB);

    //comp.AddConstant ("GL_COMBINE",        GL_COMBINE_EXT);
    //comp.AddConstant ("GL_COMBINE_RGB",    GL_COMBINE_RGB_EXT);
    //comp.AddConstant ("GL_COMBINE_ALPHA",  GL_COMBINE_ALPHA_EXT);
    //comp.AddConstant ("GL_RGB_SCALE",      GL_RGB_SCALE_EXT);
    //comp.AddConstant ("GL_ADD_SIGNED",     GL_ADD_SIGNED_EXT);
    //comp.AddConstant ("GL_INTERPOLATE",    GL_INTERPOLATE_EXT);
    //comp.AddConstant ("GL_CONSTANT",       GL_CONSTANT_EXT);
    //comp.AddConstant ("GL_PRIMARY_COLOR",  GL_PRIMARY_COLOR_EXT);
    //comp.AddConstant ("GL_PREVIOUS",       GL_PREVIOUS_EXT);
    //comp.AddConstant ("GL_SOURCE0_RGB",    GL_SOURCE0_RGB_EXT);
    //comp.AddConstant ("GL_SOURCE1_RGB",    GL_SOURCE1_RGB_EXT);
    //comp.AddConstant ("GL_SOURCE2_RGB",    GL_SOURCE2_RGB_EXT);
    //comp.AddConstant ("GL_SOURCE0_ALPHA",  GL_SOURCE0_ALPHA_EXT);
    //comp.AddConstant ("GL_SOURCE1_ALPHA",  GL_SOURCE1_ALPHA_EXT);
    //comp.AddConstant ("GL_SOURCE2_ALPHA",  GL_SOURCE2_ALPHA_EXT);
    //comp.AddConstant ("GL_OPERAND0_RGB",   GL_OPERAND0_RGB_EXT);
    //comp.AddConstant ("GL_OPERAND1_RGB",   GL_OPERAND1_RGB_EXT);
    //comp.AddConstant ("GL_OPERAND2_RGB",   GL_OPERAND2_RGB_EXT);
    //comp.AddConstant ("GL_OPERAND0_ALPHA", GL_OPERAND0_ALPHA_EXT);
    //comp.AddConstant ("GL_OPERAND1_ALPHA", GL_OPERAND1_ALPHA_EXT);
    //comp.AddConstant ("GL_OPERAND2_ALPHA", GL_OPERAND2_ALPHA_EXT);
    //comp.AddConstant ("GL_COMBINE_EXT",        GL_COMBINE_EXT);
    //comp.AddConstant ("GL_COMBINE_RGB_EXT",    GL_COMBINE_RGB_EXT);
    //comp.AddConstant ("GL_COMBINE_ALPHA_EXT",  GL_COMBINE_ALPHA_EXT);
    //comp.AddConstant ("GL_RGB_SCALE_EXT",      GL_RGB_SCALE_EXT);
    //comp.AddConstant ("GL_ADD_SIGNED_EXT",     GL_ADD_SIGNED_EXT);
    //comp.AddConstant ("GL_INTERPOLATE_EXT",    GL_INTERPOLATE_EXT);
    //comp.AddConstant ("GL_CONSTANT_EXT",       GL_CONSTANT_EXT);
    //comp.AddConstant ("GL_PRIMARY_COLOR_EXT",  GL_PRIMARY_COLOR_EXT);
    //comp.AddConstant ("GL_PREVIOUS_EXT",       GL_PREVIOUS_EXT);
    //comp.AddConstant ("GL_SOURCE0_RGB_EXT",    GL_SOURCE0_RGB_EXT);
    //comp.AddConstant ("GL_SOURCE1_RGB_EXT",    GL_SOURCE1_RGB_EXT);
    //comp.AddConstant ("GL_SOURCE2_RGB_EXT",    GL_SOURCE2_RGB_EXT);
    //comp.AddConstant ("GL_SOURCE0_ALPHA_EXT",  GL_SOURCE0_ALPHA_EXT);
    //comp.AddConstant ("GL_SOURCE1_ALPHA_EXT",  GL_SOURCE1_ALPHA_EXT);
    //comp.AddConstant ("GL_SOURCE2_ALPHA_EXT",  GL_SOURCE2_ALPHA_EXT);
    //comp.AddConstant ("GL_OPERAND0_RGB_EXT",   GL_OPERAND0_RGB_EXT);
    //comp.AddConstant ("GL_OPERAND1_RGB_EXT",   GL_OPERAND1_RGB_EXT);
    //comp.AddConstant ("GL_OPERAND2_RGB_EXT",   GL_OPERAND2_RGB_EXT);
    //comp.AddConstant ("GL_OPERAND0_ALPHA_EXT", GL_OPERAND0_ALPHA_EXT);
    //comp.AddConstant ("GL_OPERAND1_ALPHA_EXT", GL_OPERAND1_ALPHA_EXT);
    //comp.AddConstant ("GL_OPERAND2_ALPHA_EXT", GL_OPERAND2_ALPHA_EXT);

    // Register function wrappers
    comp.AddFunction ("loadimage",          WrapLoadImage,          compParamTypeList () << VTP_STRING, true, true,     VTP_INT);
    comp.AddFunction ("deleteimage",        WrapDeleteImage,        compParamTypeList () << VTP_INT,    true, false,    VTP_INT);
    comp.AddFunction ("imagewidth",         WrapImageWidth,         compParamTypeList () << VTP_INT,    true, true,     VTP_INT);
    comp.AddFunction ("imageheight",        WrapImageHeight,        compParamTypeList () << VTP_INT,    true, true,     VTP_INT);
    comp.AddFunction ("imageformat",        WrapImageFormat,        compParamTypeList () << VTP_INT,    true, true,     VTP_INT);
    comp.AddFunction ("imagedatatype",      WrapImageDataType,      compParamTypeList () << VTP_INT,    true, true,     VTP_INT);
    comp.AddFunction ("loadtexture",        WrapLoadTexture,        compParamTypeList () << VTP_STRING, true, true,     VTP_INT);
    comp.AddFunction ("loadmipmaptexture",  WrapLoadMipmapTexture,  compParamTypeList () << VTP_STRING, true, true,     VTP_INT);
    comp.AddFunction ("glgentexture",       WrapglGenTexture,       compParamTypeList (),               true, true,     VTP_INT);
    comp.AddFunction ("gldeletetexture",    WrapglDeleteTexture,    compParamTypeList () << VTP_INT,    true, false,    VTP_INT);
    comp.AddFunction ("glteximage2d",       WrapglTexImage2D,       compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("gltexsubimage2d",    WrapglTexSubImage2D,    compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("glubuild2dmipmaps",  WrapgluBuild2DMipmaps,  compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("glgetstring",        WrapglGetString,        compParamTypeList () << VTP_INT,    true, true,     VTP_STRING);
    comp.AddFunction ("extensionsupported", WrapExtensionSupported, compParamTypeList () << VTP_STRING, true, true,     VTP_INT);
    comp.AddFunction ("glmultitexcoord2f",  WrapglMultiTexCoord2f,  compParamTypeList () << VTP_INT << VTP_REAL << VTP_REAL,    true, false, VTP_INT);
    comp.AddFunction ("glmultitexcoord2d",  WrapglMultiTexCoord2d,  compParamTypeList () << VTP_INT << VTP_REAL << VTP_REAL,    true, false, VTP_INT);
    comp.AddFunction ("glactivetexture",    WrapglActiveTexture,    compParamTypeList () << VTP_INT,    true, false,    VTP_INT);
    comp.AddFunction ("maxtextureunits",    WrapMaxTextureUnits,    compParamTypeList (),               true, true,     VTP_INT);
    comp.AddFunction ("windowwidth",        WrapWindowWidth,        compParamTypeList (),               true, true,     VTP_INT);
    comp.AddFunction ("windowheight",       WrapWindowHeight,       compParamTypeList (),               true, true,     VTP_INT);
    comp.AddFunction ("glgentextures",      WrapglGenTextures,      compParamTypeList () << VTP_INT << vmValType (VTP_INT, 1, 1, true),  true, false, VTP_INT);
    comp.AddFunction ("gldeletetextures",   WrapglDeleteTextures,   compParamTypeList () << VTP_INT << vmValType (VTP_INT, 1, 1, true),  true, false, VTP_INT);
    comp.AddFunction ("glLoadMatrixd", WrapglLoadMatrixd, compParamTypeList ()<<vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glLoadMatrixf", WrapglLoadMatrixf, compParamTypeList ()<<vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glMultMatrixd", WrapglMultMatrixd, compParamTypeList ()<<vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glMultMatrixf", WrapglMultMatrixf, compParamTypeList ()<<vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glgetpolygonstipple",WrapglGetPolygonStipple,compParamTypeList () << vmValType(VTP_INT,1,1,true), true, false, VTP_INT);
    comp.AddFunction ("glpolygonstipple",   WrapglPolygonStipple,   compParamTypeList () << vmValType(VTP_INT,1,1,true), true, false, VTP_INT);
    comp.AddFunction ("glgenlists",         WrapglGenLists,         compParamTypeList () << VTP_INT,    true, true,        VTP_INT);
    comp.AddFunction ("gldeletelists",      WrapglDeleteLists,      compParamTypeList () << VTP_INT << VTP_INT, true,false,VTP_INT);
    comp.AddFunction ("glcalllists",        WrapglCallLists,        compParamTypeList () << VTP_INT << VTP_INT << vmValType(VTP_REAL,1,1,true), true,false,VTP_INT);
    comp.AddFunction ("glcalllists",        WrapglCallLists_2,      compParamTypeList () << VTP_INT << VTP_INT << vmValType(VTP_INT,1,1,true),  true,false,VTP_INT);
    comp.AddFunction ("glBegin",            WrapglBegin,            compParamTypeList () << VTP_INT,    true, false,    VTP_INT);
    comp.AddFunction ("glEnd",              WrapglEnd,              compParamTypeList (),               true, false,    VTP_INT);
    comp.AddFunction ("imagestripframes",   OldSquare_WrapImageStripFrames,   compParamTypeList () << VTP_STRING,                 true,true, VTP_INT, true);
    comp.AddFunction ("imagestripframes",   OldSquare_WrapImageStripFrames_2, compParamTypeList () << VTP_STRING << VTP_INT,      true,true, VTP_INT, true);
    comp.AddFunction ("loadimagestrip",     OldSquare_WrapLoadImageStrip,     compParamTypeList () << VTP_STRING,                 true,true, vmValType(VTP_INT,1,1,true), true);
    comp.AddFunction ("loadimagestrip",     OldSquare_WrapLoadImageStrip_2,   compParamTypeList () << VTP_STRING << VTP_INT,      true,true, vmValType(VTP_INT,1,1,true), true);
    comp.AddFunction ("loadmipmapimagestrip",OldSquare_WrapLoadMipmapImageStrip,  compParamTypeList () << VTP_STRING,             true,true, vmValType(VTP_INT,1,1,true), true);
    comp.AddFunction ("loadmipmapimagestrip",OldSquare_WrapLoadMipmapImageStrip_2,compParamTypeList () << VTP_STRING << VTP_INT,  true,true, vmValType(VTP_INT,1,1,true), true);
    comp.AddFunction ("glteximage2d",       WrapglTexImage2D_2,     compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_INT,1,1,true), true, false, VTP_INT);
    comp.AddFunction ("glteximage2d",       WrapglTexImage2D_3,     compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_REAL,1,1,true), true, false, VTP_INT);
    comp.AddFunction ("glteximage2d",       WrapglTexImage2D_4,     compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_INT,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glteximage2d",       WrapglTexImage2D_5,     compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glubuild2dmipmaps",  WrapgluBuild2DMipmaps_2,compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_INT,1,1,true), true, false, VTP_INT);
    comp.AddFunction ("glubuild2dmipmaps",  WrapgluBuild2DMipmaps_3,compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_REAL,1,1,true), true, false, VTP_INT);
    comp.AddFunction ("glubuild2dmipmaps",  WrapgluBuild2DMipmaps_4,compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_INT,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glubuild2dmipmaps",  WrapgluBuild2DMipmaps_5,compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << VTP_INT << vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glGetFloatv",        WrapglGetFloatv_2D,     compParamTypeList () << VTP_INT << vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glGetDoublev",       WrapglGetDoublev_2D,    compParamTypeList () << VTP_INT << vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glGetIntegerv",      WrapglGetIntegerv_2D,   compParamTypeList () << VTP_INT << vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("glGetBooleanv",      WrapglGetBooleanv_2D,   compParamTypeList () << VTP_INT << vmValType(VTP_REAL,2,1,true), true, false, VTP_INT);
    comp.AddFunction ("imagestripframes",   WrapImageStripFrames,       compParamTypeList () << VTP_STRING << VTP_INT << VTP_INT, true,true, VTP_INT, true);
    comp.AddFunction ("loadimagestrip",     WrapLoadImageStrip,         compParamTypeList () << VTP_STRING << VTP_INT << VTP_INT, true,true, vmValType(VTP_INT,1,1,true), true);
    comp.AddFunction ("loadmipmapimagestrip",WrapLoadMipmapImageStrip,  compParamTypeList () << VTP_STRING << VTP_INT << VTP_INT, true,true, vmValType(VTP_INT,1,1,true), true);

    // New texture loading
    comp.AddFunction ("LoadTex",                    WrapLoadTex, compParamTypeList() << VTP_STRING, true, true, VTP_INT, true);
    comp.AddFunction ("LoadTexStrip",               WrapLoadTexStrip, compParamTypeList() << VTP_STRING, true, true, vmValType(VTP_INT, 1, 1, true), true);
    comp.AddFunction ("LoadTexStrip",               WrapLoadTexStrip2, compParamTypeList() << VTP_STRING << VTP_INT << VTP_INT, true, true, vmValType(VTP_INT, 1, 1, true), true);
    comp.AddFunction ("TexStripFrames",             WrapTexStripFrames, compParamTypeList() << VTP_STRING, true, true, VTP_INT, true);
    comp.AddFunction ("TexStripFrames",             WrapTexStripFrames2, compParamTypeList() << VTP_STRING << VTP_INT << VTP_INT, true, true, VTP_INT, true);
    comp.AddFunction ("SetTexIgnoreBlankFrames",    WrapSetTexIgnoreBlankFrames, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("SetTexTransparentCol",       WrapSetTexTransparentCol, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("SetTexTransparentCol",       WrapSetTexTransparentCol2, compParamTypeList() << VTP_INT << VTP_INT << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("SetTexNoTransparentCol",     WrapSetTexNoTransparentCol, compParamTypeList(), true, false, VTP_INT);
    comp.AddFunction ("SetTexMipmap",               WrapSetTexMipmap, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("SetTexLinearFilter",         WrapSetTexLinearFilter, compParamTypeList() << VTP_INT, true, false, VTP_INT);

    // OpenGL window management
    comp.AddFunction("SetWindowWidth",              WrapSetWindowWidth, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("SetWindowHeight",             WrapSetWindowHeight, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("SetWindowFullscreen",         WrapSetWindowFullscreen, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("SetWindowBorder",             WrapSetWindowBorder, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("SetWindowBPP",                WrapSetWindowBpp, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("SetWindowStencil",            WrapSetWindowStencil, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("SetWindowTitle",              WrapSetWindowTitle, compParamTypeList() << VTP_STRING, true, false, VTP_INT);
    comp.AddFunction("SetWindowResizable",          WrapSetWindowResizable, compParamTypeList() << VTP_INT, true, false, VTP_INT);
    comp.AddFunction("WindowFullscreen",            WrapWindowFullscreen, compParamTypeList(), true, true, VTP_INT);
    comp.AddFunction("WindowBorder",                WrapWindowBorder, compParamTypeList(), true, true, VTP_INT);
    comp.AddFunction("WindowBPP",                   WrapWindowBpp, compParamTypeList(), true, true, VTP_INT);
    comp.AddFunction("WindowStencil",               WrapWindowStencil, compParamTypeList(), true, true, VTP_INT);
    comp.AddFunction("WindowTitle",                 WrapWindowTitle, compParamTypeList(), true, true, VTP_STRING);
    comp.AddFunction("WindowResizable",             WrapWindowResizable, compParamTypeList(), true, true, VTP_INT);
    comp.AddFunction("UpdateWindow",                WrapUpdateWindow, compParamTypeList(), true, true, VTP_INT, true);

	comp.AddFunction ("swapbuffers",				WrapSwapBuffers,compParamTypeList (), true, false, VTP_INT);
};


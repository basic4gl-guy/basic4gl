//---------------------------------------------------------------------------
// Created 27-Sep-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
#pragma hdrstop

#include "glTextGrid.h"
#include <assert.h>
#include "glheaders.h"
#ifndef _MSC_VER
#include <mem.h>
#endif

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

#ifndef byte
typedef unsigned char byte;
#endif

///////////////////////////////////////////////////////////////////////////////
// glTextGrid

unsigned long MakeColour (byte r, byte g, byte b, byte alpha) {
    return                      r
            |   (unsigned long) g       << 8
            |   (unsigned long) b       << 16
            |   (unsigned long) alpha   << 24;
}

glTextGrid::glTextGrid (const char *texFile, FileOpener *files, int rows, int cols, int texRows, int texCols)
    :   m_rows (rows), m_cols (cols), m_texRows (texRows), m_texCols (texCols) {

    assert (m_rows > 0);
    assert (m_cols > 0);
    assert (m_texRows > 0);
    assert (m_texCols > 0);

    // Initialise defaults
    m_showCursor    = false;
    m_cx            = 0;
    m_cy            = 0;
    m_texture       = 0;
    m_chars         = NULL;
    m_colours       = NULL;
    m_textures      = NULL;
    m_currentColour = MakeColour (220, 220, 255);
    m_currentTexture = 0;
    m_scroll        = true;

    // Load charset texture
    //glPushAttrib (GL_ALL_ATTRIB_BITS);
    //glEnable (GL_TEXTURE_2D);
    InitLoadImage (files);
    m_image = LoadImage (texFile);
    if (m_image == NULL) {
        SetError ((std::string) "Error loading bitmap: " + texFile);
        return;
    }

    // Calculate character size (in texture coordinates)
    m_charXSize = 1.0 / m_texCols;
    m_charYSize = 1.0 / m_texRows;

    // Allocate character map
    m_chars     = new char [m_rows * m_cols];
    m_colours   = new unsigned long [m_rows * m_cols];
    m_textures  = new GLuint [m_rows * m_cols];
    Clear ();
}

glTextGrid::~glTextGrid () {

    // Delete character map
    if (m_chars != NULL)
        delete[] m_chars;
    if (m_colours != NULL)
        delete[] m_colours;
    if (m_textures != NULL)
        delete[] m_textures;

    // Deallocate texture and image
    if (m_texture != 0)
        glDeleteTextures (1, &m_texture);
    if (m_image != NULL)
        delete m_image;
}

void glTextGrid::UploadCharsetTexture () {

    // Upload the character set texture into OpenGL
    glGenTextures (1, &m_texture);
    glBindTexture (GL_TEXTURE_2D, m_texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    gluBuild2DMipmaps ( GL_TEXTURE_2D,
                        (m_image->getFormat () & 0xffff) == corona::PF_R8G8B8 ? 3 : 4,
                        m_image->getWidth (),
                        m_image->getHeight (),
                        ImageFormat (m_image),
                        GL_UNSIGNED_BYTE,
                        m_image->getPixels ());
    m_currentTexture    = m_texture;
}

void glTextGrid::DeleteCharsetTexture()
{
	glDeleteTextures(1, &m_texture);
	m_texture = 0;
	m_currentTexture = 0;
}

void glTextGrid::Clear () {

    // Clear character map
    memset (m_chars,        ' ', m_rows * m_cols);

    // Move cursor home
    MoveCursor (0, 0);
}

void glTextGrid::ClearLine () {

    // Clear the current line
    memset (&m_chars [m_cy * m_cols], ' ', m_cols);
}

void glTextGrid::ClearRegion (int x1, int y1, int x2, int y2) {

    // Validate region
    if (x1 < 0)         x1 = 0;
    if (y1 < 0)         y1 = 0;
    if (x2 >= m_cols)   x2 = m_cols - 1;
    if (y2 >= m_rows)   y2 = m_rows - 1;
    if (x2 >= x1 && y2 >= y1)

        // Clear it
        for (int y = y1; y <= y2; y++)
            memset (&m_chars [y * m_cols + x1], ' ', x2 - x1 + 1);
}

const double offset = 0.000;

void glTextGrid::Draw() {

    // Create perspective transform
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
	gluOrtho2D (0, 0, 1, 1);

    // Clear model view matrix (to identity)
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    // Iterate through character map
    int index = 0;
    double  xStep = 2.0 / m_cols,           // Character size on screen in screen coordinates
            yStep = 2.0 / m_rows;
    GLuint lastTex = 0;

    for (int y = 0; y < m_rows; y++) {
        for (int x = 0; x < m_cols; x++) {

            // Get current and previous character
            unsigned char c = m_chars [index];
            unsigned long col = m_colours [index];
            GLuint tex = m_textures [index];

            // Redraw character if changed
            // Find character 2D position in texture
            if (c != ' ') {
                int charx = c % m_texCols,
                    chary = c / m_texCols;

                // Convert to texture coordinates
                double  tx = charx * m_charXSize + offset,
                        ty = 1.0 - (chary + 1) * m_charYSize + offset;

                // Find screen coordinates
                double  screenX = x * xStep - 1.0,
                        screenY = 1.0 - (y + 1) * yStep;

                // Bind appropriate texture
                if (tex != lastTex) {
                    glBindTexture (GL_TEXTURE_2D, tex);
                    lastTex = tex;
                }

                // Write character
                glColor4ubv ((unsigned char *) &col);
                glBegin (GL_QUADS);
                    glTexCoord2d (tx,                ty);               glVertex2d (screenX,           screenY);
                    glTexCoord2d (tx + m_charXSize,  ty);               glVertex2d (screenX + xStep,   screenY);
                    glTexCoord2d (tx + m_charXSize,  ty + m_charYSize); glVertex2d (screenX + xStep,   screenY + yStep);
                    glTexCoord2d (tx,                ty + m_charYSize); glVertex2d (screenX,           screenY + yStep);
                glEnd ();
            }
            index++;
        }
    }
}

void glTextGrid::ScrollUp () {

    // Move rows up
    int y;
    for (y = 1; y < m_rows; y++)
        memcpy (&m_chars [(y - 1) * m_cols], &m_chars [y * m_cols], m_cols);

    // Clear bottom line
    memset (&m_chars [(m_rows - 1) * m_cols], ' ', m_cols);

    // Do the same to the colour map
    for (y = 1; y < m_rows; y++)
        memcpy (&m_colours [(y - 1) * m_cols], &m_colours [y * m_cols], m_cols * sizeof (unsigned long));

    // And the texture line
    for (y = 1; y < m_rows; y++)
        memcpy (&m_textures [(y - 1) * m_cols], &m_textures [y * m_cols], m_cols * sizeof (GLuint));

    // Move cursor up
    CursorUp ();
}

void glTextGrid::Write (std::string s) {

    // Write out string. Split it around rows.
    int len = s.length (), index = 0;
    while (len > 0) {

        // Determine length of next substring
        int subLen = m_cols - m_cx;
        if (subLen > len)
            subLen = len;

        // Write out bit
        int offset = m_cy * m_cols + m_cx;
        if (subLen > 0) {
            memcpy (&m_chars [offset], &s.c_str () [index], subLen);
            index += subLen;
            len -= subLen;
        }

        // Set colours and textures
        int x;
        for (x = 0; x < subLen; x++)
            m_colours [offset + x] = m_currentColour;
        for (x = 0; x < subLen; x++)
            m_textures [offset + x] = m_currentTexture;

        // Advance cursor
        CursorRight (subLen);
    }
}

void glTextGrid::Resize (int rows, int cols) {
    assert (rows > 0);
    assert (cols > 0);

    m_rows = rows;
    m_cols = cols;

    // Reallocate text grid
    if (m_chars != NULL)
        delete[] m_chars;
    m_chars = new char [m_rows * m_cols];

    if (m_colours != NULL)
        delete[] m_colours;
    m_colours = new unsigned long [m_rows * m_cols];

    if (m_textures != NULL)
        delete[] m_textures;
    m_textures = new GLuint [m_rows * m_cols];

    Clear ();
}

//std::string glTextGrid::GetString (glWindow *window) {
//
//    assert (window != NULL);
//
//    // Record leftmost cursor position
//    // Cursor can not be moved further left than that point
//    int left        = m_cx;
//    bool saveCursor = m_showCursor;
//    m_showCursor    = true;
//
//    // Perform UI
//    bool done = false;
//    while (!(window->Closing () || done)) {
//
//        glClear (GL_COLOR_BUFFER_BIT);
//        Draw(0xff);
//        window->SwapBuffers ();
//
//        // Keyboard input
//        char c = 0, sc = 0;
//        while (c == 0 && sc == 0 && !window->Closing ()) {
//            sc = window->GetScanKey ();
//            if (sc == 0)
//                c = window->GetKey ();
//        }
//
//        switch (sc) {
//        case VK_LEFT:
//            if (m_cx > left)
//                m_cx--;
//            break;
//        case VK_RIGHT:
//            if (m_cx < m_cols - 1)
//                m_cx++;
//            break;
//        case VK_DELETE:
//            Delete ();
//            break;
//        case VK_BACK:
//            if (m_cx > left)
//                Backspace ();
//            break;
//        };
//        if (c == VK_RETURN)
//            done = true;
//        if (c >= ' ') {
//            if (m_cx < m_cols - 1 && Insert ()) {
//                m_chars     [m_cy * m_cols + m_cx] = c;
//                m_colours   [m_cy * m_cols + m_cx] = m_currentColour;
//                m_textures  [m_cy * m_cols + m_cx] = m_currentTexture;
//                m_cx++;
//            }
//        }
//    }
//
//    // Extract string
//    int lineOffset  = m_cy * m_cols;
//    int right       = m_cols;
//    while (right > left && m_chars [lineOffset + right - 1] <= ' ')      // Trim spaces from right
//        right--;
//    while (left < right && m_chars [lineOffset + left] <= ' ')           // Trim spaces from left
//        left++;
//    std::string result = "";
//    for (int i = left; i < right; i++)
//        result = result + m_chars [lineOffset + i];
//
//    // Restore cursor, perform newline and update screen
//    NewLine ();
//    m_showCursor = saveCursor;
//    glClear (GL_COLOR_BUFFER_BIT);
//    Draw (0xff);
//    window->SwapBuffers ();
//
//    return result;
//}

void glTextGrid::Delete () {

    // Shift text to the left
    int lineOffset = m_cy * m_cols;
    for (int x = m_cx; x < m_cols - 1; x++) {
        m_chars   [lineOffset + x] = m_chars   [lineOffset + x + 1];
        m_colours [lineOffset + x] = m_colours [lineOffset + x + 1];
    }

    // Insert space on the right
    m_chars [lineOffset + m_cols - 1] = ' ';
}

bool glTextGrid::Insert () {

    // Room to insert a space?
    int lineOffset = m_cy * m_cols;
    if (m_chars [lineOffset + m_cols - 1] <= ' ') {

        // Shift text to the right
        for (int x = m_cols - 1; x > m_cx; x--) {
            m_chars   [lineOffset + x] = m_chars   [lineOffset + x - 1];
            m_colours [lineOffset + x] = m_colours [lineOffset + x - 1];
        }

        // Insert space
        m_chars [lineOffset + m_cx] = ' ';

        return true;
    }
    else
        return false;
}

bool glTextGrid::Backspace () {
    if (m_cx > 0) {
        --m_cx;
        Delete ();
        return true;
    }
    else
        return false;
}


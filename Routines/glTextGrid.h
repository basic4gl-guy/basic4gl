//---------------------------------------------------------------------------
// Created 27-Jun-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
/*  OpenGL 2D text display.
*/

#ifndef glTextGridH
#define glTextGridH
//---------------------------------------------------------------------------
#include "HasErrorState.h"
#include "EmbeddedFiles.h"
#include "LoadImage.h"

// Constants
const byte DRAW_TEXT = 1;

unsigned long MakeColour (byte r, byte g, byte b, byte alpha = 255);

class glTextGrid : public HasErrorState {
    GLuint  m_texture;
    corona::Image* m_image;                 // Character set image. Because OpenGL context keeps getting re-created, we need to keep loading it into our texture!
    int     m_texRows, m_texCols;           // Rows and columns in texture
    double  m_charXSize, m_charYSize;       // Character size in texture coordinates
    int     m_rows, m_cols;                 // Rows and columns in grid
    char    *m_chars;                       // Character grid
    unsigned long *m_colours;               // Colour grid
    GLuint  *m_textures;                    // Texture handle grid
    int     m_cx, m_cy;                     // Cursor position
    bool    m_showCursor;
    unsigned long m_currentColour;          // Current colour
    GLuint  m_currentTexture;               // Current texture font
    bool    m_scroll;

public:
    glTextGrid (const char *texFile, FileOpener *files, int rows, int cols, int texRows, int texCols);
    ~glTextGrid ();

    void UploadCharsetTexture ();           // Upload charset texture into OpenGL
	void DeleteCharsetTexture();
    
    void Resize (int rows, int cols);
    int Rows () { return m_rows; }
    int Cols () { return m_cols; }

    // Drawing
    void Draw();

    // Scrolling
    void ScrollUp ();
    bool& Scroll () { return m_scroll; }

    // Cursor commands
    int CursorX ()     { return m_cx; }
    int CursorY ()     { return m_cy; }
    void ShowCursor ()  { m_showCursor = true; }
    void HideCursor ()  { m_showCursor = false; }
	bool IsCursorShowing() const { return m_showCursor; }
    void MoveCursor (int x, int y) {
        if (x < 0)          x = 0;
        if (x >= m_cols)    x = m_cols - 1;
        if (y < 0)          y = 0;
        if (y >= m_rows)    y = m_rows - 1;
        m_cx = x;
        m_cy = y;
    }
    void CursorUp () {
        if (--m_cy < 0)
            m_cy = 0;
    }
    void CursorDown () {
        if (++m_cy >= m_rows) {
            if (m_scroll)
                ScrollUp ();
            else
                CursorUp ();
        }
    }
    void CursorLeft () {
        if (--m_cx < 0)
            m_cx = 0;
    }
    void NewLine () {
        m_cx = 0;
        CursorDown ();
    }
    void CursorRight (int dist = 1) {
        m_cx += dist;
        while (m_cx >= m_cols) {
            m_cx -= m_cols;
            CursorDown ();
        }
    }

    // Insert/delete space
    void Delete ();
    bool Insert ();
    bool Backspace ();

    // Writing commands
    void Clear ();
    void Write (std::string s);
    void ClearLine ();
    void ClearRegion (int x1, int y1, int x2, int y2);

    // Reading commands
    char TextAt (int x, int y) {
        if (    x < 0
            ||  x >= m_cols
            ||  y < 0
            ||  y >= m_rows)
            return 0;
        else
            return m_chars [y * m_cols + x];
    }
    unsigned long ColourAt (int x, int y) {
        if (    x < 0
            ||  x >= m_cols
            ||  y < 0
            ||  y >= m_rows)
            return m_currentColour;
        else
            return m_colours [y * m_cols + x];
    }
    GLuint TextureAt (int x, int y) {
        if (    x < 0
            ||  x >= m_cols
            ||  y < 0
            ||  y >= m_rows)
            return m_currentTexture;
        else
            return m_textures [y * m_cols + x];
    }

	char const* Chars() const { return m_chars; }

    // Colour commands
    unsigned long Colour ()             { return m_currentColour; }
    void SetColour (unsigned long col)  { m_currentColour = col; }

    // Texture commands
    GLuint Texture ()                   { return m_currentTexture; }
    void SetTexture (GLuint tex)        { m_currentTexture = tex; }
    GLuint DefaultTexture () { return m_texture; }

    // Input commands
    //std::string GetString (glWindow *window);       // Allows user to type in a string. Returns string upon exit.
};

#endif

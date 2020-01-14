//---------------------------------------------------------------------------
// Created 24-Jan-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew

// A text grid with sprite support.

#ifndef glSpriteWindowH
#define glSpriteWindowH
//---------------------------------------------------------------------------
#include "glTextGrid.h"
#include "vmTypes.h"
#include <vector>

// Constants
const byte DRAW_SPRITES_INFRONT = 2;
const byte DRAW_SPRITES_BEHIND = 4;
const byte DRAW_SPRITES = DRAW_SPRITES_INFRONT | DRAW_SPRITES_BEHIND;

////////////////////////////////////////////////////////////////////////////////
// glSpriteList
//
// A list of glBasicSprite objects, sorted by Z order

class glBasicSprite;

class glSpriteList {
public:
    glBasicSprite *m_head;

    glSpriteList ();
    ~glSpriteList ();
};

enum glSpriteType { SPR_INVALID, SPR_SPRITE, SPR_TILEMAP };

////////////////////////////////////////////////////////////////////////////////
// glBasicSprite
//
// Abstract base type for 2D sprite and sprite like objects.
// (Currently this includes sprites and tile map layers.)

typedef std::vector<GLuint> TextureVec;

class glBasicSprite {
private:
    glSpriteList   *m_list;
    glBasicSprite  *m_prev, *m_next;
    vmReal           m_zOrder;
    void InternalRemove ();
    void InternalReorder (glBasicSprite *prev, glBasicSprite *next);
    void Reorder ();
	void SetBaseDefaults ();

protected:
    TextureVec      m_textures;
    virtual void InternalCopy (glBasicSprite *s);
    virtual void CheckFrame ();

public:

    // Basic fields.
    // I can't be bothered to write getters/setters for these. Just write in the
    // values, and call Render().
    vmReal  m_x, m_y, m_xSize, m_ySize, m_scale;
    vmReal  m_xCentre, m_yCentre;
    bool    m_xFlip, m_yFlip, m_visible, m_parallax, m_solid;
    vmReal  m_angle, m_colour [4];
    int     m_srcBlend, m_dstBlend;

    // Construction/destruction
    glBasicSprite ();
    glBasicSprite (GLuint tex);
    glBasicSprite (TextureVec& tex);
    virtual ~glBasicSprite ();

    // Class type identification
    virtual glSpriteType Type () = 0;

    // ZOrder and list functions
    void Insert (glSpriteList *list);
    void Remove ();
    vmReal ZOrder () const			{ return m_zOrder; }
    void SetZOrder (vmReal zOrder)  { m_zOrder = zOrder; Reorder (); }
    glBasicSprite *Prev ()			{ return m_prev; }
    glBasicSprite *Next ()          { return m_next; }

    // Texture handle storage
    void AddTexture (GLuint t) {
        m_textures.push_back (t);
        CheckFrame ();
    }
    void AddTextures (TextureVec& t) {
        m_textures.insert (m_textures.end (), t.begin (), t.end ());            // Append textures to end
        CheckFrame ();
    }
    void SetTexture (GLuint t) {
        m_textures.clear ();
        AddTexture (t);
    }
    void SetTextures (TextureVec& t) {
        m_textures.clear ();
        AddTextures (t);
    }

    // Rendering/animation
    virtual void Render (vmReal *camInv) = 0;           // camInv is the inverted camera matrix
    virtual void Animate ();
        // By default Animate does nothing. Override for types to which it is
        // relevant.
    virtual void AnimateFrame();
        // Restricted animate. Animate frames only. Do not move sprites.
        // (Again by default does nothing).

    // Copying/assignment
    bool SameTypeAs (glBasicSprite& s) {
        return s.Type () == Type ();
    }
    void Copy (glBasicSprite& s) {
        assert (SameTypeAs (s));
        InternalCopy (&s);
        Reorder ();             // (As ZOrder may have changed)
    }
    glBasicSprite& operator= (glBasicSprite& s) {
        Copy (s);
        return *this;
    }
};

////////////////////////////////////////////////////////////////////////////////
// glSprite
//
// An animated sprite

class glSprite : public glBasicSprite {
private:
    vmReal m_frame;
    void SetDefaults ();

protected:
    virtual void InternalCopy (glBasicSprite *s);
    virtual void CheckFrame ();

public:
    vmReal      m_xd, m_yd, m_angled, m_framed;
    bool        m_animLoop;

    glSprite ();
    glSprite (GLuint tex);
    glSprite (TextureVec& tex);

    // Class type identification
    virtual glSpriteType Type ();

    // Getters and setters.
    int FrameCount () const		     { return m_textures.size (); }
    vmReal Frame () const            { return m_frame; }
    void SetFrame (vmReal f)         { m_frame = f; CheckFrame (); }
    // All other fields can be accessed directly.

    // Misc
    bool AnimDone () const {
        return (!m_animLoop) && m_frame >= m_textures.size () - 0.002;
    }

    // Rendering
    virtual void Render (vmReal *camInv);

    // Animation
    virtual void Animate ();
    virtual void AnimateFrame();
};

////////////////////////////////////////////////////////////////////////////////
// glTileMap
//
// A 2D grid of texture tiles for use in 2D sprite drawing.
// Can be manipulated as a glBasicSprite.
//
// Textures are loaded with SetTexture(s) or AddTexture(s). The first texture
// loaded is index 0. The tile map is specified as a 2D grid of integers, each
// corresponding to an index in the textures array.

typedef std::vector<int> IntVec;

class glTileMap : public glBasicSprite {
private:
    int     m_xTiles, m_yTiles;
    IntVec  m_tiles;
    void SetDefaults ();

protected:
    virtual void InternalCopy (glBasicSprite *s);

public:
    bool    m_xRepeat, m_yRepeat;

    // Construction/destruction
    glTileMap ();
    glTileMap (GLuint tex);
    glTileMap (TextureVec& tex);

    // Class type identification
    virtual glSpriteType Type ();

    // Getters/Setters
    int XTiles () const { return m_xTiles; }
    int YTiles () const { return m_yTiles; }
    IntVec& Tiles ()    { return m_tiles; }
    void SetTiles (int xTiles, int yTiles, IntVec& tiles);

    // Rendering
    virtual void Render (vmReal *camInv);
};

////////////////////////////////////////////////////////////////////////////////
// glSpriteEngine
//
// A glTextGrid with added support for sprites
class glSpriteEngine {

    // Sprites
    glSpriteList m_sprites;

    // Camera settings
    vmReal m_width, m_height, m_fov;

    // Working variables
    glBasicSprite *m_cursor;

public:
    vmReal m_camX, m_camY, m_camZ;
    vmReal m_camAngle;

    glSpriteEngine ();
    void SetDefaults ();

    void AddSprite (glBasicSprite *sprite) {
        assert (sprite != NULL);
        sprite->Insert (&m_sprites);
    }

	void DrawSprites(bool inFront);

    vmReal Width () const            { return m_width; }
    vmReal Height () const           { return m_height; }
    vmReal FOV () const              { return m_fov; }
    void SetWidth  (vmReal width)    { if (width != 0)  m_width  = width; }
    void SetHeight (vmReal height)   { if (height != 0) m_height = height; }
    void SetFOV (vmReal fov)         { if (fov >= 1 && fov <= 175)   m_fov = fov; }
    void Animate () {
        for (glBasicSprite *s = m_sprites.m_head; s != NULL; s = s->Next ())
            s->Animate();
    }
    void AnimateFrames() {
        for (glBasicSprite *s = m_sprites.m_head; s != NULL; s = s->Next ())
            s->AnimateFrame();
    }
};

#endif

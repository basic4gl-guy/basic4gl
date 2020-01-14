//---------------------------------------------------------------------------
// Created 24-Jan-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew

#pragma hdrstop

#include "glSpriteEngine.h"
#include "TomTrigBasicLib.h"
#include "Misc.h"
#include "glheaders.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

////////////////////////////////////////////////////////////////////////////////
// glSpriteList

glSpriteList::glSpriteList () {
    m_head = NULL;
}

glSpriteList::~glSpriteList () {

    // Remove items from list before destroying
    while (m_head != NULL)
        m_head->Remove ();
}

////////////////////////////////////////////////////////////////////////////////
// glBasicSprite

glBasicSprite::glBasicSprite () {
    SetBaseDefaults ();
}

glBasicSprite::glBasicSprite (GLuint tex) {
	SetBaseDefaults();
    SetTexture (tex);
}

glBasicSprite::glBasicSprite (TextureVec& tex) {
	SetBaseDefaults();
    SetTextures (tex);
}

glBasicSprite::~glBasicSprite () {
    Remove ();
}

void glBasicSprite::InternalRemove () {
    assert (m_list != NULL);

    // Link prev item to next
    if (m_prev != NULL)
        m_prev->m_next = m_next;
    else
        m_list->m_head = m_next;

    // Link next item to prev
    if (m_next != NULL)
        m_next->m_prev = m_prev;

    // Unlink this item
    m_prev = NULL;
    m_next = NULL;
}

void glBasicSprite::InternalReorder (glBasicSprite *prev, glBasicSprite *next) {

    // Note: List is sorted in DESCENDING zOrder
    bool done = false;
    while (!done) {

        // Need to move forward?
        if (prev != NULL && prev->m_zOrder < m_zOrder) {
            next = prev;
            prev = prev->m_prev;
        }

        // Need to move backward?
        else if (next != NULL && next->m_zOrder > m_zOrder) {
            prev = next;
            next = next->m_next;
        }

        // Otherwise we're done
        else
            done = true;
    }

    // Insert sprite between prev and next
    assert (m_list != NULL);

    // Link prev to us
    if (prev != NULL)
        prev->m_next = this;
    else
        m_list->m_head = this;

    // Link next to us
    if (next != NULL)
        next->m_prev = this;

    // Link us to next & prev
    m_prev = prev;
    m_next = next;
}

void glBasicSprite::Reorder () {

    // Remove from linked list
    if (m_list != NULL) {

        // Save original links
        glBasicSprite *prev = m_prev, *next = m_next;
        InternalRemove ();

        // Move to correct position
        InternalReorder (prev, next);
    }
}

void glBasicSprite::Insert (glSpriteList *list) {
    assert (list != NULL);

    // Remove from previous list (if any)
    Remove ();

    // Insert into new list
    m_list = list;

    // At correct position
    InternalReorder (NULL, list->m_head);
}

void glBasicSprite::Remove () {
    if (m_list != NULL) {
        InternalRemove ();
        m_list = NULL;
    }
}

void glBasicSprite::SetBaseDefaults () {
    m_zOrder        = 0;
    m_list          = NULL;
    m_prev          = NULL;
    m_next          = NULL;
    m_x             = 0;
    m_y             = 0;
    m_xSize         = 32;
    m_ySize         = 32;
    m_scale         = 1;
    m_xCentre       = .5;
    m_yCentre       = .5;
    m_xFlip         = false;
    m_yFlip         = false;
    m_visible       = true;
    m_angle         = 0;
    m_colour [0]    = 1;
    m_colour [1]    = 1;
    m_colour [2]    = 1;
    m_colour [3]    = 1;
    m_parallax      = false;
    m_solid         = false;
    m_srcBlend      = GL_SRC_ALPHA;
    m_dstBlend      = GL_ONE_MINUS_SRC_ALPHA;
}

void glBasicSprite::InternalCopy (glBasicSprite *s) {
    SetTextures (s->m_textures);
    m_x             = s->m_x;
    m_y             = s->m_y;
    m_xSize         = s->m_xSize;
    m_ySize         = s->m_ySize;
    m_scale         = s->m_scale;
    m_xCentre       = s->m_xCentre;
    m_yCentre       = s->m_yCentre;
    m_xFlip         = s->m_xFlip;
    m_yFlip         = s->m_yFlip;
    m_visible       = s->m_visible;
    m_angle         = s->m_angle;
    m_colour [0]    = s->m_colour [0];
    m_colour [1]    = s->m_colour [1];
    m_colour [2]    = s->m_colour [2];
    m_colour [3]    = s->m_colour [3];
    m_zOrder        = s->m_zOrder;
    m_srcBlend      = s->m_srcBlend;
    m_dstBlend      = s->m_dstBlend;
}

void glBasicSprite::Animate () {
    // Default = do nothing.
    ;
}

void glBasicSprite::AnimateFrame() {
    // Default = do nothing.
    ;
}

void glBasicSprite::CheckFrame () {
    // Default = do nothing.
    ;
}

////////////////////////////////////////////////////////////////////////////////
// glSprite

glSprite::glSprite () : glBasicSprite () {
    SetDefaults ();
}

glSprite::glSprite (GLuint tex) : glBasicSprite (tex) {
    SetDefaults ();
}

glSprite::glSprite (TextureVec& tex) : glBasicSprite (tex) {
    SetDefaults ();
}

void glSprite::SetDefaults () {
    m_frame         = 0;
    m_xd            = 0;
    m_yd            = 0;
    m_angled        = 0;
    m_framed        = 0;
    m_animLoop      = true;
}

void glSprite::Render (vmReal *camInv) {

    // Render sprite using OpenGL commands.

    // Assumes that the appropriate projection/translation matrices have been setup,
    // and other OpenGL state (such as texturing & transparency) has been setup
    // accordingly.

    // Sprite must be visible
    if (!m_visible || FrameCount () == 0)
        return;

    // Setup texture and colour
    int frame = m_frame;
    assert (frame >= 0);
    assert (frame < FrameCount ());
    glBindTexture (GL_TEXTURE_2D, m_textures [frame]);
//#if sizeof(m_colour[0])==sizeof(float)
#ifdef vmRealIsFloat
    glColor4fv (m_colour);
//#elif sizeof(m_colour[0])==sizeof(double)
#else
    glColor4dv(m_colour);
#endif

    // Translation, rotation & scaling
    glTranslatef (m_x, m_y, 0);
    if (m_angle != 0)
        glRotatef (m_angle, 0, 0, 1);
    glScalef (  m_xSize * m_scale,
                m_ySize * m_scale,
                1);
    if (m_xCentre != 0 || m_yCentre != 0)
        glTranslatef (-m_xCentre, -m_yCentre, 0);

    vmReal   x1 = 0, x2 = 1,
            y1 = 1, y2 = 0;
    if (m_xFlip) {
        x1 = 1; x2 = 0;
    }
    if (m_yFlip) {
        y1 = 0; y2 = 1;
    }

    // Draw sprite
    glBegin (GL_QUADS);
        glTexCoord2f (x1, y1);  glVertex2f (0, 0);
        glTexCoord2f (x2, y1);  glVertex2f (1, 0);
        glTexCoord2f (x2, y2);  glVertex2f (1, 1);
        glTexCoord2f (x1, y2);  glVertex2f (0, 1);
    glEnd ();
}

void glSprite::InternalCopy (glBasicSprite *s) {
    glBasicSprite::InternalCopy (s);
    glSprite *spr = (glSprite *) s;
    m_frame         = spr->m_frame;
    m_xd            = spr->m_xd;
    m_yd            = spr->m_yd;
    m_angled        = spr->m_angled;
    m_framed        = spr->m_framed;
    m_animLoop      = spr->m_animLoop;
}

glSpriteType glSprite::Type () {
    return SPR_SPRITE;
}

void glSprite::Animate () {
    m_x     += m_xd;            // Simple animation
    m_y     += m_yd;
    m_angle += m_angled;
    AnimateFrame();
}

void glSprite::AnimateFrame() {
    m_frame += m_framed;
    CheckFrame ();
}

void glSprite::CheckFrame () {

    // An obscure bug which I haven't tracked down yet occasionally causes
    // m_frame to go to +INF, which causes an infinite loop.
    // Workaround for now is to reset it to 0 when goes out (sane) range.
    if (m_frame < -10000 || m_frame > 10000)
        m_frame = 0;

    if (!m_textures.empty ()) {
        if (m_animLoop) {

            // Looped animation.
            while (m_frame < 0)
                m_frame += m_textures.size ();
            while (m_frame >= m_textures.size ())
                m_frame -= m_textures.size ();
        }
        else {

            // Clamped animation
            if (m_frame < 0)
                m_frame = 0;
            if (m_frame >= m_textures.size ())
                m_frame = m_textures.size () - 0.001;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// glTileMap

glTileMap::glTileMap () : glBasicSprite () {
    SetDefaults ();
}

glTileMap::glTileMap (GLuint tex) : glBasicSprite (tex) {
    SetDefaults ();
}

glTileMap::glTileMap (TextureVec& tex) : glBasicSprite (tex) {
    SetDefaults ();
}

glSpriteType glTileMap::Type () {
    return SPR_TILEMAP;
}

void glTileMap::SetDefaults () {
    m_xTiles = 0;
    m_yTiles = 0;
    m_xCentre = 0;
    m_yCentre = 0;
    m_xRepeat = true;
    m_yRepeat = true;
    m_solid   = true;
}

void glTileMap::InternalCopy (glBasicSprite *s) {
    glBasicSprite::InternalCopy (s);
    glTileMap *t = (glTileMap *) s;
    SetTiles (t->m_xTiles, t->m_yTiles, t->m_tiles);
}

void glTileMap::SetTiles (int xTiles, int yTiles, IntVec& tiles) {
    assert (xTiles >= 0);
    assert (yTiles >= 0);
    assert (tiles.size () >= xTiles * yTiles);
    m_xTiles = xTiles;
    m_yTiles = yTiles;
    if (xTiles > 0 || yTiles > 0)
        m_tiles = tiles;
    else
        m_tiles.clear ();
}

void glTileMap::Render (vmReal *camInv) {

    // Render tile map using OpenGL commands

    // Assumes that the appropriate projection/translation matrices have been setup,
    // and other OpenGL state (such as texturing & transparency) has been setup
    // accordingly.

    // Sprite must be visible
    if (!m_visible || m_xTiles == 0 || m_yTiles == 0)
        return;

    // Setup colour
#ifdef vmRealIsFloat
    glColor4fv (m_colour);
#else
    glColor4dv (m_colour);
#endif

    // Setup translation, rotation and scaling.
    // Note:    We will setup 2 matrices.
    //          First the OpenGL matrix to perform the operations to drawn elements.
    //          Second is an INVERSE of all the operations. This will be used to
    //          determine where the screen corners map to the tile map, so we
    //          can calculate what range of tiles needs to be drawn.
    vmReal m1 [16], m2 [16];
    glPushMatrix ();

    // Translate to object position
    glTranslatef (m_x, m_y, 0);
    Translate (-m_x, -m_y, 0);
    MatrixTimesMatrix (matrix, camInv, m1);

    // Rotate by angle
    if (m_angle != 0) {
        glRotatef (m_angle, 0, 0, 1);
        RotateZ (-m_angle);
        MatrixTimesMatrix (matrix, m1, m2);
    }
    else
        CopyMatrix (m2, m1);

    // Scale to tile size
    glScalef (  m_xSize * m_scale,
                m_ySize * m_scale,
                1);
    Scale  (    1.0f / (m_xSize * m_scale),
                1.0f / (m_ySize * m_scale),
                1);
    MatrixTimesMatrix (matrix, m2, m1);

    // Centre offset
    if (m_xCentre != 0 || m_yCentre != 0) {
        glTranslatef (-m_xCentre, -m_yCentre, 0);
        Translate (m_xCentre, m_yCentre, 0);
        MatrixTimesMatrix (matrix, m1, m2);
    }
    else
        CopyMatrix (m2, m1);

    // Note: Flip not implemented yet!

    // Now we can use our m1 matrix to translate from camera space into tile space
    // Find range of tiles spanned.
    vmReal camCorner[4][4] = {{0, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 1}, {1, 1, 0, 1} };
//    vmReal camCorner[4][4] = {{.1,.1, 0, 1}, {.9,.1, 0, 1}, {.1,.9, 0, 1}, {.9,.9, 0, 1} };       // DEBUGGING!!!
    int maxX = -1000000, minX = 1000000, maxY = -1000000, minY = 1000000;
    for (int i = 0; i < 4; i++) {
        vmReal tileSpaceCorner [4];
        MatrixTimesVec (m2, camCorner [i], tileSpaceCorner);
        int x = tileSpaceCorner [0], y = tileSpaceCorner [1];
        if (x < minX)   minX = x;
        if (x > maxX)   maxX = x;
        if (y < minY)   minY = y;
        if (y > maxY)   maxY = y;
    }

    minX--;
    minY--;
    if (!m_xRepeat) {

        // Clamp x
        if (minX < 0)           minX = 0;
        if (maxX >= m_xTiles)   maxX = m_xTiles - 1;
    }
    if (!m_yRepeat) {

        // Clamp y
        if (minY < 0)           minY = 0;
        if (maxY >= m_yTiles)   maxY = m_yTiles - 1;
    }
    int startTileX = minX % m_xTiles, startTileY = minY % m_yTiles;
    if (startTileX < 0) startTileX += m_xTiles;
    if (startTileY < 0) startTileY += m_yTiles;

    // Draw tile map
    if (minX <= maxX && minY <= maxY) {
        int tileX = startTileX;
        for (int x = minX; x <= maxX; x++) {
            int offset = tileX * m_yTiles;
            int tileY = startTileY;
            for (int y = minY; y <= maxY; y++) {

                assert (tileY >= 0);
                assert (tileX >= 0);
                assert (tileY < m_yTiles);
                assert (tileX < m_xTiles);

                // Find tile index. Only draw if valid
                int tile = m_tiles [offset + tileY];
                if (tile >= 0 && tile < m_textures.size ()) {

                    // Bind texture
                    glBindTexture (GL_TEXTURE_2D, m_textures [tile]);

                    // Draw tile
                    glBegin (GL_QUADS);
                        glTexCoord2f (0, 1);  glVertex2f (x,   y);
                        glTexCoord2f (1, 1);  glVertex2f (x+1, y);
                        glTexCoord2f (1, 0);  glVertex2f (x+1, y+1);
                        glTexCoord2f (0, 0);  glVertex2f (x,   y+1);
                    glEnd ();
                }
                tileY = (tileY + 1) % m_yTiles;
            }
            tileX = (tileX + 1) % m_xTiles;
        }
    }
    glPopMatrix ();
}

////////////////////////////////////////////////////////////////////////////////
// glSpriteEngine

glSpriteEngine::glSpriteEngine ()
{
    SetDefaults ();
}

void glSpriteEngine::SetDefaults () {
    m_width     = 640;
    m_height    = 480;
    m_camX      = 0;
    m_camY      = 0;
    m_camAngle  = 0;
    m_fov       = 60;
    m_camZ      = 0;
}

void glSpriteEngine::DrawSprites (bool inFront) {

    // Setup OpenGL state.
    // Note: Most of the OpenGL state is already setup for us (in preparation for
    // drawing the text sprites.)
    // We only need to set up an appropriate projection and ensure the model view
    // matrix is set to our requirements.
    glPushAttrib (GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluOrtho2D (0, 1, 1, 0);                    // Top left corner is (0, 0). Bottom right is (1, 1).
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();

    // Apply camera transformations
    vmReal m1 [16], m2 [16];

    // Scale in window dimensions
    glScalef (1.0f / m_width, 1.0f / m_height, 1);
    Scale (m_width, m_height, 1);
    CopyMatrix (m1, matrix);

    // Camera scale and rotation
    glTranslatef (m_width / 2, m_height / 2, 0);
    Translate (-m_width / 2, -m_height / 2, 0);
    MatrixTimesMatrix (matrix, m1, m2);

    glRotatef (-m_camAngle, 0, 0, 1);
    RotateZ (m_camAngle);
    MatrixTimesMatrix (matrix, m2, m1);

    vmReal camInv [16];
    CopyMatrix (camInv, m1);
    
    // Parallax settings
    vmReal dist = m_height / (2 * tan (m_fov * M_PI / 360));         // Effective distance of screen

    if (!inFront)
        m_cursor = m_sprites.m_head;    // Reset cursor to start of list

    // Render sprites.
    // In front:    Render all remaining sprites
    // Behind:      Render all ZOrder >= 0
    while (m_cursor != NULL && (inFront || m_cursor->ZOrder () >= 0)) {

        // Sprite must be visible.
        // Must also be in front of camera (if parallax mode).
        if (m_cursor->m_visible && (!m_cursor->m_parallax || m_cursor->ZOrder () >= m_camZ - dist + 0.0001)) {
            glPushMatrix ();

            // Build rest of camera matrix.
            if (m_cursor->m_parallax) {
                vmReal parallaxFactor = dist / ((m_cursor->ZOrder () - m_camZ) + dist);

                // Update camera matrix
                glScalef (parallaxFactor, parallaxFactor, 1);
                Scale (1.0f / parallaxFactor, 1.0f / parallaxFactor, 1);
                MatrixTimesMatrix (matrix, camInv, m1);

                glTranslatef (-m_width / 2, -m_height / 2, 1);
                Translate (m_width / 2, m_height / 2, 1);
                MatrixTimesMatrix (matrix, m1, m2);
            }
            else {
                glTranslatef (-m_width / 2, -m_height / 2, 1);
                Translate (m_width / 2, m_height / 2, 1);
                MatrixTimesMatrix (matrix, camInv, m2);
            }

            glTranslatef (-m_camX, -m_camY, 0);
            Translate (m_camX, m_camY, 0);
            MatrixTimesMatrix (matrix, m2, m1);

            // Render sprite
            if (m_cursor->m_solid) {
                glDisable (GL_BLEND);
            }
            else {
                glEnable (GL_BLEND);
                glBlendFunc(m_cursor->m_srcBlend, m_cursor->m_dstBlend);
            }
            m_cursor->Render (m1);
            glPopMatrix ();
        }

        // Move on to next
        m_cursor = m_cursor->Next ();
    }

    glPopAttrib ();
}

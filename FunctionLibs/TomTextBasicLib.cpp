//---------------------------------------------------------------------------
// Created 27-Sep-2003: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
#pragma hdrstop

#include "TomTextBasicLib.h"
#include "glSpriteEngine.h"
#include "Basic4GLOpenGLObjects.h"
#include "Content2DManager.h"
#include "OpenGLInput.h"
#include "glheaders.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

// Global variables
static glTextGrid *appText = nullptr;
static glSpriteEngine spriteEngine;
static OpenGLWindowManager* windowManager = nullptr;
static Content2DManager* contentManager = nullptr;
static OpenGLKeyboard* keyboard = nullptr;
static OpenGLMouse* mouse = nullptr;

const int MAX_SPRITES = 100000;

int spriteCount;

////////////////////////////////////////////////////////////////////////////////
// glSpriteStore
//
// A store of glSprites
class glSpriteStore : public vmResourceStore<glBasicSprite *> {
protected:
    virtual void DeleteElement (int index);
public:
    glSpriteStore () : vmResourceStore<glBasicSprite *>(NULL) { ; }
};

void glSpriteStore::DeleteElement (int index) {
    delete Value (index);
}

glSpriteStore sprites;
GLuint boundSprite;

// Pre-run initialisation
static void Init (TomVM& vm) {

    // Text rendering defaults
	contentManager->Reset();
    appText->Resize (25, 40);
    appText->HideCursor ();
    appText->SetColour (MakeColour (220, 220, 255));
    appText->SetTexture (appText->DefaultTexture ());
    appText->Scroll () = true;
    //appWindow->SetDontPaint (false);

    // Sprite engine defaults
    spriteEngine.SetDefaults ();
    boundSprite = 0;
    spriteCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// "input" BASIC command long running function handler

class InputCommandHandler : public IDLL_Basic4GL_LongRunningFunction
{
	TomVM& vm;
	OpenGLKeyboard* keyboard;
	Content2DManager* contentManager;
	glTextGrid* textGrid;

	int left;					// Leftmost cursor position
	bool saveCursor;
	bool isStarted;
	int unsubscribeHandle;

	void ProcessKeys(bool forceRedraw = false);
	void Redraw();

public:
	InputCommandHandler(TomVM& vm, OpenGLKeyboard* keyboard, Content2DManager* contentManager, glTextGrid* textGrid);
	~InputCommandHandler() override;

	// IDLL_Basic4GL_LongRunningFunction
	bool IsPolled() override;
	void Poll() override;
	bool DeleteWhenDone() override;
	void Cancel() override;
};

void InputCommandHandler::ProcessKeys(bool forceRedraw)
{
	// Keyboard input
	char c, sc;
	do {
		c = 0;
		sc = keyboard->GetNextScanKey();
		if (sc == 0)
			c = keyboard->GetNextKey();

		switch (sc) {
		case VK_LEFT:
			if (textGrid->CursorX() > left) {
				textGrid->MoveCursor(textGrid->CursorX() - 1, textGrid->CursorY());
				forceRedraw = true;
			}
			break;
		case VK_RIGHT:
			if (textGrid->CursorX() < textGrid->Cols() - 1) {
				textGrid->MoveCursor(textGrid->CursorX() + 1, textGrid->CursorY());
				forceRedraw = true;
			}
			break;
		case VK_DELETE:
			textGrid->Delete();
			forceRedraw = true;
			break;
		case VK_BACK:
			if (textGrid->CursorX() > left) {
				textGrid->Backspace();
				forceRedraw = true;
			}
			break;
		}

		// Enter key
		// Note: Testing for return character (13) instead of scan key VK_ENTER, because some older Basic4GL programs
		// rely on this behaviour. I.e. they call input immediately after inkey$() returns 13, at which point
		// the scan key buffer contains a VK_ENTER, but the return character has been removed from the character 
		// buffer.
		// (The return character is a special case non-printable character that is added to the character buffer.
		// This is for compatibility with older Basic4GL versions)
		if (c == 13) {
			int lineOffset = textGrid->CursorY() * textGrid->Cols();
			int right = textGrid->Cols();
			char const* chars = textGrid->Chars();
			while (right > left && chars[lineOffset + right - 1] <= ' ')      // Trim spaces from right
				right--;
			while (left < right && chars[lineOffset + left] <= ' ')           // Trim spaces from left
				left++;
			std::string result = "";
			for (int i = left; i < right; i++)
				result = result + chars[lineOffset + i];

			// Restore cursor, perform newline and update screen
			textGrid->NewLine();
			if (!saveCursor) textGrid->HideCursor();
			Redraw();

			// Pass result to Basic4GL and end function
			vm.RegString() = result;
			vm.EndLongRunningFunction();
			return;
		}

		if (c >= ' ') {
			if (textGrid->CursorX() < textGrid->Cols() - 1 && textGrid->Insert()) {
				char buf[2];
				buf[0] = c;
				buf[1] = 0;
				std::string str(buf);
				textGrid->Write(str);
				forceRedraw = true;
			}
		}
	} while (c != 0 || sc != 0);

	if (forceRedraw)
		Redraw();
}

void InputCommandHandler::Redraw()
{
	// Draw text
	contentManager->FullRedraw();
}

InputCommandHandler::InputCommandHandler(TomVM& vm, OpenGLKeyboard* keyboard, Content2DManager* contentManager, glTextGrid* textGrid)
	: vm(vm), keyboard(keyboard), contentManager(contentManager), textGrid(textGrid), isStarted(false), unsubscribeHandle(0)
{
	// Record leftmost cursor position
	left = textGrid->CursorX();
	saveCursor = textGrid->IsCursorShowing();
	textGrid->ShowCursor();

	// Ask to be notified of keyboard events
	unsubscribeHandle = keyboard->SubscribeKeyPressed([this](bool isScanKey)
	{
		ProcessKeys();
	});

	// Clear keyboard buffers before proceeding
	keyboard->ClearKeyBuffers();
}

InputCommandHandler::~InputCommandHandler()
{
	keyboard->UnsubscribeKeyPressed(unsubscribeHandle);
	unsubscribeHandle = 0;
}

bool InputCommandHandler::IsPolled()
{
	// We want to be polled once, so that we can process any already buffered keypresses, and possibly 
	// end the long running function immediately if the buffer contains an ENTER keypress.
	// After that, if the function is still active, we don't need to be polled, and can wait for keypress
	// notifications.
	return !isStarted;
}

void InputCommandHandler::Poll()
{
	isStarted = true;
	ProcessKeys(true);
}

bool InputCommandHandler::DeleteWhenDone()
{
	return true;
}

void InputCommandHandler::Cancel()
{
	// (Nothing to do)
}

////////////////////////////////////////////////////////////////////////////////
// Function wrappers

void WrapTextMode (TomVM& vm)       { contentManager->SetDrawMode(static_cast<TextMode>(vm.GetIntParam(1))); }
void WrapCls (TomVM& vm)            { appText->Clear ();  contentManager->ChangeMade(); }
void WrapPrint (TomVM& vm)          { appText->Write (vm.GetStringParam (1)); contentManager->ChangeMade(); }
void WrapPrintr (TomVM& vm)         { appText->Write (vm.GetStringParam (1)); appText->NewLine (); contentManager->ChangeMade(); }
void WrapPrintr_2 (TomVM& vm)       { appText->NewLine (); contentManager->ChangeMade(); }
void WrapLocate (TomVM& vm)         { appText->MoveCursor (vm.GetIntParam (2), vm.GetIntParam (1)); }
//void WrapSwapBuffers (TomVM& vm)    {
//    appWindow->SwapBuffers ();
//    appWindow->SetDontPaint (false);
//}
void WrapDrawText(TomVM& vm)        { contentManager->Draw(); }
void WrapDrawText2(TomVM& vm)       { contentManager->Draw(vm.GetIntParam(1)); }
void WrapResizeText (TomVM& vm) {
    int rows = vm.GetIntParam (1), cols = vm.GetIntParam (2);
    if (rows < 1)   rows = 1;
    if (rows > 500) rows = 500;
    if (cols < 1)   cols = 1;
    if (cols > 500) cols = 500;
    appText->Resize (rows, cols);
    contentManager->ChangeMade();
}
void WrapTextRows (TomVM& vm)   { vm.Reg ().IntVal () = appText->Rows (); }
void WrapTextCols (TomVM& vm)   { vm.Reg ().IntVal () = appText->Cols (); }
void WrapInput (TomVM& vm)
{
	auto handler = new InputCommandHandler(vm, keyboard, contentManager, appText);
	vm.BeginLongRunningFn(handler);
}
void WrapInkey (TomVM& vm)      {
	char key = keyboard->GetNextKey();
    if (key != 0)   vm.RegString () = key;
    else            vm.RegString () = "";
}
void WrapInScanKey (TomVM& vm)  {
	vm.Reg().IntVal() = keyboard->GetNextScanKey();
}
void WrapClearKeys (TomVM& vm) {
	keyboard->ClearKeyBuffers();
}
void WrapShowCursor (TomVM& vm) { appText->ShowCursor (); contentManager->ChangeMade(); }
void WrapHideCursor (TomVM& vm) { appText->HideCursor (); contentManager->ChangeMade(); }
void WrapKeyDown (TomVM& vm)    {
	std::string s = vm.GetStringParam(1);
	if (s == "")    vm.Reg().IntVal() = 0;
	else            vm.Reg().IntVal() = keyboard->IsKeyDown(s[0]) ? -1 : 0;
}
void WrapScanKeyDown (TomVM& vm) {
	int index = vm.GetIntParam(1);
	if (index < 0 || index > 255)   vm.Reg().IntVal() = 0;
	else                            vm.Reg().IntVal() = keyboard->IsScanKeyDown(index) ? -1 : 0;
}
void WrapCharAt (TomVM& vm) {
    char c = appText->TextAt (vm.GetIntParam (2), vm.GetIntParam (1));
    if (c == 0)     vm.RegString () = "";
    else            vm.RegString () = c;
}
void WrapColour (TomVM& vm) {
    appText->SetColour (MakeColour (vm.GetIntParam (3), vm.GetIntParam (2), vm.GetIntParam (1)));
}
void WrapFont (TomVM& vm) {
    appText->SetTexture (vm.GetIntParam (1));
}
void WrapDefaultFont (TomVM& vm) {
    vm.Reg ().IntVal () = appText->DefaultTexture ();
}
void WrapMouseX (TomVM& vm) {
	vm.Reg().RealVal() = static_cast<vmReal>(mouse->GetX()) / windowManager->GetWindowWidth();
}
void WrapMouseY (TomVM& vm) {
	vm.Reg().RealVal() = static_cast<vmReal>(mouse->GetY()) / windowManager->GetWindowHeight();
}
void WrapMouseXD (TomVM& vm) {
	vm.Reg().RealVal() = static_cast<vmReal>(mouse->GetXD()) / windowManager->GetWindowWidth() * 2;
}
void WrapMouseYD (TomVM& vm) {
	vm.Reg().RealVal() = static_cast<vmReal>(mouse->GetYD()) / windowManager->GetWindowHeight() * 2;
}
void WrapMouseButton (TomVM& vm) {
	vm.Reg().IntVal() = mouse->GetButton(vm.GetIntParam(1)) ? -1 : 0;
}
void WrapMouseWheel (TomVM& vm) {
	vm.Reg().IntVal() = mouse->GetWheelDelta();
}

void WrapCursorCol(TomVM& vm) {
    vm.Reg().IntVal() = appText->CursorX();
}

void WrapCursorRow(TomVM& vm) {
    vm.Reg().IntVal() = appText->CursorY();
}

// Sprite functions
glSprite *NewSprite (TomVM& vm) {
    if (spriteCount < MAX_SPRITES)
    {
        // Create sprite object
        glSprite *sprite = new glSprite;
        spriteCount++;

        // Add to sprite engine (so we can draw it)
        spriteEngine.AddSprite(sprite);

        // Add to store (so we can track it), and return index to VM
        boundSprite = sprites.Alloc(sprite);
        vm.Reg ().IntVal () = boundSprite;
        return sprite;
    }
    else {
        vm.Reg().IntVal() = 0;
        return NULL;
    }
}
glTileMap *NewTileMap (TomVM& vm) {

    if (spriteCount < MAX_SPRITES)
    {
        // Create tile map object
        glTileMap *tileMap = new glTileMap;
        spriteCount++;

        // Add to sprite engine (so we can draw it)
		spriteEngine.AddSprite(tileMap);

        // Add to store (so we can track it), and return index to VM
        boundSprite = sprites.Alloc(tileMap);
        vm.Reg ().IntVal () = boundSprite;
        return tileMap;
    }
    else {
        vm.Reg().IntVal() = 0;
        return NULL;
    }
}

void WrapNewSprite (TomVM& vm) {

    // Allocate sprite
    NewSprite (vm);
    contentManager->ChangeMade();
}
void WrapNewSprite_2 (TomVM& vm) {

    // Allocate sprite, and set single texture
    glSprite *sprite = NewSprite(vm);
    if (sprite != NULL)
        sprite->SetTexture(vm.GetIntParam(1));
    contentManager->ChangeMade();
}
bool GetTextures (TomVM& vm, int paramIndex, TextureVec& dest) {

    // Read in texture array and convert to vector (for storage in sprite)
    GLuint frames [65536];
    int size = ArrayDimensionSize (vm.Data (), vm.GetIntParam (paramIndex), 0);
    if (size < 1 || size > 65536) {
        vm.FunctionError ("Texture array size must be 1-65536");
        return false;
    }
    ReadArray (vm.Data (), vm.GetIntParam (paramIndex), vmValType (VTP_INT, 1, 1, true), frames, size);

    // Convert to vector
    dest.clear ();
    for (int i = 0; i < size; i++)
        dest.push_back (frames [i]);

    return true;
}
void WrapNewSprite_3 (TomVM& vm) {

    // Allocate sprite and set an array of textures
    // Read textures
    TextureVec textures;
    if (!GetTextures (vm, 1, textures))
        return;

    // Allocate sprite
    glBasicSprite *sprite = NewSprite(vm);

    // Set textures
    if (sprite != NULL)
        sprite->SetTextures(textures);
    contentManager->ChangeMade();
}
void WrapNewTileMap (TomVM& vm) {

    // Allocate sprite
    NewTileMap (vm);
    contentManager->ChangeMade();
}
void WrapNewTileMap_2 (TomVM& vm) {

    // Allocate sprite, and set single texture
    glTileMap* sprite = NewTileMap(vm);
    if (sprite != NULL)
        sprite->SetTexture (vm.GetIntParam(1));
    contentManager->ChangeMade();
}
void WrapNewTileMap_3 (TomVM& vm) {

    // Allocate sprite and set an array of textures
    // Read textures
    TextureVec textures;
    if (!GetTextures(vm, 1, textures))
        return;

    // Allocate sprite
    glBasicSprite *sprite = NewTileMap (vm);

    // Set textures
    if (sprite != NULL)
        sprite->SetTextures (textures);
    contentManager->ChangeMade();
}
inline bool IsBasicSprite (int index) {
    return sprites.IndexStored (index);
}
inline glBasicSprite* BasicSprite (int index) {
    assert (IsBasicSprite (index));
    return sprites.Value (index);
}
inline bool IsSprite (int index) {
    return IsBasicSprite (index)
        && BasicSprite (index)->Type() == SPR_SPRITE;
}
inline glSprite* Sprite (int index) {
    assert (IsSprite (index));
    return (glSprite *) BasicSprite (index);
}
inline bool IsTileMap (int index) {
    return IsBasicSprite (index)
        && BasicSprite (index)->Type () == SPR_TILEMAP;
}
inline glTileMap* TileMap (int index) {
    assert (IsTileMap (index));
    return (glTileMap *) BasicSprite (index);
}
void WrapDeleteSprite (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsBasicSprite (index)) {
        sprites.Free (index);
        spriteCount--;
        contentManager->ChangeMade();
    }
}
void WrapBindSprite (TomVM& vm) {
    boundSprite = vm.GetIntParam (1);
}
void WrapSprSetTexture (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->SetTexture (vm.GetIntParam (1));
        contentManager->ChangeMade();
    }
}
void WrapSprSetTextures (TomVM& vm) {
    TextureVec textures;
    if (sprites.IndexStored (boundSprite) && GetTextures (vm, 1, textures)) {
        sprites.Value (boundSprite)->SetTextures (textures);
        contentManager->ChangeMade();
    }
}
void WrapSprAddTexture (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->AddTexture (vm.GetIntParam (1));
        contentManager->ChangeMade();
    }
}
void WrapSprAddTextures (TomVM& vm) {
    TextureVec textures;
    if (sprites.IndexStored (boundSprite) && GetTextures (vm, 1, textures)) {
        sprites.Value (boundSprite)->AddTextures (textures);
        contentManager->ChangeMade();
    }
}
void WrapSprSetFrame (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        Sprite (boundSprite)->SetFrame (vm.GetRealParam (1));
        contentManager->ChangeMade();
    }
}
void WrapSprSetX (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_x = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetY (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_y = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
float vec [4];
void ReadVec (TomVM& vm, int paramIndex) {

    // Read data
    vec [0] = 0;
    vec [1] = 0;
    vec [2] = 0;
    vec [3] = 1;
    ReadArray (vm.Data (), vm.GetIntParam (paramIndex), vmValType (VTP_REAL, 1, 1, true), vec, 4);
}
void WrapSprSetPos (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        ReadVec (vm, 1);
        sprites.Value (boundSprite)->m_x = vec [0];
        sprites.Value (boundSprite)->m_y = vec [1];
        contentManager->ChangeMade();
    }
}
void WrapSprSetPos_2 (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_x = vm.GetRealParam (2);
        sprites.Value (boundSprite)->m_y = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetZOrder (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->SetZOrder (vm.GetRealParam (1));
        contentManager->ChangeMade();
    }
}
void WrapSprSetXSize (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_xSize = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetYSize (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_ySize = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetSize (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        ReadVec (vm, 1);
        sprites.Value (boundSprite)->m_xSize = vec [0];
        sprites.Value (boundSprite)->m_ySize = vec [1];
        contentManager->ChangeMade();
    }
}
void WrapSprSetSize_2 (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_xSize = vm.GetRealParam (2);
        sprites.Value (boundSprite)->m_ySize = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetScale (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_scale = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetXCentre (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_xCentre = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetYCentre (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_yCentre = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetXFlip (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_xFlip = vm.GetIntParam (1) != 0;
        contentManager->ChangeMade();
    }
}
void WrapSprSetYFlip (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_yFlip = vm.GetIntParam (1) != 0;
        contentManager->ChangeMade();
    }
}
void WrapSprSetVisible (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_visible = vm.GetIntParam (1) != 0;
        contentManager->ChangeMade();
    }
}
void WrapSprSetAngle (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_angle = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetColour (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        ReadVec (vm, 1);
        int size = ArrayDimensionSize (vm.Data (), vm.GetIntParam (1), 0);
        if (size > 4)
            size = 4;
        memcpy (sprites.Value (boundSprite)->m_colour, vec, sizeof (vec [0]) * size);
        contentManager->ChangeMade();
    }
}
void WrapSprSetColour_2 (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_colour [0] = vm.GetRealParam (3);
        sprites.Value (boundSprite)->m_colour [1] = vm.GetRealParam (2);
        sprites.Value (boundSprite)->m_colour [2] = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetColour_3 (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_colour [0] = vm.GetRealParam (4);
        sprites.Value (boundSprite)->m_colour [1] = vm.GetRealParam (3);
        sprites.Value (boundSprite)->m_colour [2] = vm.GetRealParam (2);
        sprites.Value (boundSprite)->m_colour [3] = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetAlpha (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_colour [3] = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetParallax (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_parallax = vm.GetIntParam (1) != 0;
        contentManager->ChangeMade();
    }
}
void WrapSprSetSolid (TomVM& vm) {
    if (sprites.IndexStored (boundSprite)) {
        sprites.Value (boundSprite)->m_solid = vm.GetIntParam (1) != 0;
        contentManager->ChangeMade();
    }
}
void WrapResizeSpriteArea (TomVM& vm) {
    int width = vm.GetRealParam (2), height = vm.GetRealParam (1);
    if (width <= 0 || height <= 0) {
        vm.FunctionError ("Width and height must both be greater than 0");
        return;
    }
	spriteEngine.SetWidth(width);
	spriteEngine.SetHeight(height);
}
void WrapSpriteAreaWidth (TomVM& vm) {
	vm.Reg().RealVal() = spriteEngine.Width();
}
void WrapSpriteAreaHeight (TomVM& vm) {
	vm.Reg().RealVal() = spriteEngine.Height();
}
void WrapSprFrame (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            Sprite (boundSprite)->Frame () : 0;
}
void WrapSprX (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_x : 0;
}
void WrapSprY (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_y : 0;
}
void WrapSprPos (TomVM& vm) {
    vmReal result [2] = {0, 0};
    if (sprites.IndexStored (boundSprite)) {
        result [0] = sprites.Value (boundSprite)->m_x;
        result [1] = sprites.Value (boundSprite)->m_y;
    }
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 2, result);
}
void WrapSprZOrder (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->ZOrder () : 0;
}
void WrapSprXSize (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_xSize : 0;
}
void WrapSprYSize (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_ySize : 0;
}
void WrapSprScale (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_scale : 0;
}
void WrapSprXCentre (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_xCentre : 0;
}
void WrapSprYCentre (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_yCentre : 0;
}
void WrapSprXFlip (TomVM& vm) {
	vm.Reg().IntVal() = sprites.IndexStored(boundSprite) ?
                            (sprites.Value (boundSprite)->m_xFlip ? -1 : 0) : 0;
}
void WrapSprYFlip (TomVM& vm) {
	vm.Reg().IntVal() = sprites.IndexStored(boundSprite) ?
                            (sprites.Value (boundSprite)->m_yFlip ? -1 : 0) : 0;
}
void WrapSprVisible (TomVM& vm) {
	vm.Reg().IntVal() = sprites.IndexStored(boundSprite) ?
                            (sprites.Value (boundSprite)->m_visible ? -1 : 0) : 0;
}
void WrapSprAngle (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_angle : 0;
}
void WrapSprColour (TomVM& vm) {
    float result [4] = {0, 0, 0, 0};
    if (sprites.IndexStored (boundSprite))
        memcpy (result, sprites.Value (boundSprite)->m_colour, sizeof (result [0]) * 4);
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 4, result);
}
void WrapSprAlpha (TomVM& vm) {
    vm.Reg ().RealVal () =  sprites.IndexStored (boundSprite) ?
                            sprites.Value (boundSprite)->m_colour [3] : 0;
}
void WrapSprParallax (TomVM& vm) {
	vm.Reg().IntVal() = sprites.IndexStored(boundSprite) ?
                            (sprites.Value (boundSprite)->m_parallax ? -1 : 0) : 0;
}
void WrapSprSolid (TomVM& vm) {
	vm.Reg().IntVal() = sprites.IndexStored(boundSprite) ?
                            (sprites.Value (boundSprite)->m_solid ? -1 : 0) : 0;
}
void WrapSprLeft (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        glSprite *sprite = Sprite (boundSprite);
        vm.Reg ().RealVal () = sprite->m_x + -sprite->m_xCentre * (sprite->m_xSize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprRight (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        glSprite *sprite = Sprite (boundSprite);
        vm.Reg ().RealVal () = sprite->m_x + (1 - sprite->m_xCentre) * (sprite->m_xSize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprTop (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        glSprite *sprite = Sprite (boundSprite);
        vm.Reg ().RealVal () = sprite->m_y + -sprite->m_yCentre * (sprite->m_ySize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprBottom (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        glSprite *sprite = Sprite (boundSprite);
        vm.Reg ().RealVal () = sprite->m_y + (1 - sprite->m_yCentre) * (sprite->m_ySize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprXVel (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            Sprite (boundSprite)->m_xd : 0;
}
void WrapSprYVel (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            Sprite (boundSprite)->m_yd : 0;
}
void WrapSprVel (TomVM& vm) {
    vmReal result [2] = {0, 0};
    if (IsSprite (boundSprite)) {
        result [0] = Sprite (boundSprite)->m_xd;
        result [1] = Sprite (boundSprite)->m_yd;
    }
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 2, result);
}
void WrapSprSpin (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            Sprite (boundSprite)->m_angled : 0;
}
void WrapSprAnimSpeed (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            Sprite (boundSprite)->m_framed : 0;
}
void WrapSprAnimLoop (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            (Sprite (boundSprite)->m_animLoop ? -1 : 0) : 0;
}
void WrapSprAnimDone (TomVM& vm) {
    vm.Reg ().RealVal () =  IsSprite (boundSprite) ?
                            (Sprite (boundSprite)->AnimDone () ? -1 : 0) : 0;
}
void WrapSprFrame_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            Sprite (index)->Frame () : 0;
}
void WrapSprX_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_x : 0;
}
void WrapSprY_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_y : 0;
}
void WrapSprPos_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vmReal result [2] = {0, 0};
    if (sprites.IndexStored (index)) {
        result [0] = sprites.Value (index)->m_x;
        result [1] = sprites.Value (index)->m_y;
    }
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 2, result);
}
void WrapSprZOrder_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->ZOrder () : 0;
}
void WrapSprXSize_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_xSize : 0;
}
void WrapSprYSize_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_ySize : 0;
}
void WrapSprScale_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_scale : 0;
}
void WrapSprXCentre_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_xCentre : 0;
}
void WrapSprYCentre_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_yCentre : 0;
}
void WrapSprXFlip_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
	vm.Reg().IntVal() = sprites.IndexStored(index) ?
                            (sprites.Value (index)->m_xFlip ? -1 : 0) : 0;
}
void WrapSprYFlip_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
	vm.Reg().IntVal() = sprites.IndexStored(index) ?
                            (sprites.Value (index)->m_yFlip ? -1 : 0) : 0;
}
void WrapSprVisible_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () =  sprites.IndexStored (index) ?
                            (sprites.Value (index)->m_visible ? -1 : 0) : 0;
}
void WrapSprAngle_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_angle : 0;
}
void WrapSprColour_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    float result [4] = {0, 0, 0, 0};
    if (sprites.IndexStored (index))
        memcpy (result, sprites.Value (index)->m_colour, sizeof (result [0]) * 4);
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 4, result);
}
void WrapSprAlpha_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  sprites.IndexStored (index) ?
                            sprites.Value (index)->m_colour [3] : 0;
}
void WrapSprParallax_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
	vm.Reg().IntVal() = sprites.IndexStored(index) ?
                            (sprites.Value (index)->m_parallax ? -1 : 0) : 0;
}
void WrapSprSolid_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
	vm.Reg().IntVal() = sprites.IndexStored(index) ?
                            (sprites.Value (index)->m_solid ? -1 : 0) : 0;
}
void WrapSprLeft_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsSprite (index)) {
        glSprite *sprite = Sprite (index);
        vm.Reg ().RealVal () = sprite->m_x + -sprite->m_xCentre * (sprite->m_xSize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprRight_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsSprite (index)) {
        glSprite *sprite = Sprite (index);
        vm.Reg ().RealVal () = sprite->m_x + (1 - sprite->m_xCentre) * (sprite->m_xSize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprTop_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsSprite (index)) {
        glSprite *sprite = Sprite (index);
        vm.Reg ().RealVal () = sprite->m_y + -sprite->m_yCentre * (sprite->m_ySize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprBottom_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsSprite (index)) {
        glSprite *sprite = Sprite (index);
        vm.Reg ().RealVal () = sprite->m_y + (1 - sprite->m_yCentre) * (sprite->m_ySize * sprite->m_scale);
    }
    else
        vm.Reg ().RealVal () = 0;
}
void WrapSprXVel_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            Sprite (index)->m_xd : 0;
}
void WrapSprYVel_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            Sprite (index)->m_yd : 0;
}
void WrapSprVel_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vmReal result [2] = {0, 0};
    if (IsSprite (index)) {
        result [0] = Sprite (index)->m_xd;
        result [1] = Sprite (index)->m_yd;
    }
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 2, result);
}
void WrapSprSpin_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            Sprite (index)->m_angled : 0;
}
void WrapSprAnimSpeed_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            Sprite (index)->m_framed : 0;
}
void WrapSprAnimLoop_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            (Sprite (index)->m_animLoop ? -1 : 0) : 0;
}
void WrapSprAnimDone_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().RealVal () =  IsSprite (index) ?
                            (Sprite (index)->AnimDone () ? -1 : 0) : 0;
}
void WrapSprSetXVel (TomVM& vm) {
    if (IsSprite (boundSprite))
        Sprite (boundSprite)->m_xd = vm.GetRealParam (1);
}
void WrapSprSetYVel (TomVM& vm) {
    if (IsSprite (boundSprite))
        Sprite (boundSprite)->m_yd = vm.GetRealParam (1);
}
void WrapSprSetVel (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        ReadVec (vm, 1);
        Sprite (boundSprite)->m_xd = vec [0];
        Sprite (boundSprite)->m_yd = vec [1];
        contentManager->ChangeMade();
    }
}
void WrapSprSetVel_2 (TomVM& vm) {
    if (IsSprite (boundSprite)) {
        Sprite (boundSprite)->m_xd = vm.GetRealParam (2);
        Sprite (boundSprite)->m_yd = vm.GetRealParam (1);
        contentManager->ChangeMade();
    }
}
void WrapSprSetSpin (TomVM& vm) {
    if (IsSprite (boundSprite))
        Sprite (boundSprite)->m_angled = vm.GetRealParam (1);
}
void WrapSprSetAnimSpeed (TomVM& vm) {
    if (IsSprite (boundSprite))
        Sprite (boundSprite)->m_framed = vm.GetRealParam (1);
}
void WrapSprSetAnimLoop (TomVM& vm) {
    if (IsSprite (boundSprite))
        Sprite (boundSprite)->m_animLoop = vm.GetIntParam (1) != 0;
}
void WrapAnimateSprites (TomVM& vm) {
	spriteEngine.Animate();
    contentManager->ChangeMade();
}
void WrapAnimateSpriteFrames(TomVM& vm) {
	spriteEngine.AnimateFrames();
    contentManager->ChangeMade();
}
void WrapCopySprite (TomVM& vm) {

    // CopySprite (dest, source)
    // Copies sprite "source" to sprite "dest"
    // Sprites must be the same type. I.e. you can't copy a standard sprite
    // to a tile map or vice versa.
    int index = vm.GetIntParam (1);
    if (    IsBasicSprite (boundSprite) && IsBasicSprite (index)
        &&  BasicSprite (boundSprite)->Type () == BasicSprite (index)->Type ()) {
        if (IsSprite (boundSprite))
            Sprite (boundSprite)->Copy (*Sprite (index));
        else if (IsTileMap (boundSprite))
            TileMap (boundSprite)->Copy (*TileMap (index));
        contentManager->ChangeMade();
    }
}
void WrapSprType (TomVM& vm) {
    vm.Reg ().IntVal () =   IsBasicSprite (boundSprite) ?
                            BasicSprite (boundSprite)->Type () : SPR_INVALID;
}
void WrapSprXTiles (TomVM& vm) {
    vm.Reg ().IntVal () =   IsTileMap (boundSprite ) ?
                            TileMap (boundSprite)->XTiles () : 0;
}
void WrapSprYTiles (TomVM& vm) {
    vm.Reg ().IntVal () =   IsTileMap (boundSprite ) ?
                            TileMap (boundSprite)->YTiles () : 0;
}
void WrapSprType_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () =   IsBasicSprite (index) ?
                            BasicSprite (index)->Type () : SPR_INVALID;
}
void WrapSprXTiles_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () =   IsTileMap (index ) ?
                            TileMap (index)->XTiles () : 0;
}
void WrapSprYTiles_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    vm.Reg ().IntVal () =   IsTileMap (index ) ?
                            TileMap (index)->YTiles () : 0;
}
void GetTiles (TomVM& vm, int paramIndex, int& xSize, int& ySize, IntVec& dest) {

    // Read in texture array and convert to vector (for storage in sprite)
    int index = vm.GetIntParam (paramIndex);
    xSize = ArrayDimensionSize (vm.Data (), index, 0);
    ySize = ArrayDimensionSize (vm.Data (), index, 1);

    // Size must be valid and add up to 1 million or less tiles
    dest.clear ();
    if (xSize > 0 && ySize > 0) {

        // Read data into temp buffer
        int *buffer = new int [xSize * ySize];
        ReadArray (vm.Data (), index, vmValType (VTP_INT, 2, 1, true), buffer, xSize * ySize);

        // Convert to vector
        for (int i = 0; i < xSize * ySize; i++)
            dest.push_back (buffer [i]);

        // Free temp buffer
        delete[] buffer;
    }
}
void WrapSprSetTiles (TomVM& vm) {

    if (IsTileMap (boundSprite)) {

        // Get tiles from array param
        IntVec tiles;
        int xSize, ySize;
        GetTiles (vm, 1, xSize, ySize, tiles);

        // Set
        TileMap (boundSprite)->SetTiles (xSize, ySize, tiles);
        contentManager->ChangeMade();
    }
}
void WrapSprSetXRepeat (TomVM& vm) {
    if (IsTileMap (boundSprite)) {
        TileMap (boundSprite)->m_xRepeat = vm.GetIntParam (1) != 0 ? 1 : 0;
        contentManager->ChangeMade();
    }
}
void WrapSprSetYRepeat (TomVM& vm) {
    if (IsTileMap (boundSprite)) {
        TileMap (boundSprite)->m_yRepeat = vm.GetIntParam (1) != 0 ? 1 : 0;
        contentManager->ChangeMade();
    }
}
void WrapSprXRepeat (TomVM& vm) {
    if (IsTileMap (boundSprite))
        vm.Reg ().IntVal () = TileMap (boundSprite)->m_xRepeat ? -1 : 0;
    else
        vm.Reg ().IntVal () = 0;
}
void WrapSprYRepeat (TomVM& vm) {
    if (IsTileMap (boundSprite))
        vm.Reg ().IntVal () = TileMap (boundSprite)->m_yRepeat ? -1 : 0;
    else
        vm.Reg ().IntVal () = 0;
}
void WrapSprXRepeat_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsTileMap (index))
        vm.Reg ().IntVal () = TileMap (index)->m_xRepeat ? -1 : 0;
    else
        vm.Reg ().IntVal () = 0;
}
void WrapSprYRepeat_2 (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (IsTileMap (index))
        vm.Reg ().IntVal () = TileMap (index)->m_yRepeat ? -1 : 0;
    else
        vm.Reg ().IntVal () = 0;
}
void WrapSprCameraSetX (TomVM& vm) {
    spriteEngine.m_camX = vm.GetRealParam (1);
    contentManager->ChangeMade();
}
void WrapSprCameraSetY (TomVM& vm) {
    spriteEngine.m_camY = vm.GetRealParam (1);
    contentManager->ChangeMade();
}
void WrapSprCameraSetZ (TomVM& vm) {
    spriteEngine.m_camZ = vm.GetRealParam (1);
    contentManager->ChangeMade();
}
void WrapSprCameraSetPos (TomVM& vm) {
    spriteEngine.m_camX = vm.GetRealParam (2);
    spriteEngine.m_camY = vm.GetRealParam (1);
    contentManager->ChangeMade();
}
void WrapSprCameraSetPos_2 (TomVM& vm) {
    ReadVec (vm, 1);
    spriteEngine.m_camX = vec [0];
    spriteEngine.m_camY = vec [1];
    contentManager->ChangeMade();
}
void WrapSprCameraSetAngle (TomVM& vm) {
    spriteEngine.m_camAngle = vm.GetRealParam (1);
    contentManager->ChangeMade();
}
void WrapSprCameraX (TomVM& vm) {
    vm.Reg ().RealVal () = spriteEngine.m_camX;
}
void WrapSprCameraY (TomVM& vm) {
    vm.Reg ().RealVal () = spriteEngine.m_camY;
}
void WrapSprCameraZ (TomVM& vm) {
    vm.Reg ().RealVal () = spriteEngine.m_camZ;
}
void WrapSprCameraPos (TomVM& vm) {
    vmReal result [2] = {0, 0};
    result [0] = spriteEngine.m_camX;
    result [1] = spriteEngine.m_camY;
    vm.Reg ().IntVal () = FillTempRealArray (vm.Data (), vm.DataTypes (), 2, result);
}
void WrapSprCameraAngle (TomVM& vm) {
    vm.Reg ().RealVal () = spriteEngine.m_camAngle;
}
void WrapSprCameraFOV (TomVM& vm) {
    vm.Reg ().RealVal () = spriteEngine.FOV ();
}
void WrapSprCameraSetFOV (TomVM& vm) {
    spriteEngine.SetFOV (vm.GetRealParam (1));
    contentManager->ChangeMade();
}
void WrapClearLine (TomVM& vm) {
    appText->ClearLine ();
    contentManager->ChangeMade();
}
void WrapClearRegion (TomVM& vm) {
    appText->ClearRegion (vm.GetIntParam (4), vm.GetIntParam (3), vm.GetIntParam (2), vm.GetIntParam (1));
    contentManager->ChangeMade();
}

void WrapTextScroll (TomVM& vm) {
    vm.Reg ().IntVal () = appText->Scroll ();
}

void WrapSetTextScroll (TomVM& vm) {
    appText->Scroll () = vm.GetIntParam (1);
}

void WrapClearSprites(TomVM& vm) {
    sprites.Clear();
    spriteCount = 0;
    contentManager->ChangeMade();
}

void WrapSprSetBlendFunc(TomVM& vm) {
    if (sprites.IndexStored(boundSprite)) {
        glBasicSprite *sprite = sprites.Value(boundSprite);
        sprite->m_srcBlend = vm.GetIntParam(2);
        sprite->m_dstBlend = vm.GetIntParam(1);
    }
}

void DLLFUNC WrapSpriteCount(TomVM& vm) {
    vm.Reg().IntVal() = spriteCount;
}

////////////////////////////////////////////////////////////////////////////////
// Interface for DLLs

class TextAdapter : public IB4GLOpenGLText {
public:
    // IB4GLText
    virtual void DLLFUNC Print(const char *text, bool newline);
    virtual void DLLFUNC Locate(int x, int y);
    virtual void DLLFUNC Cls();
    virtual void DLLFUNC ClearRegion(int x1, int y1, int x2, int y2);
    virtual int DLLFUNC TextRows();
    virtual int DLLFUNC TextCols();
    virtual void DLLFUNC ResizeText(int cols, int rows);
    virtual void DLLFUNC SetTextScroll(bool scroll);
    virtual bool DLLFUNC TextScroll();
    virtual void DLLFUNC DrawText();
    virtual char DLLFUNC CharAt(int x, int y);

    // IB4GLOpenGLText
    virtual void DLLFUNC Font(GLuint fontTexture);
    virtual GLuint DLLFUNC DefaultFont();
    virtual void DLLFUNC SetTextMode(TextMode mode);
    virtual void DLLFUNC Color(byte red, byte green, byte blue);
} textAdapter;

void DLLFUNC TextAdapter::Print(const char *text, bool newline) {
    appText->Write(text);
    if (newline)
        appText->NewLine();
    contentManager->ChangeMade();
}
void DLLFUNC TextAdapter::Locate(int x, int y) {
    appText->MoveCursor(x, y);
}
void DLLFUNC TextAdapter::Cls() {
    appText->Clear();
    contentManager->ChangeMade();
}
void DLLFUNC TextAdapter::ClearRegion(int x1, int y1, int x2, int y2) {
    appText->ClearRegion(x1, y1, x2, y2);
    contentManager->ChangeMade();
}
int DLLFUNC TextAdapter::TextRows() {
    return appText->Rows();
}
int DLLFUNC TextAdapter::TextCols() {
    return appText->Cols();
}
void DLLFUNC TextAdapter::ResizeText(int cols, int rows) {
    if (rows < 1)   rows = 1;
    if (rows > 500) rows = 500;
    if (cols < 1)   cols = 1;
    if (cols > 500) cols = 500;
    appText->Resize (rows, cols);
    contentManager->ChangeMade();
}
void DLLFUNC TextAdapter::SetTextScroll(bool scroll) {
    appText->Scroll() = scroll;
}
bool DLLFUNC TextAdapter::TextScroll() {
    return appText->Scroll();
}
void DLLFUNC TextAdapter::DrawText() {
    contentManager->Draw();
}
char DLLFUNC TextAdapter::CharAt(int x, int y) {
    return appText->TextAt(x, y);
}
void DLLFUNC TextAdapter::Font(GLuint fontTexture) {
    appText->SetTexture(fontTexture);
}
GLuint DLLFUNC TextAdapter::DefaultFont() {
    return appText->DefaultTexture();
}
void DLLFUNC TextAdapter::SetTextMode(TextMode mode) {
	contentManager->SetDrawMode(static_cast<TextMode>(mode));
}
void DLLFUNC TextAdapter::Color(byte red, byte green, byte blue) {
    appText->SetColour(MakeColour(red, green, blue));
}

////////////////////////////////////////////////////////////////////////////////
// Intialisation

void InitTomTextBasicLib(TomBasicCompiler& comp, glTextGrid *text, OpenGLWindowManager* windowManager, Content2DManager* contentManager, OpenGLKeyboard* keyboard, OpenGLMouse* mouse) {

    // Register interfaces for plugin DLLs
    comp.Plugins().RegisterInterface((IB4GLText *) &textAdapter, "IB4GLText", 1, 0, NULL);        
    comp.Plugins().RegisterInterface((IB4GLOpenGLText *) &textAdapter, "IB4GLOpenGLText", 1, 0, NULL);        

    // Register resources
    comp.VM().AddResources (sprites);

    // Store pointers to window and text objects
    appText				= text;
	::windowManager		= windowManager;
	::contentManager	= contentManager;
	::keyboard			= keyboard;
	::mouse				= mouse;

	// Upload charset texture after window created
	contentManager->GetWindowManager().SubscribeWindowCreated(
		[text]() {
			text->UploadCharsetTexture();
		});
	contentManager->GetWindowManager().SubscribeBeforeDestroyWindow(
		[text]() {
			text->DeleteCharsetTexture();
		});

	// Register 2D content layers
	contentManager->AddContent( 0.0f, DRAW_TEXT, []() { appText->Draw(); });
	contentManager->AddContent(-1.0f, DRAW_SPRITES_BEHIND, []() { spriteEngine.DrawSprites(false); });
	contentManager->AddContent( 1.0f, DRAW_SPRITES_INFRONT, []() { spriteEngine.DrawSprites(true); });

    // Register initialisation functions
    comp.VM().AddInitFunc (Init);

    // Register constants
    comp.AddConstant("TEXT_SIMPLE", TEXT_SIMPLE);
    comp.AddConstant("TEXT_BUFFERED", TEXT_BUFFERED);
    comp.AddConstant("TEXT_OVERLAID", TEXT_OVERLAID);
    comp.AddConstant("DRAW_TEXT", DRAW_TEXT);
    comp.AddConstant("DRAW_SPRITES_BEHIND", DRAW_SPRITES_BEHIND);
    comp.AddConstant("DRAW_SPRITES_INFRONT", DRAW_SPRITES_INFRONT);
    comp.AddConstant("DRAW_SPRITES", DRAW_SPRITES);
    comp.AddConstant("VK_LBUTTON", VK_LBUTTON);             // Virtual key codes
    comp.AddConstant("VK_RBUTTON", VK_RBUTTON);
    comp.AddConstant("VK_CANCEL", VK_CANCEL);
    comp.AddConstant("VK_MBUTTON", VK_MBUTTON);
//    comp.AddConstant("VK_XBUTTON1", VK_XBUTTON1);
//    comp.AddConstant("VK_XBUTTON2", VK_XBUTTON2);
    comp.AddConstant("VK_BACK", VK_BACK);
    comp.AddConstant("VK_TAB", VK_TAB);
    comp.AddConstant("VK_CLEAR", VK_CLEAR);
    comp.AddConstant("VK_RETURN", VK_RETURN);
    comp.AddConstant("VK_SHIFT", VK_SHIFT);
    comp.AddConstant("VK_CONTROL", VK_CONTROL);
    comp.AddConstant("VK_MENU", VK_MENU);
    comp.AddConstant("VK_PAUSE", VK_PAUSE);
    comp.AddConstant("VK_CAPITAL", VK_CAPITAL);
    comp.AddConstant("VK_KANA", VK_KANA);
    comp.AddConstant("VK_HANGEUL", VK_HANGEUL);
    comp.AddConstant("VK_HANGUL", VK_HANGUL);
    comp.AddConstant("VK_JUNJA", VK_JUNJA);
    comp.AddConstant("VK_FINAL", VK_FINAL);
    comp.AddConstant("VK_HANJA", VK_HANJA);
    comp.AddConstant("VK_KANJI", VK_KANJI);
    comp.AddConstant("VK_ESCAPE", VK_ESCAPE);
    comp.AddConstant("VK_CONVERT", VK_CONVERT);
    comp.AddConstant("VK_NONCONVERT", VK_NONCONVERT);
    comp.AddConstant("VK_ACCEPT", VK_ACCEPT);
    comp.AddConstant("VK_MODECHANGE", VK_MODECHANGE);
    comp.AddConstant("VK_SPACE", VK_SPACE);
    comp.AddConstant("VK_PRIOR", VK_PRIOR);
    comp.AddConstant("VK_NEXT", VK_NEXT);
    comp.AddConstant("VK_END", VK_END);
    comp.AddConstant("VK_HOME", VK_HOME);
    comp.AddConstant("VK_LEFT", VK_LEFT);
    comp.AddConstant("VK_UP", VK_UP);
    comp.AddConstant("VK_RIGHT", VK_RIGHT);
    comp.AddConstant("VK_DOWN", VK_DOWN);
    comp.AddConstant("VK_SELECT", VK_SELECT);
    comp.AddConstant("VK_PRINT", VK_PRINT);
    comp.AddConstant("VK_EXECUTE", VK_EXECUTE);
    comp.AddConstant("VK_SNAPSHOT", VK_SNAPSHOT);
    comp.AddConstant("VK_INSERT", VK_INSERT);
    comp.AddConstant("VK_DELETE", VK_DELETE);
    comp.AddConstant("VK_HELP", VK_HELP);
    comp.AddConstant("VK_LWIN", VK_LWIN);
    comp.AddConstant("VK_RWIN", VK_RWIN);
    comp.AddConstant("VK_APPS", VK_APPS);
//    comp.AddConstant("VK_SLEEP", VK_SLEEP);
    comp.AddConstant("VK_NUMPAD0", VK_NUMPAD0);
    comp.AddConstant("VK_NUMPAD1", VK_NUMPAD1);
    comp.AddConstant("VK_NUMPAD2", VK_NUMPAD2);
    comp.AddConstant("VK_NUMPAD3", VK_NUMPAD3);
    comp.AddConstant("VK_NUMPAD4", VK_NUMPAD4);
    comp.AddConstant("VK_NUMPAD5", VK_NUMPAD5);
    comp.AddConstant("VK_NUMPAD6", VK_NUMPAD6);
    comp.AddConstant("VK_NUMPAD7", VK_NUMPAD7);
    comp.AddConstant("VK_NUMPAD8", VK_NUMPAD8);
    comp.AddConstant("VK_NUMPAD9", VK_NUMPAD9);
    comp.AddConstant("VK_MULTIPLY", VK_MULTIPLY);
    comp.AddConstant("VK_ADD", VK_ADD);
    comp.AddConstant("VK_SEPARATOR", VK_SEPARATOR);
    comp.AddConstant("VK_SUBTRACT", VK_SUBTRACT);
    comp.AddConstant("VK_DECIMAL", VK_DECIMAL);
    comp.AddConstant("VK_DIVIDE", VK_DIVIDE);
    comp.AddConstant("VK_F1", VK_F1);
    comp.AddConstant("VK_F2", VK_F2);
    comp.AddConstant("VK_F3", VK_F3);
    comp.AddConstant("VK_F4", VK_F4);
    comp.AddConstant("VK_F5", VK_F5);
    comp.AddConstant("VK_F6", VK_F6);
    comp.AddConstant("VK_F7", VK_F7);
    comp.AddConstant("VK_F8", VK_F8);
    comp.AddConstant("VK_F9", VK_F9);
    comp.AddConstant("VK_F10", VK_F10);
    comp.AddConstant("VK_F11", VK_F11);
    comp.AddConstant("VK_F12", VK_F12);
    comp.AddConstant("VK_F13", VK_F13);
    comp.AddConstant("VK_F14", VK_F14);
    comp.AddConstant("VK_F15", VK_F15);
    comp.AddConstant("VK_F16", VK_F16);
    comp.AddConstant("VK_F17", VK_F17);
    comp.AddConstant("VK_F18", VK_F18);
    comp.AddConstant("VK_F19", VK_F19);
    comp.AddConstant("VK_F20", VK_F20);
    comp.AddConstant("VK_F21", VK_F21);
    comp.AddConstant("VK_F22", VK_F22);
    comp.AddConstant("VK_F23", VK_F23);
    comp.AddConstant("VK_F24", VK_F24);
    comp.AddConstant("VK_NUMLOCK", VK_NUMLOCK);
    comp.AddConstant("VK_SCROLL", VK_SCROLL);
//    comp.AddConstant("VK_OEM_NEC_EQUAL", VK_OEM_NEC_EQUAL);
    comp.AddConstant("VK_LSHIFT", VK_LSHIFT);
    comp.AddConstant("VK_RSHIFT", VK_RSHIFT);
    comp.AddConstant("VK_LCONTROL", VK_LCONTROL);
    comp.AddConstant("VK_RCONTROL", VK_RCONTROL);
    comp.AddConstant("VK_LMENU", VK_LMENU);
    comp.AddConstant("VK_RMENU", VK_RMENU);
//    comp.AddConstant("VK_BROWSER_BACK", VK_BROWSER_BACK);
//    comp.AddConstant("VK_BROWSER_FORWARD", VK_BROWSER_FORWARD);
//    comp.AddConstant("VK_BROWSER_REFRESH", VK_BROWSER_REFRESH);
//    comp.AddConstant("VK_BROWSER_STOP", VK_BROWSER_STOP);
//    comp.AddConstant("VK_BROWSER_SEARCH", VK_BROWSER_SEARCH);
//    comp.AddConstant("VK_BROWSER_FAVORITES", VK_BROWSER_FAVORITES);
//    comp.AddConstant("VK_BROWSER_HOME", VK_BROWSER_HOME);
//    comp.AddConstant("VK_VOLUME_MUTE", VK_VOLUME_MUTE);
//    comp.AddConstant("VK_VOLUME_DOWN", VK_VOLUME_DOWN);
//    comp.AddConstant("VK_VOLUME_UP", VK_VOLUME_UP);
//    comp.AddConstant("VK_MEDIA_NEXT_TRACK", VK_MEDIA_NEXT_TRACK);
//    comp.AddConstant("VK_MEDIA_PREV_TRACK", VK_MEDIA_PREV_TRACK);
//    comp.AddConstant("VK_MEDIA_STOP", VK_MEDIA_STOP);
//    comp.AddConstant("VK_MEDIA_PLAY_PAUSE", VK_MEDIA_PLAY_PAUSE);
//    comp.AddConstant("VK_LAUNCH_MAIL", VK_LAUNCH_MAIL);
//    comp.AddConstant("VK_LAUNCH_MEDIA_SELECT", VK_LAUNCH_MEDIA_SELECT);
//    comp.AddConstant("VK_LAUNCH_APP1", VK_LAUNCH_APP1);
//    comp.AddConstant("VK_LAUNCH_APP2", VK_LAUNCH_APP2);
    comp.AddConstant("VK_PROCESSKEY", VK_PROCESSKEY);
//    comp.AddConstant("VK_ICO_CLEAR", VK_ICO_CLEAR);
//    comp.AddConstant("VK_PACKET", VK_PACKET);
//    comp.AddConstant("VK_OEM_RESET", VK_OEM_RESET);
//    comp.AddConstant("VK_OEM_JUMP", VK_OEM_JUMP);
//    comp.AddConstant("VK_OEM_PA1", VK_OEM_PA1);
//    comp.AddConstant("VK_OEM_PA2", VK_OEM_PA2);
//    comp.AddConstant("VK_OEM_PA3", VK_OEM_PA3);
//    comp.AddConstant("VK_OEM_WSCTRL", VK_OEM_WSCTRL);
//    comp.AddConstant("VK_OEM_CUSEL", VK_OEM_CUSEL);
//    comp.AddConstant("VK_OEM_ATTN", VK_OEM_ATTN);
//    comp.AddConstant("VK_OEM_FINISH", VK_OEM_FINISH);
//    comp.AddConstant("VK_OEM_COPY", VK_OEM_COPY);
//    comp.AddConstant("VK_OEM_AUTO", VK_OEM_AUTO);
//    comp.AddConstant("VK_OEM_ENLW", VK_OEM_ENLW);
//    comp.AddConstant("VK_OEM_BACKTAB", VK_OEM_BACKTAB);
    comp.AddConstant("VK_ATTN", VK_ATTN);
    comp.AddConstant("VK_CRSEL", VK_CRSEL);
    comp.AddConstant("VK_EXSEL", VK_EXSEL);
    comp.AddConstant("VK_EREOF", VK_EREOF);
    comp.AddConstant("VK_PLAY", VK_PLAY);
    comp.AddConstant("VK_ZOOM", VK_ZOOM);
    comp.AddConstant("MOUSE_LBUTTON", 0);
    comp.AddConstant("MOUSE_RBUTTON", 1);
    comp.AddConstant("MOUSE_MBUTTON", 2);

    // Register functions
    comp.AddFunction ("cls",        WrapCls,        compParamTypeList (),                           false,  false,   VTP_INT);
    comp.AddFunction ("print",      WrapPrint,      compParamTypeList () << VTP_STRING,             false,  false,   VTP_INT, true);
    comp.AddFunction ("printr",     WrapPrintr,     compParamTypeList () << VTP_STRING,             false,  false,   VTP_INT, true);
    comp.AddFunction ("printr",     WrapPrintr_2,   compParamTypeList (),                           false,  false,   VTP_INT, true);
    comp.AddFunction ("locate",     WrapLocate,     compParamTypeList () << VTP_INT << VTP_INT,     false,  false,   VTP_INT);
    //comp.AddFunction ("swapbuffers",WrapSwapBuffers,compParamTypeList (),                           true,   false,   VTP_INT);
    comp.AddFunction ("drawtext",   WrapDrawText,   compParamTypeList (),                           true,   false,   VTP_INT, true);
    comp.AddFunction ("drawtext",   WrapDrawText2,  compParamTypeList () << VTP_INT,                true,   false,   VTP_INT, true);
    comp.AddFunction ("textmode",   WrapTextMode,   compParamTypeList () << VTP_INT,                true,   false,   VTP_INT);
    comp.AddFunction ("resizetext", WrapResizeText, compParamTypeList () << VTP_INT << VTP_INT,     true,   false,   VTP_INT, true);
    comp.AddFunction ("textrows",   WrapTextRows,   compParamTypeList (),                           true,   true,    VTP_INT);
    comp.AddFunction ("textcols",   WrapTextCols,   compParamTypeList (),                           true,   true,    VTP_INT);
    comp.AddFunction ("input$",     WrapInput,      compParamTypeList (),                           true,   true,    VTP_STRING, true);
    comp.AddFunction ("inkey$",     WrapInkey,      compParamTypeList (),                           true,   true,    VTP_STRING);
    comp.AddFunction ("inscankey",  WrapInScanKey,  compParamTypeList (),                           true,   true,    VTP_INT);
    comp.AddFunction ("clearkeys",  WrapClearKeys,  compParamTypeList (),                           true,   false,   VTP_INT);
    comp.AddFunction ("showcursor", WrapShowCursor, compParamTypeList (),                           true,   false,   VTP_INT);
    comp.AddFunction ("hidecursor", WrapHideCursor, compParamTypeList (),                           true,   false,   VTP_INT);
    comp.AddFunction ("keydown",    WrapKeyDown,    compParamTypeList () << VTP_STRING,             true,   true,    VTP_INT);
    comp.AddFunction ("scankeydown",WrapScanKeyDown,compParamTypeList () << VTP_INT,                true,   true,    VTP_INT);
    comp.AddFunction ("charat$",    WrapCharAt,     compParamTypeList () << VTP_INT << VTP_INT,     true,   true,    VTP_STRING);
    comp.AddFunction ("color",      WrapColour,     compParamTypeList () << VTP_INT << VTP_INT << VTP_INT, true, false, VTP_INT);
    comp.AddFunction ("font",       WrapFont,       compParamTypeList () << VTP_INT,                true,   false,  VTP_INT);
    comp.AddFunction ("defaultfont",WrapDefaultFont,compParamTypeList (),                           true,   true,   VTP_INT);
    comp.AddFunction ("mouse_x",     WrapMouseX,    compParamTypeList (),                           true,   true,   VTP_REAL);
    comp.AddFunction ("mouse_y",     WrapMouseY,    compParamTypeList (),                           true,   true,   VTP_REAL);
    comp.AddFunction ("mouse_xd",    WrapMouseXD,   compParamTypeList (),                           true,   true,   VTP_REAL);
    comp.AddFunction ("mouse_yd",    WrapMouseYD,   compParamTypeList (),                           true,   true,   VTP_REAL);
    comp.AddFunction ("mouse_button",WrapMouseButton,compParamTypeList () << VTP_INT,               true,   true,   VTP_INT);
    comp.AddFunction ("mouse_wheel", WrapMouseWheel,compParamTypeList (),                           true,   true,   VTP_INT);
    comp.AddFunction ("clearline",   WrapClearLine, compParamTypeList (),                           true,   false,  VTP_INT, true);
    comp.AddFunction ("clearregion", WrapClearRegion,compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT, true, false, VTP_INT, true);
    comp.AddFunction ("textscroll",  WrapTextScroll,compParamTypeList (),                           true,   true,   VTP_INT);
    comp.AddFunction ("settextscroll",WrapSetTextScroll,compParamTypeList () << VTP_INT,            true,   false,  VTP_INT);
    comp.AddFunction ("CursorCol",  WrapCursorCol,  compParamTypeList(),                            true,   true,   VTP_INT);
    comp.AddFunction ("CursorRow",  WrapCursorRow,  compParamTypeList(),                            true,   true,   VTP_INT);

    comp.AddConstant ("SPR_INVALID",    SPR_INVALID);
    comp.AddConstant ("SPR_SPRITE",     SPR_SPRITE);
    comp.AddConstant ("SPR_TILEMAP",    SPR_TILEMAP);
    comp.AddFunction ("newsprite",      WrapNewSprite,      compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("newsprite",      WrapNewSprite_2,    compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("newsprite",      WrapNewSprite_3,    compParamTypeList () << vmValType (VTP_INT, 1, 1, true), true, true,  VTP_INT);
    comp.AddFunction ("newtilemap",     WrapNewTileMap,     compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("newtilemap",     WrapNewTileMap_2,   compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("newtilemap",     WrapNewTileMap_3,   compParamTypeList () << vmValType (VTP_INT, 1, 1, true), true, true,  VTP_INT);
    comp.AddFunction ("deletesprite",   WrapDeleteSprite,   compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("bindsprite",     WrapBindSprite,     compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsettexture",  WrapSprSetTexture,  compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsettextures", WrapSprSetTextures, compParamTypeList () << vmValType (VTP_INT, 1, 1, true), true, false, VTP_INT);
    comp.AddFunction ("spraddtexture",  WrapSprAddTexture,  compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("spraddtextures", WrapSprAddTextures, compParamTypeList () << vmValType (VTP_INT, 1, 1, true), true, false, VTP_INT);
    comp.AddFunction ("sprsetframe",    WrapSprSetFrame,    compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetx",        WrapSprSetX,        compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsety",        WrapSprSetY,        compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetzorder",   WrapSprSetZOrder,   compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetpos",      WrapSprSetPos,      compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),true, false, VTP_INT);
    comp.AddFunction ("sprsetpos",      WrapSprSetPos_2,    compParamTypeList () << VTP_REAL << VTP_REAL,            true, false, VTP_INT);
    comp.AddFunction ("sprsetxsize",    WrapSprSetXSize,    compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetysize",    WrapSprSetYSize,    compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetsize",     WrapSprSetSize,     compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),true, false, VTP_INT);
    comp.AddFunction ("sprsetsize",     WrapSprSetSize_2,   compParamTypeList () << VTP_REAL << VTP_REAL,            true, false, VTP_INT);
    comp.AddFunction ("sprsetscale",    WrapSprSetScale,    compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetxcentre",  WrapSprSetXCentre,  compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetycentre",  WrapSprSetYCentre,  compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetxflip",    WrapSprSetXFlip,    compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsetyflip",    WrapSprSetYFlip,    compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsetvisible",  WrapSprSetVisible,  compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsetangle",    WrapSprSetAngle,    compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetcolor",    WrapSprSetColour,   compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),true, false, VTP_INT);
    comp.AddFunction ("sprsetcolor",    WrapSprSetColour_2, compParamTypeList () << VTP_REAL << VTP_REAL << VTP_REAL,true, false, VTP_INT);
    comp.AddFunction ("sprsetcolor",    WrapSprSetColour_3, compParamTypeList () << VTP_REAL << VTP_REAL << VTP_REAL << VTP_REAL,true, false, VTP_INT);
    comp.AddFunction ("sprsetalpha",    WrapSprSetAlpha,    compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetparallax", WrapSprSetParallax, compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsetsolid",    WrapSprSetSolid,    compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("resizespritearea",   WrapResizeSpriteArea,   compParamTypeList () << VTP_REAL << VTP_REAL,                       true, false, VTP_INT);
    comp.AddFunction ("spriteareawidth",    WrapSpriteAreaWidth,    compParamTypeList (),                                               true, true,  VTP_REAL);
    comp.AddFunction ("spriteareaheight",   WrapSpriteAreaHeight,   compParamTypeList (),                                               true, true,  VTP_REAL);
    comp.AddFunction ("sprframe",       WrapSprFrame,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprx",           WrapSprX,           compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("spry",           WrapSprY,           compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprpos",         WrapSprPos,         compParamTypeList (),                                    true, true,  vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("sprzorder",      WrapSprZOrder,      compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprxsize",       WrapSprXSize,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprysize",       WrapSprYSize,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprscale",       WrapSprScale,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprxcentre",     WrapSprXCentre,     compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprycentre",     WrapSprYCentre,     compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprxflip",       WrapSprXFlip,       compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("spryflip",       WrapSprYFlip,       compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprvisible",     WrapSprVisible,     compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprangle",       WrapSprAngle,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprcolor",       WrapSprColour,      compParamTypeList (),                                    true, true,  vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("spralpha",       WrapSprAlpha,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprparallax",    WrapSprParallax,    compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprsolid",       WrapSprSolid,       compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprleft",        WrapSprLeft,        compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprright",       WrapSprRight,       compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprtop",         WrapSprTop,         compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprbottom",      WrapSprBottom,      compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprxvel",        WrapSprXVel,        compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("spryvel",        WrapSprYVel,        compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("sprvel",         WrapSprVel,         compParamTypeList (),                                    true, true,  vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("sprspin",        WrapSprSpin,        compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("spranimspeed",   WrapSprAnimSpeed,   compParamTypeList (),                                    true, true,  VTP_REAL);
    comp.AddFunction ("spranimloop",    WrapSprAnimLoop,    compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("spranimdone",    WrapSprAnimDone,    compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprframe",       WrapSprFrame_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprx",           WrapSprX_2,         compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("spry",           WrapSprY_2,         compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprpos",         WrapSprPos_2,       compParamTypeList () << VTP_INT,                         true, true,  vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("sprzorder",      WrapSprZOrder_2,    compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprxsize",       WrapSprXSize_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprysize",       WrapSprYSize_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprscale",       WrapSprScale_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprxcentre",     WrapSprXCentre_2,   compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprycentre",     WrapSprYCentre_2,   compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprxflip",       WrapSprXFlip_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("spryflip",       WrapSprYFlip_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprvisible",     WrapSprVisible_2,   compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprangle",       WrapSprAngle_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprcolor",       WrapSprColour_2,    compParamTypeList () << VTP_INT,                         true, true,  vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("spralpha",       WrapSprAlpha_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprparallax",    WrapSprParallax_2,  compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprsolid",       WrapSprSolid_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprleft",        WrapSprLeft_2,      compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprright",       WrapSprRight_2,     compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprtop",         WrapSprTop_2,       compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprbottom",      WrapSprBottom_2,    compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprxvel",        WrapSprXVel_2,      compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("spryvel",        WrapSprYVel_2,      compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("sprvel",         WrapSprVel_2,       compParamTypeList () << VTP_INT,                         true, true,  vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("sprspin",        WrapSprSpin_2,      compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("spranimspeed",   WrapSprAnimSpeed_2, compParamTypeList () << VTP_INT,                         true, true,  VTP_REAL);
    comp.AddFunction ("spranimloop",    WrapSprAnimLoop_2,  compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("spranimdone",    WrapSprAnimDone_2,  compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprsetxvel",     WrapSprSetXVel,     compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetyvel",     WrapSprSetYVel,     compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetvel",      WrapSprSetVel,      compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),true, false, VTP_INT);
    comp.AddFunction ("sprsetvel",      WrapSprSetVel_2,    compParamTypeList () << VTP_REAL << VTP_REAL,            true, false, VTP_INT);
    comp.AddFunction ("sprsetspin",     WrapSprSetSpin,     compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetanimspeed",WrapSprSetAnimSpeed,compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprsetanimloop", WrapSprSetAnimLoop, compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("animatesprites", WrapAnimateSprites, compParamTypeList (),                                    true, false, VTP_INT);
    comp.AddFunction ("animatespriteframes", WrapAnimateSpriteFrames, compParamTypeList (),                          true, false, VTP_INT);
    comp.AddFunction ("copysprite",     WrapCopySprite,     compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprtype",        WrapSprType,        compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprxtiles",      WrapSprXTiles,      compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprytiles",      WrapSprYTiles,      compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprtype",        WrapSprType_2,      compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprxtiles",      WrapSprXTiles_2,    compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprytiles",      WrapSprYTiles_2,    compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprsettiles",    WrapSprSetTiles,    compParamTypeList () << vmValType (VTP_INT, 2, 1, true), true, false, VTP_INT);
    comp.AddFunction ("sprsetxrepeat",  WrapSprSetXRepeat,  compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprsetyrepeat",  WrapSprSetYRepeat,  compParamTypeList () << VTP_INT,                         true, false, VTP_INT);
    comp.AddFunction ("sprxrepeat",     WrapSprXRepeat,     compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("spryrepeat",     WrapSprYRepeat,     compParamTypeList (),                                    true, true,  VTP_INT);
    comp.AddFunction ("sprxrepeat",     WrapSprXRepeat_2,   compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("spryrepeat",     WrapSprYRepeat_2,   compParamTypeList () << VTP_INT,                         true, true,  VTP_INT);
    comp.AddFunction ("sprcamerasetx",  WrapSprCameraSetX,  compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprcamerasety",  WrapSprCameraSetY,  compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprcamerasetz",  WrapSprCameraSetZ,  compParamTypeList () << VTP_REAL,                        true, false, VTP_INT);
    comp.AddFunction ("sprcamerasetpos",WrapSprCameraSetPos,compParamTypeList () << VTP_REAL << VTP_REAL,            true, false, VTP_INT);
    comp.AddFunction ("sprcamerasetpos",WrapSprCameraSetPos_2,compParamTypeList () << vmValType (VTP_REAL, 1, 1, true),true, false, VTP_INT);
    comp.AddFunction ("sprcamerasetangle",WrapSprCameraSetAngle,compParamTypeList () << VTP_REAL,                    true, false, VTP_INT);
    comp.AddFunction ("sprcamerax",     WrapSprCameraX,     compParamTypeList (),                                    true, true, VTP_REAL);
    comp.AddFunction ("sprcameray",     WrapSprCameraY,     compParamTypeList (),                                    true, true, VTP_REAL);
    comp.AddFunction ("sprcameraz",     WrapSprCameraZ,     compParamTypeList (),                                    true, true, VTP_REAL);
    comp.AddFunction ("sprcamerapos",   WrapSprCameraPos,   compParamTypeList (),                                    true, true, vmValType (VTP_REAL, 1, 1, true), false, true);
    comp.AddFunction ("sprcameraangle", WrapSprCameraAngle, compParamTypeList (),                                    true, true, VTP_REAL);
    comp.AddFunction ("sprcamerafov",   WrapSprCameraFOV,   compParamTypeList (),                                    true, true, VTP_REAL);
    comp.AddFunction ("sprcamerasetfov",WrapSprCameraSetFOV,compParamTypeList () << VTP_REAL,                        true, false,VTP_INT);
    comp.AddFunction ("clearsprites",   WrapClearSprites,   compParamTypeList (),                                    true, false,VTP_INT);
    comp.AddFunction ("sprsetblendfunc",WrapSprSetBlendFunc,compParamTypeList () << VTP_INT << VTP_INT,              true, false,VTP_INT);
    comp.AddFunction ("spritecount",    WrapSpriteCount,    compParamTypeList (),                                    true, true, VTP_INT);
}

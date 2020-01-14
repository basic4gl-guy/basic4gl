/*  Created 28-May-06: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
    Copyright (C) Thomas Mulgrew

    New sound engine.
*/
//---------------------------------------------------------------------------

#ifndef TomSoundBasicLibH
#define TomSoundBasicLibH
//---------------------------------------------------------------------------

#include "TomComp.h"
#include "EmbeddedFiles.h"

void InitTomSoundBasicLib(TomBasicCompiler& comp, FileOpener *_files);
void StopTomSoundBasicLib();
void CleanupTomSoundBasicLib();

#endif

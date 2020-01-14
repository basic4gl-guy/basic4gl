rem Change this to your Qt directory!
set qtdir=E:\Projects\clibraries\Qt\5.5\msvc2013

del /Q /S ReleaseImage

md ReleaseImage
cd ReleaseImage
md Help
md Help\Images
md Help\highlight
md Help\highlight\styles
md Programs
md Programs\Textures
md Programs\Data
md Programs\Files
md Programs\Sounds
md Programs\Include
md Programs\Include\MD2
md Programs\Include\BGM
md platforms

copy ..\Basic4GL\release\basic4GL.exe
copy ..\Basic4GL\release\Stub.bin
rem copy ..\Basic4GL\release\msvcp120.dll
rem copy ..\Basic4GL\release\msvcr120.dll
rem copy ..\Basic4GL\release\Qt5Core.dll
rem copy ..\Basic4GL\release\Qt5Gui.dll
rem copy ..\Basic4GL\release\Qt5Widgets.dll
rem copy ..\Basic4GL\release\platforms\qwindows.dll platforms
copy %SYSTEMROOT%\SysWOW64\msvcp120.dll
copy %SYSTEMROOT%\SysWOW64\msvcr120.dll
copy %qtdir%\bin\Qt5Core.dll
copy %qtdir%\bin\Qt5Gui.dll
copy %qtdir%\bin\Qt5Widgets.dll
copy %qtdir%\plugins\platforms\qwindows.dll platforms
copy ..\Basic4GL\B4GLSound.dll
copy ..\Distribution\*.*
copy ..\Distribution\Help\*.* Help
copy ..\Distribution\Help\highlight\*.* Help\highlight
copy ..\Distribution\Help\highlight\styles\*.* Help\highlight\styles
copy ..\Distribution\Help\Images\*.* Help\Images
copy ..\Distribution\Programs\*.gb Programs
copy ..\Distribution\Programs\Textures\*.* Programs\Textures
copy ..\Distribution\Programs\Data\*.* Programs\Data
copy ..\Distribution\Programs\Files\*.* Programs\Files
copy ..\Distribution\Programs\Sounds\*.* Programs\Sounds
copy ..\Distribution\Programs\Include\*.* Programs\Include
copy ..\Distribution\Programs\Include\MD2\*.* Programs\Include\MD2
copy ..\Distribution\Programs\Include\BGM\*.* Programs\Include\BGM

cd..

@echo off
@echo.
@echo.
@echo ; Deploy files on install

echo SetOutPath $INSTDIR
cd releaseImage
for %%f in (*.*) do @echo File "ReleaseImage\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL
cd Programs
for %%f in (*.gb) do @echo File "ReleaseImage\Programs\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Sounds
cd Sounds
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Sounds\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Files
cd ..\Files
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Files\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Textures
cd ..\Textures
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Textures\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Data
cd ..\Data
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Data\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Include
cd ..\Include
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Include\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Include\MD2
cd MD2
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Include\MD2\%%f"

echo.
echo SetOutPath $DOCUMENTS\Basic4GL\Include\BGM
cd ..\BGM
for %%f in (*.*) do @echo File "ReleaseImage\Programs\Include\BGM\%%f"

echo.
echo SetOutPath $INSTDIR\Help
cd ..\..\..\Help
for %%f in (*.*) do @echo File "ReleaseImage\Help\%%f"

echo.
echo SetOutPath $INSTDIR\Help\highlight
cd highlight
for %%f in (*.*) do @echo File "ReleaseImage\Help\highlight\%%f"

echo.
echo SetOutPath $INSTDIR\Help\highlight\styles
cd styles
for %%f in (*.*) do @echo File "ReleaseImage\Help\highlight\styles\%%f"

echo.
echo SetOutPath $INSTDIR\Help\Images
cd ..\..\Images
for %%f in (*.*) do @echo File "ReleaseImage\Help\Images\%%f"

echo.
echo SetOutPath $INSTDIR\platforms
cd ..\..\platforms
for %%f in (*.*) do @echo File "ReleaseImage\platforms\%%f"

echo.
cd ..\..
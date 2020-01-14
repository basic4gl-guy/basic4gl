@echo off
@echo.
@echo.
@echo ; Delete files again on uninstall

cd releaseImage

cd Programs

echo.
for %%f in (Sounds\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Sounds"

echo.
for %%f in (Files\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Files"

echo.
for %%f in (Textures\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Textures"

echo.
for %%f in (Data\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Data"

echo.
for %%f in (Include\MD2\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Include\MD2"

echo.
for %%f in (Include\BGM\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Include\BGM"

echo.
for %%f in (Include\*.*) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL\Include"

echo.
for %%f in (*.gb) do @echo Delete "$DOCUMENTS\Basic4GL\%%f"
echo RMDir "$DOCUMENTS\Basic4GL"

cd ..

echo.
for %%f in (Help\highlight\styles\*.*) do @echo Delete "$INSTDIR\%%f"
echo RMDIR "$INSTDIR\Help\highlight\styles"

echo.
for %%f in (Help\highlight\*.*) do @echo Delete "$INSTDIR\%%f"
echo RMDIR "$INSTDIR\Help\highlight"

echo.
for %%f in (Help\Images\*.*) do @echo Delete "$INSTDIR\%%f"
echo RMDIR "$INSTDIR\Help\Images"

echo.
for %%f in (Help\*.*) do @echo Delete "$INSTDIR\%%f"
echo RMDIR "$INSTDIR\Help"

echo.
for %%f in (platforms\*.*) do @echo Delete "$INSTDIR\%%f"
echo RMDIR "$INSTDIR\platforms"

for %%f in (*.*) do @echo Delete "$INSTDIR\%%f"
echo RMDIR "$INSTDIR"

echo.
cd..
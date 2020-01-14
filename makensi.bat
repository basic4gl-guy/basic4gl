call CreateReleaseImage
call Utils\writelist > write.txt
call Utils\deletelist > delete.txt
copy Utils\Basic4gl.nsi Basic4GL.nsi
"C:\Program Files (x86)\NSIS\makensis" Basic4GL.nsi
del Basic4GL.zip
cd ReleaseImage
"c:\program files\7-zip\7z" a -tzip -r ..\Basic4GL.zip *.*
cd ..
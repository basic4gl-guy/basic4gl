This builds Basic4GL as a standalone commandline compiler, without the IDE.
It should be capable of running all Basic4GL programs that the official released Basic4GL can run.

This version is Windows specific, but should theoretically be buildable on any Win32 C++ compiler. Although the code may require tweaking before certain compilers will accept it. GCC (as used in DevCPP) is one example.


So far I have successfully built this with:
* Microsoft Visual C++ v6 (Project workspace: Compiler.dsw) and 
* Free version of Borland C++ v5.5 (Makefile is called "makefile". Please read it for more info.)
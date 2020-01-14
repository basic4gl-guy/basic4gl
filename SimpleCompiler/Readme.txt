A simple build of the Basic4GL language compiler.

This should be the most compiler and platform independant build. It contains just one function library with a single function (print) which simply writes a string to stdout.
There is no UI, instead it will simply load and execute the file passed as the first parameter.

Recommended as a starting point for conversions to other platforms or compilers, eg:
1. Get this working.
2. Add in other libraries.
3. Build the UI.


Project file for C++ Builder: SimpleCompiler.bpr (seems to crash AFTER it's completed execution... Somewhere in STL cleanup code.)

Project file for Visual C++: SimpleCompiler.dsw
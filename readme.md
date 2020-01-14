This is the source for the original Windows version of Basic4GL, warts and all.

It's basically a raw dump from my HDD. I'm pretty sure there are absolute paths to where things are on my PC in various project files/makefiles so it needs a bit of work to get building.

Released under the New BSD license (see license.txt)

Compilers
---------

The full version builds with Microsoft Visual Studio 2013 Community Edition.

It's close to buildable with GCC, although you might want to look at the Basic4GL Mobile version instead for that (once I've uploaded it), as the work has already been done for the Android and Web Assembly versions.

There's still old project files for Visual C++ 6 (I think) and C++ Builder, but these can be ignored.

3rd party libraries
-------------------

Basic4GL uses the following 3rd party libraries (not included):
* GLFW
* Corona

Main Projects
-------------

These are the main parts of Basic4GL

### Basic4GL

Project files:
* Basic4GL/Basic4GL.sln
* Basic4GL/Basic4gl.vcxproj

This is the main Basic4GL application (IDE and compiler).

The IDE is implemented in Qt. There is a Qt Creator project file but it doesn't actually build. I use it to edit the forms, then switch back to VS in order to actually build it.

### Standalone

Project files:
* Basic4GL/Basic4GL.sln
* CommandLineCompiler/Compiler.vcxproj

Basic4GL can "generate" standalone executables. It does this by taking a .exe "stub" that contains the compiler and runtime and injecting the BASIC program as a Windows resource.

This project builds the .exe "stub".

Other projects
--------------

These aren't part of the released application, but might be useful.

### CommandLineCompiler

A command line only version of the Basic4GL compiler and runtime.
I'm not sure when I last built this, so it might need a bit more work.

### SimpleCompiler

A minimal command line compiler without the standard BASIC runtime libraries that the full Basic4GL has. In fact it only has a basic "print" command that writes to standard output.

Should be quite platform independent though.
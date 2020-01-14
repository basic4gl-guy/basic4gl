Creating Plugins for Basic4GL.
==============================

2-Aug-06 Tom

Requirements
============

To compile a Basic4GL DLL you need a C++ compiler.
I've created projects for:

* Microsoft Visual Studio C++ compiler
* CodeBlocks

You should also be able to build DLLs with other compilers such as Borland C++,
DevC++ or older versions of Visual C++ (but you'll need to be confident about
fiddling with C++ project files, include directories etc).


Getting started
===============

Visual Studio:
1. Start Visual Studio C++
2. Open PluginDLL.sln (in this folder)
3. Compile it

CodeBlocks:
1. Start Codeblocks
2. Open PluginDLL.cbp (in this folder)
3. Compile it

Look in the "Bin" subfolder (inside this folder).
There should be a file called "PluginDLL.dll". This is the plugin DLL that you
just built.

Test it as follows:
1. Copy the file into your Basic4GL directory.
2. Run Basic4GL.
3. On the menu bar, click "Programs", then "Plug-in Libraries..."
4. Look for "PluginDLL.dll"
5. Tick the tickbox. This installs the plugin into Basic4GL
6. Click OK

You should now have two extra library commands, "Max" and "MessageBox".
Test them with a BASIC program, e.g:

	print "The maximum of 4 and 7 is "; max(4, 7)


Other example projects
======================

There are a couple more example DLLs demonstrating some more advanced features,
such as returning errors and sharing objects between DLLs. These are in the
"Examples" folder.

This readme file will not cover advanced topics however.


Creating your own DLL
=====================

Use the PluginDLL project as a starting point.

1. Open the Plugin.DLL project.

2. Open the PluginDLLMain.cpp file. This is the file that you will be updating.

3. Find the "Constants" section up the top.
   Set the DESCRIPTION to a description of what your DLL does. This is what
   will be displayed in the "Plugin chooser" dialog in Basic4GL.
   You may also want to set the major and minor version number.

   IMPORTANT: If you release a DLL for other people to use then make updates
     to it, be sure to update the version number before releasing the next 
     version.
     Even better is to append the version number to the filename (e.g. 
     "TomsDLL10", "TomsDLL11" etc), so that multiple versions can exist in the 
     same folder. This ensures that people who have written Basic4GL programs 
     using the old version of your DLL will not have to update all their 
     programs to use the new version (unless they want to).

4. Decide what functions you will create for Basic4GL and what their return
   type and parameters will be.
   The Basic4GL types are: integer, float and string (text). You can also create
   arrays, or register your own structure types (these are advanced features.
   I advise sticking to simple types to start with).
   Once you've decided how many parameters there are and what their types are
   you MUST be consistent about them. If you tell Basic4GL that the 3rd
   parameter is an integer and then try to access it as a string, Basic4GL may
   crash.


5. Add your functions in the "Runtime functions" section.
   These are the functions that Basic4GL will call.
   You must declare these functions as:

   void DLLFUNC functionname(IDLL_Basic4GL_Runtime &basic4gl)

   (replace functionname with the name of your function).
   You must declare your functions EXACTLY like this or Basic4GL will crash.

   Within the function you can use the "basic4gl" object to access parameters
   passed from the BASIC program and set the return value.

   To read the parameters:
      int         param = basic4gl.GetIntParam(index);
      float       param = basic4gl.GetFloatParam(index);
      const char *param = basic4gl.GetStringParam(index);

   Where "index" is the index of the parameter counting from the RIGHT HAND
   SIDE, starting at 1.
   So if your new function is called from Basic4GL with:

      MySuperFunction(10, "Sheep", 12.5)

   Then: basic4gl.GetFloatParam(1) returns 12.5.
   basic4gl.GetStringParam(2) returns "Sheep".
   basic4gl.GetIntParam(3) returns 10.

   If your function returns a value, use the appropriate one of:
      basic4gl.SetIntResult(result);
      basic4gl.SetFloatResult(result);
      basic4gl.SetStringResult(result);

   Note: For more information about the basic4gl object, see the
   "IDLL_Basic4GL_Runtime" code in "Basic4GLDLLInterface.h" (there are a lot of
   comments).

6. Register your functions.

   Find the Pluging::Load function in the "Plugin" section.

   Add commands to register your functions, describe their parameters and return
   value (if applicable) and anything else that Basic4GL needs to know about it.
   You do this with the "registry" object (that is passed as a parameter the
   Load function).
   You will also need to use the following constants:
      DLL_BASIC4GL_INT
      DLL_BASIC4GL_FLOAT
      DLL_BASIC4GL_STRING

   To declare a function:
      registry.RegisterFunction("[Basic4GL name]", [Function name], [Return type]);

   Where:
      [Basic4GL name] is the name of the function as it will appear in the
      Basic4GL program. This may be exactly the same as it appears in the
      DLL source code, but can be different if you want.
      Be sure to choose a name that conforms to Basic4GL naming standards.
      If you put spaces or symbols in it, Basic4GL wont be able to call it.

      [Function name] is the name of the function in the "Runtime functions"
      section.

      [Return type] is one of the constants above (e.g. DLL_BASIC4GL_INT).
      Remember this must correspond to which version of basic4gl.SetXXXResult
      was will be called. If you use DLL_BASIC4GL_STRING here and then try to
      return a result with basic4gl.SetIntResult(...), Basic4GL will crash.

   If your function does not return a value, declare it with:
      registry.RegisterVoidFunction("[Basic4GL name]", [Function name]);

   After you've declared your function, you need to declare your parameters with
      registry.AddParam([Type]);

   Where type is one of the type constants (e.g. DLL_BASIC4GL_INT).
   Parameters must be declared from LEFT TO RIGHT.

   For some functions you may need to use "modifiers" to inform Basic4GL that
   they require special treatment.

      registry.ModNoBrackets()

   Informs Basic4GL that the function does not have any brackets. Generally this
   is reserved for historical functions like "print", "cls" or "locate".
   It can also be appropriate for functions that look like variables, e.g.
   a "time$" function that returns the current time.

      registry.ModTimeshare()

   This informs Basic4GL that your function might not run quickly. If your
   function might take longer than 1 second to be called 1000 times, you should
   consider using this modifier, so that Basic4GL knows to force a timesharing
   break to allow windows and etc to be updated. Otherwise your Basic4GL program
   might become unresponsive.
   This especially applies to things like file I/O, or anything that stops and
   waits for user input.


   Once all your functions have been registered, the Plugin::Load method should
   return "true" to tell Basic4GL that everything was successful.
   (If it returns false, Basic4GL will unload the plugin.)

   For more information about registering functions, see the
   "IDLL_FunctionRegistry" class in "Basic4GLDLLInterface.h"

7. If your plugin needs to take any special action when a Basic4GL program
   starts, stops or is paused, or when the DLL loads or is unloaded, add the
   appropriate code to:
      Plugin::Load
      Plugin::Unload
      Plugin::Start
      Plugin::End
      Plugin::Pause
      Plugin::Resume
      Plugin::DelayedResume

   Again for more information see the IDLL_Plugin class in
   "Basic4GLDLLInterface.h"

At this point your DLL should be ready to be compiled and tested.
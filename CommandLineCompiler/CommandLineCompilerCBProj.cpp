//---------------------------------------------------------------------------

#include <vcl.h>
#include <string>
extern void CompileAndRun(std::string programFilename); 
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("CommandLineCompilerCB.cpp", Form1);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR cmdLine, int)
{
    try
    {
         Application->Initialize();
         Application->CreateForm(__classid(TForm1), &Form1);
        CompileAndRun(cmdLine);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------

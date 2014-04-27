/*
This program (work in progress) links to libOpenBUGS.dll library,
and executes OpenBUGS commands.
The BUGS model should be specified in "modelFile.txt".
The data should be provided in "data.txt".
*/

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
using namespace std;

//Set the path for OpenBUGS library
char* OpenBUGSpath = "C:\\Program Files\\OpenBUGS\\OpenBUGS322\\libOpenBUGS.dll";
HMODULE lib = LoadLibrary(OpenBUGSpath);

//Load procedures from libOpenBUGS.dll
typedef void (*proc1)(string*, int*, int*);
proc1 CmdInterpreter = (proc1)GetProcAddress(lib,"CmdInterpreter");
typedef void (*proc2)(string*, int*, string*, int*, int*);
proc2 CharArray = (proc2)GetProcAddress(lib,"CharArray");

//Read the content of "buffer.txt"
void buffer() {
    ifstream file("buffer.txt");
    string contents((istreambuf_iterator<char>(file)),istreambuf_iterator<char>());
    cout << contents;
    remove("buffer.txt");
}

//Run CmdInterpreter procedure from OpenBUGS
void _CmdInterpreter(string cmd) {
    if (!CmdInterpreter) {printf("Unable to link to function CmdInterpreter\n"); system("PAUSE"); exit(0);}   
    int len = cmd.length(); int error = -1; int* error_ptr = &error;
    CmdInterpreter(&cmd, &len, error_ptr);
    error = *error_ptr;
    if (error > 0) {cout << "Unable to call CmdInterpreter with " << cmd << endl << "Error = " << error << endl; system("PAUSE"); exit(0);}
    buffer();
}

//Run CharArray procedure from OpenBUGS
void _CharArray(string cmd, string arg) {
    if (!CharArray) {printf("Unable to link to function CharArray\n"); system("PAUSE"); exit(0);}   
    int cmd_len = cmd.length(); 
    int arg_len = arg.length(); 
    int res = 0; int* res_ptr = &res;
    CharArray(&cmd, &cmd_len, &arg, &arg_len, res_ptr);
    buffer();
}


void myBUGS()
{
    //Input parameters
    string modelFile = "modelFile.txt";
    string dataFile = "data.txt";
    string numChains = "2";
    string nBurnin = "1000";
    string nIter = "2000";
    string nThin = "2";
    string overRelax = "0";

    //Check the link to libOpenBUGS
    if(!lib) {printf("Unable to load libOpenBUGS.dll\n"); system("PAUSE"); exit(0);}
    printf("Welcome to OpenBUGS.\n");

    //Set directories
    typedef void (*SETDIR_ptr) (string*, int*);
    SETDIR_ptr SetWorkingDir = (SETDIR_ptr)GetProcAddress(lib,"SetWorkingDir");
    if (!SetWorkingDir) {printf("Unable to link to function SetWorkingDir\n"); system("PAUSE"); exit(0);}   
    SETDIR_ptr SetTempDir = (SETDIR_ptr)GetProcAddress(lib,"SetTempDir");
    if (!SetTempDir) {printf("Unable to link to function SetTempDir\n"); system("PAUSE"); exit(0);}   
    string path = "C:\\myBUGS\\";
    int len = path.length();
    SetWorkingDir(&path, &len);
    SetTempDir(&path, &len);

    //Check the model
    _CmdInterpreter("BugsEmbed.SetFilePath('" + modelFile + "');BugsEmbed.ParseGuard;BugsEmbed.Parse");

    //Load the data
    _CmdInterpreter("BugsEmbed.SetFilePath('" + dataFile + "');BugsEmbed.LoadDataGuard;BugsEmbed.LoadData");
    
    //Compile the model
    _CmdInterpreter("BugsEmbed.CompileGuard;BugsEmbed.numChains := " + numChains + "; BugsEmbed.Compile");
    
    //Generate initial values
    _CmdInterpreter("BugsEmbed.GenerateInitsGuard; BugsEmbed.GenerateInits");

    //Update the model
    _CmdInterpreter("BugsEmbed.UpdateGuard;BugsEmbed.thin := " + nThin + ";BugsEmbed.overRelax := " + overRelax + ";BugsEmbed.updates := " + nBurnin + ";BugsEmbed.Update");

    //Set the DIC monitor
    _CmdInterpreter("DevianceEmbed.SetVariable('*');DevianceEmbed.SetGuard;DevianceEmbed.Set");
 
    //Set sample monitors
    string nodeName = "E0";
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.StatsGuard;SamplesEmbed.Labels");
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.SetGuard;SamplesEmbed.Set");
    nodeName = "Emax";
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.StatsGuard;SamplesEmbed.Labels");
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.SetGuard;SamplesEmbed.Set");
    nodeName = "tau";
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.StatsGuard;SamplesEmbed.Labels");
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.SetGuard;SamplesEmbed.Set");
    nodeName = "sigma";
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.StatsGuard;SamplesEmbed.Labels");
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.SetGuard;SamplesEmbed.Set");
    nodeName = "deviance";
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.StatsGuard;SamplesEmbed.Labels");
    _CmdInterpreter("SamplesEmbed.SetVariable('" + nodeName + "');SamplesEmbed.SetGuard;SamplesEmbed.Set");
    
    //Update the model
    _CmdInterpreter("BugsEmbed.UpdateGuard;BugsEmbed.thin := " + nThin + ";BugsEmbed.overRelax := " + overRelax + ";BugsEmbed.updates := " + nIter + ";BugsEmbed.Update");

    //List the sample monitors
    //_CmdInterpreter("SamplesEmbed.SetVariable('*');SamplesEmbed.StatsGuard;SamplesEmbed.Labels");

    //Clear the DIC monitor
    _CmdInterpreter("DevianceEmbed.StatsGuard;DevianceEmbed.Clear");
}

int main()
{
    string path = "C:\\Program Files\\OpenBUGS\\OpenBUGS322\\libOpenBUGS.dll";

    for (int i=1; i<=10; i++) {
        cout << "********** Simulation: " << i << endl;
        BUGS();
        //system("PAUSE");            
    }
    FreeLibrary(lib);
    system("PAUSE");            
}

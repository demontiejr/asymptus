/*
 * Asumptus header file
 * file: asymptus.h
 * created by Junio Bond
 */

#include <string>
#include <list>
#include <iostream>

using std::string;
using std::stringstream;
using std::cin;
using std::cout;

/*Folder that contains the CPA and .py files. (Defined in the make install)*/
const string PATH_DIR = "/usr/bin/asymptusProfiler/";
/*Folder where will be stored the temporary files*/
const string TMP_DIR = "/tmp/asymptus/";
/*Default MAX value and number of runs. (The user can change this values)*/
int MAX_INT = 150;
int N_RUNS = 5;

const int default_ = 0;
const int withargs = 1;
const int manual = 2;
const int semi = 3;

#ifdef __APPLE__
  const string extension = "dylib";
  const bool osValidity = true;
#elif __linux
  const string extension = "so";
  const bool osValidity = true;
#elif __FreeBSD__
  const string extension = "so";
  const bool osValidity = true;
#else
  const string extension = "";
  const bool osValidity = false;
#endif


/*It's responsible for perform the whole script*/
bool runScript(char **, int, int, string, string, int);
/*Generates inputs, mixing fixed with generic values*/
void mixGen(std::list<string> &, char **, int, int);
/*Generates inputs according to the string array passed*/
void defaultGen(std::list<string> &, string *, int);
/*Allows the user to manually insert the inputs*/
void manualGen(std::list<string> &);
/*Display the --help info*/
void showInfo();
/*Generates random integer value*/
string gen_int();
/*Generates random constant char*/
string gen_char();
/*Generates random constant string*/
string gen_string();
/*Generates random float number*/
string gen_float();
/*Generates randon values according to the passed token*/
string gen_value(string token);
/*Check if the necessary files are available*/
bool pathValidity();
/*Execute commands and verify possible errors*/
bool execute(string command, string name);

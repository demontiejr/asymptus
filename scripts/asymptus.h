/*
 * Asumptus header file
 * file: asymptus.h
 * created by Junio Bond
 */

#include <string>
#include <list>

using std::string;

/*It's responsible for perform the whole script*/
void runScript(char **, int, int, string, string, int);
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
void checkpath();

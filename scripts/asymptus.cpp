/*
* ToDo: Create Error chain
* Add kill to the pathChecker
*/

#include <cstring>
#include "asymptus.h"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include <iostream>
#include <cstdlib>

using std::stringstream;
using std::cin;
using std::cout;

/*Pasta onde deve ter o CPA e os arquivos .py, caso não estejam no PATH*/
const string PATH_DIR = "./bin/";
/*Pasta onde serão gerados os arquivos temporarios*/
const string TMP_DIR = "/tmp/asymptus/";

const int default_ = 0;
const int withargs = 1;
const int manual = 2;
const int semi = 3;
const int nRuns = 5;

#ifdef __APPLE__
  const string extension = "dylib";
  const bool osValid = true;
#elif __linux
  const string extension = "so";
  const bool osValid = true;
#else
  const string extension = "";
  const bool osValid = false;
#endif


int main(int argc, char** argv){
  if(!osValid){
    std::cerr << "Operating system not supported!\n";
    return 1;
  }
  checkpath();
  string showPoly = "";
  int base = 0;
  bool alreadyRun = false;
  string mkdir = "mkdir -p " + TMP_DIR;
  system(mkdir.c_str());
  for (int i = 1; i < argc; i++){ //Itera sobre a lista de argumentos
    if( ! strcmp(argv[i],"--args" )){
      base = i+1;
      runScript(argv, base, argc, showPoly, argv[1], withargs);
      alreadyRun = true;
      break;
    }else if( ! strcmp(argv[i],"--mix" )){
      base = i+1;
      runScript(argv, base, argc, showPoly, argv[1], semi);
      alreadyRun = true;
      break;
    }else if( ! strcmp(argv[i],"--man" )){
      base = i+1;
      runScript(argv, base, argc, showPoly, argv[1], manual);
      alreadyRun = true;
      break;
    }else if(! strcmp(argv[i],"--help" )){
      showInfo();
      alreadyRun = true;
      break;
    }

    if( ! strcmp(argv[i],"-v" )){
      showPoly = "-I";
    }
  }

  if( ! alreadyRun ) //roda o default
    runScript(argv, base, argc, showPoly, argv[1], default_);

  return 0;
}


void checkpath(){
  /* type
   * command -v
   * hash   */


  string python = "hash python 2>/dev/null || { echo >&2 \"Python is required but it's not installed or not in path.  Aborting.\";}";
  string clang = "hash clang 2>/dev/null || { echo >&2 \"clang (LLVM) is required but it's not installed or not in path.  Aborting.\";}";
  string clangpp = "hash clang++ 2>/dev/null || { echo >&2 \"clang++ (LLVM) is required but it's not installed or not in path.  Aborting.\";}";
  string opt = "hash opt 2>/dev/null || { echo >&2 \"opt (LLVM) is required but it's not installed or not in path.  Aborting.\";}";
  string cpa = "hash " + PATH_DIR + "cpa 2>/dev/null || { echo >&2 \"CPA is required but it's not installed or not in path.  Aborting.\"; }";
  string gen = "hash " + PATH_DIR + "gen_csv.py 2>/dev/null || { echo >&2 \"gen_csv.py script is not in your path.  Aborting.\"; }";
  string parser = "hash " + PATH_DIR + "parse_complexity.py 2>/dev/null || { echo >&2 \"parser_complexity.py is not in your path.  Aborting.\"; }";

  system(python.c_str());
  system(clang.c_str());
  system(clangpp.c_str());
  system(cpa.c_str());
  system(opt.c_str());
  system(gen.c_str());
  system(parser.c_str());

}

void showInfo(){
  stringstream info;
  info << "OVERVIEW: Asymptus - Symbolic Assymptotic Complexity Evaluator \n\
           \nUSAGE: asy <input C/Bytecode file> [options] \n\
           \nOPTIONS: \n\
           \n  --with-args  <input types>                       Allows you to choose the types for the inputs \
           \n  --mix        {generic input} <fixed inputs>      Allows you to mix generic and fixed inputs \
           \n  --man        <command line as input argument>    Allows you to provide manual inputs to your program \
	   \n\n  If you do not provide any option, the tool will automatically identify the number os argments needed \
           \n  by the program and then will generate numeric inputs to it. \
           \n\nTYPES: \
           \n  num          Generates random float numbers \
           \n  int          Generates random integer numbers \
           \n  float        Same as num \
           \n  char         Generates random single character \
           \n  string       Generates strings \
           \n  bool         Generates boolean value\n\n";

   cout << info.str();

}

void defaultGen(std::list<string> &argslist, string *commands, int nArgs){
  srand (time(NULL));
  string line = "";
  for (int i =0; i < nRuns; i++){
    for(int j = 0; j < nArgs; j++){
        line += gen_value(commands[j]) + " ";
    }
    argslist.push_back(line);
    line = "";
  }
}


void manualGen(std::list<string> &argslist){
  unsigned number = 1;
  string commandLine;
  while(1){
    cout << "Insert the input number " << number << " : ";
    std::getline (cin,commandLine);
    if(commandLine.empty())
      break;
    else
      argslist.push_back(commandLine);
   number++;
  }
}

void mixGen(std::list<string> &argslist, char **commandLine, int pos, int max){ //arrumar
  char first, last;
  unsigned length;
  bool *generics = new bool[max]; //vetor com identificacao dos genericos
  string *kind = new string[max]; //vetor com identificacao dos tipos genericos , se nao der iniciar manaulmente com false
  string *cmd = new string[max]; //linha de comando unitária
  int iv = 0;
  string line = "";

  for(int i = pos; i < max; i++, iv++){
    length  = strlen(commandLine[i]) - 1;
    cmd[iv] = string(commandLine[i]);
    first = commandLine[i][0];
    last  = commandLine[i][length];
    generics[iv] = false;
    if(first == '{' && last == '}'){
      string str(commandLine[i]);
      string substr =  str.substr(1,length-1);
      kind[iv] = substr ;
      generics[iv] = true;
    }
  }

  srand (time(NULL));
  for (int i =0; i < nRuns; i++){
    for(int j = 0; j < iv; j++){
      if(generics[j]){
        line += gen_value(kind[j]) + " ";
      }else{
        line += cmd[j] + " " ;
      }
    }
    argslist.push_back(line);
    line = "";
  }
  delete [] generics;
  delete [] kind;
  delete [] cmd;

}


void runScript(char **commandLine, int pos, int max, string showPoly, string inputFile, int method){
  string strValue = "";
  std::list<string> argslist;
  string *cmds = new string[max];
  int jump = 3;
  int num = 0;
  int iv = 0;
  string rBytecode = TMP_DIR + "_rbyte.bc";
  string optBytecode = TMP_DIR + "_optbyte.bc";
  string app = TMP_DIR + "_app.exe";
  string eqOut = TMP_DIR + "eq.out";
  string rangedValue = TMP_DIR + "ranged.value";
  string outFile = TMP_DIR + "saida";
  string cpaFile = TMP_DIR + "CPA.out";
  string fileType = inputFile.substr( inputFile.length() - 3 );
  string compile  = "clang++ -w  -mllvm -disable-llvm-optzns -emit-llvm -g -c " + inputFile + " -o " + rBytecode ;
  string copybc = "cp " + inputFile + " " + rBytecode;
  string instrument = "opt  -load RangeAnalysis."+ extension +" -load DepGraph." + extension + " -load ComplexityInference." + extension + " -mem2reg -loop-simplify \
                      -instnamer -scev-aa -basicaa -licm -instr-loop -mem2reg " + rBytecode + " -o " + optBytecode;
  string genApp = "clang++ -g -lm " + optBytecode + " -o " + app;
  string genEqs = "opt -load RangeAnalysis." + extension + " -load DepGraph." + extension + " -load ComplexityInference." + extension + " -mem2reg \
                  -correlated-propagation -instcombine -scalar-evolution -constmerge -instnamer -lcomp -disable-output " + rBytecode  + " >> " + eqOut  + " 2>&1 ";

  string getArgc = "opt -load RangeAnalysis."+ extension + " -load DepGraph."+ extension +" -load ComplexityInference."+ extension +" -instnamer -mem2reg -break-crit-edges -vssa \
                   -region-analysis " + optBytecode + " -disable-output >> " + rangedValue +  " 2>&1";

  if(fileType == ".bc" || fileType == "rbc")
    system(copybc.c_str()); 
  else
    system(compile.c_str());

  system(compile.c_str());
  system(instrument.c_str());
  system(genApp.c_str());
  system(genEqs.c_str());

  if(method == withargs){
    while(pos < max){
      cmds[iv] = string(commandLine[pos]);
      pos++; iv++;
    }
    defaultGen(argslist,cmds,iv);
  }else if(method == manual){ // inputs manuais
    manualGen(argslist);
  }else if(method == semi){ //geração parcialmente automatica
    mixGen(argslist, commandLine, pos, max);
  }else{ //geração automatica, usando range analysis
    system(getArgc.c_str());
    std::ifstream read(rangedValue.c_str());
    read>>num;
    for(int i =0; i < num; i++)
      cmds[i] = "num";
    defaultGen(argslist,cmds,num);
  }

  if(showPoly == "-I") jump++;

  for(std::list<string>::iterator it = argslist.begin(), end = argslist.end(); it != end; it++){
    string command = app + " "  + *it + " 2>&1 >> " + outFile;
    system(command.c_str());
  }


  string command5 = "python " + PATH_DIR + "gen_csv.py " + outFile + " 2>&1 >> /dev/null";
  system(command5.c_str());

  struct dirent *dp;
  DIR *dirp = opendir(".");
  string ftype;
  while ((dp = readdir(dirp)) != NULL){
    ftype = ".";
    string name(dp->d_name);
    if (name.length() > 3) ftype = name.substr( name.length() - 3 );
    if (ftype == "csv"){
      string echo = "echo \"\" >> " + cpaFile;
      string id = "echo \"=== File: " + name + "\" >> " + cpaFile;
      string cpa = PATH_DIR + "cpa " + name + " 2>/dev/null >> " + cpaFile;

      system(echo.c_str());
      system(id.c_str());
      system(cpa.c_str());

    }
  }
  (void)closedir(dirp);

  string last = "python " +  PATH_DIR + "parse_complexity.py "+ eqOut + " " + cpaFile + " " + showPoly +  " 2>&1 | c++filt 2>&1";
  system(last.c_str());

  string removeFiles = "rm " + TMP_DIR + "* *.csv";
  string removeDir = "rmdir " + TMP_DIR;
  system(removeFiles.c_str());
  system(removeDir.c_str());
  delete [] cmds;
}

string gen_int(){  
  std::stringstream str_val;  
  str_val << rand() % 300 ; /*Limited because it was generating big numbers*/
  return str_val.str();
}


string gen_char(){
  int value;
  char id;
  std::stringstream str_val;
  if(rand() % 2)
    value = rand() % 25 + 65 ; /*Limited because it was generating big numbers*/
  else
    value = rand() % 25 + 97 ; /*Limited because it was generating big numbers*/
  if(value < 0) value = value * -1;
    id = value;
    
  str_val << id;
  return str_val.str();
}

string gen_string(){
  int size = rand() % 20 + 1;
  string str = "";
  while(size>0){
    str += gen_char();
    size--;
  }
  return str;
}

string gen_float(){
  string num = gen_int();
  string mantisa = gen_int();
  return num + "." + mantisa + gen_int();
}


string gen_value(string token){
  string value = "";
  if(token == "int"){
    value = gen_int();
  }else if (token == "char"){
    value = gen_char();
  }else if (token == "string"){
    value = gen_string();
  }else if (token == "float"){
    value = gen_float();
  }else if (token == "double"){
    value = gen_float();
  }else if(token == "num"){
    value = gen_float();
  }else{
    std::cerr << "\nIncorrect type: (" << token << ")\n";
  }    
  return value;
  
}

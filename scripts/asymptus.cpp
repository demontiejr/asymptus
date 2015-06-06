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
const string PATH_DIR = "/usr/bin/asymptusProfiler/";
/*Pasta onde serão gerados os arquivos temporarios*/
const string TMP_DIR = "/tmp/asymptus/";
/*Maior inteiro limite*/
int MAX_INT = 100;

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
  /*Verifica se as aplicações necessárias estão no PATH*/
  checkpath();
  string showPoly = "";
  int base = 0;
  bool alreadyRun = false;
  /*Cria a pasta temporária para guardar os dados*/
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
    }else if(! strcmp(argv[i],"--help" ) || ! strcmp(argv[i],"-h" )){
      showInfo();
      alreadyRun = true;
      break;
    }else if(! strcmp(argv[i],"--max" ) || ! strcmp(argv[i],"-max-num" )){
      if( argc > i+1 )
        MAX_INT = atoi(argv[i+1]);
      else
        std::cerr << "Using the default max value: " << MAX_INT << "\n";
    }

    if( ! strcmp(argv[i],"-v" )){
      showPoly = "-I";
    }
  }

  /*Caso não encontre nenhuma flag válida, geramos entradas aleatórias usando RangeAnalysis*/
  if( ! alreadyRun )
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
           \nUSAGE: asymptus <C/Bytecode input file> [options] \n\
           \nOPTIONS: \n\
           \n  --args <input types>                      Allows the user to specify the types of command line arguments. \
           \n                                            It is necessary to specify as many argument as the program reads \
           \n                                            in order to have a correct execution. The types may be one of the \
           \n                                            following: int, long, float, double, num, char or string. \
           \n\n  --mix  {generic input} <fixed inputs>     Allows the user to mix random generated arguments with concrete \
           \n                                            data. When using this option, the random arguments types have to \
           \n                                            come between {}. For instance, --mix myInput {int}.\
           \n\n  --man  <command line as input argument>   This option will ask the user for only concrete inputs. Asymptus will \
           \n                                            ask for the inputs of each desired execution. An empty line means all \
           \n                                            data has been provided. \
           \n\n  -v                                        Verbose mode. Prints the polynomial for each loop inside a function.\
	   \n\n  If you do not provide any option, the tool will automatically identify the number os arguments needed \
           \n  by the program and then will generate integer values to it. \
           \n\n";

   cout << info.str();

}

/*Pega a lista de argumentos genéricos em *commands e gera entradas aleatórias*/
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


/*Armazena as entradas manuais do usuário, para posterior execução*/
void manualGen(std::list<string> &argslist){
  unsigned number = 1;
  string commandLine;
  /*Continua lendo entradas até o usuário dar um enter em branco*/
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

/*Permite o usuário mixar entradas genericas com fixas*/
void mixGen(std::list<string> &argslist, char **commandLine, int pos, int max){ //arrumar
  char first, last;
  unsigned length;
  bool *generics = new bool[max]; //vetor com identificacao das posições dos genericos
  string *kind = new string[max]; //vetor com identificacao dos tipos genericos
  string *cmd = new string[max]; //linha de comando unitária (inclui genericos + concretos)
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
  string *cmds;
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
                   -region-analysis " + optBytecode + " -disable-output >> " + rangedValue;

  if(fileType == ".bc" || fileType == "rbc")
    system(copybc.c_str()); 
  else
    system(compile.c_str());

  system(compile.c_str());
  system(instrument.c_str());
  system(genApp.c_str());
  system(genEqs.c_str());

  /*Verifica o método de entrada (--args,--mix,--man) e gera inputs*/
  if(method == withargs){
    cmds = new string[max];
    while(pos < max){
      cmds[iv] = string(commandLine[pos]);
      pos++; iv++;
    }
    defaultGen(argslist,cmds,iv);
  }else if(method == manual){
    manualGen(argslist);
  }else if(method == semi){
    mixGen(argslist, commandLine, pos, max);
  }else{ //geração automatica, usando range analysis
    system(getArgc.c_str());
    std::ifstream read(rangedValue.c_str());
    read>>num;
    cmds = new string[num];
    for(int i =0; i < num; i++)
      cmds[i] = "num";
    defaultGen(argslist,cmds,num);
  }
  /*Iteração sobre as linhas de commando, a cada iteração do usuário a aplicação é executada com uma entrada*/
  for(std::list<string>::iterator it = argslist.begin(), end = argslist.end(); it != end; it++){
    string command = app + " "  + *it + " 2>&1 >> " + outFile;
    system(command.c_str());
  }

  /*Geração dos arquivos CSV*/
  string command5 = "python " + PATH_DIR + "gen_csv.py " + outFile + " 2>&1 >> /dev/null";
  system(command5.c_str());


  /*Iteração sobre arquivos do diretório, em busca dos arquivos .csv*/
  struct dirent *dp;
  DIR *dirp = opendir(".");
  string ftype;
  while ((dp = readdir(dirp)) != NULL){
    ftype = ".";
    string name(dp->d_name);
    if (name.length() > 3) ftype = name.substr( name.length() - 3 );
    if (ftype == "csv"){ /*Se o arquivo for CSV, executamos o CPA sobre ele*/
      string echo = "echo \"\" >> " + cpaFile;
      string id = "echo \"=== File: " + name + "\" >> " + cpaFile;
      string cpa = PATH_DIR + "cpa " + name + " 2>/dev/null >> " + cpaFile;

      system(echo.c_str());
      system(id.c_str());
      system(cpa.c_str());

    }
  }
  (void)closedir(dirp);

  /*Gera as equações de complexidade*/
  string last = "python " +  PATH_DIR + "parse_complexity.py "+ eqOut + " " + cpaFile + " " + showPoly +  " 2>&1 | c++filt 2>&1";
  system(last.c_str());

  /*Remoção dos arquivos e pasta temporaria*/
  string removeFiles = "rm " + TMP_DIR + "* *.csv";
  string removeDir = "rmdir " + TMP_DIR;
  system(removeFiles.c_str());
  system(removeDir.c_str());
  delete [] cmds;
}

/*Gerador de valores inteiros*/
string gen_int(){  
  std::stringstream str_val;  
  str_val << rand() % MAX_INT ; /*Limited because it was generating big numbers*/
  return str_val.str();
}

/*Gerador de caracteres*/
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

/*Gerador de strings*/
string gen_string(){
  int size = rand() % 20 + 1;
  string str = "";
  while(size>0){
    str += gen_char();
    size--;
  }
  return str;
}

/*Gerador de numeros reais*/
string gen_float(){
  string num = gen_int();
  string mantisa = gen_int();
  return num + "." + mantisa + gen_int();
}

/*Parser, seleciona qual tipo de valor gerar*/
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
  }else if(token == "long"){
    value = gen_int() + gen_int();
  }else{
    std::cerr << "\nIncorrect type: (" << token << ")\n";
  }
  return value;
}


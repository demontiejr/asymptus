/*
* ToDo:
*
*/

#include <cstring>
#include "asymptus.h"
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include <cstdlib>

int main(int argc, char** argv){
  if (argc <= 1) {
    showInfo();
    exit(1);
  }
  if(!osValidity ){
    std::cerr << "Operating system not supported!\n";
    return 1;
  }
  string mkdir = "mkdir -p " + TMP_DIR;
  system(mkdir.c_str());
  bool executed = true;

  /*Verify if the needed applications are in the PATH*/
  if(!pathValidity()) return 1;;
  string showPoly = "";
  int base = 0;
  bool alreadyRun = false;
  /*Creating the temporary folder*/
  for (int i = 1; i < argc; i++){ //Iterating over the arguments' list
    if( ! strcmp(argv[i],"--args" )){
      base = i+1;
      executed = runScript(argv, base, argc, showPoly, argv[1], withargs);
      alreadyRun = true;
      break;
    }else if( ! strcmp(argv[i],"--mix" )){
      base = i+1;
      executed = runScript(argv, base, argc, showPoly, argv[1], semi);
      alreadyRun = true;
      break;
    }else if( ! strcmp(argv[i],"--man" )){
      base = i+1;
      executed = runScript(argv, base, argc, showPoly, argv[1], manual);
      alreadyRun = true;
      break;
    }else if(! strcmp(argv[i],"--help" ) || ! strcmp(argv[i],"-h" )){
      showInfo();
      alreadyRun = true;
      break;
    }else if(! strcmp(argv[i],"-m" ) || ! strcmp(argv[i],"--max-num" )){
      if( argc > i+1 ){
        /*avoid cases where the user has used the flag, but forgot the value or did not provide a valid one*/
	int value = atoi(argv[i+1]);
        MAX_INT = (value > 0) ? value : MAX_INT;
      }else
        std::cerr << "Using the default max value: " << MAX_INT << "\n";
    }else if( ! strcmp(argv[i],"--runs" ) ||  ! strcmp(argv[i],"-r" ) ){
      if( argc > i+1 ){
        /*avoid cases where the user has used the flag, but forgot the value or did not provide a valid one*/
        int value = atoi(argv[i+1]);
        N_RUNS = (value > 0) ? value : N_RUNS;
      }else{
        std::cerr << "Using the default number of runs: " << N_RUNS << "\n";}
    }else if( ! strcmp(argv[i],"-v" ) ||  ! strcmp(argv[i],"--verbose" )){
      showPoly = "-I";
    }
  }

  /*It addresses the case where the user did not provide any input flag*/
  if( ! alreadyRun )
    executed = runScript(argv, base, argc, showPoly, argv[1], default_);

  if(! executed )
    std::cerr << "\nnAsymptus aborted due to the previous error(s).\n";
  return 0;
}


bool pathValidity(){
  /* type
   * command -v
   * hash   */
  int id;
  bool failed = false;
  string name;
  string python = "hash python 2>/dev/null || { echo \"1 Python\" ;} >> "+TMP_DIR+"error";
  string clang = "hash clang 2>/dev/null ||  { echo \"1 clang\" ;} >> "+TMP_DIR+"error ";
  string clangpp = "hash clang++ 2>/dev/null ||  { echo \"1 clang++\" ;} >> "+TMP_DIR+"error ";
  string opt = "hash opt 2>/dev/null ||  { echo \"1 opt\" ;} >> "+TMP_DIR+"error ";
  string cpa = "ls " + PATH_DIR + "cpa 2>/dev/null 1>/dev/null ||  { echo \"2 cpa\" ;} >> "+TMP_DIR+"error";
  string gen = "ls " + PATH_DIR + "gen_csv.py 2>/dev/null 1>/dev/null ||  { echo \"2 gen_csv.py\" ;} >> "+TMP_DIR+"error";
  string parser = "ls " + PATH_DIR + "parse_complexity.py 2>/dev/null 1>/dev/null ||  { echo \"2 parse_complexity.py\" ; } >> "+TMP_DIR+"error";
  string checker = TMP_DIR + "error";
  string removeFiles = "rm " + TMP_DIR + "* 2>/dev/null";   

  system(removeFiles.c_str());
  system(python.c_str());
  system(clang.c_str());
  system(clangpp.c_str());
  system(opt.c_str());
  system(cpa.c_str());
  system(gen.c_str());
  system(parser.c_str());

  std::ifstream infile(checker.c_str());
  while (infile >> id >> name){
    if(id == 1){
      failed = true;
      std::cerr << name << " is needed, but it is not in the PATH. Aborting !\n";
    }else if(id == 2){
      failed = true;
      std::cerr << name << " is needed, but it is not in the folder: " << PATH_DIR << " . Aborting !\n";
    }
  }

  if(failed){
    string removeDir = "rmdir " + TMP_DIR;
    system(removeFiles.c_str());
    system(removeDir.c_str());
    return false; /*Files are missing, failed*/
  }
  return true; /*Everything is OK*/
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
           \n\n  --verbose  or  -v                         Verbose mode. Prints the polynomial for each loop inside a function.\
           \n\n  --max-num  or  -m  <value>                Define the max value that can be generated. (Default = 150)\
           \n\n  --runs     or  -r  <value>                Define the amount of times that the user application will run with \
           \n                                            different arguments. (Default = 5) \
	   \n\n  If you do not provide any option, the tool will automatically identify the number os arguments needed \
           \n  by the program and then will generate integer values to it. \
           \n\n";

   cout << info.str();

}

bool execute(string command, string name){
  string newCommand = command + "; echo $? >> " + TMP_DIR + "id_" + name ;
  string filePath = TMP_DIR  + "id_" + name ;
  string removeMe = "rm " + TMP_DIR  + "id_" + name ;
  int id;

  system(newCommand.c_str());
  std::ifstream read(filePath.c_str());
  read>>id;
  if(id){
    string removeFiles = "rm " + TMP_DIR + "* 2>/dev/null";  
    string removeDir = "rmdir " + TMP_DIR;
    system(removeFiles.c_str());
    system(removeDir.c_str());
    return false; /*Error during execution*/
  }
  system(removeMe.c_str());
  return true; /*Everything OK*/
}


/*Generates random values according to the generics values defined in the commands string array*/
void defaultGen(std::list<string> &argslist, string *commands, int nArgs){
  srand (time(NULL));
  string line = "";
  for (int i =0; i < N_RUNS; i++){
    for(int j = 0; j < nArgs; j++){
        line += gen_value(commands[j]) + " ";
    }
    argslist.push_back(line);
    line = "";
  }
}


/*It uses a list to stores the manual entries provided by the user*/
void manualGen(std::list<string> &argslist){
  unsigned number = 1;
  string commandLine;
  /*The function will continue to read entries until the user provide a blank entry*/
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

/*Function that allows the user to mix generics and concrete inputs*/
void mixGen(std::list<string> &argslist, char **commandLine, int pos, int max){
  char first, last;
  unsigned length;
  bool *generics = new bool[max]; /*Identify which arguments are generic.*/
  string *kind = new string[max]; /*Identify which is the generic type (num, char, string).*/
  string *cmd = new string[max];
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
  for (int i =0; i < N_RUNS; i++){
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


string getExtension(string name){
  unsigned size = name.length();
  if(size >= 3)
    return name.substr( size - 3 ); /*Return the last 3 char*/
  return ".c";
}

bool runScript(char **commandLine, int pos, int max, string showPoly, string inputFile, int method){
  string strValue = "";
  std::list<string> argslist;
  string *cmds;
  int num = 0;
  int iv = 0;
  string fileType = getExtension(inputFile);
  string rBytecode = TMP_DIR + "_rbyte.bc";
  string optBytecode = TMP_DIR + "_optbyte.bc";
  string app = TMP_DIR + "_app.exe";
  string eqOut = TMP_DIR + "eq.out";
  string rangedValue = TMP_DIR + "ranged.value";
  string outFile = TMP_DIR + "saida";
  string cpaFile = TMP_DIR + "CPA.out";
  string compile  = "clang++ -w  -mllvm -disable-llvm-optzns -emit-llvm -g -c " + inputFile + " -o " + rBytecode ;
  string copybc = "cp " + inputFile + " " + rBytecode;
  string instrument = "opt  -load RangeAnalysis."+ extension +" -load DepGraph." + extension + " -load ComplexityInference." + extension + " -mem2reg -loop-simplify \
                      -instnamer -scev-aa -basicaa -licm -instr-loop -mem2reg " + rBytecode + " -o " + optBytecode;
  string genApp = "clang++ -g -lm " + optBytecode + " -o " + app;
  string genEqs = "opt -load RangeAnalysis." + extension + " -load DepGraph." + extension + " -load ComplexityInference." + extension + " -mem2reg \
                  -correlated-propagation -instcombine -scalar-evolution -constmerge -instnamer -lcomp -disable-output " + rBytecode  + " >> " + eqOut  + " 2>&1 ";

  string getArgc = "opt -load RangeAnalysis."+ extension + " -load DepGraph."+ extension +" -load ComplexityInference."+ extension +" -instnamer -mem2reg -break-crit-edges -vssa \
                   -region-analysis " + optBytecode + " -disable-output >> " + rangedValue;


  if(fileType == ".bc" || fileType == "rbc" ){
    if(! execute(copybc, "cp")) return false;
  }else{
    if(! execute(compile, "clang++")) return false;
  }

  if(! execute(instrument, "opt")) return false;
  if(! execute(genApp, "clang++")) return false;
  if(! execute(genEqs, "opt")) return false;

  /*Verifing which is the input flag (--args,--mix,--man) and generating the inputs*/
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
  }else{ /*automatic generation using Range Analysis*/
    if(! execute(getArgc, "opt")) return false;
    std::ifstream read(rangedValue.c_str());
    read>>num;
    cmds = new string[num];
    for(int i =0; i < num; i++)
      cmds[i] = "num";
    defaultGen(argslist,cmds,num);
  }
  /*Iterating over command lines, in each iteration the user's application is run with an input*/
  for(std::list<string>::iterator it = argslist.begin(), end = argslist.end(); it != end; it++){
    string command = app + " "  + *it + " 2>&1 >> " + outFile;
    //if(! execute(command, "userApp")) return false;
    system(command.c_str());
  }

  /*Generating CSV files*/
  string command5 = "python " + PATH_DIR + "gen_csv.py " + outFile + " 2>&1 >> /dev/null";
  if(! execute(command5, "gen_csv")) return false;


  /*Searching for the .csv files*/
  struct dirent *dp;
  DIR *dirp = opendir(".");
  string ftype;
  while ((dp = readdir(dirp)) != NULL){
    ftype = ".";
    string name(dp->d_name);
    if (name.length() > 3) ftype = name.substr( name.length() - 3 );
    if (ftype == "csv"){ /*If it's a CSV file, we run CPA app using the .csv as input*/
      string echo = "echo \"\" >> " + cpaFile;
      string id = "echo \"=== File: " + name + "\" >> " + cpaFile;
      string cpa = PATH_DIR + "cpa " + name + " 2>/dev/null >> " + cpaFile;

      system(echo.c_str());
      system(id.c_str());
      system(cpa.c_str());

    }
  }
  (void)closedir(dirp);

  /*analyzing results and generating complexities*/
  string last = "python " +  PATH_DIR + "parse_complexity.py "+ eqOut + " " + cpaFile + " " + showPoly +  " 2>&1 | c++filt 2>&1";
  if(! execute(last, "parse_complexity")) return false;


  /*Removing temporary files*/
  string removeFiles = "rm " + TMP_DIR + "* *.csv";
  string removeDir = "rmdir " + TMP_DIR;
  system(removeFiles.c_str());
  system(removeDir.c_str());
  if (method != manual && method != semi) delete [] cmds;

  return true; /*Script executed as expected*/
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

/*strings generator*/
string gen_string(){
  int size = rand() % 20 + 1;
  string str = "";
  while(size>0){
    str += gen_char();
    size--;
  }
  return str;
}

/*float number generator*/
string gen_float(){
  string num = gen_int();
  string mantisa = gen_int();
  return num + "." + mantisa + gen_int();
}

/*Parser, choose which value will be generate, according to its type*/
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


#include <cstring>
#include "inGen.h"
#include <dirent.h>
#include <fstream>

using std::string;
using std::stringstream;
using std::cin;
using std::cout;

//Adicionar adaptador MAC OS - LINUX
bool runScript(char **commandLine, int pos, int max, int &returnValue, string &showPoly){
  returnValue = 0;
  int value;
  int auxPos;
  string strValue = "";
  stringstream executeLine;
  std::list<string> argslist;
  bool ret = false;  

  if(! strcmp(commandLine[pos],"-I")){
  	showPoly = "-I";
  }

  if(! strcmp(commandLine[pos],"--with-args")){ 	
  	string nomeFile = (string)commandLine[1];
    string command  = "clang++ -mllvm -disable-llvm-optzns -emit-llvm -g -c " + nomeFile + " -o __temporario1.bc";
    string command2 = "opt  -load RangeAnalysis.dylib -load DepGraph.dylib -load ComplexityInference.dylib -mem2reg -loop-simplify \
        			   -instnamer -scev-aa -basicaa -licm -instr-loop -mem2reg __temporario1.bc -o __temporario2.bc";
    string command3 = "clang++ -g -lm __temporario2.bc -o __temporario3.app";
    string command4 = "opt -load RangeAnalysis.dylib -load DepGraph.dylib -load ComplexityInference.dylib -mem2reg -correlated-propagation -instcombine \
    				   -scalar-evolution -constmerge -instnamer -lcomp -disable-output __temporario1.bc >> eq.out 2>&1 ";

    
    
    system(command.c_str());
    system(command2.c_str());
    system(command3.c_str());
    system(command4.c_str());

  	int nargs = 5; //gera 5 entradas diferentes obedecendo os criterios passados
  	int cont = 1;
    while(pos < max){
      pos++;
      executeLine << commandLine[pos] << " ";
    }    
    generator(argslist, nargs, executeLine.str());
    

    string arguments = "";
    for(std::list<string>::iterator it = argslist.begin(), end = argslist.end(); it != end; it++, cont++){
    	arguments += (" " + *it);
    	if(max-3 == cont ) { // -3 para excluir ./app sourcecode --with-args
    	  cont = 0;
    	  //cout << arguments << "\n";
    	  string command = "./__temporario3.app " + arguments + " 2>&1 >> saida";
    	  system(command.c_str());
    	  arguments = "";    	  
    	}    	
    }
  

  string command5 = "python ~/Public/scripts/bin/gen_csv.py saida 2>&1 >> /dev/null";
  system(command5.c_str());

  struct dirent *dp;
  DIR *dirp = opendir(".");
  while ((dp = readdir(dirp)) != NULL){    	 
  	string name(dp->d_name);  	  
  	if (name.find(".csv") != std::string::npos){ //garantir q ta no fim
  		string echo = "echo \"\" >> CPA.out";
  		string id = "echo \"=== File: " + name + "\" >> CPA.out";
  		string cpa = "cpa " + name + " 2>/dev/null >> CPA.out";

  		system(echo.c_str());
        system(id.c_str());
        system(cpa.c_str());

  	}  	
  }
  (void)closedir(dirp);

  

  string last = "python ~/Public/scripts/bin/parse_complexity.py eq.out CPA.out " + showPoly + " 2>&1 | c++filt 2>&1";
  system(last.c_str());
  system("rm __temporario1.bc __temporario2.bc __temporario3.app eq.out CPA.out saida *.csv");

  ret = true;
}

  return ret;
}


bool runDefault(char **commandLine, int pos, int max, int &returnValue, string &showPoly){
  returnValue = 0;
  int value;
  int auxPos;
  string strValue = "";
  stringstream executeLine;
  std::list<string> argslist;
  bool ret = false;  
  int num = 0;

  //Fix, alterar nome dos arquivos temporarios
  string nomeFile = (string)commandLine[1];    	
  string filetype = nomeFile.substr( nomeFile.length() - 3 );

  string command  = "clang++ -mllvm -disable-llvm-optzns -emit-llvm -g -c " + nomeFile + " -o __temporario1.bc";
  string command1 = "cp " + nomeFile + " __temporario1.bc";
  string command2 = "opt  -load RangeAnalysis.dylib -load DepGraph.dylib -load ComplexityInference.dylib -mem2reg -loop-simplify \
      			       -instnamer -scev-aa -basicaa -licm -instr-loop -mem2reg __temporario1.bc -o __temporario2.bc";
  string command3 = "clang++ -g -lm __temporario2.bc -o __temporario3.app";
  string command4 = "opt -load RangeAnalysis.dylib -load DepGraph.dylib -load ComplexityInference.dylib -mem2reg -correlated-propagation -instcombine \
    				   -scalar-evolution -constmerge -instnamer -lcomp -disable-output __temporario1.bc >> eq.out 2>&1 ";

  string command_ = "opt -load RangeAnalysis.dylib -load DepGraph.dylib -load ComplexityInference.dylib -instnamer -mem2reg -break-crit-edges -vssa \
  					   -region-analysis __temporario2.bc -disable-output >> rangevalue 2>&1"; //pegar este valor        

  
  if(filetype == ".bc" || filetype == "rbc")
    system(command1.c_str());	
  else
  	system(command.c_str());
  
  system(command2.c_str());
  system(command3.c_str());
  system(command4.c_str());
  system(command_.c_str());

  std::ifstream read("rangevalue");
  read>>num;    
  
  int nargs = 5; //gera 5 entradas diferentes obedecendo os criterios passados
  int cont = 1;  
  int NUMEROdeENTRADAS = num;
  
  for(int i =0; i < NUMEROdeENTRADAS; i++){      
    executeLine << "num ";
  }

  generator(argslist, nargs, executeLine.str());

  string arguments = "";  
  for(std::list<string>::iterator it = argslist.begin(), end = argslist.end(); it != end; it++, cont++){
  	arguments += (" " + *it);    
  	if(NUMEROdeENTRADAS == cont ) { // -3 para excluir ./app sourcecode --with-args
   	  cont = 0;
   	  //cout << arguments << "\n";
   	  string command = "./__temporario3.app " + arguments + " 2>&1 >> saida";
  	  system(command.c_str());
   	  arguments = "";    	  
   	}   	
  }
  

  string command5 = "python ~/Public/scripts/bin/gen_csv.py saida 2>&1 >> /dev/null";
  system(command5.c_str());

  struct dirent *dp;
  DIR *dirp = opendir(".");
  while ((dp = readdir(dirp)) != NULL){    	 
  	string name(dp->d_name);  	  
  	if (name.find(".csv") != std::string::npos){ //Precisa garantir q ta no fim
  		string echo = "echo \"\" >> CPA.out";
  		string id = "echo \"=== File: " + name + "\" >> CPA.out";
  		string cpa = "cpa " + name + " 2>/dev/null >> CPA.out";

  		system(echo.c_str());
        system(id.c_str());
        system(cpa.c_str());

  	}  	
  }
  (void)closedir(dirp);

  

  string last = "python ~/Applications/llvm-3.4/lib/Analysis/PFrogMay/scripts/parse_complexity.py eq.out CPA.out " + showPoly + " 2>&1 | c++filt 2>&1";
  system(last.c_str());
  system("rm __temporario1.bc __temporario2.bc __temporario3.app rangevalue eq.out CPA.out saida *.csv");


  return ret;
}


void checkpath(){
  /* type
   * command -v
   * hash   */


  string python = "hash python 2>/dev/null || { echo >&2 \"Python is required but it's not installed or not in path.  Aborting.\";}";
  string clang = "hash clang 2>/dev/null || { echo >&2 \"clang (LLVM) is required but it's not installed or not in path.  Aborting.\";}";
  string clangpp = "hash clang++ 2>/dev/null || { echo >&2 \"clang++ (LLVM) is required but it's not installed or not in path.  Aborting.\";}";
  string opt = "hash opt 2>/dev/null || { echo >&2 \"opt (LLVM) is required but it's not installed or not in path.  Aborting.\";}";
  string cpa = "hash cpa 2>/dev/null || { echo >&2 \"CPA is required but it's not installed or not in path.  Aborting.\"; }";

  system(python.c_str());
  system(clang.c_str());
  system(clangpp.c_str());
  system(cpa.c_str());
  system(opt.c_str());

}


/* ToDo:
 * Adicionar diferenciação c/c++
 * Adicionar diferenciação MAC e LINUX
 * Juntar as funções runScript e runDefault numa só
*/

int main(int argc, char** argv){  
  int numInputs;
  bool ret =false;
  string showPoly;

  checkpath();

  for (int i = 1; i < argc; i++){
    while(*argv[i] == ' '){
      i++;
    }
    ret |= runScript(argv, i, argc, numInputs, showPoly);
  }

  if(!ret){
  	runDefault(argv, 0, argc, numInputs, showPoly);
  }

  return 0;
}

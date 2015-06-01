/*
Random generator of inputs
*/
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <list>
#include <time.h>

using std::cout;
using std::cin;
using std::cerr;
using std::string;

void do_something(){
  int a;
}

string gen_int(){
  int value;
  std::stringstream str_val;
  //value = rand() % 200 + 1; /*Generates number between 0 and 200*/
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
    cerr << "\nIncorrect type: " << token << "\n";
  }  
  return value;
}

void display_list(std::list<string> types){
  for(std::list<string>::iterator it = types.begin(), end = types.end(); it != end; it++){
    cout << *it << "\n---\n";
  }
}

void gen_output(std::list<string> types){
  std::stringstream line;
  for(std::list<string>::iterator it = types.begin(), end = types.end(); it != end; it++){
    string generated_value = gen_value(*it);
    line << generated_value << " ";
  }
  cout << line.str() << "\n";
}


int main(int argc, char **argv){
  if(argc <= 1){
    cerr << "You must provide the number of inputs\n\n" ;
    return 0;
  }

  int num_inputs = atoi(argv[1]);
  int i = 2;
  int j = 0;
  int str_index = 0;
  string input_line;
  string token = "";
  std::stringstream commandLine;
  std::list<string> types;

  srand (time(NULL));

  while(i < argc){
    commandLine << argv[i] << " ";
    i++;
  }
  input_line = string(commandLine.str());
  i = 0;

  while(input_line[i] != '\0'){    
    j = i;
    while(input_line[j] != ' '){
      token += input_line[j];
      j++;  i++;      
      if (input_line[j] == '\0') break;
    }    
    types.push_back(token);
    token = "";
    i++;
  }

  for(int i = 0; i < num_inputs; i++){
    gen_output(types);  
  }
  
  return 0;    
}

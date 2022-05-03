//
// Created by UPDATE on 17/05/2020.
//

#ifndef HW1_CLASSES_H
#define HW1_CLASSES_H

#include <unordered_map>
#include <iostream>
#include <utility>
#include <vector>
#include <iterator>
#include <string>
#include <stack>
#include <unordered_set>
#include "output.h"
#include "bp.hpp"



using std::string;
using std::unordered_map;
using std::stack;
using std::vector;


class Var{
public:
    Var(string basicString, string basicString1, int i){
        name = basicString;
        type = basicString1;
        offset = i ;
    }
	~Var(){
	}
	
	
    string name;
    int offset;
    string type;
};

template<typename T>
class Scope{
public:
    vector<T> Elements;
};

class Func{
public:
    Func(string basicString, string basicString1, vector<std::string> a ): name(basicString), retType(basicString1), args(a)
    {
    }
	Func() {}

    string name;
    vector<string> args;
    string retType;
};



void init();
void destroy(int f = 0);
void openScope();
void closeVarScope();
void closeFuncScope();
void insertVar(string, string);
void insertFunc(string,string retType, vector<string>& args, vector<string>& names);
string checkCall(vector<pair<string,string>> types , string name, string reg);
void inLoop();
void outLoop();
string idType(string id);
void checkLoop(int i); 
void TypeError(); 
string checkReturnType(string type); 
void checkByte(string num);
void checkInt(string num);
bool checkRelOp(string, string);
int getVarOffset(string var);
string getVarType(string var);
string c2llvm_type(string t);
string loadVar(string id);

string freshVar(int i=0);
void storeVar(string id, string reg , string etype);


#endif //HW1_CLASSES_H

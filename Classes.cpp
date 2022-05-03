//
// Created by UPDATE on 17/05/2020.
//

#include "Classes.h"

using std::to_string;

extern int yylineno;
int flag = 0;
int varIndex = 0;
int strIndex = 0;


vector<Scope<Var>>* Scopes;
Scope<Func>* funcs;
unordered_map<string,string>* varsHash;
unordered_map<string,string>* funcsHash;
stack<int>* offsetsStack;

string freshVar(int i)
{	if(i==0)
		return "%t"+to_string(varIndex++);
	return "@.string_"+to_string(strIndex++);
}


void checkInt(string num)
{
	long long unsigned int n = strtoull(num.c_str(),nullptr,10);
	if(n<0 || n> 0xffffffff) exit(5);
}

void init()
{
	Scopes = new vector<Scope<Var>>();
	funcs = new Scope<Func>();
	varsHash = new unordered_map<string,string>();
	funcsHash = new unordered_map<string,string>();
	offsetsStack = new stack<int>();
	offsetsStack->push(0);
	funcs->Elements.push_back(Func("print","VOID",vector<string>(1,"STRING")));
	funcs->Elements.push_back(Func("printi","VOID",vector<string>(1,"INT")));
	(*funcsHash)["print"]="VOID";
	(*funcsHash)["printi"]="VOID";
	CodeBuffer::instance().emitGlobal("declare i32 @printf(i8*, ...)");
	CodeBuffer::instance().emitGlobal("declare void @exit(i32)");
	CodeBuffer::instance().emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    CodeBuffer::instance().emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    CodeBuffer::instance().emitGlobal("@Zero_Error = constant [23 x i8] c\"Error division by zero\\00\"");

    CodeBuffer::instance().emitGlobal("define void @printi(i32) {");
    CodeBuffer::instance().emitGlobal(
            "call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
    CodeBuffer::instance().emitGlobal("ret void");
    CodeBuffer::instance().emitGlobal("}");


    CodeBuffer::instance().emitGlobal("define void @print(i8*) {");
    CodeBuffer::instance().emitGlobal(
            "call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
    CodeBuffer::instance().emitGlobal("ret void");
    CodeBuffer::instance().emitGlobal("}");
}

void destroy(int f)
{
	if(f==0)
	{
		CodeBuffer::instance().printGlobalBuffer();
		CodeBuffer::instance().printCodeBuffer();
	}
	delete Scopes;
	delete funcs;
	delete varsHash;
	delete funcsHash;
	delete offsetsStack;
	exit(0);
}


void openScope()
{
   Scopes->push_back(Scope<Var>());
   offsetsStack->push(offsetsStack->top());
}


void closeVarScope()
{
    Scope<Var> stackTop = Scopes->back();
    for(auto itr = stackTop.Elements.begin() ; itr != stackTop.Elements.end() ; itr++)
    {
		varsHash->erase((itr)->name);
    }
    Scopes->pop_back();
    offsetsStack->pop();
}

bool mainExists()
{
    for (auto i = funcs->Elements.begin(); i!=funcs->Elements.end(); ++i) {
        if (i->name == "main" && i->args.empty() && i->retType == "VOID")
			return true;
    }
    return false;
}

void closeFuncScope()
{
	if(!mainExists()){
		output::errorMainMissing();
		destroy(1);
	}
}


void insertVar(string name , string type)
{
    auto itr = varsHash->find(name);
	auto itr1 = funcsHash->find(name);
    if(itr == varsHash->end() && itr1 == funcsHash->end())
    {
		int i = offsetsStack->top();
		Var v(name,type,i);
		Scope<Var> s = Scopes->back();
		s.Elements.push_back(v);
		Scopes->pop_back();
		Scopes->push_back(s);
        (*varsHash)[name] = type;
    }
    else{
	  output::errorDef(yylineno,name);
	  destroy(1);
   }
   int i = offsetsStack->top()+1;
   offsetsStack->pop();
   offsetsStack->push(i);
}

int getVarOffset(string var)
{
	
	for(auto it = Scopes->begin(); it != Scopes->end(); it++)
	{
		for(auto itr = it->Elements.begin()  ; itr != it->Elements.end() ; itr++)
		{
		   if(itr->name==var) return itr->offset;
		}
	}
	output::errorUndef(yylineno,var);
	exit(4);
}

string getVarType(string var)
{
	return idType(var);
}

void insertFunc(string name ,string retType, vector<string>& args , vector<string>& names)
{
	auto it = varsHash->find(name);
	auto it1 = funcsHash->find(name);
	if(it == varsHash->end() && it1 == funcsHash->end()){
		funcs->Elements.push_back(Func(name,retType,args));
        (*funcsHash)[name] = retType;
		int i = -1;
		auto itr1 = names.begin();
		for(auto itr = args.begin()  ; itr != args.end() ; itr++)
		{
		   if(varsHash->find(*itr1) != varsHash->end() || funcsHash->find(*itr1) != funcsHash->end())
		   {
				output::errorDef(yylineno,*itr1);
				destroy(1);
		   }
		   Scopes->back().Elements.push_back(Var(*itr1,*itr,i));
		   varsHash->insert({(*itr1),(*itr)});
		   itr1++;
		   i--;
		}
		/////////////////////////
		string arguments = "";
		for(auto j = args.begin(); j!= args.end(); j++)
		{
			arguments += c2llvm_type(*j);
			if((j+1)!=args.end()) arguments += ", ";
		}
		CodeBuffer::instance().emit("define "+ c2llvm_type(retType) +" @"+name+"("+arguments+")"+" { ");
		CodeBuffer::instance().emit("%stack = alloca ["+ std::to_string(args.size()+50) +" x i32]");
		int size = args.size();
		for (int i = 0; i < size; i++) {
			string ptr =freshVar();
			CodeBuffer::instance().emit(ptr + " = getelementptr [" + to_string(size+50) + " x i32], [" + to_string(size+50) + " x i32]* %stack, i32 0, i32 " + to_string(i + 50));
			string llvmType = c2llvm_type(args[i]);
			if (llvmType == "i32") CodeBuffer::instance().emit("store i32 %" + to_string(i) + ", i32* " + ptr);
			else {
				string reg = freshVar();
				CodeBuffer::instance().emit(reg + " = zext " + llvmType + " %" + to_string(i) + " to i32");
				CodeBuffer::instance().emit("store i32 " + reg + ", i32* " + ptr);
			}
		}
		
	}
	else{
	  output::errorDef(yylineno,name);
	  destroy(1);
	}
}


string checkCall(vector<pair<string,string>> types , string name, string reg)
{
	auto it = funcs->Elements.begin();
	for(; it!= funcs->Elements.end(); it++)
	{
		if((it)->name==name) break;
	}
	if(it == funcs->Elements.end()){
		output::errorUndefFunc(yylineno,name);
		destroy(1);
	}
	if(types.size()!=it->args.size()){
		output::errorPrototypeMismatch(yylineno,name,it->args);
		destroy(1);
	}
	auto j = types.begin();
	for(auto i = it->args.begin(); i!=it->args.end(); i++){
		if(*i!=(j->first)&&((j->first)!="BYTE" || (*i)!="INT")){
			output::errorPrototypeMismatch(yylineno,name,it->args);
			destroy(1);
		}
		j++;
	}
	////////////
	string args ="";
	for(auto i = 0; i<types.size(); i++)
	{
		string r = types[i].second;
		string llvmType = c2llvm_type(types[i].first);
		if(types[i].first == "BYTE" && it->args[i] == "INT"){
			r = freshVar();
			CodeBuffer::instance().emit(r + " = zext i8 " + types[i].second + " to i32");
		}
		args+= c2llvm_type(it->args[i]) + " " + r;
		if((i+1)!=types.size()) args+=", ";
	}
	if(it->retType!="VOID"){
		CodeBuffer::instance().emit(reg+" = call "+ c2llvm_type(it->retType) +" @"+name+"("+args+")");
	} else {
		CodeBuffer::instance().emit("call void @"+name+"("+args+")");
	}
	return it->retType;
}


void inLoop(){
    flag++;; // flag is a global var
}


void outLoop()
{
    flag--;
}

string idType(string id)
{
	auto it = varsHash->find(id);
	if(varsHash->end()==it){
		output::errorUndef(yylineno,id);
		destroy(1);
	}
	return it->second;
}

void checkLoop(int i)
{
	if(flag) return;
    if(i == 0 )
    {
        output::errorUnexpectedBreak(yylineno);
        exit(0);
    }

    else if(i==1)
    {
        output::errorUnexpectedContinue(yylineno);
        exit(0);
    }

}

void TypeError(){
	output::errorMismatch(yylineno);
	destroy(1);
}


string checkReturnType(string type)
{
    string t = funcs->Elements.back().retType;
    if( t == type || (t == "INT" && type == "BYTE") ) return t; 
    output::errorMismatch(yylineno);
	destroy(1);
}

void checkByte(string num)
{
    if(stoi(num) <= 255) return;
	output::errorByteTooLarge(yylineno,num);
	destroy(1);
}

bool checkRelOp(string t1, string t2)
{
	return (((t1)=="INT" || (t1)=="BYTE") && ((t2)=="INT" || (t2)=="BYTE"));
}

string c2llvm_type(string t)
{
	if(t=="VOID") return "void";
	if(t=="STRING") return "i8*";
	if(t=="INT") return "i32";
	if(t=="BYTE") return "i8";
	if(t=="BOOL") return "i1";
	exit(2);
}

string loadVar(string id)
{
	int offset = getVarOffset(id);
	Func currentF = funcs->Elements.back();
	int stackSize = currentF.args.size()+50;
	string type = getVarType(id);
	string ptr = freshVar();
	string reg = freshVar();
	if(offset < 0){
		offset = -offset-1;
		CodeBuffer::instance().emit(ptr + " = getelementptr [" + to_string(stackSize) + " x i32], [" + to_string(stackSize) + " x i32]* %stack, i32 0, i32 " + to_string(50+offset));
	}
	else {
		CodeBuffer::instance().emit(ptr + " = getelementptr [" + to_string(stackSize) + " x i32], [" + to_string(stackSize) + " x i32]* %stack, i32 0, i32 " + to_string(offset));
	}
	CodeBuffer::instance().emit(reg + " = load i32, i32* "+ptr);
	if(c2llvm_type(type)=="i32") return reg;
	string reg2 = freshVar();
	CodeBuffer::instance().emit(reg2+" = trunc i32 "+reg+" to "+c2llvm_type(type));
	return reg2;
}

void storeVar(string id, string reg , string etype)
{
	int offset = getVarOffset(id);
	Func currentF = funcs->Elements.back();
	int stackSize = currentF.args.size()+50;
	string ptr = freshVar();
	if(offset < 0){
		offset = -offset-1;
		CodeBuffer::instance().emit(ptr + " = getelementptr [" + to_string(stackSize) + " x i32], [" + to_string(stackSize) + " x i32]* %stack, i32 0, i32 " + to_string(50+offset));
	}
	else {
		CodeBuffer::instance().emit(ptr + " = getelementptr [" + to_string(stackSize) + " x i32], [" + to_string(stackSize) + " x i32]* %stack, i32 0, i32 " + to_string(offset));
	}
	string llvmType = c2llvm_type(getVarType(id));
	string ellvmType = c2llvm_type(etype);
	if(llvmType=="i32" && ellvmType == "i32"){
		CodeBuffer::instance().emit("store i32 "+ reg +", i32* "+ptr);
		return;
	}
	string tmpReg = freshVar();
	CodeBuffer::instance().emit(tmpReg+" = zext "+ellvmType+" "+ reg+" to i32");
	CodeBuffer::instance().emit("store i32 "+ tmpReg +", i32* "+ptr);
	return;
}
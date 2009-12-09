#include<iostream>
#include<fstream>
#include<string>
#include<cctype>
#undef NULL
#define SYM_MAX 10000
using namespace std;

enum token { KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST};   
string kw[21]={ "class", "method", "function", "constructor", "int", "boolean", "char", "void", 
                "var", "static", "field", "let", "do", "if", "else", "while", "return", "true", 
                "false", "null", "this"};               
char Symbol[19]={'{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|', '<', 
                 '>', '=', '_'}; 
                 
bool isopr(char ch){
     switch(ch){
                case '+':
                case '-':
                case '*':
                case '/':
                case '&':
                case '|':
                case '<':
                case '>':
                case '=': return true;
                default: return false;
     }
}

struct symbol{
       string name,type,kind;
       int index;
};

class SymbolTable{
      symbol symTab[SYM_MAX];
      public:
             int size, staticCount, fieldCount, argCount, varCount;
             SymbolTable(){
                 size=staticCount=fieldCount=argCount=varCount=0;
             }
             void define(string, string, string);
             void startnewSubroutine(){
                  size=staticCount=fieldCount=argCount=varCount=0;
             }
             bool searchSymbol(string name){
                  for(int i=0;i<size;i++)
                      if(symTab[i].name==name) return true;
                  return false;
             }
             void writePushPop(string name, fstream& stream, bool isPush){
                  for(int i=0;i<size;i++)
                  if(symTab[i].name==name){                                           
                      isPush ? stream << "push " : stream << "pop ";
                      if(symTab[i].kind=="static") stream << "static ";
                      else if(symTab[i].kind=="field") stream << "this ";
                      else if(symTab[i].kind=="arg") stream << "argument ";
                      else if(symTab[i].kind=="var") stream << "local ";
                      stream << symTab[i].index << endl;
                      return;
                  }
             }
             string typeOf(string name){
                    for(int i=0;i<size;i++)
                        if(symTab[i].name==name) return symTab[i].type;
             }
};

class JackTokenizer{
      fstream infile;
      string tokenizedFile;
      string currentToken;
      int tokenptr;
      public:
             JackTokenizer(char* filename){
                    infile.open(filename, ios::in);
                    if(infile.fail()){
                          cout << filename << " cannot be found\n"
                               << "Please check whether you have entered the correct pathname\n";
                          exit(0);
                    }
                    infile.unsetf(ios::skipws);
                    tokenptr=0;
                    tokenize();
             }
             ~JackTokenizer(){infile.close();}
             void tokenize();
             token tokenType();
             int intVal();
             friend class CompilationEngine;
};

class CompilationEngine{
      fstream outfile;
      string className;
      JackTokenizer jt;
      SymbolTable CSTab, MSTab; //Class Scope/Method Scope Symbol Table
      public:
             int ifCount, whileCount;
             CompilationEngine(char *jack)
             :jt(jack)
             {              string vm=jack;
                            vm.erase(vm.size()-5,5);
                            className=vm;
                            vm+=".vm";
                            outfile.open(vm.c_str(), ios::out);
                            ifCount=whileCount=0;
                            className.erase(0,className.find_last_of('/')+1);
                            CompileClass();
             }
             ~CompilationEngine(){outfile.close();}
             int CompileClass();
             int CompileClassVarDec();
             int CompileSubroutine();
             int CompileParameterList();
             int CompileVarDec();
             int CompileStatements();
             int CompileDo();
             int CompileSubroutineCall();
             int CompileLet();
             int CompileWhile();
             int CompileReturn();
             int CompileIf();
             int CompileExpression();
             int CompileTerm();
};

void SymbolTable::define(string name, string type, string kind){
     if(searchSymbol(name)){
         cout << "Duplication of variable " << name << endl;
         exit(0);
     }
     symTab[size].name=name;
     symTab[size].type=type;
     symTab[size].kind=kind;
     if(kind=="static") symTab[size].index=staticCount++;
     else if(kind=="field") symTab[size].index=fieldCount++;
     else if(kind=="arg") symTab[size].index=argCount++;
     else if(kind=="var") symTab[size].index=varCount++;
     else{ cout << "Invalid variable kind\n"; exit(0);}
     size++;
}

void JackTokenizer::tokenize(){
     char ch;
     int flag=0,next=0;
     while(!infile.fail()){
         if(!next)
         infile >> ch;
         next=0;
         if(flag==1&&(!(isalnum(ch)||ch=='_'))){
             tokenizedFile+=' ';
             flag=0;
         }
         while(ch==' '||ch=='\t'||ch=='\n'||ch=='\r'){
             infile >> ch;
             if(infile.fail()) return;
         }
         while(isdigit(ch)){
             tokenizedFile+=ch;
             infile >> ch;
             if(!isdigit(ch)){
                 tokenizedFile+='^';
                 if(flag==1){tokenizedFile+=' '; flag=0;}
             }
         }
         tokenizedFile+=ch;
         string s1=tokenizedFile,s2=tokenizedFile,s3=tokenizedFile,s4=tokenizedFile,s5=tokenizedFile;
         if(tokenizedFile.size()>=6)
         s1.erase(0,s1.size()-6);
         if(tokenizedFile.size()>=5)
         s2.erase(0,s2.size()-5);
         if(tokenizedFile.size()>=3)
         s3.erase(0,s3.size()-3);
         if(tokenizedFile.size()>=8)
         s4.erase(0,s4.size()-8);
         if(tokenizedFile.size()>=11)
         s5.erase(0,s5.size()-11);
         if(s1=="static"||s1=="method"||s2=="field"||s3=="var"||s4=="function"||s5=="constructor"||
            tokenizedFile[tokenizedFile.size()-1]=='('||tokenizedFile[tokenizedFile.size()-1]==','){
             ch=' ';
             while(ch==' '||ch=='\t'||ch=='\n'||ch=='\r')
             infile >> ch;
             flag=1;
             next=1;
         }
         if(tokenizedFile[tokenizedFile.size()-1]=='/'&&tokenizedFile[tokenizedFile.size()-2]=='/'){
             tokenizedFile.erase(tokenizedFile.size()-2, 2);
             infile >> ch;
             while(ch!='\n') infile >> ch;
         }
         else if(tokenizedFile[tokenizedFile.size()-2]=='/'&&tokenizedFile[tokenizedFile.size()-1]=='*'){
             char ch1,ch2;
             tokenizedFile.erase(tokenizedFile.size()-2, 2);
             infile >> ch1 >> ch2;
             while(!(ch1=='*'&&ch2=='/')){
             ch1=ch2;
             infile >> ch2;                              
             }
         }
         else if(tokenizedFile[tokenizedFile.size()-1]=='"'){
              infile >> ch;
              while(ch!='"'){tokenizedFile+=ch; infile >> ch;}
              tokenizedFile+=ch;
         }
    }
}

token JackTokenizer::tokenType(){
      for(int i=0;i<21;i++)
              if(currentToken==kw[i]) return KEYWORD;
      
      if(currentToken.size()==0){
          for(int i=0;i<19;i++)
                  if(currentToken[0]==Symbol[i]) return SYMBOL;
      }
      
      int flag=0;
      for(int i=0;i<currentToken.size();i++)
              if(currentToken[i]<'0'||currentToken[i]>'9'){flag=1; break;}
      if(flag==0) return INT_CONST;
      
      if(currentToken[0]=='"'&&currentToken[currentToken.size()-1]=='"') return STRING_CONST;
      
      if(currentToken[0]=='_'||isalpha(currentToken[0])){
          flag=0;                                                         
          for(int i=1;i<currentToken.size();i++)
                  if(!isalnum(currentToken[i])&&currentToken[i]!='_'){flag=1; break;}
          if(flag==0) return IDENTIFIER;
      }
}

int JackTokenizer::intVal(){
    int num=0;
    for(int i=0;i<currentToken.size();i++)
            num=(num*10)+currentToken[i]-'0';
    if(num>=0&&num<=32767) return num;
    cout << "Integer constant is not in the range 0-32767\n";
    exit(0);
}

int CompilationEngine::CompileClass(){
    string next=jt.tokenizedFile;
    string cur=next;
    cur.erase(5,cur.size()-5);
    if(cur=="class"){
        jt.currentToken=="class";
        jt.tokenptr+=5;
        next.erase(0,5);
        cur=next;
        cur.erase(cur.find("{"), cur.size()-cur.find("{"));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER){
             jt.tokenptr+=(next.find("{")+1);
             next.erase(0,(next.find("{")+1));
             while(next.size()>1){
                 CompileClassVarDec();
                 next=jt.tokenizedFile;
                 next.erase(0,jt.tokenptr);
                 CompileSubroutine();
                 next=jt.tokenizedFile;
                 next.erase(0,jt.tokenptr);
             }
             if(next=="}"){
                          jt.tokenptr+=1;
                          next.erase(0,1);
                          return 1;
             }
             cout << "} expected\n";
             return 0;
        }
        cout << "Identifier expected\n";
        return 0;
    }
    cout << "Keyword class expected\n";
    return 0;
}

int CompilationEngine::CompileClassVarDec(){
    string type, kind;
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(6,cur.size()-6);
    if(cur!="static")
    cur.erase(5,1);
    if(cur=="static"||cur=="field"){
        kind=cur;
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"){
            type=cur;
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            int turn=1;
            do{
                if(turn!=1){
                    jt.tokenptr+=1;
                    next.erase(0,1);
                    cur=next;
                    cur.erase(cur.find(" "),cur.size()-cur.find(" "));
                    jt.tokenptr++;
                    next.erase(0,1);
                }
                else{
                    cur=next;
                    cur.erase(cur.find(";"),cur.size()-cur.find(";"));
                    jt.currentToken=cur;
                    if(jt.tokenType()!=IDENTIFIER)
                    cur.erase(cur.find(","),cur.size()-cur.find(","));
                    jt.currentToken=cur;
                }
                if(jt.tokenType()==IDENTIFIER){
                    CSTab.define(cur,type,kind);
                    jt.tokenptr+=cur.size();
                    next.erase(0,cur.size());
                }
                turn++;
            }while(next[0]==',');
            if(next[0]==';'){
                jt.tokenptr+=1;
                return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

int CompilationEngine::CompileSubroutine(){
    string name,type;
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    MSTab.startnewSubroutine();
    string cur=next;
    cur.erase(11,cur.size()-11);
    if(cur!="constructor"){
        cur=next;
        cur.erase(8,cur.size()-8);
        if(cur!="function"){
            cur=next;
            cur.erase(6,cur.size()-6);
        }
    }
    if(cur=="constructor"||cur=="method"||cur=="function"){
        type=cur;
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"||cur=="void"){
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            cur=next;
            cur.erase(cur.find("("),cur.size()-cur.find("("));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER){
                name=cur;
                jt.tokenptr+=(cur.size()+1);
                next.erase(0,(cur.size()+1));
            }
            if(type=="method") MSTab.argCount=1;
            CompileParameterList();
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            if(next[0]==')'){
                jt.tokenptr+=1;
                next.erase(0,1);
            }
            if(next[0]=='{'){
                jt.tokenptr+=1;
                next.erase(0,1);
            }
            while(CompileVarDec());
            outfile << "function " << className << "." << name << " " << MSTab.varCount << endl;
            if(type=="method")
            outfile << "push argument 0\npop pointer 0\n";
            else if(type=="constructor"){
                 outfile << "push constant " << CSTab.fieldCount << endl
                         << "call Memory.alloc 1\npop pointer 0\n";
            }
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            CompileStatements();
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            if(next[0]=='}'){
                jt.tokenptr+=1;
                if(next[0]==' ') jt.tokenptr+=1;
                return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

int CompilationEngine::CompileParameterList(){
    string type, kind;
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur;
    if(next[0]==' '&&next[1]==')'){
        jt.tokenptr+=2;
        return 1;
    }
    kind="arg";
    do{
        if(next[0]==','){
            jt.tokenptr++;
            next.erase(0,1);
        }
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"){
            type=cur;
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            cur=next;
            if(cur.find(",")<cur.size())
            cur.erase(cur.find(","), cur.size()-cur.find(","));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER){
                MSTab.define(cur,type,kind);
                jt.tokenptr+=cur.size();
                next.erase(0,cur.size());
            }
            else{
                 cur=next;
                 cur.erase(cur.find(")"), cur.size()-cur.find(")"));
                 jt.currentToken=cur;
                 if(jt.tokenType()==IDENTIFIER){                                                            
                     MSTab.define(cur,type,kind);
                     jt.tokenptr+=cur.size();
                     next.erase(0,cur.size());
                 }
                 else return 0;
            }
        }
        else return 0;
    }while(next[0]==',');
    return 1;
}

int CompilationEngine::CompileVarDec(){
    string type, kind;
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(3,cur.size()-3);
    if(cur=="var"){
        kind=cur;
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"){
            type=cur;
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            int turn=1;
            do{
                if(turn!=1){
                    jt.tokenptr+=1;
                    next.erase(0,1);
                    cur=next;
                    cur.erase(cur.find(" "),cur.size()-cur.find(" "));
                    jt.tokenptr++;
                    next.erase(0,1);
                }
                else{
                    cur=next;
                    cur.erase(cur.find(";"),cur.size()-cur.find(";"));
                    jt.currentToken=cur;
                    if(jt.tokenType()!=IDENTIFIER)
                    cur.erase(cur.find(","),cur.size()-cur.find(","));
                    jt.currentToken=cur;
                }
                if(jt.tokenType()==IDENTIFIER){
                    MSTab.define(cur,type,kind);
                    jt.tokenptr+=cur.size();
                    next.erase(0,cur.size());
                }
                turn++;
            }while(next[0]==',');
            if(next[0]==';'){
                jt.tokenptr+=1;
                return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

int CompilationEngine::CompileStatements(){
    while(CompileLet()||CompileDo()||CompileIf()||CompileWhile());
    CompileReturn();
    return 1;
}

int CompilationEngine::CompileLet(){ 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(3,cur.size()-3);
    int flag=0;
    if(cur=="let"){
        jt.tokenptr+=3;
        next.erase(0,3);
        cur=next;
        if(cur.find("[")<cur.size()){
            cur.erase(cur.find("["), cur.size()-cur.find("["));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER) flag=1;
        }
        if(flag){
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,cur.size()+1);
            if(CompileExpression()){
                 outfile << "pop temp 7\n";
                 next=jt.tokenizedFile;
                 next.erase(0,jt.tokenptr);
                 if(next[0]==']'&&next[1]=='='){
                     jt.tokenptr+=2;
                     next.erase(0,2);
                     if(CompileExpression()){                                                
                         next=jt.tokenizedFile;
                         next.erase(0,jt.tokenptr);
                         if(next[0]==';'){
                             jt.tokenptr++;
                             if(MSTab.searchSymbol(cur))
                             MSTab.writePushPop(cur, outfile, true);
                             else if(CSTab.searchSymbol(cur))
                             CSTab.writePushPop(cur, outfile, true);
                             else{ cout << "Undeclared variable " << cur << endl; exit(0);}
                             outfile << "push temp 7\nadd\npop pointer 1\npop that 0\n";
                             return 1;
                         }
                     }
                 }
            }
        }
        else{
            cur=next;
            cur.erase(cur.find("="), cur.size()-cur.find("="));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER){
                jt.tokenptr+=(cur.size()+1);
                next.erase(0,cur.size()+1);
                if(CompileExpression()){
                    next=jt.tokenizedFile;
                    next.erase(0,jt.tokenptr);
                    if(next[0]==';'){
                         jt.tokenptr++;
                         if(MSTab.searchSymbol(cur))
                         MSTab.writePushPop(cur, outfile, false);
                         else if(CSTab.searchSymbol(cur))
                         CSTab.writePushPop(cur, outfile, false);
                         else{ cout << "Undeclared variable " << cur << endl; exit(0);}
                         return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int CompilationEngine::CompileIf(){
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(2,cur.size()-2);
    if(cur=="if"){
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        if(next[0]=='('){
            int ptr=next.find(" ");
            jt.tokenizedFile.erase(jt.tokenptr+ptr,1);
            jt.tokenptr++;
            next.erase(0,1);
            if(CompileExpression()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
                if(next[0]==')'&&next[1]=='{'){
                    int ic=ifCount++;
                    outfile << "not\nif-goto ELSE" << ic << endl;
                    jt.tokenptr+=2;
                    next.erase(0,2);
                    if(CompileStatements()){
                        next=jt.tokenizedFile;
                        next.erase(0,jt.tokenptr);
                        if(next[0]=='}'){
                            jt.tokenptr+=1;
                            next.erase(0,1);
                            cur=next;
                            cur.erase(4,cur.size()-4);
                            outfile << "goto ENDIF" << ic << "\nlabel ELSE" << ic << endl;
                            if(cur=="else"&&next[4]=='{'){
                                jt.tokenptr+=5;
                                next.erase(0,5);
                                if(CompileStatements()){
                                    next=jt.tokenizedFile;
                                    next.erase(0,jt.tokenptr);
                                    if(next[0]=='}'){
                                        jt.tokenptr+=1;
                                        next.erase(0,1);
                                    }
                                }
                            }
                            outfile << "label ENDIF" << ic << endl;
                            return 1;              
                        }
                    }
                }
            }
        }
    }
    return 0;       
}

int CompilationEngine::CompileWhile(){ 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(5,cur.size()-5);
    if(cur=="while"){         
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        if(next[0]=='('){
            int ptr=next.find(" ");
            jt.tokenizedFile.erase(jt.tokenptr+ptr,1);
            jt.tokenptr++;
            next.erase(0,1);
            int wc=whileCount++;
            outfile << "label WHILE" << wc << endl;
            if(CompileExpression()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
                if(next[0]==')'&&next[1]=='{'){
                    outfile << "not\nif-goto ENDWHILE" << wc << endl;
                    jt.tokenptr+=2;
                    next.erase(0,2);
                    if(CompileStatements()){
                        next=jt.tokenizedFile;
                        next.erase(0,jt.tokenptr);
                        if(next[0]=='}'){
                            outfile << "goto WHILE" << wc 
                                    << "\nlabel ENDWHILE" << wc << endl;
                            jt.tokenptr+=1;
                            next.erase(0,1);
                            return 1;
                        }
                    }
                }
            }
        }
    } 
    return 0;
}  

int CompilationEngine::CompileReturn(){ 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(6,cur.size()-6);
    if(cur=="return"){
        jt.tokenptr+=cur.size();
        next=jt.tokenizedFile;
        next.erase(0,jt.tokenptr);
        if(next[0]!=';'){
            if(CompileExpression()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
            }
        }
        else outfile << "push constant 0\n";        
        if(next[0]==';'){
            outfile << "return\n";
            jt.tokenptr+=1;
            next.erase(0,1);
            return 1;
        }
    }
    return 0;
}

int CompilationEngine::CompileDo(){ 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(2,cur.size()-2);
    if(cur=="do"){         
        jt.tokenptr+=cur.size();
        if(CompileSubroutineCall()){
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            if(next[0]==';'){
                jt.tokenptr++;
                outfile << "pop temp 6\n";
                return 1;
            }
        }
    }
    return 0;
}

int CompilationEngine::CompileSubroutineCall(){
    string next=jt.tokenizedFile;
    next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    int flag=0;
    string cur=next;
    if(cur.find(".")<cur.size())
    cur.erase(cur.find("."), cur.size()-cur.find("."));
    jt.currentToken=cur;
    if(jt.tokenType()==IDENTIFIER){
        jt.tokenptr+=cur.size()+1;
        next.erase(0,cur.size()+1);
        flag=1;
    }
    string cur1=next;
    cur1.erase(cur1.find("("), cur1.size()-cur1.find("("));
    jt.currentToken=cur1;
    if(jt.tokenType()==IDENTIFIER){
        jt.tokenptr+=cur1.size()+1;
        next.erase(0,cur1.size()+1);
        if(flag==1){
            if(CSTab.searchSymbol(cur)||MSTab.searchSymbol(cur)){
                CSTab.searchSymbol(cur) ? jt.currentToken=CSTab.typeOf(cur)
                                        : jt.currentToken=MSTab.typeOf(cur);
                if(jt.tokenType()==IDENTIFIER)
                CSTab.searchSymbol(cur) ? CSTab.writePushPop(cur, outfile, true)
                                        : MSTab.writePushPop(cur, outfile, true);
                else{
                     cout << cur << " is not an object and cannot invoke a method\n";
                     exit(0);
                }
                cur=jt.currentToken;
                flag++;
            }
            cur1=(cur+"."+cur1);
            flag--;
        }
        else{
             outfile << "push pointer 0\n";
             cur1=(className+"."+cur1);
             flag=1;
        }
    }
    else return 0;
    int numOfArgs=0;
    if(next[0]==' '){
        jt.tokenptr++;
        next.erase(0,1);
    }
    if(next[0]==')'){
        outfile << "call " << cur1 << " " << numOfArgs+flag << endl;
        jt.tokenptr++;
        return 1;
    }
    do{
         if(next[0]==','){
             numOfArgs++;
             jt.tokenptr++;
             next.erase(0,1);
             if(next[0]==' '){
                 jt.tokenptr++;
                 next.erase(0,1);
             }
         }
         if(CompileExpression()){
             next=jt.tokenizedFile;
             next.erase(0,jt.tokenptr);
             if(next[0]==' '){
                 jt.tokenptr++;
                 next.erase(0,1);
             }
             if(next[0]==')'){
                 numOfArgs++;
                 outfile << "call " << cur1 << " " << numOfArgs+flag << endl;
                 jt.tokenptr++;
                 next.erase(0,1);
                 return 1;
             }
         }
    }while(next[0]==',');
    return 0;
}

int CompilationEngine::CompileExpression(){
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    if(CompileTerm()){
        next=jt.tokenizedFile;
        next.erase(0,jt.tokenptr);
        while(next[0]==' '){
          jt.tokenptr++; 
          next.erase(0,1);
        }
        while(isopr(next[0])){
            char opr=next[0];
            jt.tokenptr++;
            next.erase(0,1);
            if(CompileTerm()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
            }
            else return 0;
            switch(opr){
                case '+':   outfile << "add\n";
                            break;
                case '-':   outfile << "sub\n";
                            break;
                case '*':   outfile << "call Math.multiply 2\n";
                            break;
                case '/':   outfile << "call Math.divide 2\n";
                            break;
                case '&':   outfile << "and\n";
                            break;
                case '|':   outfile << "or\n";
                            break;
                case '<':   outfile << "lt\n";
                            break;
                case '>':   outfile << "gt\n";
                            break;
                case '=':   outfile << "eq\n";
                            break;
            }
        }
        return 1;
    }
}

int CompilationEngine::CompileTerm(){
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    int flag=0;
    string cur=next,cur1=next;
    cur.erase(4,cur.size()-4);
    cur1.erase(5,cur1.size()-5);
    if(cur1=="false") cur="false";
    if(cur=="true"||cur=="null"||cur=="this"||cur=="false"){
        if(cur=="null"||cur=="false") outfile << "push constant 0\n";
        else if(cur=="true") outfile << "push constant 1\nneg\n";
        else outfile << "push pointer 0\n";
        jt.tokenptr+=cur.size();
        flag=1;
    }
    else if(next[0]=='('){
         jt.tokenptr++;
         next.erase(0,1);
         if(next[0]==' '){
          jt.tokenptr++; 
          next.erase(0,1);
         }
         if(CompileExpression()){
             next=jt.tokenizedFile;
             next.erase(0,jt.tokenptr);
             if(next[0]==' '){next.erase(0,1); jt.tokenptr++;}
             if(next[0]==')'){
                 jt.tokenptr++;
                 next.erase(0,1);
                 flag=1;
             }
         }
    }
    else if(next[0]=='~'||next[0]=='-'){
         char opr=next[0];
         jt.tokenptr++;
         next.erase(0,1);
         if(CompileTerm()){
             flag=1;
             if(opr=='~') outfile << "not\n";
             else outfile << "neg\n";
         }      
    }
    else if(next[0]=='\"'){
         jt.tokenptr++;
         next.erase(0,1);
         cur=next;
         cur.erase(cur.find("\""),cur.size()-cur.find("\""));
         string cur1=cur;
         cur="";
         for(int i=0;i<cur1.size();i++)
         if(cur1[i]!='^') cur+=cur1[i];
         outfile << "push constant " << cur.size() << endl
                 << "call String.new 1\n";
         for(int i=0;i<cur.size();i++){
             outfile << "push constant " << (int)(cur[i]) << endl
                     << "call String.appendChar 2\n";
         }
         jt.tokenptr+=cur1.size()+1;
         next.erase(0,cur1.size()+1);
         flag=1;
    }
    else if(isdigit(next[0])){
         cur=next;
         cur.erase(cur.find("^"),cur.size()-cur.find("^"));
         jt.currentToken=cur;
         if(jt.tokenType()==INT_CONST){
             outfile << "push constant " << jt.intVal() << endl;
             jt.tokenptr+=cur.size()+1;
             next.erase(0,cur.size()+1);
             flag=1;
         }
    }
    else{
         cur="";
         for(int i=0;(isalnum(next[i])||next[i]=='_')&&(!isdigit(next[0]));i++)
         cur+=next[i];
         if(cur.size()>0&&next[cur.size()]!='('&&next[cur.size()]!='.'){
             if(MSTab.searchSymbol(cur))
             MSTab.writePushPop(cur, outfile, true);
             else if(CSTab.searchSymbol(cur))
             CSTab.writePushPop(cur, outfile, true);
             else{ cout << "Undeclared variable " << cur << endl; exit(0);}
             jt.tokenptr+=cur.size();
             next.erase(0,cur.size());
             flag=1;
             if(next[0]==' '){
                 jt.tokenptr++;
                 next.erase(0,1);
             }
             if(next[0]=='['){
                 jt.tokenptr++;
                 next.erase(0,1);
                 if(CompileExpression()){
                     next=jt.tokenizedFile;
                     next.erase(0,jt.tokenptr);
                     if(next[0]==']'){
                         outfile << "add\npop pointer 1\npush that 0\n";
                         jt.tokenptr++;
                         next.erase(0,1);
                     }
                     else flag=0;
                 }
                 else flag=0;
             }
         }
         else if(CompileSubroutineCall()){
             next=jt.tokenizedFile;
             next.erase(0,jt.tokenptr);
             flag=1;
         }
    }
    if(flag==1)
        return 1;
    return 0;
}

int main(int argc, char*argv[]){
    if(argc==2){
        CompilationEngine c(argv[1]);
        cout << argv[1] << " successfully compiled\n";
    }
    else cout << "Correct Syntax is: ./JackCompiler.exe <filename>\n";
    return 0;
}

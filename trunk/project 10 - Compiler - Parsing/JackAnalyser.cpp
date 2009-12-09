#include<iostream>
#include<fstream>
#include<string>
#include<cctype>
#undef NULL
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
                          getchar();
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
      JackTokenizer jt;
      public:
             CompilationEngine(char *infilename, char* outfilename)
             :jt(infilename)
             {              outfile.open(outfilename,ios::out); 
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
    outfile << "<class>\n";
    string next=jt.tokenizedFile;
    string cur=next;
    cur.erase(5,cur.size()-5);
    if(cur=="class"){
        jt.currentToken=="class";
        outfile << "<keyword> class </keyword>\n";
        jt.tokenptr+=5;
        next.erase(0,5);
        cur=next;
        cur.erase(cur.find("{"), cur.size()-cur.find("{"));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER){
             outfile << "<identifier>" << cur << "</identifier>\n";
             outfile << "<symbol> { </symbol>\n";
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
                          outfile << "<symbol> } </symbol>\n";
                          jt.tokenptr+=1;
                          next.erase(0,1);
                          outfile << "</class>\n";
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
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(6,cur.size()-6);
    if(cur!="static")
    cur.erase(5,1);
    if(cur=="static"||cur=="field"){
        outfile << "<classVarDec>\n"; 
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"){
            if(jt.tokenType()==IDENTIFIER)
            outfile << "<identifier> " << cur << " </identifier>\n";
            else
            outfile << "<keyword> " << cur << " </keyword>\n";
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            int turn=1;
            do{
                if(turn!=1){
                    outfile << "<symbol> , </symbol>\n";
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
                    outfile << "<identifier> " << cur << "</identifier>\n";
                    jt.tokenptr+=cur.size();
                    next.erase(0,cur.size());
                }
                turn++;
            }while(next[0]==',');
            if(next[0]==';'){
                             outfile << "<symbol> ; </symbol>\n";
                             jt.tokenptr+=1;
                             next.erase(0,1);
                             outfile << "</classVarDec>\n";
                             return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

int CompilationEngine::CompileSubroutine(){
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
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
        outfile << "<subroutineDec>\n";
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"||cur=="void"){
            if(jt.tokenType()==IDENTIFIER)
            outfile << "<identifier> " << cur << " </identifier>\n";
            else
            outfile << "<keyword> " << cur << " </keyword>\n";
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            cur=next;
            cur.erase(cur.find("("),cur.size()-cur.find("("));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER){
                outfile << "<identifier> " << cur << "</identifier>\n";
                outfile << "<symbol> ( </symbol>\n";
                jt.tokenptr+=(cur.size()+1);
                next.erase(0,(cur.size()+1));
            }
            CompileParameterList();
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            if(next[0]==')'){
                outfile << "<symbol> ) </symbol>\n";
                jt.tokenptr+=1;
                next.erase(0,1);
            }
            outfile << "<subroutineBody>\n";
            if(next[0]=='{'){
                outfile << "<symbol> { </symbol>\n";
                jt.tokenptr+=1;
                next.erase(0,1);
            }
            while(CompileVarDec());
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            CompileStatements();
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            if(next[0]=='}'){
                outfile << "<symbol> } </symbol>\n";
                jt.tokenptr+=1;
                if(next[0]==' ') jt.tokenptr+=1;
                outfile << "</subroutineBody>\n";
                outfile << "</subroutineDec>\n";
                return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

int CompilationEngine::CompileParameterList(){
    outfile << "<parameterList>\n";
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur;
    if(next[0]==' '&&next[1]==')'){ 
        outfile << "</parameterList>\n";
        outfile << "<symbol> ) </symbol>\n";
        jt.tokenptr+=2; 
        return 1;
    }
    do{
        if(next[0]==','){
            outfile << "<symbol> , </symbol>\n";
            jt.tokenptr++;
            next.erase(0,1);
        }
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"){
            if(jt.tokenType()==IDENTIFIER)
            outfile << "<identifier> " << cur << " </identifier>\n";
            else
            outfile << "<keyword> " << cur << " </keyword>\n";
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            cur=next;
            cur.erase(cur.find(","), cur.size()-cur.find(","));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER){
                outfile << "<identifier> " << cur << "</identifier>\n";
                jt.tokenptr+=cur.size();
                next.erase(0,cur.size());
            }
            else{
                 cur=next;
                 cur.erase(cur.find(")"), cur.size()-cur.find(")"));
                 jt.currentToken=cur;
                 if(jt.tokenType()==IDENTIFIER){
                     outfile << "<identifier> " << cur << "</identifier>\n";
                     jt.tokenptr+=cur.size();
                     next.erase(0,cur.size());
                 }
                 else return 0;
            }
        }
        else return 0;
    }while(next[0]==',');
    outfile << "</parameterList>\n";
    return 1;
}

int CompilationEngine::CompileVarDec(){
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(3,cur.size()-3);
    if(cur=="var"){
        outfile << "<varDec>\n"; 
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        cur=next;
        cur.erase(cur.find(" "), cur.size()-cur.find(" "));
        jt.currentToken=cur;
        if(jt.tokenType()==IDENTIFIER||cur=="int"||cur=="char"||cur=="boolean"){
            if(jt.tokenType()==IDENTIFIER)
            outfile << "<identifier> " << cur << " </identifier>\n";
            else
            outfile << "<keyword> " << cur << " </keyword>\n";
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,(cur.size()+1));
            int turn=1;
            do{
                if(turn!=1){
                    outfile << "<symbol> , </symbol>\n";
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
                    outfile << "<identifier> " << cur << "</identifier>\n";
                    jt.tokenptr+=cur.size();
                    next.erase(0,cur.size());
                }
                turn++;
            }while(next[0]==',');
            if(next[0]==';'){
                             outfile << "<symbol> ; </symbol>\n";
                             jt.tokenptr+=1;
                             outfile << "</varDec>\n";
                             return 1;
            }
            return 0;
        }
        return 0;
    }
    return 0;
}

int CompilationEngine::CompileStatements(){
    outfile << "<statements>\n";
    while(CompileLet()||CompileDo()||CompileIf()||CompileWhile());
    CompileReturn();
    outfile << "</statements>\n";
    return 1;
}

int CompilationEngine::CompileLet(){ 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    string cur=next;
    cur.erase(3,cur.size()-3);
    int flag=0;
    if(cur=="let"){     
        outfile << "<letStatement>\n";    
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=3;
        next.erase(0,3);
        cur=next;
        if(cur.find("[")<cur.size()){
            cur.erase(cur.find("["), cur.size()-cur.find("["));
            jt.currentToken=cur;
            if(jt.tokenType()==IDENTIFIER) flag=1;
        }
        if(flag){
            outfile << "<identifier> " << cur << " </identifier>\n";
            outfile << "<symbol> [ </symbol>\n";
            jt.tokenptr+=(cur.size()+1);
            next.erase(0,cur.size()+1);
            if(CompileExpression()){
                 next=jt.tokenizedFile;
                 next.erase(0,jt.tokenptr);
                 if(next[0]==']'&&next[1]=='='){
                     outfile << "<symbol> ] </symbol>\n";
                     outfile << "<symbol> = </symbol>\n";
                     jt.tokenptr+=2;
                     next.erase(0,2);
                     if(CompileExpression()){                                                
                         next=jt.tokenizedFile;
                         next.erase(0,jt.tokenptr);
                         if(next[0]==';'){
                             outfile << "<symbol> ; </symbol>\n";
                             jt.tokenptr++;
                             outfile << "</letStatement>\n"; 
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
                outfile << "<identifier> " << cur << " </identifier>\n";
                outfile << "<symbol> = </symbol>\n";
                jt.tokenptr+=(cur.size()+1);
                next.erase(0,cur.size()+1);
                if(CompileExpression()){                                                
                    next=jt.tokenizedFile;
                    next.erase(0,jt.tokenptr);
                    if(next[0]==';'){
                         outfile << "<symbol> ; </symbol>\n";
                         jt.tokenptr++;
                         outfile << "</letStatement>\n"; 
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
        outfile << "<ifStatement>\n";   
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        if(next[0]=='('){
            outfile << "<symbol> ( </symbol>\n";
            int ptr=next.find(" ");
            jt.tokenizedFile.erase(jt.tokenptr+ptr,1);
            jt.tokenptr++;
            next.erase(0,1);
            if(CompileExpression()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
                if(next[0]==')'&&next[1]=='{'){
                    outfile << "<symbol> ) </symbol>\n";
                    outfile << "<symbol> { </symbol>\n";
                    jt.tokenptr+=2;
                    next.erase(0,2);
                    if(CompileStatements()){
                        next=jt.tokenizedFile;
                        next.erase(0,jt.tokenptr);
                        if(next[0]=='}'){
                            outfile << "<symbol> } </symbol>\n";
                            jt.tokenptr+=1;
                            next.erase(0,1);
                            cur=next;
                            cur.erase(4,cur.size()-4);
                            if(cur=="else"&&next[4]=='{'){
                                outfile << "<keyword> else </keyword>\n";
                                outfile << "<symbol> { </symbol>\n";
                                jt.tokenptr+=5;
                                next.erase(0,5);
                                if(CompileStatements()){
                                    next=jt.tokenizedFile;
                                    next.erase(0,jt.tokenptr);
                                    if(next[0]=='}'){
                                        outfile << "<symbol> } </symbol>\n";
                                        jt.tokenptr+=1;
                                        next.erase(0,1);
                                    }
                                }
                            }
                            outfile << "</ifStatement>\n";
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
        outfile << "<whileStatement>\n";
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        next.erase(0,cur.size());
        if(next[0]=='('){
            outfile << "<symbol> ( </symbol>\n";
            int ptr=next.find(" ");
            jt.tokenizedFile.erase(jt.tokenptr+ptr,1);
            jt.tokenptr++;
            next.erase(0,1);
            if(CompileExpression()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
                if(next[0]==')'&&next[1]=='{'){
                    outfile << "<symbol> ) </symbol>\n";
                    outfile << "<symbol> { </symbol>\n";
                    jt.tokenptr+=2;
                    next.erase(0,2);
                    if(CompileStatements()){
                        next=jt.tokenizedFile;
                        next.erase(0,jt.tokenptr);
                        if(next[0]=='}'){
                            outfile << "<symbol> } </symbol>\n";
                            jt.tokenptr+=1;
                            next.erase(0,1);
                            outfile << "</whileStatement>\n";
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
        outfile << "<returnStatement>\n";
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        next=jt.tokenizedFile;
        next.erase(0,jt.tokenptr);
        if(next[0]!=';'){
            if(CompileExpression()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
            }
        }         
        if(next[0]==';'){
            outfile << "<symbol> ; </symbol>\n";
            jt.tokenptr+=1;
            next.erase(0,1);
            outfile << "</returnStatement>\n";
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
        outfile << "<doStatement>\n";
        outfile << "<keyword> " << cur << " </keyword>\n";
        jt.tokenptr+=cur.size();
        if(CompileSubroutineCall()){
            next=jt.tokenizedFile;
            next.erase(0,jt.tokenptr);
            if(next[0]==';'){
                outfile << "<symbol> ; </symbol>\n";
                jt.tokenptr++;
                outfile << "</doStatement>\n";
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
        outfile << "<identifier> " << cur << " </identifier>\n";
        outfile << "<symbol> . </symbol>\n";
        jt.tokenptr+=cur.size()+1;
        next.erase(0,cur.size()+1);
        flag=1;
    }
    string cur1=next;
    cur1.erase(cur1.find("("), cur1.size()-cur1.find("("));
    jt.currentToken=cur1;
    if(jt.tokenType()==IDENTIFIER){
        outfile << "<identifier> " << cur1 << " </identifier>\n";
        outfile << "<symbol> ( </symbol>\n";
        outfile << "<expressionList>\n";
        jt.tokenptr+=cur1.size()+1;
        next.erase(0,cur1.size()+1);
    }
    else return 0;
    if(next[0]==' '){
        jt.tokenptr++;
        next.erase(0,1);
    }
    if(next[0]==')'){
        outfile << "</expressionList>\n";
        outfile << "<symbol> ) </symbol>\n";
        jt.tokenptr++;
        return 1;
    }
    do{
         if(next[0]==','){
             outfile << "<symbol> , </symbol>\n";
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
                 outfile << "</expressionList>\n";
                 outfile << "<symbol> ) </symbol>\n";
                 jt.tokenptr++;
                 next.erase(0,1);
                 return 1;
             }
         }
    }while(next[0]==',');
    return 0;
}

int CompilationEngine::CompileExpression(){
    outfile << "<expression>\n"; 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    if(CompileTerm()){
        next=jt.tokenizedFile;
        next.erase(0,jt.tokenptr);
        if(next[0]==' '){
          jt.tokenptr++; 
          next.erase(0,1);
        }
        while(isopr(next[0])){
            string opr="";
            opr+=next[0];
            if(next[0]=='<') opr="&lt;";
            else if(next[0]=='>') opr="&gt;";
            else if(next[0]=='"') opr="&quot;";
            else if(next[0]=='&') opr="&amp;";
            outfile << "<symbol> " << opr << " </symbol>\n";
            jt.tokenptr++;
            next.erase(0,1);
            if(CompileTerm()){
                next=jt.tokenizedFile;
                next.erase(0,jt.tokenptr);
            }
            else return 0;
        }
        outfile << "</expression>\n";
        return 1;
    }
}

int CompilationEngine::CompileTerm(){
    outfile << "<term>\n"; 
    string next=jt.tokenizedFile;
    next.erase(0,jt.tokenptr);
    int flag=0;
    string cur=next,cur1=next;
    cur.erase(4,cur.size()-4);
    cur1.erase(5,cur1.size()-5);
    if(cur1=="false") cur="false";
    if(cur=="true"||cur=="null"||cur=="this"||cur=="false"){
        outfile << "<keyword> " << cur << "</keyword>\n";
        jt.tokenptr+=cur.size();
        flag=1;
    }
    else if(next[0]=='('){
         outfile << "<symbol> ( </symbol>\n";
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
                 outfile << "<symbol> ) </symbol>\n";
                 jt.tokenptr++;
                 next.erase(0,1);
                 flag=1;
             }
         }
    }
    else if(next[0]=='~'||next[0]=='-'){
         outfile << "<symbol> " << next[0] << " </symbol>\n";
         jt.tokenptr++;
         next.erase(0,1);
         if(CompileTerm())  flag=1;           
    }
    else if(next[0]=='\"'){
         jt.tokenptr++;
         next.erase(0,1);
         cur=next;
         cur.erase(cur.find("\""),cur.size()-cur.find("\""));
         outfile << "<stringConstant> " << cur << " </stringConstant>\n";
         jt.tokenptr+=cur.size()+1;
         next.erase(0,cur.size()+1);
         flag=1;
    }
    else if(isdigit(next[0])){
         cur=next;
         cur.erase(cur.find("^"),cur.size()-cur.find("^"));
         jt.currentToken=cur;
         if(jt.tokenType()==INT_CONST){
             outfile << "<integerConstant> " << jt.intVal() << " </integerConstant>\n";
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
             outfile << "<identifier> " << cur << " </identifier>\n";
             jt.tokenptr+=cur.size();
             next.erase(0,cur.size());
             flag=1;
             if(next[0]=='['){
                 outfile << "<symbol> [ </symbol>\n";
                 jt.tokenptr++;
                 next.erase(0,1);
                 if(CompileExpression()){
                     next=jt.tokenizedFile;
                     next.erase(0,jt.tokenptr);
                     if(next[0]==']'){
                         outfile << "<symbol> ] </symbol>\n";
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
    if(flag==1){
        outfile << "</term>\n";
        return 1;
    }
    return 0;
}

int main(int argc, char*argv[]){
    if(argc==2){
    string jack=argv[1],xml=argv[1];
    xml.erase(xml.size()-5,5);
    xml+="1.xml";
    CompilationEngine c((char*)jack.c_str(),(char*)xml.c_str());
    cout << jack << " successfully parsed\n";}
    else
    cout << "Correct Syntax is: ./JackAnalyser.exe <filename>\n";
    getchar();
    return 0;
}

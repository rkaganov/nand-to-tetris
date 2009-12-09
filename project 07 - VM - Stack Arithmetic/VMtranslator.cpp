#include<fstream>
#include<iostream>
#include<string>
#define CALL_STACK_MAX 200
using namespace std;

enum cmdType {C_ARITHMETIC, C_PUSH, C_POP, C_LABEL, C_GOTO, C_IF, C_FUNCTION, C_RETURN, C_CALL};

class Parser{
      fstream VM;
      int lineNumber;
      string currentCommand;
      public:
             Parser(char* filename){
                          VM.open(filename,ios::in);
                          if(VM.fail()){ 
                          cout << filename << " cannot be found\nPlease check whether you have entered the correct pathname\n";
                          getchar(); exit(0);
                          }
                          VM.unsetf(ios::skipws);
                          lineNumber=0;
             }
             ~Parser(){VM.close();}
             bool hasMoreCommands(){
                  if(VM.fail()) return false;
                  return true;
             }
             int advance();
             cmdType commandType();
             void argument(string&,int&);
};

class CodeWriter{
      fstream Asm;
      string fileName, funcName[CALL_STACK_MAX];
      int trueCount,callCount,curFunction;
      public:
             CodeWriter(char* filename){
                              fileName=filename;
                              string file=fileName;
                              file.erase(file.size()-2,2);
                              file+="asm";
                              Asm.open(file.c_str(),ios::out);
                              fileName.erase(0,fileName.find_last_of('/')+1);
                              trueCount=0;
                              curFunction=0;
                              callCount=0;
             }
             ~CodeWriter(){
                           Asm << "(INFINITE_LOOP)\n@INFINITE_LOOP\n0;JEQ\n";
                           Asm.close();
             }                         
             void writeInit();
             void writeArithmetic(string);
             void writePushPop(cmdType,string,int);
             void writeLabel(string);
             void writeGoto(string);
             void writeIf(string);
             void writeCall(string,int);
             void writeReturn();
             void writeFunction(string,int);
};   
                    
int  Parser::advance(){
     char ch;
     string funcCommand;
     currentCommand.erase(0,currentCommand.size());
     while(hasMoreCommands()){
                 VM >> ch;
                 if(ch=='\n'){
                              if(currentCommand.size()!=0){lineNumber++; break;}
                              else continue;
                 }
                 if(ch!=' '&&ch!='\t'&&ch!='\n'&&ch!='\r'){currentCommand+=ch; funcCommand+=ch;}
                 if(ch==' '||ch=='\t'){funcCommand+=' ';}
                 if(currentCommand[currentCommand.size()-1]=='/'&&currentCommand[currentCommand.size()-2]=='/'){
                           currentCommand.erase(currentCommand.size()-2, 2);
                           funcCommand.erase(funcCommand.size()-2,2);
                           VM >> ch;
                           while(ch!='\n') VM >> ch; 
                           if(currentCommand.size()!=0){
                           lineNumber++; break;}                                     
                 }
     }
     if(!hasMoreCommands()) return -1;
     if(commandType()==C_FUNCTION||commandType()==C_CALL) currentCommand=funcCommand;
     return 0;
}

cmdType Parser::commandType(){
        if(currentCommand=="add"||currentCommand=="sub"||currentCommand=="neg"||
           currentCommand=="and"||currentCommand=="not"||currentCommand=="eq"||
           currentCommand=="gt"||currentCommand=="lt"||currentCommand=="or")
           return C_ARITHMETIC;
        if(currentCommand.find("push")==0) return C_PUSH;
        if(currentCommand.find("pop")==0) return C_POP;
        if(currentCommand.find("label")==0) return C_LABEL;
        if(currentCommand.find("goto")==0) return C_GOTO;
        if(currentCommand.find("if-goto")==0) return C_IF; 
        if(currentCommand.find("function")==0) return C_FUNCTION;
        if(currentCommand.find("call")==0) return C_CALL;
        if(currentCommand=="return") return C_RETURN;
        cout << lineNumber << ": Invalid command\n";              
        getchar();
        exit(0);
}

void Parser::argument(string& arg1, int& arg2){
    string arg=currentCommand;
    if(commandType()==C_ARITHMETIC)arg1=arg;
    else if(commandType()==C_PUSH||commandType()==C_POP){
         if(commandType()==C_PUSH) arg.erase(0,4);
         else arg.erase(0,3);
         if(arg.find("argument")==0) arg1="argument";
         else if(arg.find("local")==0) arg1="local";
         else if(arg.find("static")==0) arg1="static";
         else if(arg.find("this")==0) arg1="this";
         else if(arg.find("that")==0) arg1="that"; 
         else if(arg.find("pointer")==0) arg1="pointer";
         else if(arg.find("temp")==0) arg1="temp";
         else if(arg.find("constant")==0) arg1="constant";
         else { cout << lineNumber << ": Segment name missing in a Push/Pop instruction\n"; 
                getchar(); 
                exit(0); }
         arg.erase(0,arg1.size());
         if(arg.size()==0) { 
             cout << lineNumber << ": Segment index missing in a Push/Pop instruction\n"; 
             getchar(); 
             exit(0); 
         }
         arg2=0;
         for(int i=0;i<arg.size();i++)
         arg2=(arg2*10+arg[i]-'0');
    }
    else if(commandType()==C_LABEL){ arg.erase(0,5); arg1=arg; }
    else if(commandType()==C_GOTO){ arg.erase(0,4); arg1=arg; }
    else if(commandType()==C_IF){ arg.erase(0,7); arg1=arg; }
    else if(commandType()==C_FUNCTION||commandType()==C_CALL){
         if(commandType()==C_FUNCTION) arg.erase(0,9);
         else arg.erase(0,5);
         arg1=arg;
         arg.erase(0,arg.find(" ")+1);
         arg1.erase(arg1.find(" "),arg.size()+1);
         if(arg.size()==0) { 
             cout << lineNumber << ": Number of arguments not specified in a Function/Call instruction\n"; 
             getchar(); 
             exit(0); 
         }
         arg2=0;
         for(int i=0;i<arg.size();i++)
         arg2=(arg2*10+arg[i]-'0');
    }
}

void CodeWriter::writeInit(){
     Asm << "@256\nD=A\n@SP\nM=D\n)";
     writeCall("Sys.init",0);
}

void CodeWriter::writeArithmetic(string command){
     Asm << "@SP\nA=M-1\n";
     if(command=="neg") Asm << "M=-M\n";
     else if(command=="not") Asm << "M=!M\n";
     else{
          Asm << "D=M\n@SP\nAM=M-1\nM=0\nA=A-1\n";
          if(command=="add") Asm << "M=M+D\n";
          else if(command=="sub") Asm << "M=M-D\n";
          else if(command=="and") Asm << "M=M&D\n";
          else if(command=="or") Asm << "M=M|D\n";
          else{
               Asm << "D=M-D\n@TRUE" << trueCount << "\nD;J";
               if(command=="eq") Asm << "EQ";
               else if(command=="lt") Asm << "LT";
               else if(command=="gt") Asm << "GT";
               Asm << "\n@SP\nA=M-1\nM=0\n@FALSE" << trueCount << "\n0;JEQ\n(TRUE" << trueCount 
                   << ")\n@SP\nA=M-1\nM=-1\n" << "(FALSE" << trueCount << ")\n";
               trueCount++;
          }
     }
}

void CodeWriter::writePushPop(cmdType command, string segment, int index){
     if(segment=="static"&&index>240){
     cout << "Index of a static variable > 240\n"; getchar(); exit(0); }
     string strIndex;
     strIndex+=(index+'0');
     if(command==C_POP&&segment=="constant"){
         cout << "Popping into a constant is invalid\n"; 
         getchar(); 
         exit(0);
     }
     if(segment=="local") Asm << "@LCL\nD=M";
     else if(segment=="argument") Asm << "@ARG\nD=M";
     else if(segment=="this") Asm << "@THIS\nD=M";
     else if(segment=="that") Asm << "@THAT\nD=M";
     else if(segment=="temp") Asm << "@5\nD=A";
     else if(segment=="pointer") Asm << "@3\nD=A";
     else if(segment=="static") Asm << "@" << fileName << "." << index << "\nD=A";
     else if(segment=="constant"){
          if(index==0||index==1) Asm << "\nD=" << index;
          else Asm << "@" << index << "\nD=A";
     }
     if(segment!="static"&&segment!="constant"&&index!=0)
         Asm << "\n@" << index << "\nD=D+A";    
     if(command==C_PUSH){
         if(segment=="constant") Asm << "\n@SP\nAM=M+1\nA=A-1\nM=D\n";
         else Asm << "\nA=D\nD=M\n@SP\nAM=M+1\nA=A-1\nM=D\n";
     }
     else Asm << "\n@R13\nM=D\n@SP\nAM=M-1\nD=M\nM=0\n@R13\nA=M\nM=D\n";
}

void CodeWriter::writeLabel(string label){
     Asm << "(" << funcName[curFunction-1] << "." << label << ")\n";
}

void CodeWriter::writeGoto(string label){
     Asm << "@" << funcName[curFunction-1] << "." << label << "\n0;JMP\n";
}

void CodeWriter::writeIf(string label){
     Asm << "@SP\nA=M-1\nD=M\n@" << funcName[curFunction-1] << "." << label 
         << "\nD=D+1;JEQ\n";
}

void CodeWriter::writeCall(string functionName,int numArgs){
     callCount++;
     string pushString="\nD=A\n@SP\nAM=M+1\nA=A-1\nM=D\n";
     Asm << "@RETURN_ADDRESS." << callCount << pushString
         << "@LCL" << pushString
         << "@ARG" << pushString
         << "@THIS" << pushString
         << "@THAT" << pushString
         << "@SP\nD=A\n@" << numArgs << "\nD=D-A\n@5\nD=D-A\n@ARG\nM=D\n"
         << "@SP\nD=A\n@LCL\nM=D\n";
     writeGoto(functionName);
     Asm << "(" << "RETURN_ADDRESS." << callCount << ")\n";
     funcName[curFunction++]=functionName;
}

void CodeWriter::writeReturn(){
     Asm << "@LCL\nD=A\n@R13\nM=D\n"
         << "@5\nAD=D-A\nD=M\n@R14\nM=D\n"
         << "@SP\nM=M-1\nA=M\nD=M\nM=0\n@ARG\nM=D\n"
         << "@ARG\nD=A+1\n@SP\nM=D\n"
         << "@R13\nD=M-1\n@THAT\nM=D\n"
         << "@THIS\nDM=D-1\n"
         << "@ARG\nDM=D-1\n"
         << "@LCL\nDM=D-1\n"
         << "@R14\nA=M\n0;JMP\n";
     curFunction--;
}

void CodeWriter::writeFunction(string functionName, int numLocals){
     Asm << "@" << numLocals << "\nD=A\n" 
         << "(" << funcName[curFunction-1] << "." << functionName << ")\n"
         << "@" << funcName[curFunction-1] << "." << functionName << "\n"
         << "@SP\nAM=M+1\nA=A-1\nM=0\n"
         << "D=D-1;JEQ";
}

void translate(char* filename){
     Parser p(filename);
     CodeWriter c(filename);
     while(p.hasMoreCommands()){
         string command;
         int index=-1;
         if(p.advance()==-1) return;
         if(p.commandType()!=C_RETURN)
         p.argument(command, index);
         if(p.commandType()==C_ARITHMETIC){
             if(index==-1) c.writeArithmetic(command);
             else{
                  cout << "Invalid Arithmetic Command\n"; getchar(); exit(0);
             }
         }
         else if(p.commandType()==C_PUSH||p.commandType()==C_POP)
         c.writePushPop(p.commandType(),command,index);
     }
}

int main(int argc, char** argv){
    if(argc==2){
    translate(argv[1]);
    cout << argv[1] << " successfully translated\n";}
    else
    cout << "Correct Syntax is: ./VMtranslator.out <filename>\n";
    getchar();
    return 0;
}      

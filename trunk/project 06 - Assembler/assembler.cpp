#include<fstream>
#include<iostream>
#include<string>
#include<cctype>
#define ADDRESS_MAX 16384
using namespace std;

enum cmdType {A_COMMAND, C_COMMAND, L_COMMAND};

class SymbolTable; 

class Parser{
      fstream Hack, Asm;
      int lineNumber;
      string currentCommand;
      public:
             Parser(char* filename){
                          string file=filename;
                          Asm.open(filename,ios::in);
                          if(Asm.fail()){ 
                          cout << filename << " cannot be found\nPlease check whether you have entered the correct pathname\n";
                          getchar(); exit(0);
                          }
                          Asm.unsetf(ios::skipws);
                          file.erase(file.size()-3,3);
                          file+="hack"; 
                          Hack.open(file.c_str(),ios::out);
                          lineNumber=0;
             }
             ~Parser(){Asm.close(); Hack.close(); }
             bool hasMoreCommands(){
                  if(Asm.fail()) return false;
                  return true;
             }
             int advance();
             cmdType commandType();
             string symbol();
             string dest();
             string comp();
             string jump();
             friend SymbolTable pass1(char*);
             friend void pass2(SymbolTable,char*);
};

class Code{
      public:
             string dest(string);
             string comp(string);
             string jump(string);
};

class SymbolTable{
      struct SymTab{
          string label;
          int address;
      }st[ADDRESS_MAX+7];
      int noOfEntries;
      public:
             SymbolTable(){
                 st[0].label="SP"; st[0].address=0;
                 st[1].label="LCL"; st[1].address=1;
                 st[2].label="ARG"; st[2].address=2;
                 st[3].label="THIS"; st[3].address=3;
                 st[4].label="THAT"; st[4].address=4;
                 for(int i=0;i<16;i++){
                         char ch;
                         if(i>=10){
                             st[5+i].label="R1";
                             ch=(i%10)+'0';
                         }
                         else{
                             st[5+i].label="R";
                             ch=i+'0';
                         }
                         st[5+i].label+=ch;
                         st[5+i].address=i;
                 }
                 st[21].label="SCREEN"; st[21].address=16384;
                 st[22].label="KBD"; st[22].address=24576;
                 noOfEntries=23;
                 lastRamAddressUsed=15;
             }
             void addEntry(string,int);
             bool contains(string);
             int GetAddress(string);
             int lastRamAddressUsed;
};               
                    
int  Parser::advance(){
     char ch;
     currentCommand.erase(0,currentCommand.size());
     while(hasMoreCommands()){
                 Asm >> ch;
                 if(ch=='\n'){
                              if(commandType()==L_COMMAND) break;
                              if(currentCommand.size()!=0){lineNumber++; break;}
                              else continue;
                 }
                 if(ch!=' '&&ch!='\t'&&ch!='\n'&&ch!='\r') currentCommand+=ch;
                 if(currentCommand[currentCommand.size()-1]=='/'&&currentCommand[currentCommand.size()-2]=='/'){
                           currentCommand.erase(currentCommand.size()-2, 2);
                           Asm >> ch;
                           while(ch!='\n') Asm >> ch; 
                           if(currentCommand.size()!=0){
                           lineNumber++; break;}                                     
                 }
     }
     if(!hasMoreCommands()) return -1;
     return 0;
}

cmdType Parser::commandType(){
        if(currentCommand[0]=='@') return A_COMMAND;
        if(currentCommand[0]=='('&&currentCommand[currentCommand.size()-1]==')')
        return L_COMMAND;
        return C_COMMAND;
}

string Parser::symbol(){
       string sym;
       sym=currentCommand;
       sym.erase(0,1);
       if(commandType()==L_COMMAND)
       sym.erase(sym.size()-1,1);
       return sym;
}

string Parser::dest(){
       string destination;
       if(currentCommand[1]=='='){
           if(currentCommand[0]=='D'||currentCommand[0]=='M'||currentCommand[0]=='A'){
               destination+=currentCommand[0];
               return destination;
           }
           else{
               cout << lineNumber << ": Destination expected\n"; 
               getchar(); 
               exit(0);
           }
       }
       else if(currentCommand[2]=='='){
           destination=currentCommand;
           destination.erase(2,destination.size()-2);
           if(destination=="AD"||destination=="AM"||destination=="MD")
               return destination;
           cout << lineNumber << ": Destination expected\n"; 
           getchar();
           exit(0);
       }
       destination=currentCommand;
       destination.erase(4,destination.size()-4);
       if(destination=="AMD=")
       return "AMD"; 
       return "";
}

string Parser::comp(){
       string compute;
       compute=currentCommand;
       if(currentCommand[1]=='=')
       compute.erase(0,2);
       else if(currentCommand[2]=='=')
       compute.erase(0,3);
       else if(currentCommand[3]=='=')
       compute.erase(0,4);
       int pos=0;
       while(true){
                   if(compute[pos]==';'||pos==compute.size())
                   break;
                   pos++;
       }
       if(pos<compute.size())
       compute.erase(pos,compute.size()-pos);
       if(compute=="0"||compute=="1"||compute=="-1"||compute=="D"||compute=="A"||
          compute=="M"||compute=="!D"||compute=="!A"||compute=="!M"||compute=="-A"||
          compute=="-M"||compute=="-D")
       return compute;
       if((compute[0]=='D'||compute[0]=='A'||compute[0]=='M')&&
          (compute[1]=='+'||compute[1]=='-')&&
          (compute[2]=='D'||compute[2]=='A'||compute[2]=='M'||compute[2]=='1'))
       return compute;
       if((compute[0]=='D'||compute[0]=='A'||compute[0]=='M')&&
          (compute[1]=='&'||compute[1]=='|')&&
          (compute[2]=='D'||compute[2]=='A'||compute[2]=='M'))
       return compute;
       cout << lineNumber << currentCommand << ": Expression expected\n";
       getchar();
       exit(0);
}

string Parser::jump(){
       string jmp;
       jmp=currentCommand;
       int pos=0;
       while(true){
                   if(jmp[pos]==';'||pos==jmp.size())
                   break;
                   pos++;
       }
       if(pos<jmp.size())
       jmp.erase(0,pos+1);
       else
       jmp.erase(0,pos);
       return jmp;
}

string Code::dest(string mnemonic){
       if(mnemonic=="") return "000";
       if(mnemonic=="M") return "001";
       if(mnemonic=="D") return "010";
       if(mnemonic=="MD") return "011";
       if(mnemonic=="A") return "100";
       if(mnemonic=="AM") return "101";
       if(mnemonic=="AD") return "110";
       if(mnemonic=="AMD") return "111";
}

string Code::comp(string mnemonic){
       string bitVector="0000000";
       if(mnemonic.find("M")!=-1)
       bitVector[0]='1';//a bit = 1 for M
       if(mnemonic.find("D")==-1)
       bitVector[1]='1';//zx=1 if D is not an operand
       if(mnemonic.find("A")==-1&&mnemonic.find("M")==-1)
       bitVector[3]='1';//zy=1 if A/M is not an operand
       //2's complement subtraction a-b can be implemented as !(!a+b) where + indicates addition.
       //-1 can be written as !0.
       //These deductions are used in the following few instructions.
       //for eg: D+1 = D-(-1) = D-(!0) can be interpreted as !(!D+!0)
       //        which implies that nx=1, ny=1 and no=1
       if(mnemonic.find("-")!=-1){
           if(mnemonic[0]=='D'&&mnemonic[2]!='1') bitVector[2]='1';
           if(mnemonic[2]=='D'&&mnemonic[0]!='1') bitVector[4]='1';
           if(mnemonic[0]=='D'&&mnemonic[2]=='1') bitVector[4]='1';
           if(mnemonic[0]!='D'&&mnemonic[2]=='1') bitVector[2]='1';
           if(mnemonic[1]=='D'&&mnemonic.size()==2) bitVector[4]='1';
           if(mnemonic[1]!='D'&&mnemonic.size()==2) bitVector[2]='1';
       }
       if(mnemonic.find("!")!=-1||mnemonic.size()==1){
           if(mnemonic.find("D")!=-1) bitVector[4]='1';
           if(mnemonic.find("A")!=-1||mnemonic.find("M")!=-1) bitVector[2]='1';
       }
       if(mnemonic.find("|")!=-1||mnemonic.find("+1")!=-1||mnemonic=="1")
       bitVector[2]=bitVector[4]='1'; 
       if((mnemonic.find("+")!=-1)||(mnemonic.find("-")!=-1)||(mnemonic=="0")||(mnemonic=="1"))
       bitVector[5]='1';//any instruction that performs addition or gets reduced to an 
                        //addition operation would imply that bit f=1
       if(((mnemonic.find("-")!=-1)||(mnemonic.find("1")!=-1)||(mnemonic.find("!")!=-1)||
           (mnemonic.find("|")!=-1))&&(mnemonic.find("-1")==-1))
       bitVector[6]='1'; 
       return bitVector;
}

string Code::jump(string mnemonic){
       if(mnemonic=="") return "000";
       if(mnemonic=="JGT") return "001";
       if(mnemonic=="JEQ") return "010";
       if(mnemonic=="JGE") return "011";
       if(mnemonic=="JLT") return "100";
       if(mnemonic=="JNE") return "101";
       if(mnemonic=="JLE") return "110";
       if(mnemonic=="JMP") return "111";
}

void SymbolTable::addEntry(string symbol, int address){
     for(int i=0;i<symbol.size();i++){
         if(!isdigit(symbol[i])) break;
         if(i==symbol.size()-1) return;
     }//if the symbol is a number, then don't add it to the symbol table
     if(contains(symbol)){
        for(int i=0;i<noOfEntries;i++)
             if(st[i].label==symbol){ st[i].address=address; break; }
     }//if the symbol is already there in the table, update the address if necessary
     else{             
         st[noOfEntries].label=symbol;
         st[noOfEntries].address=address;
         noOfEntries++;
     }//if the symbol is not present in the table, add it to the table.
}

bool SymbolTable::contains(string symbol){
     for(int i=0;i<noOfEntries;i++)
             if(st[i].label==symbol) return true;
     return false;
}

int SymbolTable::GetAddress(string symbol){
    for(int i=0;i<noOfEntries;i++)
             if(st[i].label==symbol) return st[i].address;
}      

SymbolTable pass1(char* filename){
            Parser p(filename);
            SymbolTable s;
            while(p.hasMoreCommands()){
                if(p.advance()==-1) return s;
                if(p.commandType()==A_COMMAND&&!s.contains(p.symbol()))
                     s.addEntry(p.symbol(),-1);
                else if(p.commandType()==L_COMMAND) 
                     s.addEntry(p.symbol(),p.lineNumber);//lineNumber indicates ROM address
            }
            return s;
}                               

void pass2(SymbolTable s,char* filename){
     Parser p(filename);
     Code c;
     int a=0;
     while(p.hasMoreCommands()){
             if(p.advance()==-1) return;
             if(p.commandType()==A_COMMAND){
                 int num=0;
                 string sym=p.symbol();
                 if(s.contains(sym)){                                           
                     num=s.GetAddress(sym);
                     if(num==-1){
                         num=++s.lastRamAddressUsed;
                         s.addEntry(sym,num);
                     }
                 }
                 else
                     for(int i=0;i<sym.size();i++)
                     num=(num*10+sym[i]-'0');
                 string bin="";
                 for(int i=num;i/2>0;i/=2){
                    string str="";
                    str+=('0'+i%2);
                    bin=str+bin;
                 }
                 if(num!=0)
                 bin="1"+bin;
                 for(int i=bin.size();i<16;i++)
                 bin="0"+bin;
                 p.Hack << bin << endl;
             }
             else if(p.commandType()==C_COMMAND){
                  p.Hack << "111" << c.comp(p.comp())
                         << c.dest(p.dest())
                         << c.jump(p.jump()) << endl;
             }
     }
}

int main(int argc, char** argv){
    if(argc==2){
    cout << "Please wait...\n\n";
    pass2(pass1(argv[1]),argv[1]);
    cout << argv[1] << " successfully assembled\n";}
    else
    cout << "Correct Syntax is: ./assembler.out <filename>\n";
    getchar();
    return 0;
}      

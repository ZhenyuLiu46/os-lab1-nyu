//  main.cpp
//  OS-Lab1
//
//  Created by Zhenyu Liu on 2/7/19.
//  Copyright © 2019 Zhenyu Liu. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <unordered_map>
#include <map>
#include <iomanip>
using namespace std;


void printFormat(int Num, int address) {
    cout << setw(3) << setfill('0') << Num << ": " << setw(4) << address;
    cout << setfill(' ');
}

class valuetype {
public:
    int val;
    int moduleidx;
    bool used;
    bool multiple;
    bool uselistused;

    valuetype( int value, int midx, bool u, bool m, bool uu )
    {
        val = value;
        moduleidx = midx;
        // for defined but never used;
        used = u;
        multiple = m;
        //for uselistused but external address did not refer.
        uselistused = uu;
    }
};


class tokenize {
    char * pch;
    vector<string> symbol;
    vector<int> val;
    ifstream& inputFile;
    string str;
    int linenum = 0;
    int lineoffset = 0;
    char* linestart;
    int currlinelen = 0;
    
public:
    
    void parseerror(int errcode) {
        const char* errstr[] = {
            "NUM_EXPECTED",             // Number expect
            "SYM_EXPECTED",             // Symbol Expected
            "ADDR_EXPECTED",            // Addressing Expected which is A/E/I/R
            "SYM_TOO_LONG",             // Symbol Name is too long
            "TOO_MANY_DEF_IN_MODULE",      //>16
            "TOO_MANY_USE_IN_MODULE",      // > 16
            "TOO_MANY_INSTR"             // total num_instr exceeds memory size (512)
        };
        
        printf("Parse Error line %d offset %d: %s\n", linenum, lineoffset, errstr[errcode]);
        
        exit(-1); // stop and exit
    }
    
    tokenize(ifstream &in) : inputFile(in)
    {
        getline(inputFile, str);
        linenum++;
        linestart = strdup(str.c_str());
        currlinelen = strlen(str.c_str());
        pch = strtok(linestart," \t");
    }
    char* getToken(){
        int loop = 0;
        while( pch == NULL && !inputFile.eof()){
            getline(inputFile, str);
            int temp = strlen(str.c_str());
            if ( !inputFile.eof()){
                currlinelen = temp;
            }
            
            lineoffset = 0;
            linenum++;
            loop++;
            linestart = strdup(str.c_str());
            pch = strtok(linestart," \t");
        }
        
        
        if( inputFile.eof()){
            
            linenum = linenum - 1;
            lineoffset = currlinelen + 1;
            
            
            char* temp = pch;
            return temp;
        } else{
            char* temp = pch;
            lineoffset = pch - linestart + 1;
            pch = strtok (NULL, " \t");
            return temp;
        }
    }
    
    int readFirstInt ()
    {
        char* temp = getToken();
        if (temp != NULL) {
            for(int i = 0; i < strlen(temp); i++){
                if(!isdigit(temp[i])){
                    parseerror(0);
                }
            }
            int count = atoi(temp);
            return count;
        }
        return -1;
    }
   
    int readInt ()
    {
        char* temp = getToken();
        if (temp != NULL) {
            for(int i = 0; i < strlen(temp); i++){
                if(!isdigit(temp[i])){
                    parseerror(0);
                }
            }
            int count = atoi(temp);
            return count;
        }
        parseerror(0);
        return -1;
    }
    
    char* readSymbol ()
    {
        char* temp = getToken();
        
        if (temp != NULL) {
            if(('a' <= temp[0] && temp[0] <= 'z') || ('A' <= temp[0] && temp[0] <= 'Z')){
                for(int i = 1; i < strlen(temp); i++){
                    if(i == 16){
                        parseerror(3);
                    }
                    if(('a'<=temp[i] && temp[i]<='z') || ('A' <= temp[i] && temp[i] <= 'Z') || ('0' <= temp[i] && temp[i] <= '9')) continue;
                    else
                        parseerror(1);
                }
                return temp;
            } else{
                parseerror(1);
            }
        }
        parseerror(1);
        return temp;
    }
    
    char* readIAER ()
    {
        char* temp = getToken();
        if (temp != NULL) {
            if(temp[0] == 'A' || temp[0] == 'E' || temp[0] == 'I' || temp[0] == 'R'){
                return temp;
            } else{
                cout<< temp;
                parseerror(2);
                return temp;
            }
        }
        parseerror(2);
        return temp;
    }
    
    int readAddrInt ()
    {
        char* temp = getToken();
        if (temp != NULL) {
            for(int i = 0; i < strlen(temp); i++){
                if(!isdigit(temp[i])){
                    parseerror(0);
                }
            }
            int count = atoi(temp);
            return count;
        }
        parseerror(0);
        return -1;
    }
};


int main(int argc, const char * argv[]) {
    string fileName = argv[1];
    vector<string> symbol;
    vector<int> val;
    int codecount = 0;
    int accumCodecount = 0;
    int baseAddr = 0;
    int moduleNum = 0;
    vector<int> baseAddrVect;
    unordered_map<string, int> umap;
    unordered_map<string, valuetype> map;
    
    //pass one=====================================================================
    ifstream inputFile;
    //inputFile.open("/Users/zhenyuliu/Desktop/OS/lab1samples/input-20");
    inputFile.open(fileName);
    tokenize token = tokenize(inputFile);
    if(inputFile.is_open()){
        
        while (!inputFile.eof() )
        {
            //process defnition list
            int defcount = token.readFirstInt();
            if(defcount == -1) break;
            moduleNum++;
            if(defcount > 16){ // TOO_MANY_DEF_IN_MODULE
                token.parseerror(4);
            }
            accumCodecount += codecount;
            for(int i = 0; i < defcount; i++){
                string tempString = string(token.readSymbol());
                int tempVal = token.readInt() + accumCodecount;
                
                unordered_map<string,int>::const_iterator it = umap.find (tempString);

                if ( it == umap.end() ) {
                    umap[tempString] = tempVal;
                    valuetype v = valuetype(tempVal, moduleNum, false, false, false);
                    map.insert({tempString, v});
                    symbol.push_back(tempString);
                    val.push_back(tempVal);
                } else {
                    map.at(tempString).multiple = true;
                }
            }
            
            //process use list
            int usecount = token.readInt();
            if(usecount > 16){ // TOO_MANY_USE_IN_MODULE
                token.parseerror(5);
            }
            for(int i = 0; i < usecount; i++){
                token.readSymbol();
            }
            
            //process program text
            codecount = token.readInt();
            if(accumCodecount + codecount > 512){ // "TOO_MANY_INSTR"
                token.parseerror(6);
            }
            for(int i = 0; i < codecount; i++){
                token.readIAER();
                token.readAddrInt();
            }
            
            for( int i = 0; i < symbol.size();i ++){
                if(val[i] - accumCodecount >= codecount){
                    printf("Warning: Module %d: %s too big %d (max=%d) assume zero relative\n", moduleNum,symbol[i].c_str(),val[i],codecount -1);
                    val[i] = accumCodecount;
                    umap[symbol[i]] = accumCodecount;
                    map.at(symbol[i]).val = accumCodecount;
                }
            }
            baseAddrVect.push_back(baseAddr);
            baseAddr += codecount;
        }
        
        // print
        cout << "Symbol Table" << endl;
        
        for(int i = 0; i < symbol.size();i ++){
            cout << symbol[i] << "=" << val[i] << " ";
            if(map.at(symbol[i]).multiple == true){
                printf("Error: This variable is multiple times defined; first value used\n");
            } else{
                cout<<endl;
            }
        }
    }
  
    inputFile.close();
    cout << endl;
    
    //pass two=====================================================================
    //inputFile.open("/Users/zhenyuliu/Desktop/OS/lab1samples/input-20");
    inputFile.open(fileName);
    int id = 0;
    tokenize token2 = tokenize(inputFile);
    if(inputFile.is_open()){
        cout << "Memory Map" << endl;
        for(int x = 0; x < moduleNum; x++){
            int base = baseAddrVect[x];
            
            //process defnition list************************
            int defcount = token2.readFirstInt();
            
            if(defcount == -1) break;
            if(defcount > 16){ // TOO_MANY_DEF_IN_MODULE
                token2.parseerror(4);
            }
            
            for(int i = 0; i < defcount; i++){
                token2.readSymbol();
                token2.readInt();
            }
            
            //process use list************************
            int usecount = token2.readInt();
            if(usecount > 16){ // TOO_MANY_USE_IN_MODULE
                token2.parseerror(5);
            }
            vector<string> usedsymbol;
            unordered_map<string, valuetype> usedmap;
            for(int i = 0; i < usecount; i++){
                string temp = string(token2.readSymbol());
                usedsymbol.push_back(temp);
                
                valuetype vx = valuetype(0, x+1, false, false, false);
                usedmap.insert({temp, vx});
                
                unordered_map<string,int>::const_iterator it = umap.find (temp);
                
                if ( it == umap.end() ){
                    continue;
                }
                
                map.at(temp).used = true;
               
            }
            
            //process program text************************
            codecount = token2.readInt();
            if(codecount > 512){ // "TOO_MANY_INSTR"
                token2.parseerror(6);
            }
            for(int i = 0; i < codecount; i++){
                string tempIAER = token2.readIAER();
                if (tempIAER == "R"){
                    int temp = token2.readAddrInt();
                    if(temp >= 10000){
                        temp = 9999;
                        printFormat(id, temp);
                        cout << " ";
                        printf("Error: Illegal opcode; treated as 9999\n");
                    }
                    
                    else if(temp % 1000 >= codecount){
                        printFormat(id, temp / 1000 * 1000 + base);
                        cout << " ";
                        printf("Error: Relative address exceeds module size; zero used\n");
                    } else{
                        printFormat(id, temp + base);
                        cout << endl;
                    }
                    id++;
                } else if ( tempIAER == "I" ){
                    int temp = token2.readAddrInt();
                    if(temp >= 10000){
                        temp = 9999;
                        printFormat(id, temp);
                        cout << " ";
                        printf("Error: Illegal immediate value; treated as 9999\n");
                    } else {
                        printFormat(id, temp);
                        cout << endl;
                    }
                    
                    id++;
                } else if ( tempIAER == "A" ){
                    int temp = token2.readAddrInt();
                    if ( temp % 1000 > 512){
                        printFormat(id, temp / 1000 * 1000);
                        cout <<" ";
                        printf("Error: Absolute address exceeds machine size; zero used\n");
                    } else {
                        printFormat(id, temp);
                        cout << endl;
                    }
                    id++;
                }
                if ( tempIAER == "E" ) {
                    int temp = token2.readAddrInt();
                    int idx = temp % 1000;
                    int real = temp - idx;
                    
                    if(idx >= usedsymbol.size()){
                        printFormat(id, temp );
                        cout << " ";
                        printf("Error: External address exceeds length of uselist; treated as immediate\n");
                    } else {
                        unordered_map<string,int>::const_iterator it = umap.find (usedsymbol[idx]);
                        usedmap.at(usedsymbol[idx]).uselistused = true;
                        if ( it == umap.end() ){
                            printFormat(id, temp / 1000 * 1000);
                            cout << " ";
                            printf("Error: %s is not defined; zero used\n", usedsymbol[idx].c_str());
                        } else {
                            
                            int addon = umap[usedsymbol[idx]];
                            printFormat(id, real + addon);
                            cout << endl;
                        }
                    }
                    id++;
                }
            }
            
            for(int i = 0; i < usedsymbol.size(); i++){
                if( usedmap.at(usedsymbol[i]).uselistused == false){
                    printf("Warning: Module %d: %s appeared in the uselist but was not actually used\n", usedmap.at(usedsymbol[i]).moduleidx, usedsymbol[i].c_str());
                }
            }
        }
        cout << endl;
        
        for(int i = 0; i < symbol.size(); i++){
            unordered_map<string,int>::const_iterator it = umap.find (symbol[i]);
            
            if ( it == umap.end() ){
                continue;
            } else if(map.at(symbol[i]).used == false){
                printf("Warning: Module %d: %s was defined but never used\n", map.at(symbol[i]).moduleidx, symbol[i].c_str());
            }
        }
    }
    inputFile.close();
    
    return 0;
}


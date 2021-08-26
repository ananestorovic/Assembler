//
// Created by ss on 8/20/21.
//

#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#include "exceptions.h"
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <list>

using namespace std;

class Assembler {

private:
    Assembler() = default;

    struct symbolTableEntry {
        string symbolName;
        int section;
        int value;
        bool isGlobal;
        static int generateId;
        int size;
        int id;

        symbolTableEntry(string symbolName = "", int section = -2, int value = -2, bool isGlobal = false,
                         int size = 0) {
            this->symbolName = symbolName;
            this->section = section;
            this->value = value;
            this->isGlobal = isGlobal;
            this->id = generateId++;
            this->size = size;

        }
    };


    struct relocationTableEntry {
        int offset;
        string typeOfRelocation;
        int value;
        string section;

        relocationTableEntry(int offset = -1, string typeOfRelocation = "", int value = -1, string section="") {

            this->offset = offset;
            this->typeOfRelocation = typeOfRelocation;
            this->value = value;
            this->section=section;
        }
    };

    static unique_ptr<Assembler> instance;
    string inputFileName;
    string outputFileName = "out.o";
    static int locationCounter;
    static string currentSectionName;
    static bool isGlobalFirst;
    static bool isEnd;
    static bool first;
    vector<char> bytes;

    map<string, symbolTableEntry> symbolTable;
    map<string, list<relocationTableEntry>> relocationTable;
    map<string, vector<char> > codeBySection;


public:


    explicit Assembler(string inputFileName);

    void printSymbolTable();

    static Assembler &getInstance();

    //pomocne funkcije za direktive
    bool isDirective(string line);

    bool isInstruction(string line);

    bool checkIfLabel(string line);

    bool checkIfLabelIsOk(string labelName);

    bool processAfterLabel(string line);

    bool checkIfSection(string line);

    bool checkIfEqu(string line);

    bool checkIfWord(string line);

    bool checkIfSkip(string line);

    bool checkIfGlobal(string line);

    bool checkIfExtern(string line);

    bool checkIfEnd(string line);

    int literalToDecimal(string literal);

    //pomocne funkcije za instrukcije
    bool checkIfNoOperand(string line);

    bool checkIfOneOpReg(string line);

    bool checkIfJump(string line); //ako nije neki od ovih dole greska

    bool checkIfJumpAbsolute(string line);

    bool checkIfJumpPcRel(string line);

    bool checkIfJumpRegDir(string line);

    bool checkIfJumpRegInd(string line);

    bool checkIfJumpRegIndDis(string line);

    bool checkIfJumpMemDir(string line);

    bool checkIfLdStr(string line);

    bool checkIfLdStrAbsolute(string line);

    bool checkIfLdStrPcRel(string line);

    bool checkIfLdStrRegDir(string line);

    bool checkIfLdStrRegInd(string line);

    bool checkIfLdStrRegIndDis(string line);

    bool checkIfLdStrMemDir(string line);

    bool checkIfInstrWithTwoReg(string line);


    //prvi prolaz za direktive
    void processInputFile(const string &inputFileName);

    void processInputFile();

    void processLabel(string labelName);

    void processSectionFirstPass(string line);

    void processEqu(string name, int value);

    void processWordFirstPass(string line);

    void processSkipFirstPass(string line);

    void processExtern(string line);


    //drugi prolaz za direktive
    void processInputSecondPass();

    void processSectionSecondPass(string line);

    void processWordSecondPass(string line);

    void processSkipSecondPass(string line);

    void processGlobalSecondPass(string line);

    //drugi prolaz za instrukcije
    void processIfNoOperand(string line);

    void processIfOneOpReg(string line);

    void processJumpAbsolute(string line);

    void processJumpPcRel(string basicString);

    void processJumpRegDir(string basicString);

    void processJumpRegInd(string basicString);

    void processJumpRegIndDis(string basicString);

    void processJumpMemDir(string basicString);

    void processLdStrAbsolute(string basicString);

    void processLdStrPcRel(string basicString);

    void processLdStrRegDir(string basicString);

    void processLdStrRegInd(string basicString);

    void processLdStrRegIndDis(string basicString);

    void processLdStrMemDir(string basicString);

    void processInstrWithTwoREg(string basicString);

    string toBinary(int n);

    void processWordHelper(int value);

    void printRelocationTable();

    void printCodeBySection();
};


#endif //ASSEMBLER_ASSEMBLER_H

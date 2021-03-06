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
    Assembler() {
        isLabelWithoutCode = false;
    };

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

        relocationTableEntry(int offset = 0, string typeOfRelocation = "", int value = -1, string section = "") {

            this->offset = offset;
            this->typeOfRelocation = typeOfRelocation;
            this->value = value;
            this->section = section;
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
    bool isLabelWithoutCode;

    map<string, symbolTableEntry> symbolTable;
    map<string, list<relocationTableEntry>> relocationTable;
    map<string, vector<char> > codeBySection;
    map<string, int> instrDescription = {
            {"halt", 0x00},
            {"iret", 0x20},
            {"ret",  0x40},
            {"int",  0x10},
            {"call", 0x30},
            {"jmp",  0x50},
            {"jeq",  0x51},
            {"jne",  0x52},
            {"jgt",  0x53},
            {"ldr",  0xA0},
            {"shr",  0x91},
            {"str",  0xB0},
            {"push", 0xB0},
            {"pop",  0xA0},
            {"xchg", 0x60},
            {"add",  0x70},
            {"sub",  0x71},
            {"mul",  0x72},
            {"div",  0x73},
            {"cmp",  0x74},
            {"not",  0x80},
            {"and",  0x81},
            {"or",   0x82},
            {"xor",  0x83},
            {"test", 0x84},
            {"shl",  0x90}

    };


public:


    explicit Assembler(string inputFileName);


    void processInputFile(const string &inputFileName);

    void processInputFile();


    static Assembler &getInstance();


    void printSymbolTable();

    void printRelocationTable();

    void printCodeBySection();



    string removeSpaces(string &line);

    int literalToDecimal(string literal);

    bool isComment(string line);


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


    bool checkIfNoOperand(string line);

    bool checkIfOneOpReg(string line);

    bool checkIfJump(string line);

    bool checkIfJumpAbsolute(string instruction, string operand);

    bool checkIfJumpPcRel(string instruction, string operand);

    bool checkIfJumpRegDir(string instruction, string operand);

    bool checkIfJumpRegInd(string instruction, string operand);

    bool checkIfJumpRegIndDis(string instruction, string operand);

    bool checkIfJumpMemDir(string instruction, string operand);

    bool checkIfLdStr(string line);

    bool checkIfLdStrAbsolute(string instruction, int reg, string operand);

    bool checkIfLdStrPcRel(string instruction, int reg, string operand);

    bool checkIfLdStrRegDir(string instruction, int reg, string operand);

    bool checkIfLdStrRegInd(string instruction, int reg, string operand);

    bool checkIfLdStrRegIndDis(string instruction, int reg, string operand);

    bool checkIfLdStrMemDir(string instruction, int reg, string operand);

    bool checkIfInstrWithTwoReg(string line);


    void processLabel(string labelName);

    void processSectionFirstPass(string line);

    void processEqu(string name, int value);

    void processWordFirstPass(string line);

    void processSkipFirstPass(string line);

    void processExtern(string line);


    void processSectionSecondPass(string line);

    void processWordSecondPass(string line);

    void processSkipSecondPass(string line);

    void processGlobalSecondPass(string line);



    void instr2Bytes(string instrDescr, int regsDescr);

    void instr3Bytes(string instrDescr, int regsDescr, int addrMode);

    void instr5Bytes(string instrDescr, int regsDescr, int addrMode, int value);

    void processWordHelper(int value);

    int processAbsoluteAddressingSymbol(string line);

    int processPcRelativeAddressingSymbol(string line);



    void processIfNoOperand(string line);

    void processIfOneOpReg(string line, int reg);

    void processJumpAbsolute(string instruction, string operand);

    void processJumpPcRel(string instruction, string operand);

    void processJumpRegDir(string instruction, int reg);

    void processJumpRegInd(string instruction, int reg);

    void processJumpRegIndDis(string instruction, int reg, string dis);

    void processJumpMemDir(string instruction, string operand);

    void processLdStrAbsolute(string instruction, int reg, string operand);

    void processLdStrPcRel(string instruction, int reg, string operand);

    void processLdStrRegDir(string instruction, int reg, string operand);

    void processLdStrRegInd(string instruction, int reg, string operand);

    void processLdStrRegIndDis(string instruction, int reg, string operand);

    void processLdStrMemDir(string instruction, int reg, string operand);

    void processInstrWithTwoReg(string instruction, int reg);


};


#endif //ASSEMBLER_ASSEMBLER_H

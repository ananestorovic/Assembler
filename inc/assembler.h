//
// Created by ss on 8/20/21.
//

#ifndef ASSEMBLER_ASSEMBLER_H
#define ASSEMBLER_ASSEMBLER_H

#include "exceptions.h"
#include <string>
#include <memory>
#include <map>
using namespace std;

class Assembler {

private:
    Assembler() = default;

    struct symbolTableEntry {
        string symbolName;
        int section;
        int value;
        bool isGlobal;
        bool isDefined;
        static int id;
        int size;

        symbolTableEntry(string symbolName = "", int section = -2, int value = -2, bool isGlobal = false,
                         bool isDefined = false, int size = 0) {
            this->symbolName = symbolName;
            this->section = section;
            this->value = value;
            this->isGlobal = isGlobal;
            this->isDefined = isDefined;
            this->id = id++;
            this->size = size;

        }
    };

    struct relocationTable { // NE ZNAM STA OVDE TREBA
        int offset;
        string typeOfRelocation;
        //bool is_data; // sta znaci da li je instrukcija ili podatak
        //string section;
        //int addend;
    };
    static unique_ptr<Assembler> instance;
    string inputFileName;
    string outputFileName;
    static int locationCounter;
    static string currentSectionName;
    static bool isGlobalFirst;
    static bool isEnd;
    static bool first;

    map<string, symbolTableEntry> symbolTable; //pretraga po nazivu simbola


public:


    explicit Assembler(string inputFileName);

    //pomocne funkcije
    bool isDirective(string line);

    bool checkIfLabel(string line);

    bool checkIfLabelIsOk(string labelName);

    void checkWhatIsAfterLabel(string line);

    bool checkIfSection(string line);

    bool checkIfEqu(string line);

    bool checkIfWord(string line);

    bool checkIfSkip(string line);

    bool checkIfGlobal(string line);

    bool checkIfExtern(string line);

    bool checkIfEnd(string line);

    int literalToDecimal(string literal);


    //prvi prolaz
    void processInputFileFirstPass(const string &inputFIleName);

    void processLabel(string labelName);

    void processSectionFirstPass(string line);

    void processEqu(string name, int value);

    void processWordFirstPass(string line);

    void processSkipFirstPass(string line);

    void processExtern(string line);

    void processInstructionsFirstPass(string line);


    //drugi prolaz
    void processInputSecondPass();

    void processSectionSecondPass(string line);

    void processWordSecondPass(string line);

    void processSkipSecondPass(string line);

    void processGlobalSecondPass(string line);


    static Assembler &getInstance();
};


#endif //ASSEMBLER_ASSEMBLER_H

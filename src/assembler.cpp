//
// Created by ss on 8/20/21.
//

#include "assembler.h"
#include "regexes.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

//ocisti spejsove!!

int Assembler::symbolTableEntry::generateId = 1;
string Assembler::currentSectionName = "";
bool Assembler::isGlobalFirst = true;
bool Assembler::isEnd = false;
bool Assembler::first = true;
int Assembler::locationCounter = 0;
unique_ptr<Assembler> Assembler::instance = std::unique_ptr<Assembler>(new Assembler());


void Assembler::processInputFile(const string &inputFileName) {
    try {
        this->inputFileName = inputFileName;
        first = true;
        processInputFile();
        printSymbolTable();
        first = false;
        //processInputFile();
    } catch (const exception &error) {
        cout << error.what();
    }
}

void Assembler::processInputFile() {
    ifstream file(this->inputFileName);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            if (isEnd == true) break;
            if (isDirective(line)) continue;
            if (isInstruction(line)) continue;

            printf("%s\n", line.c_str());
            throw NothingError();
        }
        isEnd = false;
        currentSectionName = "";
        locationCounter = 0;
        //msm da je ovo za global bilo dovoljno proveravati pri prvom prolazu
        file.close();
    } else throw FileError();

}

bool Assembler::isDirective(string line) {
    if (checkIfSection(line) || checkIfEnd(line) || checkIfEqu(line) || checkIfExtern(line)
        || checkIfGlobal(line) || checkIfSkip(line) || checkIfWord(line)
        || checkIfLabel(line))
        return true; // proveri posle da li ovde mogu sve funkcije
    else return false;
}

bool Assembler::checkIfSection(string line) {
    smatch sectionName;
    if (regex_match(line, sectionName, rx_section_directive)) {
        if (first) {
            processSectionFirstPass(sectionName.str(1));
            isGlobalFirst = false;
        } else processSectionSecondPass(sectionName.str(1));
        return true;
    }
    return false;
}

void Assembler::processSectionFirstPass(string line) {
    if (symbolTable.find(line) != symbolTable.end()) throw SectionError();
    if (currentSectionName != "") {
        Assembler::symbolTableEntry &previousSymbol = symbolTable.find(currentSectionName)->second;
        previousSymbol.size = locationCounter;
    }
    locationCounter = 0;
    Assembler::symbolTableEntry newSymbol = symbolTableEntry();
    newSymbol.symbolName = line;
    newSymbol.section = newSymbol.id;
    newSymbol.value = 0;
    currentSectionName = newSymbol.symbolName;
    symbolTable.insert(make_pair(line, newSymbol));

}

bool Assembler::checkIfEqu(string line) {
    smatch equParts;
    string name;
    string valueString;
    int value;
    if (regex_match(line, equParts, rx_equ_directive)) {
        if (first) {
            name = equParts.str(1);
            valueString = equParts.str(2);
            if (symbolTable.find(name) != symbolTable.end()) throw EquDefError();
            isGlobalFirst = false;
            value = literalToDecimal(valueString);
            processEqu(name, value);
        }
        return true;
    }
    return false;
}

void Assembler::processEqu(string name, int value) {
    Assembler::symbolTableEntry newSymbol = symbolTableEntry();
    newSymbol.value = value;
    newSymbol.section = -1;
    newSymbol.symbolName = name;
    symbolTable.insert(make_pair(name, newSymbol));
}


int Assembler::literalToDecimal(string literal) {
    smatch helper;
    if (regex_match(literal, helper, rxIsLiteral)) {
        return std::stoi(literal, nullptr, 0);
    }
    throw LiteralError();
}

bool Assembler::checkIfLabel(string line) {
    smatch label;
    string labelName;
    string command;

    if (regex_match(line, label, rx_label_with_command)) {
        labelName = label.str(1);
        command = label.str(2);
        if (checkIfLabelIsOk(labelName)) {
            processLabel(labelName);
            return processAfterLabel(command);

        }
    }
    return false;
}

bool Assembler::checkIfLabelIsOk(string labelName) {
    if (symbolTable.find(labelName) != symbolTable.end()) throw LabelDefError();
    if (currentSectionName == "") throw LabelSectionError();
    return true;
}

void Assembler::processLabel(string labelName) {
    if (first) {
        isGlobalFirst = false;
        symbolTableEntry newSymbol = symbolTableEntry();
        newSymbol.symbolName = labelName;
        newSymbol.value = locationCounter;
        newSymbol.section = symbolTable.find(currentSectionName)->second.id;
        symbolTable.insert(make_pair(labelName, newSymbol));
    }
}

bool Assembler::checkIfWord(string line) {
    smatch word;
    string data;
    if (regex_match(line, word, rx_word_directive)) {
        if (currentSectionName == "") throw WordSectionError();
        data = word.str(1);
        stringstream ss(data);
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            if (first) processWordFirstPass(substr);
            else processWordSecondPass(substr);
        }
        return true;
    }


    return false;
}

void Assembler::processWordFirstPass(string line) {
    isGlobalFirst = false;
    locationCounter += 2;
}

bool Assembler::checkIfSkip(string line) {
    smatch skip;
    string valueString;
    if (regex_match(line, skip, rx_skip_directive)) {
        valueString = skip.str(1);
        if (first) {
            if (currentSectionName == "") throw SkipSectionError();
            processSkipFirstPass(valueString);
        } else processSkipSecondPass(valueString);
        return true;
    }
    return false;
}

void Assembler::processSkipFirstPass(string line) {
    isGlobalFirst = false;
    int value = literalToDecimal(line);
    locationCounter += value;
}

bool Assembler::checkIfGlobal(string line) {
    smatch global;
    string globalString;
    if (regex_match(line, global, rx_global_directive)) {
        if (first) {
            if (!isGlobalFirst) throw GlobalFirstError(); //mora global da bude prvi
            return true;
        } else {
            globalString = global.str(1);
            stringstream ss(globalString);
            while (ss.good()) {
                string substr;
                getline(ss, substr, ',');
                processGlobalSecondPass(substr);
            }
            return true;
        }
    }
    return false;
}

bool Assembler::checkIfExtern(string line) {
    smatch externM;
    string externString;
    if (regex_match(line, externM, rx_extern_directive)) {
        if (first) {
            isGlobalFirst = false;
            externString = externM.str(1);
            stringstream ss(externString);
            while (ss.good()) {
                string substr;
                getline(ss, substr, ',');
                processExtern(substr);
            }
        }
        return true;
    }
    return false;
}

void Assembler::processExtern(string line) {
    if (symbolTable.find(line) != symbolTable.end()) throw ExternExistError();

    Assembler::symbolTableEntry newSymbol = symbolTableEntry();
    newSymbol.isGlobal = true;
    newSymbol.section = 0; //mora li eksplicitno undefined??
    newSymbol.symbolName = line;
    newSymbol.value = 0;
    symbolTable.insert(make_pair(line, newSymbol));
}

bool Assembler::checkIfEnd(string line) {
    smatch end;
    if (regex_match(line, end, rx_end_directive)) {
        isGlobalFirst = false;
        isEnd = true; //AZURIRAJ NA TRUE POSLE PRVOG PROLAZA!!
        if (currentSectionName != "") {
            symbolTable.find(currentSectionName)->second.size = locationCounter;
        }
        return true;

    } else return false;
}

bool Assembler::isInstruction(string line) {
    if(checkIfNoOperand(line) || checkIfLdStr(line)||
    checkIfJump(line)|| checkIfInstrWithTwoReg(line) || checkIfOneOpReg(line))
        return true;
    else return false;
    }




Assembler &Assembler::getInstance() {
    return *instance.get();
}

void Assembler::processSectionSecondPass(string line) {

}

void Assembler::processSkipSecondPass(string line) {

}

void Assembler::processGlobalSecondPass(string line) {

}

void Assembler::processWordSecondPass(string line) {

}

void Assembler::processInputSecondPass() {

}

bool Assembler::processAfterLabel(string line) {
    if (line.empty()) return true;
    if (checkIfSection(line) || checkIfEnd(line) || checkIfEqu(line) || checkIfExtern(line)
        || checkIfGlobal(line) || checkIfSkip(line) || checkIfWord(line) || isInstruction(line))
        return true;
    return false;
}

bool Assembler::checkIfNoOperand(string line) {
    smatch instruction;
    if(regex_match(line, instruction, rx_no_operand_instruction)){
        if (first) { locationCounter += 1; }
        else processIfNoOperand(line);
        return true;
    }
    return false;
}

bool Assembler::checkIfOneOpReg(string line) {
    smatch instruction;
    if(regex_match(line, instruction, rx_one_operand_register_instruction)){
        if (first) {
            string instr=instruction.str(1);
            if(instr=="int"|| instr=="not") locationCounter+=2;
            if(instr=="pop" || instr=="push") locationCounter+=3;
        }
        else processIfOneOpReg(line);
        return true;
    }
    return false;
}


void Assembler::printSymbolTable() {
    ofstream file(this->outputFileName);

    file << "Symbol table:" << endl;
    file << "Value\tType\tSection\t\tName\t\tId\t\tSize" << endl;
    for (auto &entry: symbolTable) {
        file << hex << setfill('0') << setw(8) << (0xffff & entry.second.value) << "\t";

        if (entry.second.isGlobal == false)
            file << "l\t";
        else {
            file << "g\t";
        }
        file << dec << setw(8) << entry.second.section << "\t" << setfill(' ') << setw(8) << entry.second.symbolName
             << "\t"
             << setfill('0')
             << setw(8) << (0xffff & entry.second.id) << "\t";
        file << dec << setw(8) << setfill('0') << entry.second.size << endl;
    }
    file << dec;
}

bool Assembler::checkIfJump(string line) {
    smatch jump;
    if(regex_match(line, jump,rx_one_operand_all_kind_addressing_jumps)){
        //string jmp=jump.str(1);
        string operand=jump.str(2);
        if(checkIfJumpAbsolute(operand)|| checkIfJumpMemDir(operand)|| checkIfJumpPcRel(operand)
            || checkIfJumpRegInd(operand) || checkIfJumpRegDir(operand)|| checkIfJumpRegIndDis(operand))
            return true;
        else throw AddressingError();
    }
    return false;
}

bool Assembler::checkIfJumpAbsolute(string line) {
    smatch jump;
    if(regex_match(line, jump, rx_jmp_address_syntax_notation_absolute)){
        if(first) locationCounter+=5;
        else processJumpAbsolute(line); //ili da prosledim smatch
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpPcRel(string line) {
    smatch jump;
    if(regex_match(line, jump, rx_jmp_address_syntax_notation_symbol_pc_relative)){
        if(first) locationCounter+=5;
        else processJumpPcRel(line);
        return true;
    }
    return false;
}

void Assembler::processJumpPcRel(string basicString) {

}

bool Assembler::checkIfJumpRegDir(string line) {
    smatch jump;
    if(regex_match(line, jump, rx_jmp_address_syntax_notation_regdir)){
        if(first) locationCounter+=3;
        else processJumpRegDir(line);
        return true;
    }

    return false;
}

void Assembler::processJumpRegDir(string basicString) {

}

bool Assembler::checkIfJumpRegInd(string line) {
    smatch jump;
    if(regex_match(line, jump, rx_jmp_address_syntax_notation_regind)){
        if(first) locationCounter+=3;
        else processJumpRegInd(line);
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpRegIndDis(string line) {
    smatch jump;
    if(regex_match(line, jump, rx_jmp_address_syntax_notation_regind_with_displacement)){
        if(first) locationCounter+=5;
        else processJumpRegIndDis(line);
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpMemDir(string line) {
    smatch jump;
    if(regex_match(line, jump, rx_jmp_address_syntax_notation_memdir)){
        if(first) locationCounter+=5;
        else processJumpMemDir(line);
        return true;
    }
    return false;
}

void Assembler::processJumpRegInd(string basicString) {

}

bool Assembler::checkIfLdStr(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_two_operand_all_kind_addressing_load_store)){
        string operand=regop.str(3);
        if(checkIfLdStrAbsolute(operand)|| checkIfLdStrMemDir(operand)|| checkIfLdStrPcRel(operand)||
        checkIfLdStrRegDir(operand)|| checkIfLdStrRegInd(operand) || checkIfLdStrRegIndDis(operand))
            return true;
        else throw AddressingError();
    }
    return false;
}

void Assembler::processJumpRegIndDis(string basicString) {

}

void Assembler::processJumpMemDir(string basicString) {

}

bool Assembler::checkIfLdStrAbsolute(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_load_store_address_syntax_notation_absolute)){
        if(first) locationCounter+=5;
        else processLdStrAbsolute(line);
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrPcRel(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_load_store_address_syntax_notation_pc_relative)){
        if(first) locationCounter+=5;
        else processLdStrPcRel(line);
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrRegDir(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_load_store_address_syntax_notation_regdir)){
        if(first) locationCounter+=3;
        else processLdStrRegDir(line);
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrRegInd(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_load_store_address_syntax_notation_regind)){
        if(first) locationCounter+=3;
        else processLdStrRegInd(line);
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrRegIndDis(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_load_store_address_syntax_notation_regind_with_displacement)){
        if(first) locationCounter+=5;
        else processLdStrRegIndDis(line);
        return true;
    }

    return false;
}

void Assembler::processLdStrAbsolute(string basicString) {

}

void Assembler::processLdStrPcRel(string basicString) {

}

void Assembler::processLdStrRegDir(string basicString) {

}

void Assembler::processLdStrRegInd(string basicString) {

}

bool Assembler::checkIfLdStrMemDir(string line) {
    smatch regop;
    if(regex_match(line, regop, rx_load_store_address_syntax_notation_memdir)){
        if(first) locationCounter+=5;
        else processLdStrMemDir(line);
        return true;
    }

    return false;
}

bool Assembler::checkIfInstrWithTwoReg(string line) {
    smatch tworeg;
    if(regex_match(line, tworeg, rx_load_store_address_syntax_notation_absolute)){
        if(first) locationCounter+=2;
        else processInstrWithTwoREg(line);
        return true;
    }
    return false;
}

void Assembler::processLdStrRegIndDis(string basicString) {

}

void Assembler::processLdStrMemDir(string basicString) {

}

void Assembler::processInstrWithTwoREg(string basicString) {

}

void Assembler::processIfNoOperand(string line) {

}

void Assembler::processIfOneOpReg(string line) {

}

void Assembler::processJumpAbsolute(string line) {

}






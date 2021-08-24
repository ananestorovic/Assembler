//
// Created by ss on 8/20/21.
//

#include "assembler.h"
#include "regexes.h"
#include <fstream>
#include <iostream>

using namespace std;

int Assembler::symbolTableEntry::id = 1;
string Assembler::currentSectionName = "";
bool Assembler::isGlobalFirst = false;
bool Assembler::isEnd = false;
bool Assembler::first = true;
int Assembler::locationCounter = 0;
unique_ptr<Assembler> Assembler::instance = std::unique_ptr<Assembler>(new Assembler());


void Assembler::processInputFileFirstPass(const string &inputFileName) {
    this->inputFileName = inputFileName;
    ifstream file(this->inputFileName);
    if (file.is_open()) {
        string line;
        first = true;
        while (getline(file, line)) {
            if (isEnd == true) break;
            if (isDirective(line)) continue;
            //processInstructionsFirstPass(line);

            //printf("%s", line.c_str());
        }
        isEnd = false;
        first = false;
        currentSectionName = "";
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
            if (isGlobalFirst == false) throw GlobalFirstError();
            processSectionFirstPass(sectionName.str(1));
        } else {
            processSectionSecondPass(sectionName.str(1));
        }
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
    currentSectionName = newSymbol.symbolName;
    symbolTable.insert(make_pair(line, newSymbol));

}

bool Assembler::checkIfEqu(string line) {
    smatch equParts;
    string name;
    string valueString;
    int value;
    if (regex_search(line, equParts, rx_equ_directive)) {
        name = equParts.str(1);
        valueString = equParts.str(2);
    } else return false;
    if (symbolTable.find(name) != symbolTable.end()) {
        if (symbolTable.find(name)->second.isDefined == true) throw EquDefError();
        if (symbolTable.find(name)->second.section == 0) throw EquExternError(); // kod mene extern ima 0 za sekciju
    }
    if (isGlobalFirst == false) throw GlobalFirstError();
    value = literalToDecimal(valueString);
    processEqu(name, value);
    return true;
}

void Assembler::processEqu(string name, int value) {
    if (symbolTable.find(name) != symbolTable.end()) {
        symbolTable.find(name)->second.isDefined = true;
        symbolTable.find(name)->second.value = value;
        symbolTable.find(name)->second.section = -1; // za equ mi je sekcija -1
    } else {
        Assembler::symbolTableEntry newSymbol = symbolTableEntry();
        newSymbol.isDefined = true;
        newSymbol.value = value;
        newSymbol.section = -1;
        newSymbol.symbolName = name;
        symbolTable.insert(make_pair(name, newSymbol));
    }

}

int Assembler::literalToDecimal(string literal) { //prepravi ovo
    smatch number;
    int decimal;
    if (regex_search(literal, number, rx_literal_decimal))
        decimal = stoi(number.str(1));
    else {
        regex_search(literal, number, rx_literal_hexadecimal);
        stringstream ss;
        ss << number.str(1).substr(2);
        ss >> hex >> decimal;
    }
    return decimal;
}

bool Assembler::checkIfLabel(string line) {
    smatch label;
    string labelName;
    string command;
    if (regex_match(line, label, rx_label_only)) {
        labelName = label.str(1);
        if (checkIfLabelIsOk(labelName)) {
            processLabel(labelName);
            return true;
        }
    } else if (regex_match(line, label, rx_label_with_command)) {
        labelName = label.str(1);
        command = label.str(2);
        if (checkIfLabelIsOk(labelName)) {
            checkWhatIsAfterLabel(command);
            processLabel(labelName);
            return true;
        }
    }
    return false;
}

bool Assembler::checkIfLabelIsOk(string labelName) {
    if (symbolTable.find(labelName) != symbolTable.end()) {
        if (symbolTable.find(labelName)->second.section == -2)throw LabelSectionError();
        if (symbolTable.find(labelName)->second.isDefined == true) throw LabelDefError();
        if (currentSectionName == "") throw LabelExternError();
    }
    return true;
}

void Assembler::processLabel(string labelName) {
    if (isGlobalFirst == false) throw GlobalFirstError();
    if (symbolTable.find(labelName) != symbolTable.end()) {
        symbolTable.find(labelName)->second.value = locationCounter;
        symbolTable.find(labelName)->second.isDefined = true;
        symbolTable.find(labelName)->second.isGlobal = false; // ne znam treba li ovo
        symbolTable.find(labelName)->second.section = symbolTable.find(currentSectionName)->second.id;
    } else {
        symbolTableEntry newSymbol = symbolTableEntry();
        newSymbol.symbolName = labelName;
        newSymbol.value = locationCounter;
        newSymbol.isDefined = true;
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
    if (isGlobalFirst == false) throw GlobalFirstError();
    locationCounter += 2;
    symbolTable.find(line)->second.size += 2;
}

bool Assembler::checkIfSkip(string line) {
    smatch skip;
    string valueString;
    if (regex_match(line, skip, rx_skip_directive)) {
        valueString = skip.str(1);
        if (first) {
            if (currentSectionName == "") throw SkipSectionError();
            processSkipFirstPass(valueString);
            return true;
        } else processSkipSecondPass(valueString);
    }
    return false;
}

void Assembler::processSkipFirstPass(string line) {
    if (isGlobalFirst == false) throw GlobalFirstError();
    int value = literalToDecimal(line);
    locationCounter += value;
}

bool Assembler::checkIfGlobal(string line) {
    smatch global;
    string globalString;
    if (regex_match(line, global, rx_global_directive)) {
        isGlobalFirst = true; //global mora biti prva u kodu
        if (first) return true;
        else {
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
        if (isGlobalFirst == false) throw GlobalFirstError();
        externString = externM.str(1);
        stringstream ss(externString);
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            processExtern(substr);
        }
        return true;
    }
    return false;
}

void Assembler::processExtern(string line) {
    if (symbolTable.find(line) != symbolTable.end()) throw ExternExistError();
    else {
        Assembler::symbolTableEntry newSymbol = symbolTableEntry();
        newSymbol.isGlobal = true;
        newSymbol.section = 0; //mora li eksplicitno undefined??
        newSymbol.symbolName = line;
        newSymbol.value = 0;
        symbolTable.insert(make_pair(line, newSymbol));

    }

}

bool Assembler::checkIfEnd(string line) {
    smatch end;
    if (regex_match(line, end, rx_end_directive)) {
        if (isGlobalFirst == false) throw GlobalFirstError();
        isEnd = true; //AZURIRAJ NA TRUE POSLE PRVOG PROLAZA!!
        return true;
    } else return false;
}

void Assembler::processInstructionsFirstPass(string line) {
    smatch instruction;
    string instructionString;
    if (regex_match(line, instruction, rx_no_operand_instruction)) {
        locationCounter += 1;
        instructionString = instruction.str(1);
        if (instructionString == "int" || instructionString == "not") locationCounter += 2;
        if (instructionString == "push" || instructionString == "pop") locationCounter += 3;
        return;
    }
    if (regex_match(line, instruction, rx_no_operand_instruction)) {


    }


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

void Assembler::checkWhatIsAfterLabel(string line) {

}

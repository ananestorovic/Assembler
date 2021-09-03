#include "assembler.h"
#include "regexes.h"
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;


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

        first = false;
        processInputFile();

        printSymbolTable();
        printRelocationTable();
        printCodeBySection();
    } catch (const exception &error) {
        cout << error.what();
    }
}


void Assembler::processInputFile() {
    ifstream file(this->inputFileName);
    if (file.is_open()) {
        string line;
        smatch emptyLine;
        while (getline(file, line)) {
            if (isEnd == true) break;
            if (line.empty()) continue;
            if (isComment(line)) continue;
            if (isDirective(line)) continue;
            if (isInstruction(line)) continue;

            line = removeSpaces(line);
            if (line.empty()) continue;

            printf("%s\n", line.c_str());
            throw NothingError();
        }
        if (!isEnd) throw NotEndError();
        isEnd = false;
        currentSectionName = "";
        locationCounter = 0;
        file.close();
    } else throw FileError();

}

Assembler &Assembler::getInstance() {
    return *instance.get();
}

void Assembler::printSymbolTable() {
    ofstream file(this->outputFileName);

    file << "Symbol table:" << endl;
    file << "Value\tType\tSection\t\tName\t\tId\t\tSize" << endl;
    for (auto &entry: symbolTable) {
        file << dec << setfill('0') << setw(8) << entry.second.value << "\t";
        //file << hex << setfill('0') << setw(8) << (0xffff & entry.second.value) << "\t";

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
    file << dec << endl;


}


void Assembler::printRelocationTable() {

    ofstream file(this->outputFileName, ios::app);

    file << "Relocation table:" << endl << endl;

    for (const auto &oneSection: relocationTable) {
        if (oneSection.second.empty()) continue;
        file << "section name: " << oneSection.first << endl << endl;
        file << setfill(' ') << setw(8) << "Offset" << "\t\t"
             << setfill(' ') << setw(8) << "Type" << "\t\t"
             << setfill(' ') << setw(8) << "Value" << endl;
        for (const auto &entry: oneSection.second) {
            file << hex << setfill('0') << setw(8) << (0xffff & entry.offset) << "\t\t";
            file << setfill('0') << setw(8) << entry.typeOfRelocation << "\t\t";
            file << dec << setw(8) << setfill('0') << entry.value << "\t\t" << endl << endl;
        }
    }
    file << dec << endl;
}

void Assembler::printCodeBySection() {
    ofstream file(this->outputFileName, ios::app);

    file << "Code:" << endl << endl;

    for (auto &oneSection: codeBySection) {
        if (oneSection.second.empty()) continue;
        file << "section name: " << oneSection.first << endl << endl;

        for (const auto &byte: oneSection.second) {

            file << hex << setw(2) << setfill('0') << (((int) byte) & 0xFF) << "\t";


        }
        file << dec << endl << endl;

    }
}


string Assembler::removeSpaces(string &line) {

    return regex_replace(line, removeSpaceAndTabsReg, "");
}

int Assembler::literalToDecimal(string literal) {
    smatch helper;
    if (regex_match(literal, helper, literalReg)) {
        return std::stoi(literal, nullptr, 0);
    }
    throw LiteralError();
}

bool Assembler::isComment(string line) {
    smatch comment;
    if (regex_match(line, comment, commentReg)) {
        return true;
    } else return false;
}


bool Assembler::isDirective(string line) {
    if (checkIfSection(line) || checkIfEnd(line) || checkIfEqu(line) || checkIfExtern(line)
        || checkIfGlobal(line) || checkIfSkip(line) || checkIfWord(line)
        || checkIfLabel(line))
        return true;
    else return false;
}


bool Assembler::isInstruction(string line) {
    if (checkIfNoOperand(line) || checkIfLdStr(line) ||
        checkIfJump(line) || checkIfInstrWithTwoReg(line) || checkIfOneOpReg(line)) {
        isLabelWithoutCode = false;
        return true;
    } else return false;
}


bool Assembler::checkIfSection(string line) {
    smatch sectionName;
    if (regex_match(line, sectionName, sectionDirectiveReg)) {
        string name = sectionName.str(1);
        if (first) {
            codeBySection.insert(make_pair(name, bytes));
            relocationTable.insert(make_pair(name, list<relocationTableEntry>()));
            processSectionFirstPass(name);
            isGlobalFirst = false;
        } else processSectionSecondPass(name);
        return true;
    }
    return false;
}

bool Assembler::checkIfEqu(string line) {
    smatch equParts;
    string name;
    string valueString;
    int value;
    if (regex_match(line, equParts, equDirectiveReg)) {
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

bool Assembler::checkIfLabel(string line) {
    smatch label;
    string labelName;
    string command;

    if (regex_match(line, label, labelAndOptionalCommand)) {
        labelName = label.str(1);
        command = label.str(2);
        if (first) {
            if (checkIfLabelIsOk(labelName)) {
                isLabelWithoutCode = true;
                processLabel(labelName);
                return processAfterLabel(command);

            }
        } else return processAfterLabel(command);
    }
    return false;
}

bool Assembler::checkIfLabelIsOk(string labelName) {
    if (symbolTable.find(labelName) != symbolTable.end()) throw LabelDefError();
    if (currentSectionName == "") throw LabelSectionError();
    return true;
}

bool Assembler::checkIfWord(string line) {
    smatch word;
    string data;
    if (regex_match(line, word, wordDirectiveReg)) {
        if (currentSectionName == "") throw WordSectionError();
        data = word.str(1);
        stringstream ss(data);
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            substr = removeSpaces(substr);
            if (first) processWordFirstPass(substr);
            else processWordSecondPass(substr);
        }
        return true;
    }


    return false;
}


bool Assembler::checkIfSkip(string line) {
    smatch skip;
    string valueString;
    if (regex_match(line, skip, skipDirectiveReg)) {
        valueString = skip.str(1);
        if (first) {
            if (currentSectionName == "") throw SkipSectionError();
            processSkipFirstPass(valueString);
        } else processSkipSecondPass(valueString);
        return true;
    }
    return false;
}

bool Assembler::checkIfGlobal(string line) {
    smatch global;
    string globalString;
    if (regex_match(line, global, globalDirectiveReg)) {
        if (first) {
            if (!isGlobalFirst) throw GlobalFirstError(); //mora global da bude prvi
            return true;
        } else {
            globalString = global.str(1);
            stringstream ss(globalString);
            while (ss.good()) {
                string substr;
                getline(ss, substr, ',');
                substr = removeSpaces(substr);
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
    if (regex_match(line, externM, externDirectiveReg)) {
        if (first) {
            isGlobalFirst = false;
            externString = externM.str(1);
            stringstream ss(externString);
            while (ss.good()) {
                string substr;
                getline(ss, substr, ',');
                substr = removeSpaces(substr);
                processExtern(substr);
            }
        }
        return true;
    }
    return false;
}

bool Assembler::checkIfEnd(string line) {
    smatch end;
    if (regex_match(line, end, endDirectiveReg)) {
        if ((currentSectionName != "") && first) {

            if (isLabelWithoutCode) throw NothingAfterLabelError();
            isLabelWithoutCode = false;
            symbolTable.find(currentSectionName)->second.size = locationCounter;
        }
        isGlobalFirst = false;
        isEnd = true;
        return true;
    } else return false;
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


void Assembler::processEqu(string name, int value) {
    Assembler::symbolTableEntry newSymbol = symbolTableEntry();
    newSymbol.value = value;
    newSymbol.section = -1;
    newSymbol.symbolName = name;
    symbolTable.insert(make_pair(name, newSymbol));
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


void Assembler::processWordFirstPass(string line) {
    isLabelWithoutCode = false;
    isGlobalFirst = false;
    locationCounter += 2;
}


void Assembler::processSkipFirstPass(string line) {
    isGlobalFirst = false;
    isLabelWithoutCode = false;
    int value = literalToDecimal(line);
    locationCounter += value;
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


void Assembler::processSectionSecondPass(string line) {
    locationCounter = 0;
    currentSectionName = symbolTable.find(line)->second.symbolName;
}

void Assembler::processSkipSecondPass(string line) {
    int value = literalToDecimal(line);
    locationCounter += value;
    while (value--) {
        codeBySection.find(currentSectionName)->second.push_back(0);
    }

}

void Assembler::processGlobalSecondPass(string line) {
    if (symbolTable.find(line) == symbolTable.end()) throw NotDefGlobalSecondPassError();
    symbolTable.find(line)->second.isGlobal = true;
}


void Assembler::processWordSecondPass(string line) {


    smatch literal;
    if (regex_match(line, literal, literalReg)) {

        int value = literalToDecimal(line);
        processWordHelper(value);
        locationCounter += 2;
        return;
    }
    smatch symbol;
    if (symbolTable.find(line) == symbolTable.end()) throw NotDefWordSecondPassError();
    if (regex_match(line, symbol, symbolReg)) {

        if (symbolTable.find(line) == symbolTable.end()) throw NotDefSymbol();

        if (symbolTable.find(line)->second.section == -1) {

            int value = symbolTable.find(line)->second.value;
            processWordHelper(value);
            locationCounter += 2;
            return;

        }

        if (!(symbolTable.find(line)->second.isGlobal)) {

            int value = symbolTable.find(line)->second.value;
            processWordHelper(value);
            relocationTableEntry rT = relocationTableEntry();
            rT.offset = locationCounter;
            locationCounter += 2;
            rT.typeOfRelocation = "R_386_16";
            rT.value = symbolTable.find(line)->second.section;
            relocationTable[currentSectionName].push_back(rT);
            return;

        } else {
            processWordHelper(0);
            relocationTableEntry rT = relocationTableEntry();
            rT.offset = locationCounter;
            rT.typeOfRelocation = "R_386_16";
            rT.value = symbolTable.find(line)->second.id;
            relocationTable[currentSectionName].push_back(rT);
            locationCounter += 2;
            return;

        }
    }

    throw AfterWordError();


}


bool Assembler::processAfterLabel(string line) {
    if (line.empty()) {
        return true;
    }
    if (checkIfSection(line) || checkIfEnd(line) || checkIfEqu(line) || checkIfExtern(line)
        || checkIfGlobal(line) || checkIfSkip(line) || checkIfWord(line) || isInstruction(line))
        return true;
    return false;
}


void Assembler::instr2Bytes(string instrDescr, int regsDescr) {

    codeBySection[currentSectionName].push_back(instrDescription[instrDescr]);
    codeBySection[currentSectionName].push_back(regsDescr);

    return;
}

void Assembler::instr3Bytes(string instrDescr, int regsDescr, int addrMode) {


    codeBySection[currentSectionName].push_back(instrDescription[instrDescr]);
    codeBySection[currentSectionName].push_back(regsDescr);
    codeBySection[currentSectionName].push_back(addrMode);

    return;
}

void Assembler::instr5Bytes(string instrDescr, int regsDescr, int addrMode, int value) {

    codeBySection[currentSectionName].push_back(instrDescription[instrDescr]);
    codeBySection[currentSectionName].push_back(regsDescr);
    codeBySection[currentSectionName].push_back(addrMode);
    codeBySection[currentSectionName].push_back(value & 0xff);
    codeBySection[currentSectionName].push_back((value >> 8) & 0xff);


    return;

}

void Assembler::processWordHelper(int value) {


    char hB = (((unsigned) value) >> 8) & 0xFF;
    char lB = ((unsigned) value) & 0xFF;

    codeBySection[currentSectionName].push_back(lB);
    codeBySection[currentSectionName].push_back(hB);
    return;

}

int Assembler::processAbsoluteAddressingSymbol(string line) {

    if (symbolTable.find(line) != symbolTable.end()) {
        if (symbolTable.find(line)->second.section == -1) return symbolTable.find(line)->second.value;
        else {
            relocationTableEntry rT = relocationTableEntry();
            rT.offset += locationCounter + 3;
            rT.typeOfRelocation = "R_386_16";
            rT.section = currentSectionName;

            if (symbolTable.find(line)->second.isGlobal || (symbolTable.find(line)->second.section == 0)) {
                rT.value = symbolTable.find(line)->second.id;
                relocationTable[currentSectionName].push_back(rT);
                return 0;
            } else {
                rT.value = symbolTable.find(line)->second.section;
                relocationTable[currentSectionName].push_back(rT);
                return symbolTable.find(line)->second.value;
            }
        }

    } else throw NotDefSymbol();
}

int Assembler::processPcRelativeAddressingSymbol(string line) {
    int ret = -2;
    if (symbolTable.find(line) != symbolTable.end()) {
        relocationTableEntry rT = relocationTableEntry();
        if (symbolTable.find(line)->second.section == -1) {
            ret = (symbolTable.find(line)->second.value) + ret - (locationCounter + 3);
            return ret;
        } else {
            rT.offset += locationCounter + 3;
            rT.typeOfRelocation = "R_386_PC16";
            rT.section = currentSectionName;

            if (symbolTable.find(line)->second.isGlobal || (symbolTable.find(line)->second.section == 0)) {
                if (symbolTable.find(line)->second.section == symbolTable.find(currentSectionName)->second.id) {
                    ret = (symbolTable.find(line)->second.value) + ret - (locationCounter + 3);
                    return ret;
                }

                rT.value = symbolTable.find(line)->second.id;
            } else {
                if (symbolTable.find(line)->second.section == symbolTable.find(currentSectionName)->second.id) {
                    ret = (symbolTable.find(line)->second.value) + ret - (locationCounter + 3);
                    return ret;
                } else {
                    rT.value = symbolTable.find(line)->second.section;
                    int a = symbolTable.find(line)->second.value;
                    ret += a;
                }
            }
            relocationTable[currentSectionName].push_back(rT);
            return ret;
        }

    } else throw NotDefSymbol();
}





bool Assembler::checkIfNoOperand(string line) {
    smatch instruction;
    if (regex_match(line, instruction, noOperandInstructionReg)) {
        if (!first) {
            string nameInstruction = instruction.str(1);
            processIfNoOperand(nameInstruction);
        }
        locationCounter += 1;
        return true;
    }
    return false;
}

bool Assembler::checkIfOneOpReg(string line) {
    smatch instruction;
    if (regex_match(line, instruction, oneOperandInstructionRegisterOnlyReg)) {
        string instr = instruction.str(1);
        if (!first) {
            int reg;
            string leftover = instruction.str(2);
            if (leftover == "psw") reg = 8;
            else reg = instruction.str(2).at(1) - '0'; //mozda je moglo drugacije
            processIfOneOpReg(instr, reg);
        }
        if (instr == "int" || instr == "not") locationCounter += 2;
        if (instr == "pop" || instr == "push") locationCounter += 3;
        return true;
    }
    return false;
}


bool Assembler::checkIfJump(string line) {
    smatch jump;
    if (regex_match(line, jump, jumpInstructionsReg)) {
        string instruction = jump.str(1);
        string operand = jump.str(2);
        if (checkIfJumpRegDir(instruction, operand) || checkIfJumpAbsolute(instruction, operand) ||
            checkIfJumpMemDir(instruction, operand) ||
            checkIfJumpPcRel(instruction, operand)
            || checkIfJumpRegInd(instruction, operand) ||
            checkIfJumpRegIndDis(instruction, operand))
            return true;
        else throw AddressingError();
    }
    return false;
}

bool Assembler::checkIfJumpAbsolute(string instruction, string operand) {
    smatch jump;
    if (regex_match(operand, jump, jmpInstructionAbsoluteAddrReg)) {
        if (!first) processJumpAbsolute(instruction, operand);
        locationCounter += 5;
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpPcRel(string instruction, string operand) {
    smatch jump;
    if (regex_match(operand, jump, jmpInstructionPCAddrReg)) {
        if (!first)  {
            string op = jump.str(1);
            processJumpPcRel(instruction, op);
        }
        locationCounter+=5;
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpRegDir(string instruction, string operand) {
    smatch jump;
    if (regex_match(operand, jump, jmpInstructionRegisterDirAddrReg)) {
        if (!first){
            int reg = (jump.str(1) == "psw") ? 8 : jump.str(1).at(1) - '0';
            processJumpRegDir(instruction, reg);
        }
        locationCounter += 3;
        return true;
    }

    return false;
}

bool Assembler::checkIfJumpRegInd(string instruction, string operand) {
    smatch jump;
    if (regex_match(operand, jump, jmpInstructionRegisterIndAddrReg)) {
        if (!first)
        {
            int reg = (jump.str(1) == "psw") ? 8 : jump.str(1).at(1) - '0';
            processJumpRegInd(instruction, reg);
        }
        locationCounter += 3;
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpRegIndDis(string instruction, string operand) {
    smatch jump;
    if (regex_match(operand, jump, jmpInstructionRegisterIndWithDisplacementsAddrReg)) {
        if (!first)
       {
            string dis = jump.str(2);
            int reg = (jump.str(1) == "psw") ? 8 : jump.str(1).at(1) - '0';
            processJumpRegIndDis(instruction, reg, dis);
        }
        locationCounter += 5;
        return true;
    }
    return false;
}

bool Assembler::checkIfJumpMemDir(string instruction, string operand) {
    smatch jump;
    if (regex_match(operand, jump, jmpInstructionMemdirAddrReg)) {
        operand = jump.str(1);
        if (!first) processJumpMemDir(instruction, operand);
        locationCounter += 5;
        return true;
    }
    return false;
}

void Assembler::processIfNoOperand(string instruction) {
    if (instruction == "halt") {
        codeBySection[currentSectionName].push_back(instrDescription[instruction]);
    } else if (instruction == "iret") {
        codeBySection[currentSectionName].push_back(instrDescription[instruction]);
    } else if (instruction == "ret") {
        codeBySection[currentSectionName].push_back(instrDescription[instruction]);
    }

    return;

}

void Assembler::processIfOneOpReg(string line, int reg) {
    if (line == "int") {
        codeBySection[currentSectionName].push_back(locationCounter);
        codeBySection[currentSectionName].push_back(instrDescription[line]);
        codeBySection[currentSectionName].push_back((reg << 4) + 15);

    } else if (line == "not") {
        codeBySection[currentSectionName].push_back(locationCounter);
        codeBySection[currentSectionName].push_back(instrDescription[line]);
        codeBySection[currentSectionName].push_back((reg << 4) + 15);

    } else if (line == "pop") {
        codeBySection[currentSectionName].push_back(locationCounter);
        codeBySection[currentSectionName].push_back(instrDescription[line]);
        codeBySection[currentSectionName].push_back((reg << 4) + 6);
        codeBySection[currentSectionName].push_back(literalToDecimal("0x42"));

    } else if (line == "push") {
        codeBySection[currentSectionName].push_back(locationCounter);
        codeBySection[currentSectionName].push_back(instrDescription[line]);
        codeBySection[currentSectionName].push_back((reg << 4) + 6);
        codeBySection[currentSectionName].push_back(literalToDecimal("0x12"));

    }

    return;

}

void Assembler::processJumpAbsolute(string instruction, string operand) {
    int reg = 0xFF;
    int adr = 0;
    int value;
    if (regex_match(operand, symbolReg)) value = processAbsoluteAddressingSymbol(operand);
    else value = literalToDecimal(operand);

    instr5Bytes(instruction, reg, adr, value);

}

void Assembler::processJumpPcRel(string instruction, string operand) {
    int reg = 0xF7;
    int adr = 0x05;
    int value = processPcRelativeAddressingSymbol(operand);

    instr5Bytes(instruction, reg, adr, value);
    return;

}

void Assembler::processJumpRegDir(string instruction, int reg) {
    int adr = 0x01;

    instr3Bytes(instruction, reg, adr);
    return;
}

void Assembler::processJumpRegInd(string instruction, int reg) {
    int adr = 0x02;

    instr3Bytes(instruction, reg, adr);
    return;
}

void Assembler::processJumpRegIndDis(string instruction, int reg, string dis) {
    int adr = 0x03;
    int value = literalToDecimal(dis);

    instr5Bytes(instruction, reg, adr, value);
    return;

}

void Assembler::processJumpMemDir(string instruction, string operand) {
    int reg = 0xFF;
    int adr = 0x04;
    int value;
    if (regex_match(operand, symbolReg)) value = processAbsoluteAddressingSymbol(operand);
    else value = literalToDecimal(operand);

    instr5Bytes(instruction, reg, adr, value);

}

bool Assembler::checkIfLdStr(string line) {
    smatch regop;
    if (regex_match(line, regop, twoOperandInstructionOneRegisterOtherAnyAddrMode)) {
        string instruction = regop.str(1);
        int regD = (regop.str(2) == "psw") ? 8 : literalToDecimal(regop.str(2).substr(1));
        regD <<= 4;
        string operand = regop.str(3);
        if (checkIfLdStrRegDir(instruction, regD, operand) || checkIfLdStrAbsolute(instruction, regD, operand) ||
            checkIfLdStrMemDir(instruction, regD, operand) ||
            checkIfLdStrPcRel(instruction, regD, operand) ||
            checkIfLdStrRegInd(instruction, regD, operand) ||
            checkIfLdStrRegIndDis(instruction, regD, operand))
            return true;
        else throw AddressingError();
    }
    return false;
}


bool Assembler::checkIfLdStrAbsolute(string instruction, int reg, string operand) {
    smatch regop;
    if (regex_match(operand, regop, dataInstructionAbsoluteAddrReg)) {
        if (!first) {
            operand = regop.str(1);
            processLdStrAbsolute(instruction, reg, operand);
        }
        locationCounter += 5;
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrPcRel(string instruction, int reg, string operand) {
    smatch regop;
    if (regex_match(operand, regop, dataInstructionPCAddrReg)) {
        if (!first) {
            operand = regop.str(1);
            processLdStrPcRel(instruction, reg, operand);
        }
        locationCounter += 5;
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrRegDir(string instruction, int reg, string operand) {
    smatch regop;
    if (regex_match(operand, regop, dataInstructionRegisterDirAddrReg)) {
        if (!first) {
            int regs = (regop.str(1) == "psw") ? 8 : (regop.str(1).at(1) - '0');
            regs += reg;
            processLdStrRegDir(instruction, regs, operand);
        }
        locationCounter += 3;
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrRegInd(string instruction, int reg, string operand) {
    smatch regop;
    if (regex_match(operand, regop, dataInstructionRegisterIndAddrReg)) {
        if (!first) {
            int regs = (regop.str(1) == "psw") ? 8 : (regop.str(1).at(1) - '0');
            regs += reg;
            processLdStrRegInd(instruction, regs, operand);
        }
        locationCounter += 3;
        return true;
    }

    return false;
}

bool Assembler::checkIfLdStrRegIndDis(string instruction, int reg, string operand) {
    smatch regop;
    if (regex_match(operand, regop, dataInstructionRegisterIndWithDisplacementsAddrReg)) {
        if (!first) {
            int regs = (regop.str(1) == "psw") ? 8 : (regop.str(1).at(1) - '0');
            regs += reg;
            string dis = regop.str(2);
            processLdStrRegIndDis(instruction, regs, dis);
        }
        locationCounter += 5;
        return true;
    }

    return false;
}


bool Assembler::checkIfLdStrMemDir(string instruction, int reg, string operand) {
    smatch regop;
    if (regex_match(operand, regop, dataInstructionMemdirAddrReg)) {
        if (!first) {
            int regs = reg + 0x0F;
            processLdStrMemDir(instruction, regs, operand);
        }
        locationCounter += 5;
        return true;
    }

    return false;
}


void Assembler::processLdStrAbsolute(string instruction, int reg, string operand) {
    int regs = reg + 0xF;
    int adr = 0;
    int value;
    if (regex_match(operand, symbolReg)) {
        value = processAbsoluteAddressingSymbol(operand);
        instr5Bytes(instruction, regs, adr, value);
    } else {
        value = literalToDecimal(operand);
        instr5Bytes(instruction, regs, adr, value);
    }
    return;
}

void Assembler::processLdStrPcRel(string instruction, int reg, string operand) {
    int regs = reg + 0x07;
    int adr = 0x03;
    int value = processPcRelativeAddressingSymbol(operand);
    instr5Bytes(instruction, regs, adr, value);
    return;
}

void Assembler::processLdStrRegDir(string instruction, int reg, string operand) {
    int adr = 0x01;

    instr3Bytes(instruction, reg, adr);
    return;

}

void Assembler::processLdStrRegInd(string instruction, int reg, string operand) {
    int adr = 0x02;

    instr3Bytes(instruction, reg, adr);
    return;

}

void Assembler::processLdStrRegIndDis(string instruction, int reg, string operand) {
    int adr = 0x03;
    int value;

    if (regex_match(operand, symbolReg)) value = processAbsoluteAddressingSymbol(operand);
    else value = literalToDecimal(operand);

    instr5Bytes(instruction, reg, adr, value);
    return;
}

void Assembler::processLdStrMemDir(string instruction, int reg, string operand) {
    int adr = 0x04;
    int value;

    if (regex_match(operand, symbolReg)) value = processAbsoluteAddressingSymbol(operand);
    else value = literalToDecimal(operand);

    instr5Bytes(instruction, reg, adr, value);
    return;
}

bool Assembler::checkIfInstrWithTwoReg(string line) {
    smatch tworeg;
    if (regex_match(line, tworeg, twoOperandInstructionBothRegisterReg)) {
        if (!first) {
            string instruction = tworeg.str(1);
            int regD = (tworeg.str(2) == "psw") ? 8 : (tworeg.str(2).at(1) - '0');
            regD <<= 4;
            regD += (tworeg.str(3) == "psw") ? 8 : (tworeg.str(3).at(1) - '0');
            processInstrWithTwoReg(instruction, regD);
        }
        locationCounter += 2;
        return true;
    }
    return false;
}

void Assembler::processInstrWithTwoReg(string instruction, int reg) {
    instr2Bytes(instruction, reg);
}











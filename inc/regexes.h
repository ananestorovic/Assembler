
#ifndef ASSEMBLER_REGEXES_H
#define ASSEMBLER_REGEXES_H

#include <regex>
#include <string>

using namespace std;


// helper strings
string literalRegStr = R"reg(-?(?:0|[1-9]\d*)|0(x|X)[\da-fA-F]{1,4})reg";
string symbolRegStr = R"reg([a-zA-Z]\w*)reg";

string registerRegStr = R"reg(r[0-7]|psw|sp|pc)reg"; // there are 8 registers: r0 - r7


// combined helper strings
string symbolOrLiteralRegStr = "(" + symbolRegStr + R"(|)" + literalRegStr + ")";


//space
string spacesOrTabsRegStr = R"reg([ \t]*)reg";

//comment optional
string commentOptRegStr = R"reg((?:#.*)?)reg";

//comment
string commentRegStr = R"reg((?:#.*))reg";

//begin of line
string beginLineRegStr = R"(^)" + spacesOrTabsRegStr;
//end of line
string endLineRegStr = spacesOrTabsRegStr + commentOptRegStr + R"($)";

regex removeSpaceAndTabsReg(R"reg([ \t])reg");


// regexes for assembly directives
regex globalDirectiveReg(
        beginLineRegStr + R"(\.global )" + spacesOrTabsRegStr + "(" + symbolRegStr + "(" + spacesOrTabsRegStr + "," +
        spacesOrTabsRegStr + symbolRegStr + ")*)" + endLineRegStr);
regex externDirectiveReg(
        beginLineRegStr + R"(\.extern )" + spacesOrTabsRegStr + "(" + symbolRegStr + "(" + spacesOrTabsRegStr + "," +
        spacesOrTabsRegStr + symbolRegStr + ")*)" + endLineRegStr);
regex sectionDirectiveReg(
        beginLineRegStr + R"(\.section )" + spacesOrTabsRegStr + "(" + symbolRegStr + ")" + endLineRegStr);
regex wordDirectiveReg(
        beginLineRegStr + R"(\.word )" + spacesOrTabsRegStr + "(" + symbolOrLiteralRegStr + "(" + spacesOrTabsRegStr +
        "," + spacesOrTabsRegStr + symbolOrLiteralRegStr + ")*)" + endLineRegStr);
regex skipDirectiveReg(beginLineRegStr + R"(\.skip )" + spacesOrTabsRegStr + "(" + literalRegStr + ")" + endLineRegStr);
regex equDirectiveReg(
        beginLineRegStr + R"(\.equ )" + spacesOrTabsRegStr + "(" + symbolRegStr + ")" + spacesOrTabsRegStr + "," +
        spacesOrTabsRegStr + "(" + literalRegStr + ")" + endLineRegStr);
regex endDirectiveReg(beginLineRegStr + R"(\.end)" + spacesOrTabsRegStr + endLineRegStr);


regex labelAndOptionalCommand(
        beginLineRegStr + "(" + symbolRegStr + "):" + spacesOrTabsRegStr + "(.*)$"); // something is after label

// data - symbols and numbers - regexes
regex symbolReg("^" + symbolRegStr + "$");
regex literalReg("^" + literalRegStr + "$");

regex commentReg(beginLineRegStr + commentRegStr + "$");


// instructions:


// instructions that operate only with registers
regex noOperandInstructionReg(beginLineRegStr + "(halt|iret|ret)" + endLineRegStr);
regex oneOperandInstructionRegisterOnlyReg(
        beginLineRegStr + "(push|pop|int|not) " + spacesOrTabsRegStr + "(" + registerRegStr + ")" + endLineRegStr);
regex twoOperandInstructionBothRegisterReg(
        beginLineRegStr + "(add|sub|mul|div|cmp|and|or|xor|shl|shr|test|xchg) " + spacesOrTabsRegStr +
        "(" + registerRegStr + ")" + spacesOrTabsRegStr + "," + spacesOrTabsRegStr + "(" + registerRegStr + ")" +
        endLineRegStr);

// instructions with operand (all kind)
// jumps
regex jumpInstructionsReg(
        beginLineRegStr + "(call|jmp|jeq|jne|jgt) " + spacesOrTabsRegStr + "([^#]*)" + endLineRegStr);
// load/store
regex twoOperandInstructionOneRegisterOtherAnyAddrMode(
        beginLineRegStr + "(ldr|str) " + spacesOrTabsRegStr + "(" + registerRegStr + ")" +
        spacesOrTabsRegStr + "," + spacesOrTabsRegStr + "([^#]*)" + endLineRegStr);


string registerAddrModeDataRegStr = "(" + registerRegStr + ")";
string registerIndAddrModeDataRegStr =
        R"reg(\[)reg" + spacesOrTabsRegStr + "(" + registerRegStr + ")" + spacesOrTabsRegStr + R"reg(\])reg";
string registerIndWithDisplacementAddrModeDataRegStr =
        R"reg(\[)reg" + spacesOrTabsRegStr + "(" + registerRegStr + ")" + spacesOrTabsRegStr +
        R"reg(\+)reg" + spacesOrTabsRegStr + "(" + symbolOrLiteralRegStr + ")" + spacesOrTabsRegStr + R"reg(\])reg";


// syntax notation for operand in instruction for jumps
regex jmpInstructionRegisterDirAddrReg(R"reg(^\*)reg" + registerAddrModeDataRegStr + "$");
regex jmpInstructionRegisterIndAddrReg(R"reg(^\*)reg" + registerIndAddrModeDataRegStr + "$");
regex jmpInstructionRegisterIndWithDisplacementsAddrReg(
        R"reg(^\*)reg" + registerIndWithDisplacementAddrModeDataRegStr + "$");
regex jmpInstructionPCAddrReg("^%" + symbolOrLiteralRegStr + "$");
regex jmpInstructionAbsoluteAddrReg("^" + symbolOrLiteralRegStr + "$");
regex jmpInstructionMemdirAddrReg(R"reg(^\*)reg" + symbolOrLiteralRegStr + "$");


// syntax notation for operand in instruction for load/store
regex dataInstructionRegisterDirAddrReg("^" + registerAddrModeDataRegStr + "$");
regex dataInstructionRegisterIndAddrReg("^" + registerIndAddrModeDataRegStr + "$");
regex dataInstructionRegisterIndWithDisplacementsAddrReg("^" + registerIndWithDisplacementAddrModeDataRegStr + "$");
regex dataInstructionPCAddrReg("^%" + symbolOrLiteralRegStr + "$");
regex dataInstructionAbsoluteAddrReg(R"(^\$)" + symbolOrLiteralRegStr + "$");
regex dataInstructionMemdirAddrReg("^" + symbolOrLiteralRegStr + "$");


#endif //ASSEMBLER_REGEXES_H




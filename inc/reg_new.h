#ifndef ASSEMBLER_REGEXES_H
#define ASSEMBLER_REGEXES_H

#include <regex>
#include <string>

using namespace std;


// //ODAVDE KRECE

//string sh_literal_decimal = "-?[1-9]\\d*";                // PREPRAVLJANEI this is a decimal number (positive or negative)
//string sh_literal_hexadecimal = "0(x|X)[\\da-fA-F]{1,4}"; // PREPRAVLJANEI this is a hexadecimal number
string literalRegStr = R"reg(-?[1-9]\d*|0(x|X)[\da-fA-F]{1,4})reg";

//sh_symbol
string symbolRegStr = R"reg([a-zA-Z]\w*)reg"; // symbol can start only with letter and can contain letters, digits and _

//sh_register_range
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


regex commentReg("(" + commentRegStr + ")");

//Need to add trim
// regex rx_find_tabs("\\t");
regex removeSpaceAndTabsReg(R"reg([ \t])reg");

// regexes for assembly directives
// rx_word_directive
regex wordDirectiveReg(
        beginLineRegStr + R"(\.word)" + spacesOrTabsRegStr + "(" + symbolOrLiteralRegStr + "(" + spacesOrTabsRegStr +
        "," + spacesOrTabsRegStr + symbolOrLiteralRegStr + ")*)" + endLineRegStr);
// rx_extern_directive
regex externDirecitveReg(
        beginLineRegStr + R"(\.extern)" + spacesOrTabsRegStr + "(" + symbolRegStr + "(" + spacesOrTabsRegStr + "," +
        spacesOrTabsRegStr + symbolRegStr + ")*)" + endLineRegStr);
//rx_global_directive
regex globalDirectiveReg(
        beginLineRegStr + R"(\.global)" + spacesOrTabsRegStr + "(" + symbolRegStr + "(" + spacesOrTabsRegStr + "," +
        spacesOrTabsRegStr + symbolRegStr + ")*)" + endLineRegStr);
// rx_extern_directive
regex sectionDirectiveReg(
        beginLineRegStr + R"(\.section)" + spacesOrTabsRegStr + "(" + symbolRegStr + ")" + endLineRegStr);
// rx_end_directive
regex endDirectiveReg(beginLineRegStr + R"(\.end)" + spacesOrTabsRegStr + endLineRegStr);
// rx_equ_directive
regex equDirectiveReg(
        beginLineRegStr + R"(\.equ)" + spacesOrTabsRegStr + "(" + symbolRegStr + ")" + spacesOrTabsRegStr + "," +
        spacesOrTabsRegStr + "(" + literalRegStr + ")" + endLineRegStr);

// labels are in form -> 'symbol:'
// label has to be at the beginning of the line
// if there is not anything, label is associated with next command           // nothing is after label
// label_with_command_after tako nesto
regex labelAndOptionalCommanf(
        beginLineRegStr + "(" + symbolRegStr + "):(.*)" + endLineRegStr); // something is after label

// data - symbols and numbers - regexes

//rx symbol
regex symbolReg("^" + symbolRegStr + "$");
//rxIsLiteral
regex literalReg("^" + literalRegStr + "$");

// instructions:
//Zaboravih da napisem imena
regex noOperandInstructionReg(beginLineRegStr + "(halt|iret|ret)" + endLineRegStr);
regex oneOperandInstructionRegisterOnlyReg(
        beginLineRegStr + "(push|pop|int|not)" + spacesOrTabsRegStr + "(" + registerRegStr + ")" + endLineRegStr);
regex twoOperandInstructionBothRegisterReg(
        beginLineRegStr + "(add|sub|mul|div|cmp|and|or|xor|shl|shr|test|xchg)" + spacesOrTabsRegStr +
        "(" + registerRegStr + ")" + spacesOrTabsRegStr + "," + spacesOrTabsRegStr + "(" + registerRegStr + ")" +
        endLineRegStr);

// instructions with operand (all kind)
// jumps
regex jumpInstructionsReg(beginLineRegStr + "(jmp|jeq|jne|jgt|call) (.*)" + endLineRegStr);
// load/store
regex twoOperandInstructionOneRegisterOtherAnyAddrMode(beginLineRegStr + "(ldr|str)" + spacesOrTabsRegStr +
                                                       "(" + registerRegStr + ")" + spacesOrTabsRegStr + "," +
                                                       spacesOrTabsRegStr + "(.*)" + endLineRegStr);

string registerAddrModeDataRegStr = "(" + registerRegStr + ")";
string registerIndAddrModeDataRegStr =
        R"reg(\[)reg" + spacesOrTabsRegStr + "(" + registerRegStr + ")" + spacesOrTabsRegStr + R"reg(\])reg";
string registerIndWithDisplacementAddrModeDataRegStr =
        R"reg(\[)reg" + spacesOrTabsRegStr + "(" + registerRegStr + ")" + spacesOrTabsRegStr +
        R"reg(\+)reg" + spacesOrTabsRegStr + "(" + symbolOrLiteralRegStr + ")" + spacesOrTabsRegStr + R"reg(\])reg";

// syntax notation for operand in instruction for jumps
// regex rx_jmp_address_syntax_notation_absolute("^(" + sh_symbol_or_literal + ")$");
// regex rx_jmp_address_syntax_notation_memdir("^\\*(" + sh_symbol_or_literal + ")$");
// regex rx_jmp_address_syntax_notation_symbol_pc_relative("^%(" + sh_symbol + ")$");
// regex rx_jmp_address_syntax_notation_regdir("^\\*(r[0-7]|psw)$");
// regex rx_jmp_address_syntax_notation_regind("^\\*\\[(r[0-7]|psw)\\]$");
// regex rx_jmp_address_syntax_notation_regind_with_displacement(
//         "^\\*\\[(r[0-7]|psw) \\+ (" + sh_symbol_or_literal + ")\\]$");

// syntax notation for operand in instruction for jumps
regex jmpInstructionRegisterDirAddrReg(R"reg(^\*)reg" + registerAddrModeDataRegStr + "$");
regex jmpInstructionRegisterIndAddrReg(R"reg(^\*)reg" + registerIndAddrModeDataRegStr + "$");
regex jmpInstructionRegisterIndWithDisplacementsAddrReg(
        R"reg(^\*)reg" + registerIndWithDisplacementAddrModeDataRegStr + "$");
regex jmpInstructionPCAddrReg("^%" + symbolOrLiteralRegStr + "$");
regex jmpInstructionAbsoluteAddrReg("^" + symbolOrLiteralRegStr + "$");
regex jmpInstructionMemdirAddrReg(R"reg(^\*)reg" + symbolOrLiteralRegStr + "$");

// syntax notation for operand in instruction for load/store
// regex rx_load_store_address_syntax_notation_absolute("^\\$(" + sh_symbol_or_literal + ")$");
// regex rx_load_store_address_syntax_notation_memdir("^(" + sh_symbol_or_literal + ")$");
// regex rx_load_store_address_syntax_notation_pc_relative("^%(" + sh_symbol + ")$");
// regex rx_load_store_address_syntax_notation_regdir("^(r[0-7]|psw)$");
// regex rx_load_store_address_syntax_notation_regind("^\\[(r[0-7]|psw)\\]$");
// regex rx_load_store_address_syntax_notation_regind_with_displacement(
//         "^\\[(r[0-7]|psw) \\+ (" + sh_symbol_or_literal + ")\\]$");

// syntax notation for operand in instruction for load/store
regex dataInstructionRegisterDirAddrReg("^" + registerAddrModeDataRegStr + "$");
regex dataInstructionRegisterIndAddrReg("^" + registerIndAddrModeDataRegStr + "$");
regex dataInstructionRegisterIndWithDisplacementsAddrReg("^" + registerIndWithDisplacementAddrModeDataRegStr + "$");
regex dataInstructionPCAddrReg("^%" + symbolOrLiteralRegStr + "$");
regex dataInstructionAbsoluteAddrReg(R"(^\$)" + symbolOrLiteralRegStr + "$");
regex dataInstructionMemdirAddrReg("^" + symbolOrLiteralRegStr + "$");

// syntax notation for operand in instruction for load or store

#endif //ASSEMBLER_REGEXES_H
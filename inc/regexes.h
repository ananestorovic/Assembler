
#ifndef ASSEMBLER_REGEXES_H
#define ASSEMBLER_REGEXES_H

#include <regex>
#include <string>

using namespace std;

//moj
//
//
regex any_symbol("(.*)");

// regexes for cleaning input file
regex rx_remove_comments("([^#]*)#.*");
regex rx_find_extra_spaces(" {2,}");
regex rx_find_tabs("\\t");
//regex rx_find_empty_strings(""); // this lines will be skipped manually
regex rx_remove_boundary_spaces("^( *)([^ ].*[^ ])( *)$");
regex rx_find_comma_spaces(" ?, ?");
regex rx_find_columns_spaces(" ?: ?");


// helper strings
string sh_literal_decimal = "-?[1-9][0-9]*";        // PREPRAVLJANEI this is a decimal number (positive or negative)
string sh_literal_hexadecimal = "0x[0-9A-F]{1,4}"; // PREPRAVLJANEI this is a hexadecimal number
string sh_symbol = "[a-zA-Z][a-zA-Z0-9_]*";    // symbol can start only with letter and can contain letters, digits and _
string sh_register_range = "[0-7]";            // there are 8 registers: r0 - r7

// combined helper strings
string sh_symbol_or_literal = sh_symbol + "|" + sh_literal_decimal + "|" + sh_literal_hexadecimal;

// regexes for assembly directives
regex rx_global_directive("^\\.global (" + sh_symbol + "(," + sh_symbol + ")*)$");
regex rx_extern_directive("^\\.extern (" + sh_symbol + "(," + sh_symbol + ")*)$");
regex rx_section_directive("^\\.section (" + sh_symbol + ")$");
regex rx_word_directive("^\\.word ((" + sh_symbol_or_literal + ")(,(" + sh_symbol_or_literal + "))*)$");
regex rx_skip_directive("^\\.skip (" + sh_literal_decimal + "|" + sh_literal_hexadecimal + ")$");
regex rx_equ_directive("^\\.equ (" + sh_symbol + "),\\s(" + sh_literal_decimal + "|" + sh_literal_hexadecimal + ")$"); //OVO NE VALJA!!!
regex rx_end_directive("^\\.end$");

// labels are in form -> 'symbol:'
// label has to be at the beginning of the line
// if there is not anything, label is associated with next command
regex rx_label_only("^(" + sh_symbol + "):$");             // nothing is after label
regex rx_label_with_command("^(" + sh_symbol + "):(.*)$"); // something is after label

// data - symbols and numbers - regexes
regex rx_symbol("^(" + sh_symbol + ")$");
regex rxIsLiteral(sh_literal_decimal + "|" + sh_literal_hexadecimal);
regex rx_literal_decimal("^(" + sh_literal_decimal + ")$");
regex rx_literal_hexadecimal("^(" + sh_literal_hexadecimal + ")$");

// instructions:
// instructions that operate only with registers
regex rx_no_operand_instruction("^(halt|iret|ret)$");
regex rx_one_operand_register_instruction("^(push|pop|int|not) (r[0-7]|psw)$");
regex rx_two_operand_register_instruction(
        "^(xchg|add|sub|mul|div|cmp|and|or|xor|test|shl|shr) (r[0-7]|psw),(r[0-7]|psw)$");

// instructions with operand (all kind)
// jumps
regex rx_one_operand_all_kind_addressing_jumps("^(call|jmp|jeq|jne|jgt) (.*)$");
// load/store
regex rx_two_operand_all_kind_addressing_load_store("^(ldr|str) (r[0-7]|psw),(.*)$");

// syntax notation for operand in instruction for jumps
regex rx_jmp_address_syntax_notation_absolute("^(" + sh_symbol_or_literal + ")$");
regex rx_jmp_address_syntax_notation_memdir("^\\*(" + sh_symbol_or_literal + ")$");
regex rx_jmp_address_syntax_notation_symbol_pc_relative("^%(" + sh_symbol + ")$");
regex rx_jmp_address_syntax_notation_regdir("^\\*(r[0-7]|psw)$");
regex rx_jmp_address_syntax_notation_regind("^\\*\\[(r[0-7]|psw)\\]$");
regex rx_jmp_address_syntax_notation_regind_with_displacement(
        "^\\*\\[(r[0-7]|psw) \\+ (" + sh_symbol_or_literal + ")\\]$");

// syntax notation for operand in instruction for load/store
regex rx_load_store_address_syntax_notation_absolute("^\\$(" + sh_symbol_or_literal + ")$");
regex rx_load_store_address_syntax_notation_memdir("^(" + sh_symbol_or_literal + ")$");
regex rx_load_store_address_syntax_notation_pc_relative("^%(" + sh_symbol + ")$");
regex rx_load_store_address_syntax_notation_regdir("^(r[0-7]|psw)$");
regex rx_load_store_address_syntax_notation_regind("^\\[(r[0-7]|psw)\\]$");
regex rx_load_store_address_syntax_notation_regind_with_displacement(
        "^\\[(r[0-7]|psw) \\+ (" + sh_symbol_or_literal + ")\\]$");

// syntax notation for operand in instruction for load or store




#endif //ASSEMBLER_REGEXES_H

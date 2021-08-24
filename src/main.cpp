#include <iostream>
#include "assembler.h"

int main(int argc, char* argv[]) {




    Assembler::getInstance().processInputFileFirstPass("../tests/proba.s");


    std::cout << "Hello, World!" << std::endl;
    return 0;
}

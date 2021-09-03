#include <iostream>
#include <string.h>
#include "assembler.h"
using namespace std;

int main(int argc, char *argv[]) {

    if (argc != 4) {
        cout << "Arguments not ok" << endl;
        return -1;
    }

    if (strcmp(argv[1], "-o") != 0) {
      cout << "Arguments not ok" << endl;
        return -1;
    }

    Assembler::getInstance().processInputFile(argv[3]);


    cout << "Done!" << endl;
    return 0;
}

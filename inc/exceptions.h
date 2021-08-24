//
// Created by ss on 8/20/21.
//

#ifndef ASSEMBLER_EXCEPTIONS_H
#define ASSEMBLER_EXCEPTIONS_H
#include <exception>
#include <string>

class FileError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Problem sa fajlom\n";
    };
};

class SectionError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ova sekcija vec postoji!\n";
    };
};

class ExternError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ovaj extn simbol vec postoji!\n";
    };
};

class EquExternError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ovo je extern simbol i ne sme sa equ!\n";
    };
};

class EquDefError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ovo je vec definisan simbol i ne sme sa equ!\n";
    };
};

class LabelExternError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ovo je extern simbol i ne sme sa labelom!\n";
    };
};

class LabelDefError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ovo je vec definisan simbol i ne sme sa labelom!\n";
    };
};

class LabelSectionError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Mora da postoji sekcija uokviru koje ce ova labela biti definisana!\n";
    };
};

class WordSectionError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Mora da postoji sekcija uokviru koje ce word biti definisan!\n";
    };
};

class SkipSectionError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Mora da postoji sekcija uokviru koje ce skip biti definisan!\n";
    };
};

class GlobalFirstError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Global mora biti prvi!\n";
    };

};

class ExternExistError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ovaj extern simbol vec postoji!\n";
    };

};

class LiteralError : public std::exception {
public:
    virtual const char* what() const noexcept {
        return "Ocekivan literal, ali nije literal!\n";
    };

};

#endif //ASSEMBLER_EXCEPTIONS_H

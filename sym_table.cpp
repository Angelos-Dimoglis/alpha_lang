#include "sym_table.h"

Symbol* emptySymbol = new Symbol;

node* SymTable::scopeNode(unsigned int scope) {
    if (scopeHeads.find(scope) != scopeHeads.end()) {
        return scopeHeads[scope];
    }
    return nullptr;
}

int SymTable::hashFunction(const std::string& key) {
    int hash = 0;
    for (char ch : key) {
        hash += ch;
    }
    return hash % tableSize;
}

Symbol* createSymbol(enum SymbolType type) {
    if (type < 3) {
        return new Variable();
    }
    else {
        return new Function();
    }
}

SymTable::SymTable() {
    Initialize();
}

void SymTable::Initialize() {
    for (int i = 0; i < 12; i++) {
        Insert(library_functions[i], LIBFUNC, 0, 0, std::list<Variable*>());
    }
}

bool SymTable::libfunc_check(const std::string& name) {
    for (int i = 0; i < 12; i++) {
        if (name == library_functions[i]) {
            return true;
        }
    }
    return false;
}

void SymTable::Insert(const std::string& name, enum SymbolType type, unsigned int line,
                        unsigned int scope, std::list<Variable*> arguments) {
    if (libfunc_check(name) && (type != LIBFUNC)) {
        throw std::runtime_error("Name can't be a library function.");
    }
    int index = hashFunction(name);
    Symbol* newSymbol = createSymbol(type);
    node* currentCollision = table[index];
    node* currentScope = scopeNode(scope);
    if (name.empty()) {
        newSymbol->name = "_f1";
    }
    else {
        newSymbol->name = name;
    }
    newSymbol->scope = scope;
    newSymbol->line = line;
    newSymbol->type = type;
    newSymbol->isActive = true;
    if (type == USERFUNC) {
        ((Function*)newSymbol)->arguments = arguments;
    }
    node* n = new node(*newSymbol);
    if (currentCollision != nullptr) {
        while (currentCollision->nextCollision) {
            currentCollision = currentCollision->nextCollision;
        }
        currentCollision->nextCollision = n;
    }
    else {
        table[index] = n;
    }
    if (currentScope != nullptr) {
        while (currentScope->nextScope != nullptr) {
            currentScope = currentScope->nextScope;
        }
        currentScope->nextScope = n;
    }
    else {
        scopeHeads[scope] = n;
    }
}

Symbol* SymTable::Lookup(const std::string& name, int scope, bool mode) {
    node* current;
    if (!mode) {
        current = scopeNode(scope);
        while (current != nullptr) {
            if (current->sym.name == name && current->sym.isActive) {
                return &(current->sym);
            }
            current = current->nextScope;
        }
    }
    else {
        for (int i = scope; i >= 0; i--) {
            current = scopeNode(i);
            while (current != nullptr) {
                if (current->sym.name == name && current->sym.isActive) {
                    return &(current->sym);
                }
                current = current->nextScope;
            }
        }
    }
    return emptySymbol;
}

void SymTable::Hide(unsigned int scope) {
    node* current = scopeNode(scope);
    while (current != nullptr) {
        current->sym.isActive = false;
        current = current->nextScope;
    }
}

void SymTable::PrintTable() {
    for (unsigned int i = 0; i < scopeHeads.size(); i++) {
        node* current = scopeHeads[i];
        std::cout << "-----------    Scope #" << i << "    -----------" << std::endl;
        while (current != nullptr) {
            std::cout << "\"" << current->sym.name << "\" ";
            switch (current->sym.type)
            {
            case GLOBAL:
                std::cout << "[global variable] ";
                break;
            case _LOCAL:
                std::cout << "[local variable] ";
                break;
            case FORMAL:
                std::cout << "[formal argument] ";
                break;
            case USERFUNC:
                std::cout << "[user function] ";
                break;
            case LIBFUNC:
                std::cout << "[library function] ";
                break;
            default:
                std::cout << "[unknown] ";
                break;
            }
            std::cout << "(line " << current->sym.line << ") ";
            std::cout << "(scope " << current->sym.scope << ")" << std::endl;
            current = current->nextScope;
        }
        std::cout << std::endl;
    }
}

void SymTable::freeTable() {
    for (int i = 0; i < tableSize; i++) {
        node* current = table[i];
        node* temp = nullptr;
        while (current != nullptr) {
            temp = current;
            current = current->nextCollision;
            delete temp;
        }
    }
    delete emptySymbol;
}
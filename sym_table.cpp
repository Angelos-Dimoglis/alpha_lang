#include "sym_table.h"

Symbol* emptySymbol = new Symbol();

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
    Insert("print", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("input", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("objectmemberkeys", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("objecttotalmembers", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("objectcopy", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("totalarguments", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("argument", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("typeof", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("strtonum", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("sqrt", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("cos", LIBFUNC, 0, 0, std::list<Variable>());
    Insert("sin", LIBFUNC, 0, 0, std::list<Variable>());
}

void SymTable::Insert(const std::string& name, enum SymbolType type, unsigned int line,
                        unsigned int scope, std::list<Variable> arguments) {
    Symbol* temp = Lookup(name);
    if (temp->name.empty() || !(temp->isActive)) {
        int index = hashFunction(name);
        Symbol* newSymbol = createSymbol(type);
        node* currentCollision = table[index];
        node* currentScope = scopeNode(scope);
        newSymbol->name = name;
        newSymbol->scope = scope;
        newSymbol->line = line;
        newSymbol->type = type;
        newSymbol->isActive = true;
        if (type > 2) {
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
}

Symbol* SymTable::Lookup(const std::string& name) {
    int index = hashFunction(name);
    node* current = table[index];
    while (current != nullptr) {
        if (current->sym.name == name) {
            return &(current->sym);
        }
        current = current->nextCollision;
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
            case 0:
                std::cout << "[global variable] ";
                break;
            case 1:
                std::cout << "[local variable] ";
                break;
            case 2:
                std::cout << "[formal argument] ";
                break;
            case 3:
                std::cout << "[user function] ";
                break;
            case 4:
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
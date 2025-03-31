#include "sym_table.h"
#include <iostream>
#include <memory>

node* SymTable::scopeNode(unsigned int scope) {
    node* current;
    for (int i = 0; i < tableSize; i++) {
        current = table[i];
        while (current != nullptr) {
            if (current->sym.scope = scope) {
                return current;
            }
            current = current->nextCollision;
        }
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
    if (type < 4) {
        return new Variable();
    }
    else {
        return new Function();
    }
}

void SymTable::Insert(const std::string& name, unsigned int scope,
                        enum SymbolType type, std::list<Variable> arguments) {
    if (Lookup(name).name.empty()) {
        int index = hashFunction(name);
        Symbol* newSymbol = createSymbol(type);
        node* currentCollision = table[index];
        if (currentCollision != nullptr) {
            while (currentCollision->nextCollision) {
                currentCollision = currentCollision->nextCollision;
            }
        }
        node* currentScope = scopeNode(scope);
        if (currentScope != nullptr) {
            while (currentScope->nextScope != nullptr) {
                currentScope = currentScope->nextScope;
            }
        }
        newSymbol->name = name;
        newSymbol->scope = scope;
        newSymbol->type = type;
        newSymbol->isActive = true;
        if (type > 3) {
            ((Function*)newSymbol)->arguments = arguments;
        }
        node* n = new node(*newSymbol);
        if (currentCollision != nullptr) {
            currentCollision->nextCollision = n;
        }
        else {
            table[index] = n;
        }
        if (currentScope != nullptr) {
            currentScope->nextScope = n;
        }
    }
}

Symbol SymTable::Lookup(const std::string& name) {
    int index = hashFunction(name);
    node* current = table[index];
    while (current != nullptr) {
        if (current->sym.name == name) {
            return current->sym;
        }
        current = current->nextCollision;
    }
    Symbol* temp = new Symbol();
    return *temp;
}

void SymTable::Hide(const std::string& name) {
    Symbol hSym = Lookup(name);
    hSym.isActive = false;
}
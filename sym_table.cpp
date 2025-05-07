#include "sym_table.h"

Symbol* emptySymbol = new Symbol;

node* SymTable::scopeNode(unsigned int scope) {
    if (scopeHeads.find(scope) != scopeHeads.end()) {
        return scopeHeads[scope];
    }
    return nullptr;
}

node* SymTable::collisionNode(const string& key) {
    if (table.find(key) != table.end()) {
        return table[key];
    }
    return nullptr;
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
    for (int i = 0; i < LIB_MAX; i++) {
        Insert(library_functions[i], libfunc, 0, 0, list<Variable*>());
    }
}

bool SymTable::libfunc_check(const string& name) {
    for (int i = 0; i < LIB_MAX; i++) {
        if (name == library_functions[i]) {
            return true;
        }
    }
    return false;
}

void SymTable::Insert(const string& name, enum SymbolType type, unsigned int line,
                        unsigned int scope, list<Variable*> arguments) {
    if (libfunc_check(name) && (type != libfunc)) {
        throw runtime_error("Name \"" + name + "\" clashes with a library function.");
    }
    Symbol* newSymbol = createSymbol(type);
    node* currentCollision = collisionNode(name);
    node* currentScope = scopeNode(scope);
    newSymbol->name = name;
    newSymbol->scope = scope;
    newSymbol->line = line;
    newSymbol->type = type;
    newSymbol->isActive = true;
    if (type == userfunc) {
        ((Function*)newSymbol)->arguments = arguments;
    }
    node* n = new node(*newSymbol);
    if (currentCollision != nullptr) {
        while (currentCollision->nextCollision != nullptr) {
            currentCollision = currentCollision->nextCollision;
        }
        currentCollision->nextCollision = n;
    }
    else {
        table[name] = n;
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

Symbol* SymTable::Lookup(const string& name, int scope, bool mode) {
    node* current;
    if (mode == THIS_SCOPE) {
        current = scopeNode(scope);
        while (current != nullptr) {
            if (current->sym.name == name && current->sym.isActive) {
                return &(current->sym);
            }
            current = current->nextScope;
        }
    }
    else if (mode == ALL_SCOPES) {
        for (int i = scope; i >= 0; i--) {
            for (current = scopeNode(i); current != nullptr; current = current->nextScope) {
                if (current->sym.name == name && current->sym.isActive) {
                    return &(current->sym);
                }
            }
        }
    }
    return nullptr;
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
        cout << "-----------    Scope #" << i << "    -----------" << endl;
        while (current != nullptr) {
            cout << "\"" << current->sym.name << "\" ";
            switch (current->sym.type)
            {
            case global:
                cout << "[global variable] ";
                break;
            case local:
                cout << "[local variable] ";
                break;
            case formal:
                cout << "[formal argument] ";
                break;
            case userfunc:
                cout << "[user function] ";
                break;
            case libfunc:
                cout << "[library function] ";
                break;
            default:
                cout << "[unknown] ";
                break;
            }
            cout << "(line " << current->sym.line << ") ";
            cout << "(scope " << current->sym.scope << ")" << endl;
            current = current->nextScope;
        }
        cout << endl;
    }
}

void SymTable::freeTable() {
    for (auto& pair : table) {
        node* current = pair.second->nextCollision;
        while (current != nullptr) {
            node* temp = current;
            current = current->nextCollision;
            delete temp;
        }
        delete pair.second;
    }
    table.clear();
    delete emptySymbol;
}

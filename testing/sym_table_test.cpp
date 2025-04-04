#include "sym_table.h"

int main() {
    SymTable st;
    Symbol test;
    std::list<Variable*> testList;
    if (st.Lookup("x", 0, true)->name.empty()) {
        st.Insert("x", GLOBAL, 1, 0, std::list<Variable*>());
    }
    if (st.Lookup("foo", 0, false)->name.empty()) {
        st.Insert("foo", USERFUNC, 2, 0, std::list<Variable*>());
    }
    if (st.Lookup("foo", 1, false)->name.empty()) {
        st.Insert("foo", USERFUNC, 3, 1, std::list<Variable*>());
    }
    if (st.Lookup("bar", 2, false)->name.empty()) {
        st.Insert("bar", FORMAL, 3, 2, std::list<Variable*>());
    }
    if (st.Lookup("foo", 2, false)->name.empty()) {
        st.Insert("foo", LOCAL, 4, 2, std::list<Variable*>());
    }
    if (st.Lookup("foo", 0, false)->name.empty()) {
        st.Insert("foo", USERFUNC, 2, 0, std::list<Variable*>());
    }
    st.Lookup("bar", 0, true);
    st.Lookup("foo", 0, true);
    st.Hide(2);
    if (st.Lookup("foo", 2, false)->name.empty()) {
        st.Insert("foo", LOCAL, 8, 2, std::list<Variable*>());
    }
    st.PrintTable();
    st.freeTable();
    return 0;
}
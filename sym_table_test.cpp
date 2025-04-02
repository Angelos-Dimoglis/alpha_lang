#include "sym_table.h"

int main() {
    SymTable st;
    Symbol test;
    std::list<Variable*> testList;
    try {
        st.Insert("x", GLOBAL, 1, 0, std::list<Variable*>());
        st.Insert("foo", USERFUNC, 2, 0, std::list<Variable*>());
        st.Insert("bar", FORMAL, 3, 2, std::list<Variable*>());
        testList.push_back((Variable*)st.Lookup("bar", 2));
        st.Insert("foo", USERFUNC, 3, 1, testList);
        st.Insert("foo", LOCAL, 4, 2, std::list<Variable*>());
        st.Insert("foo", LOCAL, 8, 2, std::list<Variable*>());
    } catch(const std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }
    st.PrintTable();
    st.freeTable();
    return 0;
}
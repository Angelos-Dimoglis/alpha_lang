#include "sym_table.h"

int main() {
    SymTable st;
    Symbol test;
    std::list<Variable> testList;
    Variable* v = new Variable;
    st.Insert("x", GLOBAL, 0, 0, testList);
    testList.push_back(*v);
    st.Insert("foo", USERFUNC, 1, 0, testList);
    st.Insert("foo2", USERFUNC, 3, 2, testList);
    st.Insert("print", LIBFUNC, 3, 1, testList);
    std::cout << st.Lookup("print").isActive << std::endl;
    st.Hide("print");
    std::cout << st.Lookup("print").isActive << std::endl;
    st.PrintTable();
    return 0;
}
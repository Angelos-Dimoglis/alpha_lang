#pragma once
#include "sym_table.h"

#ifndef ALL_SCOPES

#define ALL_SCOPES true
#define THIS_SCOPE false

#endif

using namespace std;

Symbol* add_local_id(const string name);

Symbol* lookup_global_id(const string name);

Symbol* add_id(const string name);

Symbol* add_func(const string name);

Symbol* add_formal_argument(const string name);

Symbol* check_lvalue(const string name);
#pragma once
#include "sym_table.h"

#ifndef ALL_SCOPES

#define ALL_SCOPES true
#define THIS_SCOPE false

#endif

using namespace std;

Variable* add_local_id(const string name);

Symbol* lookup_global_id(const string name);

Variable* add_id(const string name);

Function* add_func(const string name);

Variable* add_formal_argument(const string name);

Variable* check_lvalue(const string name);
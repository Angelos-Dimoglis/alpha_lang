#pragma once
#include "sym_table.h"

using namespace std;

void add_local_id(const string name);

void lookup_global_id(const string name);

void add_id(const string name);

void add_func(const string name);

void add_formal_argument(const string name);

void check_lvalue(const string name);
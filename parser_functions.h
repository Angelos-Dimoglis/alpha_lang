#pragma once
#include "sym_table.h"

using namespace std;

void add_local_id(const string name);

void lookup_global_id(const string name);

void add_id(const string name);

void add_func(const string name, list<Variable*> args);
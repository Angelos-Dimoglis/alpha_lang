#pragma once

#include <string>

using namespace std;

enum vmopcode {
    assign_v = 0,
    add_v,
    sub_v,
    mul_v,
    div_v,
    mod_v,
    uminus_v,
    if_eq_v,
    if_not_eq_v,
    if_less_eq_v,
    if_greater_eq_v,
    if_less_v,
    if_greater_v,
    jump_v,
    call_v,
    param_v,
    return_v,
    getretval_v,
    funcstart_v,
    funcend_v,
    newtable_v,
    tablegetelem_v,
    tablesetelem_v,
    nop_v
};

enum vmarg_t {
    label_a    = 0,
    global_a   = 1,
    formal_a   = 2,
    local_a    = 3,
    number_a   = 4,
    string_a   = 5,
    bool_a     = 6,
    nil_a      = 7,
    userfunc_a = 8,
    libfunc_a  = 9,
    retval_a   = 10
};

struct vmarg {
    vmarg_t type;
    unsigned val;
};

struct instruction {
    vmopcode opcode;
    vmarg result;
    vmarg arg1;
    vmarg arg2;
    unsigned srcLine;

    instruction() {
        opcode = nop_v;
        arg1.type = nil_a;
        arg2.type = nil_a;
        result.type = nil_a;
        srcLine = -1;
    }
};

struct userfunc {
    unsigned address;
    unsigned localSize;
    string id;
};

#pragma once

#include <string>

using namespace std;

enum vmopcode {
    assign_v       = 0,
    add_v          = 1,
    sub_v          = 2,
    mul_v          = 3,
    div_v          = 4,
    mod_v          = 5,
    uminus_v       = 6,
    and_v          = 7,
    or_v           = 8,
    not_v          = 9,
    jump_v         = 10,
    jeq_v          = 11,
    jne_v          = 12,
    jle_v          = 13,
    jge_v          = 14,
    jlt_v          = 15,
    jgt_v          = 16,
    call_v         = 17,
    pusharg_v      = 18,
    funcenter_v    = 19,
    funcexit_v     = 20,
    newtable_v     = 21,
    tablegetelem_v = 22,
    tablesetelem_v = 23,
    nop_v          = 24
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
};

struct userfunc {
    unsigned address;
    unsigned localSize;
    string id;
};

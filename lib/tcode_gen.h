enum vmopcode {
    assign_v, add_v, sub_v,
    mul_v, div_v, mod_v,
    uminus_v, and_v, or_v,
    not_v, jeq_v, jne_v,
    jle_v, jge_v, jlt_v,
    jgt_v, call_v, pusharg_v,
    funcenter_v, funcexit_v, newtable_v,
    tablegetelem_v, tablesetelem_v, nop_v
};

enum vmarg_t {
    label_a, global_a, formal_a, local_a,
    number_a, string_a, bool_a, nil_a,
    userfunc_a, libfunc_a, retval_a
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

struct incomplete_jump {
    unsigned instrNo;
    unsigned iadress;
    incomplete_jump *next;
};

incomplete_jump *ij_head = 0;
unsigned ij_total = 0;

void add_incomplete_jump (unsigned instrNo, unsigned iaddress);
unsigned consts_newstring (string s);
unsigned consts_newnumber (double n);
unsigned libfuncs_newused (string s);

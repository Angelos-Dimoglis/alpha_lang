#include "../lib/icode_gen.h"

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

extern void generate_ADD (quad *);
extern void generate_SUB (quad *);
extern void generate_MUL (quad *);
extern void generate_DIV (quad *);
extern void generate_MOD (quad *);
extern void generate_NEWTABLE (quad *);
extern void generate_TABLEGETELEM (quad *);
extern void generate_TABLESETELEM (quad *);
extern void generate_ASSIGN (quad *);
extern void generate_NOP (quad *);
extern void generate_JUMP (quad *);
extern void generate_IFEQ (quad *);
extern void generate_IFNOTEQ (quad *);
extern void generate_IFGREATER (quad *);
extern void generate_IFGREATEREQ (quad *);
extern void generate_IFLESS (quad *);
extern void generate_IFLESSEQ (quad *);
extern void generate_NOT (quad *);
extern void generate_OR (quad *);
extern void generate_PARAM (quad *);
extern void generate_CALL (quad *);
extern void generate_GETRETVAL (quad *);
extern void generate_FUNCSTART (quad *);
extern void generate_RETURN (quad *);
extern void generate_FUNCEND (quad *);

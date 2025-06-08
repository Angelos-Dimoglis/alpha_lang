#include "../lib/icode_gen.h"
#include <unordered_map>
#include <variant>
#include <vector>

vector<double> numConsts;
unsigned totalNumConsts;
vector<string> stringConsts;
unsigned totalStringConts;
vector<string> namedLibfuncs;
unsigned totalNamedLibfuncs;
vector<struct userfunc> userFuncs;
unsigned totalUserFuncs;

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
    jeq_v          = 10,
    jne_v          = 11,
    jle_v          = 12,
    jge_v          = 13,
    jlt_v          = 14,
    jgt_v          = 15,
    call_v         = 16,
    pusharg_v      = 17,
    funcenter_v    = 18,
    funcexit_v     = 19,
    newtable_v     = 20,
    tablegetelem_v = 21,
    tablesetelem_v = 22,
    nop_v          = 23
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

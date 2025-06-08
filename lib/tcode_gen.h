#include "../lib/icode_gen.h"
#include <unordered_map>
#include <variant>

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

enum avm_memcell_t {
    number_m    = 0,
    string_m    = 1,
    bool_m      = 2,
    table_m     = 3,
    userfunc_m  = 4,
    libfunc_m   = 5,
    nil_m       = 6,
    undef_m     = 7 
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

struct avm_memcell {
    avm_memcell_t type;
    variant<double, string, bool, avm_table*, unsigned> data;

    bool operator==(const avm_memcell& other) const {
        return type == other.type && data == other.data;
    }
};

struct avm_memcell_hash {
    size_t operator()(const avm_memcell& m) const {
        size_t hashValue = hash<int>()(static_cast<int>(m.type));

        if (holds_alternative<double>(m.data)) {
            hashValue ^= hash<double>()(get<double>(m.data));
        }
        else if (holds_alternative<string>(m.data)) {
            hashValue ^= hash<string>()(get<string>(m.data));
        }
        else if (holds_alternative<bool>(m.data)) {
            hashValue ^= hash<bool>()(get<bool>(m.data));
        }
        else if (holds_alternative<avm_table*>(m.data)) {
            hashValue ^= hash<avm_table*>()(get<avm_table*>(m.data));
        }
        else if (holds_alternative<unsigned>(m.data)) {
            hashValue ^= hash<unsigned>()(get<unsigned>(m.data));
        }
        return hashValue;
    }
};

struct avm_table {
    unsigned refCounter;
    unordered_map<avm_memcell, avm_memcell, avm_memcell_hash> indexed;
    unsigned total;

    avm_table() : refCounter(0), total(0) {}
};

avm_memcell* avm_tablegetelem(avm_memcell* key);
void avm_tablesetelem(avm_memcell* key, avm_memcell* value);

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

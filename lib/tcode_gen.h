#include <unordered_map>
#include <variant>
#include <vector>

#include "../lib/icode_gen.h"
#include "../lib/avm_instr_set.h"

vector<double> numConsts;
unsigned totalNumConsts;
vector<string> stringConsts;
unsigned totalStringConts;
vector<string> namedLibfuncs;
unsigned totalNamedLibfuncs;
vector<struct userfunc> userFuncs;
unsigned totalUserFuncs;

using namespace std;

struct incomplete_jump {
    unsigned instrNo;
    unsigned iadress;
    incomplete_jump *next;
};

// arrays of constant values and functions
unsigned consts_newstring (string s);
unsigned consts_newnumber (double n);
unsigned libfuncs_newused (string s);
// TODO: missing userfuncs (see lec 14 slide 10)

incomplete_jump *ij_head = 0;
unsigned ij_total = 0;

void add_incomplete_jump (unsigned instrNo, unsigned iaddress);

void make_number_operand (vmarg *arg, double val);
void make_bool_operand (vmarg *arg, unsigned val);
void make_retval_operand (vmarg *arg);

void generate_ADD (quad *);
void generate_SUB (quad *);
void generate_MUL (quad *);
void generate_DIV (quad *);
void generate_MOD (quad *);
void generate_NEWTABLE (quad *);
void generate_TABLEGETELEM (quad *);
void generate_TABLESETELEM (quad *);
void generate_ASSIGN (quad *);
void generate_NOP (quad *);
void generate_JUMP (quad *);
void generate_IFEQ (quad *);
void generate_IFNOTEQ (quad *);
void generate_IFGREATER (quad *);
void generate_IFGREATEREQ (quad *);
void generate_IFLESS (quad *);
void generate_IFLESSEQ (quad *);
void generate_NOT (quad *);
void generate_OR (quad *);
void generate_AND (quad *);
void generate_PARAM (quad *);
void generate_CALL (quad *);
void generate_GETRETVAL (quad *);
void generate_FUNCSTART (quad *);
void generate_RETURN (quad *);
void generate_FUNCEND (quad *);

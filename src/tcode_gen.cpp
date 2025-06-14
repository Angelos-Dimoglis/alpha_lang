#include <cassert>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <variant>
#include <vector>

#include "../lib/icode_gen.h"
#include "../lib/avm_instr_set.h"
#include "../lib/tcode_gen.h"

extern quad *quads;
extern unsigned total;
extern unsigned int curr_quad;

using namespace std;

struct incomplete_jump {
    unsigned tcode_address;
    unsigned icode_address;
    incomplete_jump *next;
};

void make_operand (expr *e, vmarg *arg);
void make_retval_operand (vmarg *arg);

void emit_instr(instruction instr);
unsigned int next_instr_label();

void add_incomplete_jump (unsigned instrNo, unsigned iaddress);

void generate_ASSIGN (quad *);
void generate_ADD (quad *);
void generate_SUB (quad *);
void generate_MUL (quad *);
void generate_DIV (quad *);
void generate_MOD (quad *);
void generate_UMINUS(quad *);
void generate_IF_EQ (quad *);
void generate_IF_NOT_EQ (quad *);
void generate_IF_LESS_EQ (quad *);
void generate_IF_GREATER_EQ (quad *);
void generate_IF_LESS (quad *);
void generate_IF_GREATER (quad *);
void generate_JUMP (quad *);
void generate_CALL (quad *);
void generate_PARAM (quad *);
void generate_RETURN (quad *);
void generate_GETRETVAL (quad *);
void generate_FUNCSTART (quad *);
void generate_FUNCEND (quad *);
void generate_NEWTABLE (quad *);
void generate_TABLEGETELEM (quad *);
void generate_TABLESETELEM (quad *);
void generate_NOP (quad *);

vector <instruction> tcode_instructions;
vector <incomplete_jump> incomplete_jumps;

vector<double> all_num_consts;
vector <string> all_str_consts;
vector <string> all_lib_funcs = {
    "print",
    "input",
    "objectmemberkeys",
    "objecttotalmembers",
    "objectcopy",
    "totalarguments",
    "argument",
    "typeof",
    "strtonum",
    "sqrt",
    "cos",
    "sin"
};

vector <struct user_func> user_funcs;

unsigned new_string (string s) {
    for (int i = 0; i < all_str_consts.size(); i++) {
        if (all_str_consts[i] == s) {
            return i;
        }
    }
    all_str_consts.push_back(s);
    return all_str_consts.size() - 1;
}

unsigned new_number (double n) {
    for (int i = 0; i < all_num_consts.size(); i++) {
        if (all_num_consts[i] == n) {
            return i;
        }
    }
    all_num_consts.push_back(n);
    return all_num_consts.size() - 1;
}

unsigned new_lib_func (string s) {
    for (int i = 0; i < all_lib_funcs.size(); i++) {
        if (all_lib_funcs[i] == s) {
            return i;
        }
    }
    assert(false);
}

// translate expr to vmarg
void make_operand (expr *e, vmarg *arg) {

    if (e == nullptr) {
        arg->type = nil_a;
        arg->val = -1;
        return;
    }

    // use a variable for storage
    switch (e->type) {
        case var_e:
        case table_item_e:
        case arith_expr_e:
        case bool_expr_e:
        case assign_expr_e:
        case new_table_e: {
            Variable *var = (Variable *) e->sym;

            arg->val = var->offset;

            switch (var->space) {
                case program_var: arg->type = global_a; break;
                case function_local: arg->type = local_a; break;
                case formal_arg: arg->type = formal_a; break;
                default: assert(0);
            }
            break;
        }

        // constants
        case const_bool_e: {
            arg->val = e->bool_const;
            arg->type = bool_a;
            break;
        }

        case const_string_e: {
            arg->val = new_string(e->str_const);
            arg->type = string_a;
            break;
        }

        case const_num_e: {
            arg->val = new_number(e->num_const);
            arg->type = number_a;
            break;
        }

        case const_nil_e: {
            arg->type = nil_a;
            break;
        }

        // functions
        case program_func_e: {
            arg->type = userfunc_a;
            arg->val = ((Function*)e->sym)->taddress;
            break;
        }

        case library_func_e: {
            arg->type = libfunc_a;
            arg->val = new_lib_func(e->sym->name);
            break;
        }
    
        default: assert(0);
    }
}

void make_retval_operand (vmarg *arg) {
    arg->type = retval_a;
    arg->val = 0;
}

// TODO: patch incomplete jumps (pseudo-code at lec 14 slide 15)

// ### generating target code ###

typedef void (*generator_func_t) (quad*);

generator_func_t generators [] = {
    generate_ASSIGN,
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_UMINUS,
    generate_IF_EQ,
    generate_IF_NOT_EQ,
    generate_IF_LESS_EQ,
    generate_IF_GREATER_EQ,
    generate_IF_LESS,
    generate_IF_GREATER,
    generate_JUMP,
    generate_CALL,
    generate_PARAM,
    generate_RETURN,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_FUNCEND,
    generate_NEWTABLE,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_NOP
};

void emit_instr(instruction instr) {
    instruction tcode_instruction;
    tcode_instruction.opcode = instr.opcode;
    tcode_instruction.arg1 = instr.arg1;
    tcode_instruction.arg2 = instr.arg2;
    tcode_instruction.result = instr.result;
    tcode_instruction.srcLine = instr.srcLine;
    tcode_instructions.push_back(tcode_instruction);
}

unsigned int next_instr_label() {
    return tcode_instructions.size();
}

static void generate(vmopcode op, quad *q) {
    instruction tcode_instruction;
    tcode_instruction.opcode = op;
    make_operand(q->arg1, &tcode_instruction.arg1);
    make_operand(q->arg2, &tcode_instruction.arg2);
    make_operand(q->result, &tcode_instruction.result);
    tcode_instruction.srcLine = q->line;
    q->taddress = next_instr_label();
    emit_instr(tcode_instruction);
}

static void generate_relational(vmopcode op, quad *q) {
    instruction tcode_instruction;
    tcode_instruction.opcode = op;

    make_operand(q->arg1, &tcode_instruction.arg1);
    make_operand(q->arg2, &tcode_instruction.arg2);

    tcode_instruction.result.type = label_a;
    if (q->label < curr_quad) {
        tcode_instruction.result.val = quads[q->label - 1].taddress; //+-1 due to indexing from 0
    } else {
        add_incomplete_jump(next_instr_label(), q->label);
    }
    tcode_instruction.srcLine = q->line;
    q->taddress = next_instr_label();
    emit_instr(tcode_instruction);
}

void add_incomplete_jump(unsigned int tcode_address, unsigned int icode_address) {
    incomplete_jump incomplete_jump;
    incomplete_jump.tcode_address = tcode_address;
    incomplete_jump.icode_address = icode_address;
    incomplete_jumps.push_back(incomplete_jump);
}

// arithmetic expressions
void generate_ADD(quad *q) {
    generate(add_v, q);
}

void generate_SUB(quad *q) {
    generate(sub_v, q);
}

void generate_MUL(quad *q) {
    generate(mul_v, q);
}

void generate_DIV(quad *q) {
    generate(div_v, q);
}

void generate_MOD(quad *q) {
    generate(mod_v, q);
}

void generate_UMINUS(quad *q) {
    instruction t;
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    t.opcode = mul_v;
    make_operand(q->arg1, &t.arg1);
    make_operand(q->arg1, &t.arg2);
    t.arg2.val = new_number(-1);
    t.arg2.type = number_a;
    make_operand(q->result, &t.result);
    emit_instr(t);
}

// table-related
void generate_NEWTABLE(quad *q) {
    generate(newtable_v, q);
}

void generate_TABLEGETELEM(quad *q) {
    generate(tablegetelem_v, q);
}

void generate_TABLESETELEM(quad *q) {
    generate(tablesetelem_v, q);
}

// other expressions

void generate_ASSIGN(quad *q) {
    generate(assign_v, q);
}

void generate_NOP(quad *q) {
    instruction tcode_instruction;
    tcode_instruction.opcode = nop_v;
    tcode_instruction.srcLine = q->line;
    emit_instr(tcode_instruction);
}

// relational expressions
void generate_JUMP(quad *q) {
    generate_relational(jump_v, q);
}

void generate_IF_EQ(quad *q) {
    generate_relational(if_eq_v, q);
}

void generate_IF_NOT_EQ(quad *q) {
    generate_relational(if_not_eq_v, q);
}

void generate_IF_GREATER(quad *q) {
    generate_relational(if_greater_v, q);
}

void generate_IF_GREATER_EQ(quad *q) {
    generate_relational(if_greater_eq_v, q);
}

void generate_IF_LESS(quad *q) {
    generate_relational(if_less_v, q);
}

void generate_IF_LESS_EQ(quad *q) {
    generate_relational(if_less_eq_v, q);
}

static void reset_operand(vmarg *arg) {
    arg->val = -1;
}

void generate_PARAM(quad *q) {
    instruction t;
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    t.opcode = param_v;
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_CALL(quad *q) {
    instruction t;
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    t.opcode = call_v;
    t.srcLine = q->line;
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_GETRETVAL(quad *q) {
    instruction t;
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    t.opcode = assign_v;
    t.srcLine = q->line;
    make_operand(q->result, &t.result);
    make_retval_operand(&t.arg1);
    emit_instr(t);
}

void generate_FUNCSTART(quad *q) {
    instruction t;
    Function *f = (Function *) q->result->sym;
    f->taddress = next_instr_label();
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    user_funcs.push_back(user_func(f->taddress, f->num_of_locals, f->name));
    t.opcode = funcstart_v;
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_RETURN(quad *q) {
    instruction t;
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    t.opcode = assign_v;
    make_operand(q->arg1, &t.arg1);
    make_retval_operand(&t.result);
    emit_instr(t);

}


void generate_FUNCEND(quad *q) {
    /* Function *f = func_stack.back();
    returnList *reader = f->returnList;
    while (reader) {
        patchinstr(reader->instrLabel, next_instr_label());
        reader = reader->next;
    } */
    instruction t;
    t.srcLine = q->line;
    q->taddress = next_instr_label();
    t.opcode = funcend_v;
    make_operand(q->result, &t.result);
    emit_instr(t);
    //func_stack.pop_back();
}

void patch_incomplete_jumps() {
    for (auto incomplete_jump: incomplete_jumps) {
        // we store the destination instruction's number in the target code instruction's "result" field
        if (incomplete_jump.icode_address == curr_quad + 1) {
            tcode_instructions[incomplete_jump.tcode_address].result.val =
                tcode_instructions.size();
        } else {
            tcode_instructions[incomplete_jump.tcode_address].result.val =
                quads[incomplete_jump.icode_address].taddress;
        }
    }
}

void generate_target_code() {
    unsigned int num_of_quads = curr_quad;
    for (unsigned int i = 1; i < num_of_quads; i++) {
        curr_quad = i;
        (*generators[quads[i].op])(&quads[i]);
    }

    patch_incomplete_jumps();
}

vector<string> opcode_to_string_arr = {
    "assign",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "if_eq",
    "if_not_eq",
    "if_less_eq",
    "if_greater_eq",
    "if_less",
    "if_greater",
    "jump",
    "call",
    "param",
    "funcstart",
    "funcend",
    "newtable",
    "tablegetelem",
    "tablesetelem",
    "nop"
};

vector<string> arg_to_string_arr = {
    "label_a",
    "global_a",
    "formal_a",
    "local_a",
    "number_a",
    "string_a",
    "bool_a",
    "nil_a",
    "userfunc_a",
    "libfunc_a",
    "retval_a"
};

void create_binary_file(string name) {
    unsigned long int str_len;
    ofstream file(name);

    if (file.is_open()) {
        file << 69420 << " magic_number" << endl;

        file << all_str_consts.size() << " constant_strings" << endl;
        for (string str : all_str_consts) {
            file << "\t" + str << endl;
        }

        file << all_num_consts.size() << " constant_numbers" << endl;
        for (double s : all_num_consts) {
            file << "\t" + to_string(s) << endl;
        }

        file << user_funcs.size() << " user_functions" << endl;
        for (struct user_func &u : user_funcs) {
            file << "\t" + u.id + " " + to_string(u.address) + " " + to_string(u.localSize) << endl;
        }

        file << all_lib_funcs.size() << " library_functions" << endl;
        for (string lib : all_lib_funcs) {
            file << "\t" + lib << endl;
        }

        file << tcode_instructions.size() << " instructions" << endl;
        for (instruction i : tcode_instructions) {
            file << "\t" + opcode_to_string_arr[i.opcode] << "\t";

            file << arg_to_string_arr[i.result.type] + " " + to_string(i.result.val) + "\t";

            vmarg_t type = i.arg1.type;
            file << arg_to_string_arr[type] + " " << ((type != nil_a) ? to_string(i.arg1.val) : string("0")) << "\t";
            

            type = i.arg2.type;
            file << arg_to_string_arr[type] + " " << ((type != nil_a) ? to_string(i.arg2.val) : string("0")) << "\t";
            

            file << endl;
        }

        file.close();
    }else {
        cerr << "Could not open file\n";
    }
}

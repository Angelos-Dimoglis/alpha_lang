#include <cassert>
#include <iostream>
#include <fstream>

#include "../lib/tcode_gen.h"

extern quad *quads;
extern unsigned int curr_quad;
extern unsigned total;

vector <instruction> tc_instructions;
vector <incomplete_jump> incomplete_jumps;
vector<Symbol *> func_stack;

vector<double> tc_numConsts;
vector <string> tc_strConsts;
vector<bool> tc_boolConsts;
vector <string> tc_libFuncs;
vector <struct userfunc> tc_userFuncs;

// translate expr to vmarg
void make_operand (expr *e, vmarg *arg) {

    // use a variable for storage
    switch (e->type) {
        case var_e:
        case table_item_e:
        case arith_expr_e:
        case bool_expr_e:
        case new_table_e: {
            Variable *var = (Variable *) e->sym;

            arg->val = var->offset;

            switch (var->space) {
                case program_var: arg->type = global_a; break;
                case function_local: arg->type = local_a; break;
                case formal_arg: arg->type = formal_a; break;
                default: assert(0);
            }
        }

        // constants
        case const_bool_e: {
            arg->val = e->bool_const;
            arg->type = bool_a; break;
        }

        case const_string_e: {
            arg->val = consts_newstring(e->str_const);
            arg->type = string_a; break;
        }

        case const_num_e: {
            arg->val = consts_newnumber(e->num_const);
            arg->type = number_a; break;
        }

        case const_nil_e: arg->type = nil_a; break;

        // functions
        case program_func_e: {
            arg->type = userfunc_a;
            arg->val = ((Function*)e->sym)->taddress;
            break;
        }

        case library_func_e: {
            arg->type = libfunc_a;
            arg->val = libfuncs_newused(e->sym->name);
            break;
        }
    
        default: assert(0);
    }
}

/* Helper functions to produce common arguments for generated instructions,
 * like 1, 0, "true", "false" and function return values.
 */
void make_number_operand (vmarg *arg, double val) {
    arg->val = consts_newnumber(val);
    arg->type = number_a;
}

void make_bool_operand (vmarg *arg, unsigned val) {
    arg->val = val;
    arg->type = bool_a;
}

void make_retval_operand (vmarg *arg) {
    arg->type = retval_a;
}

// TODO: patch incomplete jumps (pseudo-code at lec 14 slide 15)

// ### generating target code ###

/* generate (op, quad) (arithmetic, tables, assign and nop)
 * generate_relational (op, quad) (jump and ifs) 
 * generate_<NOT, OR, AND> (quad)
 * generate_PARAM (quad)
 * generate_CALL (quad)
 * generate_GETRETVAL (quad)
 * generate_FUNCSTART (quad)
 * generate_RETURN (quad)
 * generate_FUNCEND (quad)
 * we also need a func_stack for the returns
 */

typedef void (*generator_func_t) (quad*);

generator_func_t generators [] = {
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_NEWTABLE,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_ASSIGN,
    generate_NOP,
    generate_JUMP,
    generate_IFEQ,
    generate_IFNOTEQ,
    generate_IFGREATER,
    generate_IFGREATEREQ,
    generate_IFLESS,
    generate_IFLESSEQ,
    generate_NOT,
    generate_OR,
    generate_PARAM,
    generate_CALL,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_RETURN,
    generate_FUNCEND
};

void emit_instr(instruction instr) {
    instruction tc_i;
    tc_i.opcode = instr.opcode;
    tc_i.arg1 = instr.arg1;
    tc_i.arg2 = instr.arg2;
    tc_i.result = instr.result;
    tc_i.srcLine = instr.srcLine;
    tc_instructions.push_back(tc_i);
}

unsigned int next_instr_label() {
    return tc_instructions.size();
}

static void generate(vmopcode op, quad *q) {
    instruction tc_i;
    tc_i.opcode = op;
    make_operand(q->arg1, &tc_i.arg1);
    make_operand(q->arg2, &tc_i.arg2);
    make_operand(q->result, &tc_i.result);
    q->taddress = next_instr_label();
    emit_instr(tc_i);
}

static void generate_relational(vmopcode op, quad *q) {
    instruction tc_i;
    tc_i.opcode = op;

    make_operand(q->arg1, &tc_i.arg1);
    make_operand(q->arg2, &tc_i.arg2);

    tc_i.result.type = label_a;
    if (q->label < curr_quad) {
        tc_i.result.val = quads[q->label - 1].taddress; //+-1 due to indexing from 0
    } else {
        add_incomplete_jump(next_instr_label(), q->label - 1);
    }
    q->taddress = next_instr_label();
    emit_instr(tc_i);
}

void add_incomplete_jump(unsigned int i_label, unsigned int quad_label) {
    incomplete_jump inc_jump;
    inc_jump.instrNo = i_label;
    inc_jump.iaddress = quad_label;
    incomplete_jumps.push_back(inc_jump);
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
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = mul_v;
    make_operand(q->arg1, &t.arg1);
    make_operand(q->arg1, &t.arg2);
    t.arg2.val = consts_newnumber(-1);
    t.arg2.type = number_a;
    make_operand(q->result, &t.result);
    t.srcLine = q->line;
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
    instruction tc_i;
    tc_i.opcode = nop_v;
    tc_i.srcLine = next_instr_label();
    emit_instr(tc_i);
}

// relational expressions
void generate_JUMP(quad *q) {
    generate_relational(jump_v, q);
}

void generate_IF_EQ(quad *q) {
    generate_relational(jeq_v, q);
}

void generate_IF_NEQ(quad *q) {
    generate_relational(jne_v, q);
}

void generate_IF_GRTR(quad *q) {
    generate_relational(jgt_v, q);
}

void generate_IF_GRTR_EQ(quad *q) {
    generate_relational(jge_v, q);
}

void generate_IF_LESS(quad *q) {
    generate_relational(jlt_v, q);
}

void generate_IF_LESS_EQ(quad *q) {
    generate_relational(jle_v, q);
}

static void reset_operand(vmarg *arg) {
    arg->val = -1;
}

void generate_NOT(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = jeq_v;
    make_operand(q->arg1, &t.arg1);
    make_bool_operand(&t.arg2, false);
    t.result.type = label_a;
    t.result.val = next_instr_label() + 3;
    emit_instr(t);

    t.opcode = assign_v;
    make_bool_operand(&t.arg1, false);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    emit_instr(t);

    t.opcode = jump_v;
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    t.result.type = label_a;
    t.result.val = next_instr_label() + 2;
    emit_instr(t);

    t.opcode = assign_v;
    make_bool_operand(&t.arg1, true);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    emit_instr(t);

}

void generate_OR(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = jeq_v;
    make_operand(q->arg1, &t.arg1);
    make_bool_operand(&t.arg2, true);
    t.result.type = label_a;
    t.result.val = next_instr_label() + 4;
    emit_instr(t);
    make_operand(q->arg2, &t.arg1);
    t.result.val = next_instr_label() + 3;
    emit_instr(t);
    t.opcode = assign_v;
    make_bool_operand(&t.arg1, false);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    emit_instr(t);
    t.opcode = jump_v;
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    t.result.type = label_a;
    t.result.val = next_instr_label() + 2;
    emit_instr(t);
    t.opcode = assign_v;
    make_bool_operand(&t.arg1, true);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_AND(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = jeq_v;
    make_operand(q->arg1, &t.arg1);
    make_bool_operand(&t.arg2, false);
    t.result.type = label_a;
    t.result.val = next_instr_label() + 4;
    emit_instr(t);
    make_operand(q->arg2, &t.arg1);
    t.result.val = next_instr_label() + 3;
    emit_instr(t);
    t.opcode = assign_v;
    make_bool_operand(&t.arg1, true);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    emit_instr(t);
    t.opcode = jump_v;
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    t.result.type = label_a;
    t.result.val = next_instr_label() + 2;
    emit_instr(t);
    t.opcode = assign_v;
    make_bool_operand(&t.arg1, false);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_PARAM(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = pusharg_v;
    make_operand(q->arg1, &t.result);
    emit_instr(t);
}

void generate_CALL(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = call_v;
    t.srcLine = next_instr_label();
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_GETRETVAL(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = assign_v;
    t.srcLine = next_instr_label();
    make_operand(q->result, &t.result);
    make_retval_operand(&t.arg1);
    emit_instr(t);
}

void generate_FUNCSTART(quad *q) {
    Function *f = (Function *) q->result->sym;
    f->taddress = next_instr_label();
    q->taddress = next_instr_label();
    func_stack.push_back(f);
    instruction t;
    t.opcode = funcenter_v;
    make_operand(q->result, &t.result);
    emit_instr(t);
}

void generate_RETURN(quad *q) {
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = assign_v;
    make_retval_operand(&t.result);
    make_operand(q->arg1, &t.arg1);
    emit_instr(t);
    Function *f = func_stack.back();
    //append
    unsigned int instrLabel = next_instr_label();
    returnList *newnode = f->returnList;
    if (newnode == nullptr) {
        newnode = new returnList;
        newnode->instrLabel = instrLabel;
        newnode->next = NULL;
        f->funcVal.returnList = newnode;
    } else {
        returnList *tmp = new returnList;
        tmp->instrLabel = instrLabel;
        tmp->next = NULL;
        returnList *reader = newnode;
        while (reader->next != NULL) { reader = reader->next; }
        reader->next = tmp;
    }
    //append
    assert(f->funcVal.returnList);
    t.srcLine = next_instr_label();

}

void patchinstr(unsigned int instrind, unsigned int taddress) {
    if (instrind >= tc_instructions.size()) cerr << "ERROR AT PATCHINSTR" << endl;
    tc_instructions.at(instrind).result.val = taddress;
}

void generate_FUNCEND(quad *q) {
    Function *f = func_stack.back();
    returnList *reader = f->returnList;
    while (reader) {
        patchinstr(reader->instrLabel, next_instr_label());
        reader = reader->next;
    }
    q->taddress = next_instr_label();
    instruction t;
    t.opcode = funcexit_v;
    make_operand(q->arg1, &t.result);
    emit_instr(t);
    //func_stack.pop_back();
}

void patch_incomplete_jumps() {
    for (auto inc_j: incomplete_jumps) {
        // we store the destination instruction's number in the target code instruction's "result" field
        if (inc_j.iaddress == total) {
            tc_instructions.at(inc_j.instrNo).result.val = tc_instructions.size();
        } else {
            tc_instructions.at(inc_j.instrNo).result.val =
                    quads[inc_j.iaddress].taddress /*+ 1*/; //+-1 due to indexing from 0
        }
    }
}

void generate_target_code() {
    for (unsigned int i = 0; i < total; ++i) {
        curr_quad = i;
        (*generators[quads[i].op])(&quads[i]);
    }

    patch_incomplete_jumps();

}

void print_const_num_table() {
    if (tc_numConsts.empty()) {
        cout << "-=- Empty num consts table -=-" << endl;
        return;
    }
    unsigned int i;
    cout << "------------ Num consts ------------" << endl;
    for (i = 0; i < tc_numConsts.size(); i++) {
        cout << i << ":" << tc_numConsts.at(i) << endl;
    }
    cout << "------------------------------------" << endl;
}

void print_const_bool_table() {
    if (tc_boolConsts.empty()) {
        cout << "-=- Empty bool consts table -=-" << endl;
        return;
    }
    unsigned int i;
    cout << "------------ Bool consts ------------" << endl;
    for (i = 0; i < tc_boolConsts.size(); i++) {
        cout << i << ":" << tc_boolConsts.at(i) << endl;
    }
    cout << "-------------------------------------" << endl;
}

void print_const_str_table() {
    if (tc_strConsts.empty()) {
        cout << "-=- Empty string consts table -=-" << endl;
        return;
    }
    unsigned int i;
    cout << "------------ String consts ------------" << endl;
    for (i = 0; i < tc_strConsts.size(); i++) {
        cout << i << ":" << tc_strConsts.at(i) << endl;
    }
    cout << "---------------------------------------" << endl;
}

void print_const_libfunc_table() {
    if (tc_libFuncs.empty()) {
        cout << "-=- Empty lib func consts table -=-" << endl;
        return;
    }
    unsigned int i;
    cout << "------------ LibFunc consts ------------" << endl;
    for (i = 0; i < tc_libFuncs.size(); i++) {
        cout << i << ":" << tc_libFuncs.at(i) << endl;
    }
    cout << "----------------------------------------" << endl;
}

void print_const_userfunc_table() {
    if (tc_userFuncs.empty()) {
        cout << "-=- Empty user func consts table -=-" << endl;
        return;
    }
    unsigned int i;
    cout << "------------ UsrFunc consts ------------" << endl;
    for (i = 0; i < tc_userFuncs.size(); i++) {
        cout << i << ":" << tc_userFuncs.at(i).id << endl;
    }
    cout << "----------------------------------------" << endl;
}

void print_const_tables() {
    print_const_num_table();
    print_const_bool_table();
    print_const_str_table();
    print_const_userfunc_table();
    print_const_libfunc_table();
}

void print_target_code() {
    unsigned int index = 0;
    cout << "instr#\t\topcode\t\tresult\t\t\targ1\t\t\targ2" << endl;
    cout << "-------------------------------------------------------------------------------------------------" << endl;
    for (instruction inst: tc_instructions) {
        string arg1_value = "";
        string arg2_value = "";
        /*static int i=0;
        cout<<i++<<"th inst :"<<inst.opcode<<endl;*/
        if (!inst.arg1.is_null) {
            switch (inst.arg1.type) {
                case bool_a:
                    arg1_value = "\'" + BoolToString(tc_boolConsts.at(inst.arg1.val)) + "\'";
                    break;
                case string_a:
                    arg1_value = "\"" + tc_strConsts.at(inst.arg1.val) + "\"";
                    break;
                case number_a:
                    arg1_value = to_string(tc_numConsts.at(inst.arg1.val));
                    break;
                case userfunc_a:
                    arg1_value = tc_userFuncs.at(inst.arg1.val).id;
                    break;
                case libfunc_a:
                    arg1_value = tc_libFuncs.at(inst.arg1.val);
                    break;
                case global_a:
                case formal_a:
                case local_a:
                    arg1_value = inst.arg1.var_id;
                    break;
                case nil_a:
                    arg1_value = "nil";
                    break;
                case retval_a:
                    arg1_value = to_string(inst.arg1.val);
                    break;
                default:
                    arg1_value = inst.arg1.var_id;
                    break;
            }
        }
        if (!inst.arg2.is_null) {
            switch (inst.arg2.type) {
                case bool_a:
                    arg2_value = "\'" + BoolToString(tc_boolConsts.at(inst.arg2.val)) + "\'";
                    break;
                case string_a:
                    arg2_value = "\"" + tc_strConsts.at(inst.arg2.val) + "\"";
                    break;
                case number_a:
                    arg2_value = to_string(tc_numConsts.at(inst.arg2.val));
                    break;
                case userfunc_a:
                    arg2_value = tc_userFuncs.at(inst.arg2.val).id;
                    break;
                case libfunc_a:
                    arg2_value = tc_libFuncs.at(inst.arg2.val);
                    break;
                case global_a:
                case formal_a:
                case local_a:
                    arg2_value = inst.arg2.var_id;
                    break;
                case nil_a:
                    arg2_value = "nil";
                    break;
                case retval_a:
                    arg2_value = to_string(inst.arg2.val);
                    break;
                default:
                    arg2_value = inst.arg2.var_id;
                    break;
            }
        }
        string label = "";
        if (
            inst.opcode == jeq_v ||
            inst.opcode == jne_v ||
            inst.opcode == jle_v ||
            inst.opcode == jge_v ||
            inst.opcode == jlt_v ||
            inst.opcode == jgt_v ||
            inst.opcode == jump_v
        ) {
            label = to_string(inst.srcLine);
        }

        string result_value = "";
        if (!inst.result.is_null) {
            switch (inst.result.type) {
                case global_a:
                case formal_a:
                case local_a:
                    result_value = inst.result.var_id;
                    break;
                case libfunc_a:
                    result_value = tc_libFuncs.at(inst.result.val);
                    break;
                case userfunc_a:
                    result_value = tc_userFuncs.at(inst.result.val).id;
                    break;
                case string_a:
                    result_value = "\"" + tc_strConsts.at(inst.result.val) + "\"";
                    break;
                case number_a:
                    result_value = to_string(tc_numConsts.at(inst.result.val));
                    break;
                case bool_a:
                    result_value = tc_boolConsts.at(inst.result.val) ? "true" : "false";
                    break;
                default:
                    result_value = inst.result.var_id;
                    break;
            }
        }
        cout << index << ":\t\t" << vmopcodeStrings[inst.opcode] << "\t\t" << inst.result.type << " ("
             << vmargStrings[inst.result.type] << "), " << inst.result.val;

        if (inst.result.type != label_a) cout << ":" << result_value;

        if (!inst.arg1.is_null) {
            cout << "\t\t" << inst.arg1.type << " (" << vmargStrings[inst.arg1.type] << "), "
                 << inst.arg1.val << ":" << arg1_value;
        } else { cout << "             "; }
        if (!inst.arg2.is_null)
            cout << "\t\t" << inst.arg2.type << " ("
                 << vmargStrings[inst.arg2.type] << "), " << inst.arg2.val << ":"
                 << arg2_value << endl;
        else cout << "             " << endl;
        index++;
    }
}

void createbin() {
    unsigned int magic = 340;
    unsigned long int str_len;
    FILE* bytefile = fopen("output.abc", "w");;
    size_t i;

    // magic number 
    fwrite(&magic, sizeof(unsigned int), 1, bytefile);
    // global offset
    fwrite(&programVarOffset, sizeof(unsigned int), 1, bytefile);

    // numerical consts
    unsigned int numsSize = tc_numConsts.size();
    fwrite(&numsSize, sizeof(unsigned int), 1, bytefile);
    for (i = 0; i < numsSize; i++) {
        fwrite(&tc_numConsts[i], sizeof(double), 1, bytefile);
    }

    // string consts
    unsigned int strSize = tc_strConsts.size();
    fwrite(&strSize, sizeof(unsigned int), 1, bytefile);
    for (i = 0; i < strSize; i++) {
        str_len = tc_strConsts[i].length();
        fwrite(&str_len, sizeof(unsigned long int), 1, bytefile);
        fwrite(tc_strConsts[i].c_str(), sizeof(char), str_len + 1, bytefile);
    }

    // bool consts
    unsigned int boolsSize = tc_boolConsts.size();
    fwrite(&boolsSize, sizeof(unsigned int), 1, bytefile);
    for (i = 0; i < boolsSize; i++) {
        unsigned int boolAsInt = tc_boolConsts[i] ? 1 : 0;
        fwrite(&boolAsInt, sizeof(unsigned int), 1, bytefile);
    }

    // Library functions
    unsigned int libFuncsSize = tc_libFuncs.size();
    fwrite(&libFuncsSize, sizeof(unsigned int), 1, bytefile);
    for (i = 0; i < libFuncsSize; i++) {
        str_len = tc_libFuncs[i].length();
        fwrite(&str_len, sizeof(unsigned long int), 1, bytefile);
        fwrite(tc_libFuncs[i].c_str(), sizeof(char), str_len + 1, bytefile);
    }

    // User functions
    unsigned int usrFuncsSize = tc_userFuncs.size();
    fwrite(&usrFuncsSize, sizeof(unsigned int), 1, bytefile);
    for (i = 0; i < usrFuncsSize; i++) {
        fwrite(&tc_userFuncs[i].address, sizeof(unsigned int), 1, bytefile);
        fwrite(&tc_userFuncs[i].total_locals, sizeof(unsigned int), 1, bytefile);

        str_len = tc_userFuncs[i].id.length();
        fwrite(&str_len, sizeof(unsigned long int), 1, bytefile);
        fwrite(tc_userFuncs[i].id.c_str(), sizeof(char), str_len + 1, bytefile);
    }

    // instructions
    unsigned int instrSize = tc_instructions.size();
    fwrite(&instrSize, sizeof(unsigned int), 1, bytefile);
    for (i = 0; i < instrSize; i++) {
        fwrite(&(tc_instructions[i].opcode), sizeof(vmopcode), 1, bytefile);
        // result
        unsigned int isResultNull = 0; // all commands have a res
        fwrite(&isResultNull,sizeof(unsigned int),1,bytefile);
        fwrite(&(tc_instructions[i].result.type), sizeof(vmarg_t), 1, bytefile);
        fwrite(&(tc_instructions[i].result.val), sizeof(unsigned int), 1, bytefile);

        unsigned int isArg1Null = tc_instructions[i].arg1.is_null;
        fwrite(&isArg1Null, sizeof(unsigned int), 1, bytefile);
        if (!isArg1Null) {
            fwrite(&(tc_instructions[i].arg1.type), sizeof(vmarg_t), 1, bytefile);
            fwrite(&(tc_instructions[i].arg1.val), sizeof(unsigned int), 1, bytefile);
        }

        unsigned int isArg2Null = tc_instructions[i].arg2.is_null;
        fwrite(&isArg2Null, sizeof(unsigned int), 1, bytefile);
        if (!isArg2Null) {
            fwrite(&(tc_instructions[i].arg2.type), sizeof(vmarg_t), 1, bytefile);
            fwrite(&(tc_instructions[i].arg2.val), sizeof(unsigned int), 1, bytefile);
        }

        fwrite(&(tc_instructions[i].srcLine), sizeof(unsigned int), 1, bytefile);
    }

    fclose(bytefile);
}

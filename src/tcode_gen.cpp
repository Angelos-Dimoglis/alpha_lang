#include <cassert>

#include "../lib/tcode_gen.h"

extern quad *quads;
extern unsigned total;

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

void generate (void) {
    for (unsigned i = 0; i < total; i++)
        (*generators[quads[i].op]) (quads + i);
}

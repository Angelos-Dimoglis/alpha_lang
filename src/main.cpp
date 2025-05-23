#include <stdio.h>
#include <string>
#include <unistd.h>
#include <array>

#include "../lib/parser.hpp"
#include "../lib/icode_gen.h"

extern FILE *yyin;
extern FILE *yyout;
extern int print_lexer_tokens;
extern quad *quads;
extern unsigned total;
extern unsigned int curr_quad;

std::string exec_command(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = popen(cmd, "r");

    if (!pipe)
        throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    pclose(pipe);
    return result;
}

string opcode_to_string(iopcode opcode) {

    switch (opcode) {
        case assign: return "assign";
        case add: return "add";
        case sub: return "sub";
        case mul: return "mul";
        case _div: return "div";
        case mod: return "mod";
        case uminus: return "uminus";
        case _and: return "and";
        case _or: return "or";
        case _not: return "not";
        case if_eq: return "if_eq";
        case if_noteq: return "if_noteq";
        case if_lesseq: return "if_lesseq";
        case if_greatereq: return "if_greatereq";
        case if_less: return "if_less";
        case if_greater: return "if_greater";
        case jump: return "jump";
        case call: return "call";
        case param: return "param";
        case ret: return "return";
        case get_ret_val: return "get_ret_val";
        case func_start: return "func_start";
        case func_end: return "func_end";
        case table_create: return "table_create";
        case table_get_elem: return "table_get_elem";
        case table_set_elem: return "table_set_elem";
        default:
            assert(0);
    }

    return "";
}

string doubleToString(double value) {
    string result = to_string(value);
    
    // Remove trailing zeros after decimal point
    size_t dot_pos = result.find('.');
    if (dot_pos != string::npos) {
        // Trim trailing zeros
        result.erase(result.find_last_not_of('0') + 1);
        // If the decimal point is now the last character, remove it too
        if (result.back() == '.') {
            result.pop_back();
        }
    }
    return result;
}

string print_expr_content(expr* exp) {
    if (!exp)
        return "";

    string str;
    switch (exp->type) {
        case const_num_e:
            str = doubleToString(exp->num_const);
            break;
        case const_bool_e:
            str = (exp->bool_const ? "TRUE" : "FALSE");
            break;
        case const_string_e:
            str = exp->str_const;
            break;
        case const_nil_e:
            str = "NIL";
            break;
        default:
            if (exp->sym)
                str = exp->sym->name;
            else
                str = "";
    }

    return str;
}

void print_quad (struct quad *q, int index, FILE *output) {
    fprintf(output, "%d:|", index);
    fprintf(output, "%s|", opcode_to_string(q->op).c_str());

    if (q->result)
        fprintf(output, "%s", print_expr_content(q->result).c_str());
    fprintf(output, "|");

    if (q->arg1)
        fprintf(output, "%s", print_expr_content(q->arg1).c_str());
    fprintf(output, "|");

    if (q->arg2)
        fprintf(output, "%s", print_expr_content(q->arg2).c_str());
    fprintf(output, "|");

    if (q->label)
        fprintf(output, "%d", q->label);
    fprintf(output, "|");
}

void write_quads (FILE *output, const char* filename, bool output_file_set) {
    // create a temporary file
    char temp_name[] = "/tmp/quadsXXXXXX";
    int fd = mkstemp(temp_name);
    if (fd == -1) {
        perror("mkstemp failed");
        return;
    }

    FILE* temp = fdopen(fd, "w");
    if (!temp) {
        perror("fdopen failed");
        close(fd);
        return;
    }

    // write unformatted quads to temp file
    const string highlight = "\033[48;5;240m"; // Gray background
    const string reset = "\033[0m"; // Reset formatting
    fprintf(temp, "quad#|opcode|result|arg1|arg2|label");
    for (int i = 1; i < curr_quad; i++) {
        fprintf(temp, "\n");
        if (i % 2 == 1)
            fprintf(temp, "%s", highlight.c_str());
        print_quad(&(quads[i]), i, temp);
        if (i % 2 == 1)
            fprintf(temp, "%s", reset.c_str());
    }

    fprintf(temp, "%s",reset.c_str());
    fclose(temp);

    // call column
    std::string cmd = "column -t -s '|' < ";
    cmd += temp_name;

    std::string formatted = exec_command(cmd.c_str());

    // output the formatted result
    FILE* final_out = output_file_set ? fopen(filename, "w") : stdout;
    if (!final_out) {
        fprintf(stderr, "Cannot open output file: %s\n", filename);
        unlink(temp_name);
        return;
    }

    fprintf(final_out, "%s", formatted.c_str());

    if (output_file_set)
        fclose(final_out);

    unlink(temp_name);
}

int main (int argc, char **argv) {
    bool print_symtable_flag = false,
         input_file_set = false,
         output_file_set = false;

    string output_file = "/dev/stdout";

    char c;
    while ((c = getopt(argc, argv, "dsti:o:")) != -1) {
        switch (c) {
            case 'd': // debug
                yydebug = 1;
                break;
            case 's': // (SYMBOLS) print symbol table
                print_symtable_flag = true;
                break;
            case 't': // (TOKENS) print lexer tokens
                print_lexer_tokens = 1;
                break;
            case 'i': // input file

                if (optarg != nullptr)
                    input_file_set = true;

                if (!(yyin = fopen(optarg, "r"))) {
                    fprintf(stderr, "Cannot read file: %s\n", optarg);
                    return 1;
                }

                break;
            case 'o': // output file

                if (optarg != nullptr)
                    output_file_set = true;

                if (!(yyout = fopen(optarg, "w"))) {
                    fprintf(stderr, "Cannot write file: %s\n", optarg);
                    return 1;
                }
                
                output_file = std::string(optarg);
                
                break;
            default:
                fprintf(stderr, "Unknown option: %s\n", optarg);
                assert(0);
        }
    }

    // default input is stdin
    if (!input_file_set)
        yyin = stdin;

    // default output is stdout
    if (!output_file_set)
        yyout = stdout;

    yyparse();

    if (print_symtable_flag)
        sym_table.PrintTable();

    write_quads(yyout, output_file.c_str(), output_file_set);

    // cleanup
    fclose(yyin);
    fclose(yyout);
    //free_token_list();
    sym_table.freeTable();  

    return 0;
}

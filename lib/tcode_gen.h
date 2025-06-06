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

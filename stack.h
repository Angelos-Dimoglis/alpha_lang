#ifndef STACK_H
#define STACK_H

#include <stdbool.h>

// Stack node structure
typedef struct StackNode {
    int data;
    struct StackNode* next;
} StackNode;

// Stack structure
typedef struct {
    StackNode* top;
} Stack;

// Function prototypes
Stack *initStack();
bool isEmpty(Stack* stack);
void push(Stack* stack, int value);
int pop(Stack* stack);
int peek(Stack* stack);
void freeStack(Stack* stack);

#endif

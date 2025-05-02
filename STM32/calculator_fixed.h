#ifndef CALCULATOR_FIXED_H
#define CALCULATOR_FIXED_H

#include <math.h>

typedef enum {
    OPERAND,
    OPERATOR,
    VARIABLE,
} TokenType;

typedef struct {
    TokenType type;
    union {
        float operand;
        char operator;
        float variable;
    } value;
} Token;

int isOperator(char c);
int precedence(char op);
Token *InfixToPostfix(char *myFunction);
float evaluatePostfix(Token *postfix, float x_value);
void printTokens(Token *output);

#endif

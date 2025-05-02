#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <math.h>

typedef enum {S_START, S_OPERAND, S_OPERATOR, S_OPEN, S_CLOSE, S_ERROR, S_END} state_t;

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
        float variable;  // giá trị này không sử dụng trong tính toán, chỉ để phân biệt token VARIABLE
    } value;
} Token;

int isOperator(char c);
int precedence(char op);
Token *InfixtoPostfix(char *myFunction);
float evaluatePostfix(Token *postfix, float x_value);
void printTokens(Token *output);

#endif

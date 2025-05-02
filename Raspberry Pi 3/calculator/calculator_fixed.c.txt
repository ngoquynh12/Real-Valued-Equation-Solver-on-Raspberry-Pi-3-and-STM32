
#include "calculator_fixed.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#define MAX_EXPR 200
#define MAX 500

int gcd(int a, int b) {
    return (b == 0) ? a : gcd(b, a % b);
}
int isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

int precedence(char op) {
    switch(op) {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case '^': return 3;
        default: return 0;
    }
}

Token *InfixtoPostfix(char *myFunction) {
    char *eq = strchr(myFunction, '=');
    if(eq != NULL) {
        char newStr[MAX_EXPR];
        *eq = '\0';
        snprintf(newStr, sizeof(newStr), "%s - (%s)", myFunction, eq + 1);
        strcpy(myFunction, newStr);
    }

    Token *output = (Token *)malloc(MAX * sizeof(Token));
    int outputIndex = 0;
    char stack[MAX];
    int stackTop = -1;

    while (*myFunction != '\0') {
        if (isspace(*myFunction)) {
            myFunction++;
            continue;
        }

        if (isdigit(*myFunction) || *myFunction == '.') {
            float operand = 0.0, divisor = 1.0;
            int decimal_flag = 0;
            while (isdigit(*myFunction) || *myFunction == '.') {
                if (*myFunction == '.') {
                    decimal_flag = 1;
                } else {
                    if (!decimal_flag) {
                        operand = operand * 10 + (*myFunction - '0');
                    } else {
                        divisor *= 10;
                        operand += (*myFunction - '0') / divisor;
                    }
                }
                myFunction++;
            }
            output[outputIndex].type = OPERAND;
            output[outputIndex++].value.operand = operand;
            continue;
        }

        if (*myFunction == 'x') {
            output[outputIndex].type = VARIABLE;
            output[outputIndex++].value.variable = 0.0;
            myFunction++;
            continue;
        }

        if (*myFunction == '(') {
            stack[++stackTop] = *myFunction;
            myFunction++;
            continue;
        }

        if (*myFunction == ')') {
            while (stackTop >= 0 && stack[stackTop] != '(') {
                output[outputIndex].type = OPERATOR;
                output[outputIndex++].value.operator = stack[stackTop--];
            }
            if (stackTop >= 0) stackTop--;  // pop '('
            myFunction++;
            continue;
        }

        if (isOperator(*myFunction)) {
            while (stackTop >= 0 && isOperator(stack[stackTop]) &&
                   precedence(stack[stackTop]) >= precedence(*myFunction)) {
                output[outputIndex].type = OPERATOR;
                output[outputIndex++].value.operator = stack[stackTop--];
            }
            stack[++stackTop] = *myFunction;
            myFunction++;
            continue;
        }

        printf("Input function error!!!\n");
        free(output);
        return NULL;
    }

    while (stackTop >= 0) {
        output[outputIndex].type = OPERATOR;
        output[outputIndex++].value.operator = stack[stackTop--];
    }

    output[outputIndex].type = OPERATOR;
    output[outputIndex++].value.operator = 'E';
    return output;
}
float safeRationalPow(float a, float b) {
    // Nếu a không âm thì dùng hàm pow bình thường
    if(a >= 0)
        return pow(a, b);

    // a < 0: Cố gắng biểu diễn số mũ b dưới dạng phân số.
    // Ở đây ta giả sử b có 1 chữ số sau dấu phẩy (scale = 10)
    float scale = 10.0f;
    int p = (int)round(b * scale);  // tử số
    int q = (int)scale;             // mẫu số ban đầu
    
    int g = gcd(abs(p), q);
    p /= g;
    q /= g;

    // Nếu mẫu số q chẵn -> không xác định theo số thực
    if(q % 2 == 0)
        return NAN;

    // Quy ước: Nếu tử số p lẻ, kết quả là âm; nếu p chẵn, kết quả là dương.
    int sign = (p % 2 != 0) ? -1 : 1;
    return sign * pow(fabs(a), b);
}
float evaluatePostfix(Token *postfix, float x_value) {
    float stack[MAX];
    int stackTop = -1;
    int i = 0;
    while (!(postfix[i].type == OPERATOR && postfix[i].value.operator == 'E')) {
        if (postfix[i].type == OPERAND) {
            stack[++stackTop] = postfix[i].value.operand;
        } else if (postfix[i].type == VARIABLE) {
            stack[++stackTop] = x_value;
        } else if (postfix[i].type == OPERATOR) {
            if (stackTop < 1) {
                printf("Error: insufficient operands.\n");
                return NAN;
            }
            float b = stack[stackTop--];
            float a = stack[stackTop--];
            float res = 0.0;
            switch (postfix[i].value.operator) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': 
                    if (b == 0) return NAN;
                    res = a / b; break;
                    case '^': 
                    res = safeRationalPow(a, b);
                    break;
                  
                default:
                    printf("Unknown operator %c\n", postfix[i].value.operator);
                    return NAN;
            }
            stack[++stackTop] = res;
        }
        i++;
    }
    return (stackTop == 0) ? stack[0] : NAN;
}

void printTokens(Token *output) {
    int i = 0;
    printf("Postfix: ");
    while (!(output[i].type == OPERATOR && output[i].value.operator == 'E')) {
        if (output[i].type == OPERAND)
            printf("[%.2f] ", output[i].value.operand);
        else if (output[i].type == OPERATOR)
            printf("[%c] ", output[i].value.operator);
        else if (output[i].type == VARIABLE)
            printf("[x] ");
        i++;
    }
    printf("\n");
}

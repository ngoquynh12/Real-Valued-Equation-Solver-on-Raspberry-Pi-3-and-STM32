#include "calculator_fixed.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MAX_EXPR 256
#define MAX      512

// Hàm tiền xử lý: chuyển ^-x → ^(0-x)
static void preprocessCaretMinus(char *expr) {
    char buf[MAX_EXPR * 3];
    int i = 0, j = 0;
    while (expr[i]) {
        if (expr[i] == '^' && expr[i+1] == '-') {
            buf[j++] = '^';
            buf[j++] = '(';
            buf[j++] = '0';
            buf[j++] = '-';
            i += 2;
            if (expr[i] == 'x' || expr[i] == 'X') {
                buf[j++] = expr[i++];
            } else if (isdigit(expr[i]) || expr[i] == '.') {
                while (isdigit(expr[i]) || expr[i] == '.') buf[j++] = expr[i++];
            } else if (expr[i] == '(') {
                int depth = 0;
                do {
                    if (expr[i] == '(') depth++;
                    else if (expr[i] == ')') depth--;
                    buf[j++] = expr[i++];
                } while (expr[i] && depth > 0);
            }
            buf[j++] = ')';
        } else {
            buf[j++] = expr[i++];
        }
    }
    buf[j] = '\0';
    strcpy(expr, buf);
}

int gcd(int a, int b) { return b ? gcd(b, a % b) : a; }
int isOperator(char c) { return c=='+'||c=='-'||c=='*'||c=='/'||c=='^'; }
int precedence(char op) {
    switch(op) {
        case '+': case '-': return 1;
        case '*': case '/': return 2;
        case '^':           return 3;
        default:            return 0;
    }
}

Token *InfixToPostfix(char *expr) {
    preprocessCaretMinus(expr);
    char *eq = strchr(expr, '=');
    if (eq) {
        char tmp[MAX_EXPR];
        *eq = '\0';
        snprintf(tmp, sizeof(tmp), "%s-(%s)", expr, eq+1);
        strcpy(expr, tmp);
    }

    Token *output = malloc(MAX * sizeof(Token));
    if (!output) return NULL;
    int outIdx = 0;
    char stack[MAX];
    int top = -1;
    char *start = expr;

    while (*expr) {
        if (isspace(*expr)) { expr++; continue; }

        if ((isdigit(*expr) || *expr == '.') ||
            (*expr == '-' && (expr == start || *(expr - 1) == '(' || isOperator(*(expr - 1))) &&
             (isdigit(*(expr + 1)) || *(expr + 1) == '.' || *(expr + 1) == 'x' || *(expr + 1) == 'X')))
        {
            int neg = 0;
            if (*expr == '-') { neg = 1; expr++; }
            double val = 0.0, div = 1.0;
            int dec = 0;
            while (isdigit(*expr) || *expr == '.') {
                if (*expr == '.') dec = 1;
                else if (!dec) val = val * 10 + (*expr - '0');
                else { div *= 10; val += (*expr - '0') / div; }
                expr++;
            }
            if (neg) val = -val;
            output[outIdx].type = OPERAND;
            output[outIdx++].value.operand = (float)val;
            continue;
        }

        if (*expr == 'x' || *expr == 'X') {
            output[outIdx].type = VARIABLE;
            output[outIdx++].value.variable = 0.0f;
            expr++; continue;
        }

        if (*expr == '(') { stack[++top] = '('; expr++; continue; }

        if (*expr == ')') {
            while (top >= 0 && stack[top] != '(') {
                output[outIdx].type = OPERATOR;
                output[outIdx++].value.operator = stack[top--];
            }
            if (top >= 0) top--;
            expr++; continue;
        }

        if (isOperator(*expr)) {
            char op = *expr;
            while (top >= 0 && isOperator(stack[top]) &&
                   ( precedence(stack[top]) > precedence(op) ||
                    (precedence(stack[top]) == precedence(op) && op != '^') ))
            {
                output[outIdx].type = OPERATOR;
                output[outIdx++].value.operator = stack[top--];
            }
            stack[++top] = op;
            expr++; continue;
        }

        printf("Input function error tại '%c'!\n", *expr);
        free(output);
        return NULL;
    }

    while (top >= 0) {
        output[outIdx].type = OPERATOR;
        output[outIdx++].value.operator = stack[top--];
    }
    output[outIdx].type = OPERATOR;
    output[outIdx++].value.operator = 'E';
    return output;
}

float safeRationalPow(float a, float b) {
    if (a == 0 && b < 0) return NAN;
    if (a >= 0) return powf(a, b);
    double ip;
    if (modf(b, &ip) == 0.0) {
        int sign = ((long long)ip & 1) ? -1 : 1;
        return sign * powf(-a, b);
    }
    int best_p = 0, best_q = 0; double best_err = 1e9;
    for (int q = 1; q <= 9; ++q) {
        int p = (int)round(b * q);
        double err = fabs(b - (double)p / q);
        if (err < 1e-6 && err < best_err) { best_err = err; best_p = p; best_q = q; }
    }
    if (!best_q || (best_q & 1) == 0) return NAN;
    int sign = (abs(best_p) & 1) ? -1 : 1;
    return sign * powf(-a, b);
}

float evaluatePostfix(Token *post, float x_val) {
    float stack[MAX];
    int top = -1, i = 0;
    while (!(post[i].type == OPERATOR && post[i].value.operator == 'E')) {
        if (post[i].type == OPERAND) {
            stack[++top] = post[i].value.operand;
        } else if (post[i].type == VARIABLE) {
            stack[++top] = x_val;
        } else {
            if (top < 1) {
                if (top == 0 && post[i].value.operator == '-') {
                    float a = stack[top--];
                    stack[++top] = -a;
                    i++;
                    continue;
                }
                return NAN;
            }
            float b = stack[top--], a = stack[top--], r = 0.0f;
            if (isnan(a) || isnan(b)) return NAN;
            switch (post[i].value.operator) {
                case '+': r = a + b; break;
                case '-': r = a - b; break;
                case '*': r = a * b; break;
                case '/': if (b == 0) return NAN; r = a / b; break;
                case '^': r = safeRationalPow(a, b); break;
                default : return NAN;
            }
            stack[++top] = r;
        }
        i++;
    }
    return (top == 0) ? stack[0] : NAN;
}

void printTokens(Token *out) {
    int i = 0;
    printf("Postfix: ");
    while (!(out[i].type == OPERATOR && out[i].value.operator == 'E')) {
        if (out[i].type == OPERAND)
            printf("[%.2f] ", out[i].value.operand);
        else if (out[i].type == VARIABLE)
            printf("[x] ");
        else
            printf("[%c] ", out[i].value.operator);
        ++i;
    }
    printf("\n");
}

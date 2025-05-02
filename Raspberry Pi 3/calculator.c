#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#define MAX 100

/*
S_START: Trạng thái bắt đầu
S_OPERAND: Trạng thái nhận toán hạng
S_OPERATOR: Trạng thái nhận toán tử
S_OPEN: Trạng thái nhận dấu mở ngoặc
S_CLOSE: Trạng thái nhận dấu đóng ngoặc
S_ERROR: Trạng thái lỗi
S_END: Trạng thái kết thúc
*/
typedef enum {S_START, S_OPERAND, S_OPERATOR, S_OPEN, S_CLOSE, S_ERROR, S_END} state_t;

typedef enum {
    OPERAND,
    OPERATOR,
    VARIABLE
} TokenType;

typedef struct {
    TokenType type;
    union {
        float operand;
        char operator;
        float variable;  // giá trị này không sử dụng trong tính toán, chỉ để phân biệt token VARIABLE
    } value;
} Token;

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

Token *infixToPostfix(char* myFunction){
    state_t current_state = S_START;
    Token *output = (Token *)malloc(MAX * sizeof(Token));
    int outputIndex = 0;
    char stack[MAX];
    int stackTop = -1;

    while (1){
        switch (current_state){
            case S_START:
                if (isspace(*myFunction)) {
                    myFunction++; // bỏ qua khoảng trắng
                    continue;
                }
                if (isdigit(*myFunction) || *myFunction == '.' || *myFunction == 'x') {
                    current_state = S_OPERAND;
                }
                else if (*myFunction == '(') {
                    current_state = S_OPEN;
                }
                else if (*myFunction == '\0') {
                    current_state = S_END;
                }
                else {
                    current_state = S_ERROR;
                }
                break;

            case S_OPERAND: {
                float operand = 0.0;
                int decimal_flag = 0;
                float decimal_divisor = 1.0;
                // Nếu là số
                if (isdigit(*myFunction) || *myFunction == '.') {
                    while (isdigit(*myFunction) || *myFunction == '.') {
                        if (*myFunction == '.') {
                            decimal_flag = 1;
                        }
                        else {
                            if (decimal_flag == 0) {
                                operand = operand * 10 + (*myFunction - '0');
                            }
                            else {
                                decimal_divisor *= 10;
                                operand = operand + (*myFunction - '0') / decimal_divisor;
                            }
                        }
                        myFunction++;
                    }
                    output[outputIndex].type = OPERAND;
                    output[outputIndex].value.operand = operand;
                    outputIndex++;
                }
                // Nếu là biến x
                else if (*myFunction == 'x') {
                    myFunction++;
                    output[outputIndex].type = VARIABLE;
                    // Giá trị của biến không được lưu trong token, sẽ thay thế khi tính toán
                    output[outputIndex].value.variable = 0.0;
                    outputIndex++;                    
                }       
                if (isspace(*myFunction)) {
                    while (isspace(*myFunction)) myFunction++;
                }
                if (isOperator(*myFunction)) {
                    current_state = S_OPERATOR;
                }
                else if (*myFunction == ')') {
                    current_state = S_CLOSE;
                }
                else if (*myFunction == '\0') {
                    current_state = S_END;
                }
                else if (*myFunction == '(') {
                    current_state = S_OPEN;
                }
                else {
                    current_state = S_ERROR;
                }
                break;
            }

            case S_OPERATOR:
                // Xử lý ưu tiên toán tử trên stack
                while (stackTop >= 0 && isOperator(stack[stackTop]) && 
                       precedence(stack[stackTop]) >= precedence(*myFunction)) {
                    output[outputIndex].type = OPERATOR;
                    output[outputIndex].value.operator = stack[stackTop];
                    outputIndex++;
                    stackTop--;
                }
                stack[++stackTop] = *myFunction;
                myFunction++;
                current_state = S_START;
                break;

            case S_OPEN:
                stack[++stackTop] = *myFunction; // lưu dấu '(' vào stack
                myFunction++;
                current_state = S_START;
                break;

            case S_CLOSE:
                while (stackTop >= 0 && stack[stackTop] != '(') {
                    output[outputIndex].type = OPERATOR;
                    output[outputIndex].value.operator = stack[stackTop];
                    outputIndex++;
                    stackTop--;
                }
                if (stackTop >= 0) stackTop--; // bỏ dấu '('
                myFunction++;
                if (isspace(*myFunction)) {
                    while (isspace(*myFunction)) myFunction++;
                }
                if (isOperator(*myFunction)) {
                    current_state = S_OPERATOR;
                }
                else if (*myFunction == ')') {
                    current_state = S_CLOSE;
                }
                else if (*myFunction == '\0') {
                    current_state = S_END;
                }
                else if (isdigit(*myFunction) || *myFunction == '.' || *myFunction == 'x' || *myFunction == '(') {
                    current_state = S_OPERAND;
                }
                else {
                    current_state = S_ERROR;
                }
                break;

            case S_END:
                while (stackTop >= 0) {
                    output[outputIndex].type = OPERATOR;
                    output[outputIndex].value.operator = stack[stackTop];
                    outputIndex++;
                    stackTop--;
                }
                // Dấu 'E' báo kết thúc token
                output[outputIndex].type = OPERATOR;
                output[outputIndex].value.operator = 'E';
                outputIndex++;
                return output;
                break;

            case S_ERROR:
            default:
                printf("Input function error!!!\n");
                free(output);
                return NULL;
        }
    }
}

// Hàm đánh giá biểu thức dạng postfix với giá trị của x
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
                return 0;
            }
            float b = stack[stackTop--];
            float a = stack[stackTop--];
            float res = 0.0;
            switch (postfix[i].value.operator) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': 
                    if (b == 0) {
                        printf("Error: Division by zero.\n");
                        return 0;
                    }
                    res = a / b; 
                    break;
                case '^': res = pow(a, b); break;
                default:
                    printf("Error: Unknown operator %c.\n", postfix[i].value.operator);
                    return 0;
            }
            stack[++stackTop] = res;
        }
        i++;
    }
    if (stackTop != 0) {
        printf("Error: Invalid postfix expression.\n");
        return 0;
    }
    return stack[stackTop];
}

void printTokens(Token *output) {
    int i = 0;
    printf("Output Tokens: ");
    while (!(output[i].type == OPERATOR && output[i].value.operator == 'E')) {
        if (output[i].type == OPERAND) {
            printf("[%.2f] ", output[i].value.operand);
        } else if (output[i].type == OPERATOR) {
            printf("[%c] ", output[i].value.operator);
        } else if (output[i].type == VARIABLE) {
            printf("[x] ");
        }
        i++;
    }
    printf("\n");
}

int main(){
    Token *output;
    float x;
    char str[MAX];
    
    printf("Nhập biểu thức: ");
    fgets(str, MAX, stdin);
    str[strcspn(str, "\n")] = 0;  // loại bỏ ký tự newline

    output = infixToPostfix(str);
    if (output == NULL) {
        return 1;
    }
    
    printf("Nhập giá trị x: ");
    scanf("%f", &x);

    printTokens(output);
    float result = evaluatePostfix(output, x);
    printf("Giá trị của biểu thức với x = %.2f là: %.2f\n", x, result);

    free(output);
    return 0;
}

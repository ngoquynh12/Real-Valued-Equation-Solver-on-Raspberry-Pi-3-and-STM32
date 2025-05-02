#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "findroot.h"
#include "calculator_fixed.h"

#define MAX 256

int main() {
    Token *output;
    char str[MAX];
    int choice;

    printf("Chon che do:\n");
    printf("1. Tim nghiem x cho phuong trinh f(x) = 0\n");
    printf("2. Tinh gia tri bieu thuc voi x da cho\n");
    printf("Nhap lua chon (1 hoac 2): ");
    scanf("%d", &choice);
    getchar();

    printf("Nhap bieu thuc: ");
    fgets(str, MAX, stdin);
    str[strcspn(str, "\n")] = 0;

    output = InfixToPostfix(str);
    if (output == NULL) {
        printf("Loi chuyen doi bieu thuc!\n");
        return 1;
    }

    printTokens(output);

    if (choice == 1) {
        struct timespec start, end;

        // --- Secant ---
        clock_gettime(CLOCK_MONOTONIC, &start);
        double resultSecant = secantMethod(output);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_secant = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        if (fabs(resultSecant - roundf(resultSecant)) < TOL)
            resultSecant = roundf(resultSecant);

        // // --- Bisection ---
        clock_gettime(CLOCK_MONOTONIC, &start);
        double resultBisection = bisectionMethod(output);
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed_bisection = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        // // --- Newton ---
         clock_gettime(CLOCK_MONOTONIC, &start);
         double resultNewton = NewtonRaphsonMethod(output);
         clock_gettime(CLOCK_MONOTONIC, &end);
         double elapsed_newton = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        if (fabs(resultBisection - roundf(resultBisection)) < TOL)
          resultBisection = roundf(resultBisection);

        // --- In ket qua ---
        printf("\nSecant = %.7f\n", resultSecant);
        printf("Thoi gian Secant: %.6f giay\n\n", elapsed_secant);

        printf("Bisection = %.7f\n", resultBisection);
         printf("Thoi gian Bisection: %.6f giay\n\n", elapsed_bisection);

         printf("Newton Raphson = %.7f\n", resultNewton);
         printf("Thoi gian Newton: %.6f giay\n", elapsed_newton);
    } 
    else if (choice == 2) {
        float x;
        printf("Nhap gia tri cua x: ");
        scanf("%f", &x);

        float result = evaluatePostfix(output, x);
        printf("Ket qua: f(%.5f) = %.5f\n", x, result);
    } 
    else {
        printf("Lua chon khong hop le!\n");
    }

    free(output);
    return 0;
}
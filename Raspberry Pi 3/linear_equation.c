#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include "findroot.h"
#include "calculator_fixed.h"

#define MAX 256
#define NUM_THREADS 1

int found = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
double best_result = 0.0;

typedef struct {
    Token *postfix;
    double result;
} ThreadData;

void *findrootSecant(void *arg) {
    ThreadData *d = arg;
    d->result = secantMethod(d->postfix);
    pthread_mutex_lock(&mutex);
    if (!found) {
        best_result = d->result;
        found = 1;
        printf("Secant tim duoc nghiem la: %f\n", best_result);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}
void *findrootBisection(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = bisectionMethod(data->postfix);
    pthread_mutex_lock(&mutex);
    if (!found) {
        best_result = data->result;
        found = 1;
        printf("Bisection found root: %f\n", best_result);
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootNewton(void *arg) {
    ThreadData *d = arg;
    d->result = NewtonRaphsonMethod(d->postfix);

    if (!isnan(d->result)) {
        printf("Newton-Raphson tim duoc nghiem: %f\n", d->result);
        pthread_mutex_lock(&mutex);
        if (!found || fabs(d->result) < fabs(best_result)) {
            best_result = d->result;
            found = 1;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    struct timespec start, end;
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

    output = InfixtoPostfix(str);
    if (output == NULL) {
        printf("Loi chuyen doi bieu thuc!\n");
        return 1;
    }

    printTokens(output);

    if (choice == 1) {
        struct timespec start, end;
        double elapsed_secant, elapsed_bisection, elapsed_newton;
        float resultSecant, resultBisection, resultNewton;
        
        //Secant
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_t threadSecant;
        ThreadData dataSecant;
        dataSecant.postfix = output;
        pthread_create(&threadSecant, NULL, findrootSecant, (void *)&dataSecant);
        pthread_join(threadSecant, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_secant = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        resultSecant = dataSecant.result;  // <-- Gán giá trị trả về cho resultSecant
        printf("Thoi gian tim nghiem Secant: %f giay\n\n", elapsed_secant);
        
        //Bisection
        found = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_t threadBisection;
        ThreadData dataBisection;
        dataBisection.postfix = output;
        pthread_create(&threadBisection, NULL, findrootBisection, (void *)&dataBisection);
        pthread_join(threadBisection, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_bisection = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        resultBisection = dataBisection.result;
        printf("Thoi gian tim nghiem Bisection: %f giay\n\n", elapsed_bisection);

        //Newton
        found = 0;
        clock_gettime(CLOCK_MONOTONIC, &start);
        pthread_t threadNewton;
        ThreadData dataNewton;
        dataNewton.postfix = output;
        pthread_create(&threadNewton, NULL, findrootNewton, (void *)&dataNewton);
        pthread_join(threadNewton, NULL);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_newton = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        resultNewton = dataNewton.result;  // <-- Gán giá trị trả về cho resultNewton
        printf("Thoi gian tim nghiem Newton: %f giay\n\n", elapsed_newton);
    
     // Tính sai số của mỗi nghiệm: sai số được tính theo |f(x)| 
     float errorSecant = fabs(evaluatePostfix(output, resultSecant));
     float errorBisection = fabs(evaluatePostfix(output, resultBisection));
     float errorNewton = fabs(evaluatePostfix(output, resultNewton));
     
    // So sánh để chọn nghiệm tốt nhất (nghiệm có sai số nhỏ hơn)
    float best_result = resultSecant;
    float minError = errorSecant;

    if (errorBisection < minError) {
        best_result = resultBisection;
        minError = errorBisection;
    }
    if (errorNewton < minError) {
        best_result = resultNewton;
        minError = errorNewton;
    }

    // Làm tròn nếu kết quả gần với số nguyên (dựa trên ngưỡng TOL)
    if (fabs(best_result - roundf(best_result)) < TOL)
        best_result = roundf(best_result);
    printf("Nghiem tot nhat: x = %.7f\n", best_result);
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

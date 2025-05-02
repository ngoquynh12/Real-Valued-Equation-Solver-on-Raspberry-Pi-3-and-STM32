#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "findroot.h"
#include "calculator_fixed.h"

#define MAX_ITER_1 10000
#define MAX_ITER 10000
#define TOL 1e-12
#define EPSILON 1e-6
#define START -20
#define END 20
#define STEP 0.01

int expressionRequiresPositiveDomain(Token *postfix)
{
    int i = 0;
    /* ký hiệu 'E' cuối mảng postfix (sentinel) */
    while (!(postfix[i].type == OPERATOR && postfix[i].value.operator == 'E'))
    {
        if (postfix[i].type == OPERATOR && postfix[i].value.operator == '^') {
            if (i >= 1 && postfix[i-1].type == OPERAND) {
                double exp = postfix[i-1].value.operand;
                if (fabs(exp - round(exp)) > 1e-6)   /* mũ lẻ → cần miền dương */
                    return 1;
            } else {
                return 1;
            }
        }
        if (postfix[i].type == OPERATOR && postfix[i].value.operator == '/')
            return 1;
        ++i;
    }
    return 0;
}

/* ─────────── PHƯƠNG PHÁP SECANT (đã sửa) ─────────── */
double secantMethod(Token *postfix)
{
    /* 1) Quét miền [START, END] để tìm cặp đổi dấu tốt nhất */
    const double lower = START;
    const double upper = END;
    const double step  = STEP;

    double best_x0 = lower, best_x1 = lower + step;
    double min_fx  = DBL_MAX;
    int    haveBracket = 0;

    for (double x = lower; x <= upper - step; x += step) {
        double fx  = evaluatePostfix(postfix, x);
        double fx2 = evaluatePostfix(postfix, x + step);

        if (!isfinite(fx)  || !isfinite(fx2))        /* bỏ NaN/Inf */
            continue;

        /* Ưu tiên cặp đổi dấu */
        if (fx * fx2 < 0.0) {
            best_x0 = x;
            best_x1 = x + step;
            haveBracket = 1;
            break;                                   /* đủ điều kiện -> dừng quét */
        }

        /* Dự phòng: vị trí |f| nhỏ nhất */
        double absf = fabs(fx);
        if (absf < min_fx) {
            min_fx  = absf;
            best_x0 = x;
            best_x1 = x + step;
        }
    }

    /* Nếu ngay cả |f| nhỏ nhất cũng ≈ 0 → trả ngay */
    if (!haveBracket && min_fx < SCAN_TOL)
        return best_x0;

    /* 2) Khởi tạo Secant */
    double x0 = best_x0, x1 = best_x1;
    double f0 = evaluatePostfix(postfix, x0);
    double f1 = evaluatePostfix(postfix, x1);
    if (!isfinite(f0) || !isfinite(f1))
        return NAN;

    /* 3) Vòng lặp Secant */
    double x2 = x1;
    for (int iter = 0; iter < MAX_ITER; ++iter) {
        double denom = f1 - f0;
        if (fabs(denom) < 1e-14)
            denom = copysign(1e-14, denom);          /* tránh chia 0 */

        double x2_new = x1 - f1 * (x1 - x0) / denom;

        /* ép trong biên quét */
        if (x2_new < lower) x2_new = lower;
        if (x2_new > upper) x2_new = upper;

        double f2 = evaluatePostfix(postfix, x2_new);
        if (!isfinite(f2))
            break;                                   /* không thể tiến xa hơn */

        if (fabs(x2_new - x1) < TOL) {               /* hội tụ vị trí */
            x2 = x2_new;
            break;
        }

        /* cập nhật */
        x0 = x1; f0 = f1;
        x1 = x2_new; f1 = f2;
        x2 = x2_new;
    }

    /* 4) Tinh chỉnh thêm vài bước secant hẹp quanh nghiệm */
    double f_final = evaluatePostfix(postfix, x2);
    if (isfinite(f_final) && fabs(f_final) < 1e-9) {
        double r0 = x2 - step, r1 = x2 + step;
        double fr0 = evaluatePostfix(postfix, r0);
        double fr1 = evaluatePostfix(postfix, r1);

        for (int i = 0; i < 5; ++i) {
            double d = fr1 - fr0;
            if (fabs(d) < 1e-14) break;

            double rr = r1 - fr1 * (r1 - r0) / d;
            if (rr < lower || rr > upper) break;

            double frr = evaluatePostfix(postfix, rr);
            if (!isfinite(frr)) break;

            r0 = r1; fr0 = fr1;
            r1 = rr; fr1 = frr;
        }

        double refined = r1;
        /* nếu gần số nguyên → làm tròn cho đẹp */
        if (fabs(refined - round(refined)) < 1e-5)
            return round(refined);
        return refined;
    }

    /* 5) Trả kết quả cuối cùng */
    return x2;
}

// //Bisection
// // Function to find an interval containing a root
int findRootInterval(Token *postfix, float *a, float *b) {
    float prev_x = START;
     float prev_f = evaluatePostfix(postfix, prev_x);
     float best_x   = prev_x;
     float best_fx  = fabsf(prev_f);
     for (float x = START + STEP; x <= END; x += STEP) {
        float f_x = evaluatePostfix(postfix, x);
        float abs_fx = fabsf(f_x);
      if (prev_f * f_x < 0) {  // Sign change indicates a root in [prev_x, x]
            *a = prev_x;
            *b = x;
         return 1;
        }
        if (abs_fx < best_fx) {
            best_fx = abs_fx;
            best_x  = x;
        }
        prev_x = x;
       prev_f = f_x;
   }
   if (best_fx < EPSILON) {
    *a = best_x - STEP;
    *b = best_x + STEP;
    return 1;
}
     printf("No root interval found!\n");
     return 0;  // No root found
 } 


// // Bisection method to find the root
 float bisectionMethod(Token *postfix) {
     float a, b;
     if (!findRootInterval(postfix, &a, &b)) {
         return NAN;     }

    float fa = evaluatePostfix(postfix, a);
     float fb = evaluatePostfix(postfix, b);
   
    
    if (fa * fb > 0) {
         printf("Error: f(a) and f(b) must have opposite signs!\n");
         return NAN;
     }

   

     int iter = 0;
     float c, fc;

     while ((b - a) > EPSILON && iter < MAX_ITER_1) {
         c = (a + b) / 2;
         fc = evaluatePostfix(postfix, c);
         if (fabs(fc) < EPSILON) { 
            return c;
         }    
         if (fa * fc < 0) { 
            b = c;
             fb = fc;
         } else { 
             a = c;
             fa = fc;
         }

         iter++;
     }

//     printf("Bisection method stopped at x = %.4f after %d iterations\n", (a + b) / 2, iter);
     return (a + b) / 2;
 }

// //Newton 
// // Tính đạo hàm gần đúng f'(x)
double derivative(Token *postfix, double x) {
    double h = 1e-6;
    return (evaluatePostfix(postfix, x + h) - evaluatePostfix(postfix, x - h)) / (2 * h);
}

double NewtonRaphsonMethod (Token *postfix){
    if(postfix == NULL){
        printf("Loi chua nhap bieu thuc!");
        return NAN;
    }
    int positiveDomain = expressionRequiresPositiveDomain(postfix);
   
    float x1 = positiveDomain ? 0.0001 : -10;
    float x2 = x1 + 0.5;
    int rep = 0;
    int found_interval = 0;
    while(rep < MAX_ITER){
        float fa = evaluatePostfix(postfix, x1);
        float fb = evaluatePostfix(postfix, x2);
       
        if(isnan(fa)||isnan(fb)){
            x1 = x2;
            x2 += 0.5;
            rep++;
            continue;
        }
        if (fa == 0) {
            //printf("Khoang cach ly nghiem: [%.3f, %.3f]\n", x1, x1);
            return x1;
        }
        if (fb == 0) {
            //printf("Khoang cach ly nghiem: [%.3f, %.3f]\n", x2, x2);
            return x2;
        }
        if (fa*fb < 0){
            found_interval = 1;   
            break;
        }
        x1 = x2;
        x2 += 0.5;
        rep++;
    }
    if(!found_interval){
        //printf("Khong tim thay khoang cach ly nghiem!\n");
        return NAN;
    }
    //printf("Khoang cach ly nghiem: [%.3f, %.3f]\n", x1, x2);
    float x0 = (x1+x2)/2;
    rep = 0;
    while(rep < MAX_ITER){
        if(positiveDomain && x0 <=0) x0 = 0.00001; 
        float fx = evaluatePostfix(postfix, x0);
        if (fabs(fx) < SCAN_TOL){
            return x0;
        }
        float dfx = derivative(postfix,x0);

        int retry = 0;
        while(fabs(dfx) < SCAN_TOL && retry < 5){
            printf("Dao ham gan 0 tai x = %.5f, thu diem khac\n", x0);
            x0 += 0.1;
            if (positiveDomain && x0 <= 0) x0 = 0.0001;  // Đảm bảo x > 0
            dfx = derivative(postfix, x0);
            retry++;
        }
       
        if(fabs(dfx) < SCAN_TOL){
            printf("Newton-Raphson that bai: Dao ham qua nho!\n");
            return NAN;
        }
        float x1_new = x0 - (fx / dfx);

        if (positiveDomain && x1_new <= 0) x1_new = 0.0001;  // Đảm bảo x > 0
        if (fabs(x1_new - x0) < SCAN_TOL) {
            return x1_new;
        }
        x0 = x1_new;
        rep++;
    }
    printf("Newton-Raphson that bai: Khong hoi tu sau %d lan lap.\n", MAX_ITER);
    return NAN;
}
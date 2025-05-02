#ifndef FINDROOT_H
#define FINDROOT_H

#include "calculator_fixed.h"

#define MAX_ITER 10000
#define TOL      1e-12
#define SCAN_TOL 1e-6

int expressionRequiresPositiveDomain(Token *postfix);
double secantMethod(Token *postfix);
int expressionRequiresPositiveDomain(Token *postfix);
float bisectionMethod(Token *postfix);
int findRootInterval(Token *postfix, float *a, float *b);
double derivative(Token *postfix, double x);
double NewtonRaphsonMethod (Token *postfix);

#endif

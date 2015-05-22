#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define min(a,b) a<b ? a : b

/*
*	Newton Interpolation Polynomial translated from cpa.m (Matlab version)
*	To execute the program you just need to pass as first argument the name
*	of the csv file that contain the data.
*	ex: ./program selection_sort.c_15.csv
*/


/*Complexity polynomial analiser (Function cpa)
%
%   Input parameters:
%      x: vector of abscissas,
%      y: vector of ordinates.
%
%   Output parameters:
%      n: degree of the polynomial,
%      C: coefficients of the polynomial, such that,
%         P(x) = C(1)x^n + C(2)x^(n-1) + C(3)x^(n-2) +...+ C(2)x + C(1),
%      SumRes: summation of abs((y_k-P(x_k))/y_k).
%*/

double fabs(double value){
    if (value < 0){
        return value*(-1);
    }
    return value;
}

double MaxAbs(double *values, int size){
    double max = fabs(values[0]);
    for (int i = 1; i < size; i++){
        if (fabs(values[i]) > max){
            max = fabs(values[i]);
        }
    }
    return max;
}


double *cpa(double *x, double *y, int size, int *n, double *SumRes){
	const int n_max = 5;
   	const double eps =   2.22045e-016;
	double tol = MaxAbs(y, size) * eps;
	int m = size;
	int minor = min(m-1, n_max);
	double *Dely = (double*)malloc(size*sizeof(double));
	int np1;
	double *C = (double*)malloc(size*sizeof(double));

	for (int k = 0; k < m; k++)
		Dely[k] = y[k];
//  calculating divided differences
	int k = 0;
	short difzero = 0;
	int saida = 0;
	while ((k <= minor) && (!difzero)){
		k++;
		int kp1 = k+1;
		difzero = 1;
		for (int i = m-1; i+1 >= kp1; i--){ //loop em C
			Dely[i] = (Dely[i] - Dely[i-1])/(x[i] - x[i-k]);
			difzero = difzero & fabs(Dely[i]) < tol;
		}
		//printf("%2d %2i", k, difzero);
		/*for (int i = m-1; i+1 >= k+1; i--){
			printf("%10.2e",Dely[i]); 

		}
		printf("\n");*/
	}

	if (difzero && (k<m)){
		*n = k - 1; //  degree of the polynomial
		np1 = k;
	}
	else{
	     if (k == m){
		fprintf(stderr,"Warning: considering the number of points can not be\n");
      		fprintf(stderr,"         defined if the algorithm is polynomial.\n");
      		fprintf(stderr,"         n = %d.\n",k-1);
      	      }else{
      		fprintf(stderr,"Warning: the algorithm seems not to be polynomial,\n");
      		fprintf(stderr,"         however, if it is then the order > %d\n",n_max);
	      }
		*n = 0;
		C[0] = 0;
		*SumRes = 0;
		free(Dely);
		return 0;
	}

	// Calculating coeffiecients of the polynomial
	for (int k = *n; k >= 1; k--) { 
    	for (int i = k; i <= *n; i++) {
    		Dely[i - 1] -= Dely[i] * x[k - 1];
    	}
    }

    for (int i = 1; i <= np1; i++) {
	    C[i - 1] = Dely[np1 - i];
    }
    //calculating summation of abs((y_k-P(x_k))/y_k)
    *SumRes = 0;
    double t;
    for (int k = 0; k < m; k++){
    	//evaluating the polynomial at x_k
    	t = C[0];
    	for (int i = 1; i < np1; i++){
    		t = t * x[k] + C[i];
    	}
    	*SumRes += fabs(1 - t / y[k]);
    }
    if (*SumRes > tol)
    	fprintf(stderr,"Warning: the algorithm can not be polynomial.\n");
    free(Dely);
    //printf("\nSumRes = %g Tolerance = %g\n",*SumRes,tol);
    return C;
}

int gcd(int a, int b) {
    int t;
    while (a) {
        t = a;
        a = b % a;
        b = t;
    }
    return b;
}

double gcd(double *array, int size) {
    int ans = (int) array[0] * 10000;
    for(int i=1; i < size; i++) {
        ans = gcd(ans, (int) array[i] * 10000);
    }
    return ((double)ans) / 10000;
}

void showResults(int complexidade, double *C, int SumRes, char *variable, int Const) {
    int k = complexidade;
	int i;	
	size_t ln = strlen(variable) - 1;
	if (variable[ln] == '\n')
	    variable[ln] = '\0';
	if (complexidade>1)
    	    printf("O(%s^%d)\n",variable,complexidade);
        else if(complexidade == 1)
    	    printf("O(%s)\n",variable);

}

void showPolynomial(int complexidade, double *C, int SumRes, char *variable, int Const){
    double d = gcd(C, complexidade);
    int k = complexidade;
    int i;
    if (complexidade) {
        printf("Polynomial = ");
        for (i = 0; i <= complexidade; i++) {
            double c = C[i] / d;
            if (c == 0) {
                k--;
                continue;
            }
            if (k == 1) {
                printf("%.2lf * %s + ", c, variable);  
            } else if (i < complexidade) {
                printf("%.2lf * %s^%d + ", c, variable, k);
            } else {
                printf("%.2lf\n", c);
            }
            k--;
        }
    }
}

short check_initial(int argments){
	if (argments != 2){
		printf("Syntax: ./program FILE.CSV\n");
		return 1;
	}
	return 0;
}

int main(int argc, char **argv){
    if (check_initial(argc)) return 1;
    FILE *csv;
    csv = fopen(argv[1], "r");
    int degree = -1;
    double f1 = 0,f2 = 0;
    double SumRes = 0;
    char variable[100];
    int size = 0;
    int i = 0;
    
    fgets(variable, 100 ,csv);
    fscanf(csv,"%d", &size);
    double *Xaxis = (double*)malloc(size*sizeof(double));
    double *Yaxis = (double*)malloc(size*sizeof(double));
    
    while (fscanf(csv, "%lf,%lf\n", &f1, &f2) == 2){
        Xaxis[i] = f1;
        Yaxis[i] = f2;
        i++;
        if (i >= size) break;
        if (feof(csv)){
            printf("\nErro: Numero de linhas passado é superior a quantidade de pontos listados\n");
            return 1;
        }
    }
    
    double *C = cpa(Xaxis, Yaxis, size, &degree, &SumRes);
    showResults(degree, C, SumRes, variable, 0);
    showPolynomial(degree, C, SumRes, variable, 0);
    free(C);
    
    free(Xaxis);
    free(Yaxis);
    fclose(csv);
    return 0;
}


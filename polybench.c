#include <math.h>

void kernel_correlation(int m, int n,
			float float_n,
			float **data,
			float **corr,
			float *mean,
			float *stddev)
{
  int i, j, k;

  float eps = 0.1;

  for (j = 0; j < m; j++)
    {
      mean[j] = 0.0;
      for (i = 0; i < n; i++)
	mean[j] += data[i][j];
      mean[j] /= float_n;
    }


   for (j = 0; j < m; j++)
    {
      stddev[j] = 0.0;
      for (i = 0; i < n; i++)
        stddev[j] += (data[i][j] - mean[j]) * (data[i][j] - mean[j]);
      stddev[j] /= float_n;
      stddev[j] = stddev[j];
      /* The following in an inelegant but usual way to handle
         near-zero std. dev. values, which below would cause a zero-
         divide. */
      stddev[j] = stddev[j] <= eps ? 1.0 : stddev[j];
    }

  /* Center and reduce the column vectors. */
  for (i = 0; i < n; i++)
    for (j = 0; j < m; j++)
      {
        data[i][j] -= mean[j];
        data[i][j] /= sqrt(float_n) * stddev[j];
      }

  /* Calculate the m * m correlation matrix. */
  for (i = 0; i < m-1; i++)
    {
      corr[i][i] = 1.0;
      for (j = i+1; j < m; j++)
        {
          corr[i][j] = 0.0;
          for (k = 0; k < n; k++)
            corr[i][j] += (data[k][i] * data[k][j]);
          corr[j][i] = corr[i][j];
        }
    }
  corr[m-1][m-1] = 1.0;
}

void kernel_covariance(int m, int n,
		       float float_n,
		       float **data,
		       float **cov,
		       float *mean)
{
  int i, j, k;

  for (j = 0; j < m; j++)
    {
      mean[j] = 0.0;
      for (i = 0; i < n; i++)
        mean[j] += data[i][j];
      mean[j] /= float_n;
    }

  for (i = 0; i < n; i++)
    for (j = 0; j < m; j++)
      data[i][j] -= mean[j];

  for (i = 0; i < m; i++)
    for (j = i; j < m; j++)
      {
        cov[i][j] = 0.0;
        for (k = 0; k < n; k++)
	  cov[i][j] += data[k][i] * data[k][j];
        cov[i][j] /= (float_n - 1.0);
        cov[j][i] = cov[i][j];
      }
}

void kernel_gemm(int ni, int nj, int nk,
		 float alpha,
		 float beta,
		 float **C,
		 float **A,
		 float **B)
{
  int i, j, k;

  for (i = 0; i < ni; i++) {
    for (j = 0; j < nj; j++)
	C[i][j] *= beta;
    for (k = 0; k < nk; k++) {
       for (j = 0; j < nj; j++)
	  C[i][j] += alpha * A[i][k] * B[k][j];
    }
  }

}

void kernel_gemver(int n,
		   float alpha,
		   float beta,
		   float **A,
		   float *u1,
		   float *v1,
		   float *u2,
		   float *v2,
		   float *w,
		   float *x,
		   float *y,
		   float *z)
{
  int i, j;

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      A[i][j] = A[i][j] + u1[i] * v1[j] + u2[i] * v2[j];

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      x[i] = x[i] + beta * A[j][i] * y[j];

  for (i = 0; i < n; i++)
    x[i] = x[i] + z[i];

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      w[i] = w[i] +  alpha * A[i][j] * x[j];
}

void kernel_gesummv(int n,
		    int alpha,
		    int beta,
		    float **A,
		    float **B,
		    float *tmp,
		    float *x,
		    float *y)
{
  int i, j;

  for (i = 0; i < n; i++)
    {
      tmp[i] = 0.0;
      y[i] = 0.0;
      for (j = 0; j < n; j++)
	{
	  tmp[i] = A[i][j] * x[j] + tmp[i];
	  y[i] = B[i][j] * x[j] + y[i];
	}
      y[i] = alpha * tmp[i] + beta * y[i];
    }
}

void kernel_symm(int m, int n,
		 float alpha,
		 float beta,
		 float **C,
		 float **A,
		 float **B)
{
  int i, j, k;
  float temp2;

   for (i = 0; i < m; i++)
      for (j = 0; j < n; j++ )
      {
        temp2 = 0;
        for (k = 0; k < i; k++) {
           C[k][j] += alpha*B[i][j] * A[i][k];
           temp2 += B[k][j] * A[i][k];
        }
        C[i][j] = beta * C[i][j] + alpha*B[i][j] * A[i][i] + alpha * temp2;
     }

}

void kernel_syr2k(int n, int m,
		  float alpha,
		  float beta,
		  float **C,
		  float **A,
		  float **B)
{
  int i, j, k;


  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      C[i][j] *= beta;
  for (i = 0; i < n; i++)
    for (k = 0; k < m; k++) {
      for (j = 0; j < n; j++)
	{
	  C[i][j] += A[j][k] * alpha*B[i][k] + B[j][k] * alpha*A[i][k];
	}
     }

}

void kernel_syrk(int n, int m,
		 float alpha,
		 float beta,
		 float **C,
		 float **A)
{
  int i, j, k;

  for (i = 0; i < n; i++) {
    for (j = 0; j <= i; j++)
      C[i][j] *= beta;
    for (k = 0; k < m; k++) {
      for (j = 0; j <= i; j++)
        C[i][j] += alpha * A[i][k] * A[j][k];
    }
  }

}

void kernel_trmm(int m, int n,
		 float alpha,
		 float **A,
		 float **B)
{
  int i, j, k;
  float temp;

  for (i = 0; i < m; i++)
     for (j = 0; j < n; j++) {
        for (k = i+1; k < m; k++) 
           B[i][j] += A[k][i] * B[k][j];
        B[i][j] = alpha * B[i][j];
     }

}

void kernel_2mm(int ni, int nj, int nk, int nl, int alpha, int beta,
                float **tmp, float **A, float **B, float **C, float **D) {                                                                                
  /* D := alpha*A*B*C + beta*D */                                                
  for (int i = 0; i < ni; i++)                                                   
    for (int j = 0; j < nj; j++) {                                                                          
      tmp[i][j] = 0;                                                               
      for (int k = 0; k < nk; ++k)                                                 
        tmp[i][j] += alpha * A[i][k] * B[k][j];
    }

  for (int i = 0; i < ni; i++)                                                   
    for (int j = 0; j < nl; j++) {
      D[i][j] *= beta;                                                             
      for (int k = 0; k < nj; ++k)                                                 
        D[i][j] += tmp[i][k] * C[k][j];
    }                                                                             
}

void kernel_3mm(int ni, int nj, int nk, int nl, int nm,
		float **E,
		float **A,
		float **B,
		float **F,
		float **C,
		float **D,
		float **G)
{
  int i, j, k;

  /* E := A*B */
  for (i = 0; i < ni; i++)
    for (j = 0; j < nj; j++)
      {
	E[i][j] = 0.0;
	for (k = 0; k < nk; ++k)
	  E[i][j] += A[i][k] * B[k][j];
      }
  /* F := C*D */
  for (i = 0; i < nj; i++)
    for (j = 0; j < nl; j++)
      {
	F[i][j] = 0.0;
	for (k = 0; k < nm; ++k)
	  F[i][j] += C[i][k] * D[k][j];
      }
  /* G := E*F */
  for (i = 0; i < ni; i++)
    for (j = 0; j < nl; j++)
      {
	G[i][j] = 0.0;
	for (k = 0; k < nj; ++k)
	  G[i][j] += E[i][k] * F[k][j];
      }

}

void kernel_atax(int m, int n,
		 float **A,
		 float *x,
		 float *y,
		 float *tmp)
{
  int i, j;

  for (i = 0; i < n; i++)
    y[i] = 0;
  for (i = 0; i < m; i++)
    {
      tmp[i] = 0.0;
      for (j = 0; j < n; j++)
	tmp[i] = tmp[i] + A[i][j] * x[j];
      for (j = 0; j < n; j++)
	y[j] = y[j] + A[i][j] * tmp[i];
    }
}

void kernel_bicg(int m, int n,
		 float **A,
		 float *s,
		 float *q,
		 float *p,
		 float *r)
{
  int i, j;

  for (i = 0; i < m; i++)
    s[i] = 0;
  for (i = 0; i < n; i++)
    {
      q[i] = 0.0;
      for (j = 0; j < m; j++)
	{
	  s[j] = s[j] + r[i] * A[i][j];
	  q[i] = q[i] + A[i][j] * p[j];
	}
    }

}

void kernel_doitgen(int nr, int nq, int np,
		    float ***A,
		    float **C4,
		    float *sum)
{
  int r, q, p, s;

  for (r = 0; r < nr; r++)
    for (q = 0; q < nq; q++)  {
      for (p = 0; p < np; p++)  {
	sum[p] = 0.0;
	for (s = 0; s < np; s++)
	  sum[p] += A[r][q][s] * C4[s][p];
      }
      for (p = 0; p < np; p++)
	A[r][q][p] = sum[p];
    }
}

void kernel_mvt(int n,
		float *x1,
		float *x2,
		float *y_1,
		float *y_2,
		float **A)
{
  int i, j;

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      x1[i] = x1[i] + A[i][j] * y_1[j];
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++)
      x2[i] = x2[i] + A[j][i] * y_2[j];

}

void kernel_cholesky(int n,
		     float **A)
{
  int i, j, k;

  for (i = 0; i < n; i++) {
     //j<i
     for (j = 0; j < i; j++) {
        for (k = 0; k < j; k++) {
           A[i][j] -= A[i][k] * A[j][k];
        }
        A[i][j] /= A[j][j];
     }
     // i==j case
     for (k = 0; k < i; k++) {
        A[i][i] -= A[i][k] * A[i][k];
     }
     A[i][i] = sqrt(A[i][i]);
  }

}

void kernel_durbin(int n,
		   float *r,
		   float *y)
{
 float z[n];
 float alpha;
 float beta;
 float sum;

 int i,k;

 y[0] = -r[0];
 beta = 1.0;
 alpha = -r[0];

 for (k = 1; k < n; k++) {
   beta = (1-alpha*alpha)*beta;
   sum = 0.0;
   for (i=0; i<k; i++) {
      sum += r[k-i-1]*y[i];
   }
   alpha = - (r[k] + sum)/beta;

   for (i=0; i<k; i++) {
      z[i] = y[i] + alpha*y[k-i-1];
   }
   for (i=0; i<k; i++) {
     y[i] = z[i];
   }
   y[k] = alpha;
 }

}

void kernel_gramschmidt(int m, int n,
			float **A,
			float **R,
			float **Q)
{
  int i, j, k;

  float nrm;

  for (k = 0; k < n; k++)
    {
      nrm = 0.0;
      for (i = 0; i < m; i++)
        nrm += A[i][k] * A[i][k];
      R[k][k] = sqrt(nrm);
      for (i = 0; i < m; i++)
        Q[i][k] = A[i][k] / R[k][k];
      for (j = k + 1; j < n; j++)
	{
	  R[k][j] = 0.0;
	  for (i = 0; i < m; i++)
	    R[k][j] += Q[i][k] * A[i][j];
	  for (i = 0; i < m; i++)
	    A[i][j] = A[i][j] - Q[i][k] * R[k][j];
	}
    }

}

void kernel_lu(int **A, int n) {
  for (int k = 0; k < n; k++) {                                                                        
    for (int j = k + 1; j < n; j++)
      A[k][j] = A[k][j] / A[k][k];
      for(int i = k + 1; i < n; i++)
    for (int j = k + 1; j < n; j++)
      A[i][j] = A[i][j] + A[i][k] * A[k][j];
  }
}

void kernel_ludcmp(int n,
		   float **A,
		   float *b,
		   float *x,
		   float *y)
{
  int i, j, k;

  float w;

  for (i = 0; i < n; i++) {
    for (j = 0; j <i; j++) {
       w = A[i][j];
       for (k = 0; k < j; k++) {
          w -= A[i][k] * A[k][j];
       }
        A[i][j] = w / A[j][j];
    }
   for (j = i; j < n; j++) {
       w = A[i][j];
       for (k = 0; k < i; k++) {
          w -= A[i][k] * A[k][j];
       }
       A[i][j] = w;
    }
  }

  for (i = 0; i < n; i++) {
     w = b[i];
     for (j = 0; j < i; j++)
        w -= A[i][j] * y[j];
     y[i] = w;
  }

   for (i = n-1; i >=0; i--) {
     w = y[i];
     for (j = i+1; j < n; j++)
        w -= A[i][j] * x[j];
     x[i] = w / A[i][i];
  }

}

void kernel_trisolv(int n,
		    float **L,
		    float *x,
		    float *b)
{
  int i, j;

  for (i = 0; i < n; i++)
    {
      x[i] = b[i];
      for (j = 0; j <i; j++)
        x[i] -= L[i][j] * x[j];
      x[i] = x[i] / L[i][i];
    }

}

void kernel_deriche(int w, int h, float alpha,
       float **imgIn,
       float **imgOut,
       float **y1,
       float **y2) {
    int i,j;
    float xm1, tm1, ym1, ym2;
    float xp1, xp2;
    float tp1, tp2;
    float yp1, yp2;
   
    float k;
    float a1, a2, a3, a4, a5, a6, a7, a8;
    float b1, b2, c1, c2;

#pragma scop
   k = (1.0-exp(-alpha))*(1.0-exp(-alpha))/((1.0)+(2.0)*alpha*exp(-alpha)-exp((2.0)*alpha));
   a1 = a5 = k;
   a2 = a6 = k*exp(-alpha)*(alpha-(1.0));
   a3 = a7 = k*exp(-alpha)*(alpha+(1.0));
   a4 = a8 = -k*exp((-2.0)*alpha);
   b1 =  pow((2.0),-alpha);
   b2 = -exp((-2.0)*alpha);
   c1 = c2 = 1;

   for (i=0; i<w; i++) {
        ym1 = (0.0);
        ym2 = (0.0);
        xm1 = (0.0);
        for (j=0; j<h; j++) {
            y1[i][j] = a1*imgIn[i][j] + a2*xm1 + b1*ym1 + b2*ym2;
            xm1 = imgIn[i][j];
            ym2 = ym1;
            ym1 = y1[i][j];
        }
    }

    for (i=0; i<w; i++) {
        yp1 = (0.0);
        yp2 = (0.0);
        xp1 = (0.0);
        xp2 = (0.0);
        for (j=h-1; j>=0; j--) {
            y2[i][j] = a3*xp1 + a4*xp2 + b1*yp1 + b2*yp2;
            xp2 = xp1;
            xp1 = imgIn[i][j];
            yp2 = yp1;
            yp1 = y2[i][j];
        }
    }

    for (i=0; i<w; i++)
        for (j=0; j<h; j++) {
            imgOut[i][j] = c1 * (y1[i][j] + y2[i][j]);
        }

    for (j=0; j<h; j++) {
        tm1 = (0.0);
        ym1 = (0.0);
        ym2 = (0.0);
        for (i=0; i<w; i++) {
            y1[i][j] = a5*imgOut[i][j] + a6*tm1 + b1*ym1 + b2*ym2;
            tm1 = imgOut[i][j];
            ym2 = ym1;
            ym1 = y1 [i][j];
        }
    }
    

    for (j=0; j<h; j++) {
        tp1 = (0.0);
        tp2 = (0.0);
        yp1 = (0.0);
        yp2 = (0.0);
        for (i=w-1; i>=0; i--) {
            y2[i][j] = a7*tp1 + a8*tp2 + b1*yp1 + b2*yp2;
            tp2 = tp1;
            tp1 = imgOut[i][j];
            yp2 = yp1;
            yp1 = y2[i][j];
        }
    }

    for (i=0; i<w; i++)
        for (j=0; j<h; j++)
            imgOut[i][j] = c2*(y1[i][j] + y2[i][j]);

}

void kernel_floyd_warshall(int **path, int n) {
  for (int k = 0; k < n; k++) {                                                                            
    for(int i = 0; i < n; i++)                                                 
      for (int j = 0; j < n; j++)                                                  
        path[i][j] = path[i][j] < path[i][k] + path[k][j] ?                        
          path[i][j] : path[i][k] + path[k][j];                                    
  }                                                                                                                                                             
}

/* seq was actually of type base, defined as typedef
 char base. The defines were actually outside the 
 function.*/
void kernel_nussinov(int n, char *seq,
			   float **table)
{
 #define match(b1, b2) (((b1)+(b2)) == 3 ? 1 : 0)
 #define max_score(s1, s2) ((s1 >= s2) ? s1 : s2)
  int i, j, k;

 for (i = n-1; i >= 0; i--) {
  for (j=i+1; j<n; j++) {

   if (j-1>=0) 
      table[i][j] = max_score(table[i][j], table[i][j-1]);
   if (i+1<n) 
      table[i][j] = max_score(table[i][j], table[i+1][j]);

   if (j-1>=0 && i+1<n) {
     /* don't allow adjacent elements to bond */
     if (i<j-1) 
        table[i][j] = max_score(table[i][j], table[i+1][j-1]+match(seq[i], seq[j]));
     else 
        table[i][j] = max_score(table[i][j], table[i+1][j-1]);
   }

   for (k=i+1; k<j; k++) {
      table[i][j] = max_score(table[i][j], table[i][k] + table[k+1][j]);
   }
  }
 }
}

void kernel_adi(int tsteps, int n,
		float **u,
		float **v,
		float **p,
		float **q)
{
  int t, i, j;
  float DX, DY, DT;
  float B1, B2;
  float mul1, mul2;
  float a, b, c, d, e, f;

  DX = (1.0)/(float)n;
  DY = (1.0)/(float)n;
  DT = (1.0)/(float)tsteps;
  B1 = (2.0);
  B2 = (1.0);
  mul1 = B1 * DT / (DX * DX);
  mul2 = B2 * DT / (DY * DY);

  a = -mul1 /  (2.0);
  b = (1.0)+mul1;
  c = a;
  d = -mul2 / (2.0);
  e = (1.0)+mul2;
  f = d;

 for (t=1; t<=tsteps; t++) {
    //Column Sweep
    for (i=1; i<n-1; i++) {
      v[0][i] = (1.0);
      p[i][0] = (0.0);
      q[i][0] = v[0][i];
      for (j=1; j<n-1; j++) {
        p[i][j] = -c / (a*p[i][j-1]+b);
        q[i][j] = (-d*u[j][i-1]+((1.0)+(2.0)*d)*u[j][i] - f*u[j][i+1]-a*q[i][j-1])/(a*p[i][j-1]+b);
      }
      
      v[n-1][i] = (1.0);
      for (j=n-2; j>=1; j--) {
        v[j][i] = p[i][j] * v[j+1][i] + q[i][j];
      }
    }
    //Row Sweep
    for (i=1; i<n-1; i++) {
      u[i][0] = (1.0);
      p[i][0] = (0.0);
      q[i][0] = u[i][0];
      for (j=1; j<n-1; j++) {
        p[i][j] = -f / (d*p[i][j-1]+e);
        q[i][j] = (-a*v[i-1][j]+((1.0)+(2.0)*a)*v[i][j] - c*v[i+1][j]-d*q[i][j-1])/(d*p[i][j-1]+e);
      }
      u[i][n-1] = (1.0);
      for (j=n-2; j>=1; j--) {
        u[i][j] = p[i][j] * u[i][j+1] + q[i][j];
      }
    }
  }
}

void kernel_fdtd_2d(int tmax, int nx, int ny,
                   float **ex, float **ey, float **hz, float *_fict_) {                                                            
  for(int t = 0; t < tmax; t++) {                                                                            
    for (int j = 0; j < ny; j++)                                               
      ey[0][j] = _fict_[t];                                                        
    for (int i = 1; i < nx; i++)                                               
      for (int j = 0; j < ny; j++)                                                 
        ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);                           
    for (int i = 0; i < nx; i++)                                               
      for (int j = 1; j < ny; j++)                                                 
        ex[i][j] = ex[i][j] - 0.5*(hz[i][j]-hz[i][j-1]);                           
    for (int i = 0; i < nx - 1; i++)                                           
      for (int j = 0; j < ny - 1; j++)                                             
        hz[i][j] = hz[i][j] - 0.7*  (ex[i][j+1] - ex[i][j] +                       
                       ey[i+1][j] - ey[i][j]);                                   
    }
}

void kernel_heat_3d(int tsteps,
		      int n,
		      float ***A,
		      float ***B)
{
  int t, i, j, k;

    for (t = 1; t <= tsteps; t++) {
        for (i = 1; i < n-1; i++) {
            for (j = 1; j < n-1; j++) {
                for (k = 1; k < n-1; k++) {
                    B[i][j][k] =   (0.125) * (A[i+1][j][k] - (2.0) * A[i][j][k] + A[i-1][j][k])
                                 + (0.125) * (A[i][j+1][k] - (2.0) * A[i][j][k] + A[i][j-1][k])
                                 + (0.125) * (A[i][j][k+1] - (2.0) * A[i][j][k] + A[i][j][k-1])
                                 + A[i][j][k];
                }
            }
        }
        for (i = 1; i < n-1; i++) {
           for (j = 1; j < n-1; j++) {
               for (k = 1; k < n-1; k++) {
                   A[i][j][k] =   (0.125) * (B[i+1][j][k] - (2.0) * B[i][j][k] + B[i-1][j][k])
                                + (0.125) * (B[i][j+1][k] - (2.0) * B[i][j][k] + B[i][j-1][k])
                                + (0.125) * (B[i][j][k+1] - (2.0) * B[i][j][k] + B[i][j][k-1])
                                + B[i][j][k];
               }
           }
       }
    }
}

void kernel_jacobi_1d(int tsteps,
			    int n,
			    float *A,
			    float *B)
{
  int t, i;

  for (t = 0; t < tsteps; t++)
    {
      for (i = 1; i < n - 1; i++)
	B[i] = 0.33333 * (A[i-1] + A[i] + A[i + 1]);
      for (i = 1; i < n - 1; i++)
	A[i] = 0.33333 * (B[i-1] + B[i] + B[i + 1]);
    }
}

void kernel_jacobi_2d(int n, int tsteps, float **A, float **B) {
  for (int t = 0; t < tsteps; t++) {
    for (int i = 1; i < n - 1; i++)
      for (int j = 1; j < n - 1; j++)
        B[i][j] = 0.2 * (A[i][j] + A[i][j-1] + A[i][1+j] 
                         + A[1+i][j] + A[i-1][j]);
 
    for (int i = 1; i < n-1; i++)
      for (int j = 1; j < n-1; j++)
        A[i][j] = B[i][j];
  }
}

void kernel_seidel_2d(int n, int tsteps, float **A) {
  for (int t = 0; t <= tsteps - 1; t++)
    for (int i = 1; i <= n - 2; i++)
      for (int j = 1; j <= n - 2; j++)
        A[i][j] = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1]
           + A[i][j-1] + A[i][j] + A[i][j+1]
           + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/9.0;
}


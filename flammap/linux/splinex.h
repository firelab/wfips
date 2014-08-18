#ifndef SPLINEX
#define SPLINEX

void spline(double x[], double y[], int n, double yp1, double ypn, double y2[]);
void splint(double xa[], double ya[], double y2a[], long n, double x, double *y);
void ratint(double xa[], double ya[], long n, double x, double *y, double *dy);
void polint(float xa[], float ya[], long n, float x, float *y, float *dy);
void locate(float xx[], unsigned long n, float x, unsigned long *j);

#endif // SPLINEX

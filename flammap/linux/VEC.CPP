#include "stdafx.h"
#include "vec.h"

vec operator+(vec &u, vec &v)
{
	return vec(u.x+v.x, u.y+v.y);
}

vec operator-(vec &u, vec &v)
{
	return vec(u.x-v.x, u.y-v.y);
}

vec operator*(double c, vec &v)
{
	return vec(c*v.x, c*v.y);
}

vec &operator+=(vec &u, vec &v)
{
	u.x+=v.x;
	u.y+=v.y;

	return u;
}

vec &operator-=(vec &u, vec &v)
{
	u.x-=v.x;
	u.y-=v.y;

	return u;
}

vec &operator*=(vec &v, double c)
{
	v.x*=c;
	v.y*=c;

	return v;
}

int Orientation(vec &P, vec&Q, vec &R)
//int Orientation(double x1, double y1, double x2, double y2, double x3, double y3);
{
	const double EPS=1e-6;
//	vec P(x1, y1);
//	vec Q(x2, y2);		// first
//	vec R(x3, y3);		// second
	vec A, B;
	double determinant;

	A=Q-P;
	B=R-P;
	determinant=A.x*B.y-A.y*B.x;

	return (determinant < -EPS ? -1 : determinant > EPS);
}

int Inside(vec &P, vec &A, vec &B, vec &C)
{// P lies in side triangle ABC
	int OrientABP = Orientation(A,B,P);

	return (Orientation(B, C, P) == OrientABP &&
		   Orientation(C, A, P) == OrientABP);
}




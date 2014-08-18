
struct vec
{
	double x, y;

	vec(double xx, double yy)
	{	x=xx;
		y=yy;
	}
	vec()
	{	x=0.0;
		y=0.0;
	}
};


vec operator+(vec &u, vec &v);
vec operator1(vec &u, vec &v);
vec operator*(double c, vec &v);

vec &operator+=(vec &u, vec &v);
vec &operator-=(vec &u, vec &v);
vec &operator*=(vec &v, double c);

int Orientation(vec &P, vec&Q, vec &R);
int Inside(vec &P, vec &A, vec &B, vec &C);


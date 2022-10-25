#include "perlin_noise.h"
#include "math.h"

// I use these in many of my programs now...

PerlinNoise::PerlinNoise()
{
  persistence = 0;
  frequency = 0;
  amplitude  = 0;
  octaves = 0;
  randomseed = 0;
}

PerlinNoise::PerlinNoise(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed)
{
  persistence = _persistence;
  frequency = _frequency;
  amplitude  = _amplitude;
  octaves = _octaves;
  randomseed = 2 + _randomseed * _randomseed;
}

void PerlinNoise::Set(double _persistence, double _frequency, double _amplitude, int _octaves, int _randomseed)
{
  persistence = _persistence;
  frequency = _frequency;
  amplitude  = _amplitude;
  octaves = _octaves;
  randomseed = 2 + _randomseed * _randomseed;
}

double PerlinNoise::GetHeight(double x, double y) const
{
  return amplitude * Total(x, y);
}

double PerlinNoise::Total(double i, double j) const
{
    //properties of one octave (changing each loop)
    double t = 0.0f;
    double _amplitude = 1;
    double freq = frequency;

    for(int k = 0; k < octaves; k++) 
    {
        t += GetValue(j * freq + randomseed, i * freq + randomseed) * _amplitude;
        _amplitude *= persistence;
        freq *= 2;
    }

    return t;
}

double PerlinNoise::GetValue(double x, double y) const
{
    int Xint = (int)x;
    int Yint = (int)y;
    double Xfrac = x - Xint;
    double Yfrac = y - Yint;

  //noise values
  double n01 = Noise(Xint-1, Yint-1);
  double n02 = Noise(Xint+1, Yint-1);
  double n03 = Noise(Xint-1, Yint+1);
  double n04 = Noise(Xint+1, Yint+1);
  double n05 = Noise(Xint-1, Yint);
  double n06 = Noise(Xint+1, Yint);
  double n07 = Noise(Xint, Yint-1);
  double n08 = Noise(Xint, Yint+1);
  double n09 = Noise(Xint, Yint);

  double n12 = Noise(Xint+2, Yint-1);
  double n14 = Noise(Xint+2, Yint+1);
  double n16 = Noise(Xint+2, Yint);

  double n23 = Noise(Xint-1, Yint+2);
  double n24 = Noise(Xint+1, Yint+2);
  double n28 = Noise(Xint, Yint+2);

  double n34 = Noise(Xint+2, Yint+2);

    //find the noise values of the four corners
    double x0y0 = 0.0625*(n01+n02+n03+n04) + 0.125*(n05+n06+n07+n08) + 0.25*(n09);  
    double x1y0 = 0.0625*(n07+n12+n08+n14) + 0.125*(n09+n16+n02+n04) + 0.25*(n06);  
    double x0y1 = 0.0625*(n05+n06+n23+n24) + 0.125*(n03+n04+n09+n28) + 0.25*(n08);  
    double x1y1 = 0.0625*(n09+n16+n28+n34) + 0.125*(n08+n14+n06+n24) + 0.25*(n04);  

    //interpolate between those values according to the x and y fractions
    double v1 = Interpolate(x0y0, x1y0, Xfrac); //interpolate in x direction (y)
    double v2 = Interpolate(x0y1, x1y1, Xfrac); //interpolate in x direction (y+1)
    double fin = Interpolate(v1, v2, Yfrac);  //interpolate in y direction

    return fin;
}

double PerlinNoise::Interpolate(double a,double b,double x) const
{
	static double half=0.5f;
	double res;
	__asm {
		fld		qword ptr [x]	// load x
		fldpi					// load PI (FPU's version)
		fmulp	st(1),st		// multiply and just leave result
		fcos					// built in FPU Cosine
		fld1					// built in FPU 1.0000
		fsubrp	st(1),st		// Subtract and just leave result
		fmul	qword ptr [half]// My 0.5
		fstp	qword ptr [res]	// pop and store in res
		fld1					// built in FPU 1.0000
		fsub	qword ptr [res]	// subtract res from 1.0000
		fmul	qword ptr [a]	// multiply by a
		fld		qword ptr [b]	// Load b on stack
		fmul	qword ptr [res]	// multiply b with res
		faddp	st(1),st		// add, leave result in st(1) and pop
		fstp	qword ptr [res]	// store final result in res
	}
	return res;
}

double PerlinNoise::Noise(int x, int y) const
{
	static double v1 = 0.931322574615478515625e-9;
	double res;
	int n;

	__asm {		// going to ignore the ^ power of bit
		// n=x+y*57
		mov		eax,dword ptr [y]
		imul	eax,eax,39h
		add		eax,dword ptr [x]
		mov		ebx,eax				// is n
		// n=(n<<13)^n
		shl		eax,0Dh
		xor		eax,ebx
		mov		ebx,eax				// is n again
		// int t = (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff;
		imul	eax,ebx
		imul	eax,eax,3D73h
		add		eax,0C0aeh
		imul	eax,ebx
		add		eax,5208DD0Dh
		and		eax,7FFFFFFFh
		mov		dword ptr [n],eax
		// return 1.0 - double(t) * 0.931322574615478515625e-9;
		fild	dword ptr [n]
		fmul	qword ptr [v1]
		fld1
		fsubrp	st(1),st
		fstp	qword ptr [res]
	}
	return res;
}

double PerlinNoise::GetHeightTiled(float ox, float oy, int w, int h)
{
	double u,v;
	double n00, n01, n10, n11;
	double x,y;

	double ret;

	x = (double)ox;
	y = (double)oy;

	u = x / (double)w;
	v = y / (double)h;

	n00 = GetHeight(x,y);
	n01 = GetHeight(x,y+(double)h);
	n10 = GetHeight(x+(double)w,y);
	n11 = GetHeight(x+(double)w,y+(double)h);

	ret = ( u*v*n00 + u*(1-v)*n01 + (1-u)*v*n10 + (1-u)*(1-v)*n11 );

	return ret;
}

//////////////////////////////////////////////////////////////////////////
//
// Extensions to the Perlin Noise functions for easier access
//
//////////////////////////////////////////////////////////////////////////

int PerlinNoise::GetHeightByte(float x, float y)
{
	float hgt=( (float)GetHeight( (double)x,(double)y ) + 1.0f ) / 2.0f;
	int ret=(int)( hgt * 255.0f );
	if (ret<0) ret=0; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightByteAbs(float x, float y)
{
	float hgt=fabs( (float)GetHeight( (double)x,(double)y ) );
	int ret=(int)( hgt * 255.0f );
	if (ret<0) ret=0; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightByteTop(float x, float y)
{
	float hgt=(float)GetHeight( (double)x,(double)y );
	int ret=(int)( hgt * 255.0f );
	if (ret<0) ret=0; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightSigned(float x, float y)
{
	float hgt=(float)GetHeight( (double)x,(double)y );
	int ret=(int)( hgt * 255.0f );
	if (ret<255) ret=-255; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightTiledByte(float ox, float oy, int w, int h)
{
	float hgt=( (float)GetHeightTiled(ox,oy,w,h) + 1.0f ) / 2.0f;
	int ret=(int)( hgt * 255.0f );
	if (ret<0) ret=0; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightTiledByteAbs(float ox, float oy, int w, int h)
{
	float hgt=fabs( (float)GetHeightTiled(ox,oy,w,h) );
	int ret=(int)( hgt * 255.0f );
	if (ret<0) ret=0; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightTiledByteTop(float ox, float oy, int w, int h)
{
	float hgt=(float)GetHeightTiled(ox,oy,w,h);
	int ret=(int)( hgt * 255.0f );
	if (ret<0) ret=0; else if (ret>255) ret=255;
	return ret;
}

int PerlinNoise::GetHeightTiledSigned(float ox, float oy, int w, int h)
{
	float hgt=(float)GetHeightTiled(ox,oy,w,h);
	int ret=(int)( hgt * 255.0f );
	if (ret<255) ret=-255; else if (ret>255) ret=255;
	return ret;
}
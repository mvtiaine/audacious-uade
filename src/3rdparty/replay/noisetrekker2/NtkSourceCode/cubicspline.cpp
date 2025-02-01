// SPDX-License-Identifier: LicenseRef-NoiseTrekker2
// CUBIC SPLINE FUNCTION
//
// Arguru
//

class Cubic
{
public:
	Cubic();

	float Work(float yo,float y0,float y1,float y2,unsigned __int32 res, long offset,long length);

	// Work function. Where all is cooked :]
	// yo = y[-1] [sample at x-1]
	// y0 = y[0]  [sample at x (input)]
	// y1 = y[1]  [sample at x+1]
	// y2 = y[2]  [sample at x+2]
	
	// res= distance between two neighboughs sample points [y0 and y1] 
	//      ,so [0...1.0]. You have to multiply this distance * RESOLUTION used
	//      on the  spline conversion table. [256 by default]
    // If you are using 256 is asumed you are using 8 bit decimal
	// fixed point offsets for resampling.

	// offset = sample offset [info to avoid go out of bounds on sample reading ]
	// offset = sample length [info to avoid go out of bounds on sample reading ]

private:
	int RESOLUTION; // Currently is 256, that's enough...
    float at[1024];
	float bt[1024];
	float ct[1024];
	float dt[1024];

};

Cubic::Cubic() /* The resampler constructor */
{
RESOLUTION=1024;
// Initialize table...
for (int i=0;i<RESOLUTION;i++)
{
    float x = (float)i/(float)RESOLUTION;
    at[i] = -0.5*x*x*x+x*x-0.5*x;
    bt[i] = 1.5*x*x*x-2.5*x*x+1;
    ct[i] = -1.5*x*x*x+2*x*x+0.5*x;
    dt[i] = 0.5*x*x*x-0.5*x*x;
}

}

// Work body

float Cubic::Work(float yo,float y0,float y1,float y2,unsigned __int32 res, long offset,long length)
{
	res=res>>22;
	if(offset==0)yo=0;
	if(offset+2>length)y1=0;
	if(offset+3>length)y2=0;

	return at[res]*yo+bt[res]*y0+ct[res]*y1+dt[res]*y2;
}

// Despiste this CubicSpline function, only for debuggin purposes, not optimized
float CubicSpline(float oy1,float y0,float y1, float y2, float x)
{
float a=(3*(y0-y1)-oy1+y2)*0.5;
float b=2*y1+oy1-(5*y0+y2)*0.5;
float c=(y1-oy1)*0.5;
return a*x*x*x+b*x*x+c*x+y0;
}

/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image;
int xs, ys;
int reset = 1;

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
	unsigned char		pixel[3];
	unsigned char		dummy;
	char  				foo[8];
	int   				i;
	FILE				*fd;

	if (reset) {          /* open and load texture file */
		fopen_s(&fd, "texture", "rb");
		if (fd == NULL) {
			fprintf (stderr, "texture file not found\n");
			exit(-1);
		}
		fscanf(fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
		image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
		if (image == NULL) {
			fprintf (stderr, "malloc for texture image failed\n");
			exit(-1);
		}

		for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
			fread(pixel, sizeof(pixel), 1, fd);
			image[i][RED] = (float)((int)pixel[RED]) * (1.0f / 255.0f);
			image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0f / 255.0f);
			image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0f / 255.0f);
		}

		reset = 0;          /* init is done */
		fclose(fd);
	}

	/* bounds-test u,v to make sure nothing will overflow image array bounds */
	if (u > 1.0f || u < 0.0f)
		u = u - (int)u;
	if (v > 1.0f || v < 0.0f)
		v = v - (int)v;

	/* determine texel corner values and perform bilinear interpolation */
	// a *---* b
	//   |   |
	// d *---* c
	int au = (int)floor(u * xs),	av = (int)floor(v * ys);
	int bu = (int)ceil(u * xs),		bv = (int)floor(v * ys);
	int cu = (int)ceil(u * xs),		cv = (int)ceil(v * ys);
	int du = (int)floor(u * xs),	dv = (int)ceil(v * ys);

	// fractional offset between texels
	float s = (u * xs) - au;
	float t = (v * ys) - av;

	// border checking - if the value is outside, then switch it to the other side
	if (au < 0)		au = xs;
	if (au >= xs)	au = 0;
	if (av < 0)		av = ys;
	if (av >= ys)	av = 0;
	
	if (bu < 0)		bu = xs;
	if (bu >= xs)	bu = 0;
	if (bv < 0)		bv = ys;
	if (bv >= ys)	bv = 0;
	
	if (cu < 0)		cu = xs;
	if (cu >= xs)	cu = 0;
	if (cv < 0)		cv = ys;
	if (cv >= ys)	cv = 0;
	
	if (du < 0)		du = xs;
	if (du >= xs)	du = 0;
	if (dv < 0)		dv = ys;
	if (dv >= ys)	dv = 0;
	
	// indexes into array
	int a = au + av*xs;
	int b = bu + bv*xs;
	int c = cu + cv*xs;
	int d = du + dv*xs;

	/* set color to interpolated GzColor value and return */
	color[0] = s*t*image[c][0] + (1-s)*t*image[d][0] + s*(1-t)*image[b][0] + (1-s)*(1-t)*image[a][0];
	color[1] = s*t*image[c][1] + (1-s)*t*image[d][1] + s*(1-t)*image[b][1] + (1-s)*(1-t)*image[a][1];
	color[2] = s*t*image[c][2] + (1-s)*t*image[d][2] + s*(1-t)*image[b][2] + (1-s)*(1-t)*image[a][2];

	return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
	if (reset)
	{
		xs = 512;
		ys = 512;

		image = new GzColor[xs*ys];

		float checkDim = 512.0f / 10.0f;
		for (int x=0; x<512; x++)
		{
			for (int y=0; y<512; y++)
			{
				float val = (float)(sin(((float)x/512.0f)*(checkDim/6.28)) * sin(((float)y/512.0f)*(checkDim/6.28)));
				if (val > 0)
					val = 1.0f;
				else
					val = 0.0f;
				image[(x*512) + y][0] = val;
				image[(x*512) + y][1] = val;
				image[(x*512) + y][2] = val;
			}
		}
		reset = 0;
	}

	tex_fun(u,v,color);

	return GZ_SUCCESS;
}


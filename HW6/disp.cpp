/*   CS580 HW   */
#include	<limits.h>
#include    "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"

FILE* outf_disp = fopen("output_disp.txt", "w");


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* create a framebuffer:
 -- allocate memory for framebuffer : (sizeof)GzPixel x width x height
 -- pass back pointer 
*/
	if (width > MAXXRES)
		width = MAXXRES;
	if (width <= 0)
		width = 1;
	if (height > MAXYRES)
		height = MAXYRES;
	if (height <= 0)
		height = 1;

	//framebuffer = (char**)malloc(sizeof(GzPixel) * width * height);
	*framebuffer = new char[sizeof(GzPixel) * width * height * 3];

	if (!framebuffer)
		return GZ_FAILURE;

	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, GzDisplayClass dispClass, int xRes, int yRes)
{
/* create a display:
  -- allocate memory for indicated class and resolution
  -- pass back pointer to GzDisplay object in display
*/
	*display = new GzDisplay[1];
	display[0]->dispClass = dispClass;
	display[0]->xres = xRes;
	display[0]->yres = yRes;

	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* clean up, free memory */
	free(display->fbuf);
	display->fbuf = NULL;

	fclose(outf_disp);

	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes, GzDisplayClass	*dispClass)
{
/* pass back values for an open display */
	if (!display)
		return GZ_FAILURE;

	*xRes = display->xres;
	*yRes = display->yres;
	*dispClass = display->dispClass;

	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* set everything to some default values - start a new frame */
	GzPixel defaultPixel;
	defaultPixel.alpha = 1;
	defaultPixel.z = INT_MAX;
	defaultPixel.red = 127 << 4;
	defaultPixel.green = 111 << 4;
	defaultPixel.blue = 95 << 4;

	display->fbuf = new GzPixel[display->xres * display->yres];

	for (unsigned short y = 0; y < display->yres; y++)
		for (unsigned short x = 0; x < display->xres; x++)
			memcpy(&display->fbuf[(y * display->xres) + x], &defaultPixel, sizeof(GzPixel));

	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* write pixel values into the display */
	if (i < 0 || i >= display->xres || j < 0 || j >= display->yres)
		return GZ_FAILURE;

	unsigned int index = (j * display->yres) + i; // calculate index into framebuffer once

	// early exit if new pixel is hidden by current pixel
	if (z > display->fbuf[index].z)
		return GZ_FAILURE;
	// discard pixel if z < 0
	if (z < 0)
		return GZ_FAILURE;
		
	Clamp(r, 0, 4095);
	Clamp(g, 0, 4095);
	Clamp(b, 0, 4095);

	display->fbuf[index].red = r;
	display->fbuf[index].green = g;
	display->fbuf[index].blue = b;
	display->fbuf[index].alpha = a;
	display->fbuf[index].z = z;

	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
	/* pass back pixel value in the display */
	/* check display class to see what vars are valid */
	if (i < 0 || i >= display->xres || j < 0 || j >= display->yres)
		return GZ_FAILURE;

	unsigned int index = (i * display->yres) + j; // calculate index into framebuffer once

	*r = display->fbuf[index].red;
	*g = display->fbuf[index].green;
	*b = display->fbuf[index].blue;
	*a = display->fbuf[index].alpha;
	*z = display->fbuf[index].z;

	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{
	/* write pixels to ppm file based on display class -- "P6 %d %d 255\r" */
	if (!outfile)
		return GZ_FAILURE;

	fprintf(outfile, "P6 %d %d 255\r", display->xres, display->yres); // write PPM file header

	// fill PPM file with RGB data
	for (unsigned int i=0; i < ((unsigned int)display->xres * (unsigned int)display->yres); i++)
		fprintf(outfile, "%c%c%c", 
			display->fbuf[i].red >> 4, 
			display->fbuf[i].green >> 4, 
			display->fbuf[i].blue >> 4);

	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{
	/* write pixels to framebuffer: 
		- Put the pixels into the frame buffer
		- Caution: store the pixel to the frame buffer as the order of blue, green, and red 
		- Not red, green, and blue !!!
	*/

	for (unsigned int i=0; i < ((unsigned int)display->xres * (unsigned int)display->yres); i++)
	{
		framebuffer[(i*3)+0] = display->fbuf[i].blue >> 4;
		framebuffer[(i*3)+1] = display->fbuf[i].green >> 4;
		framebuffer[(i*3)+2] = display->fbuf[i].red >> 4;
	}

	return GZ_SUCCESS;
}

inline int Clamp(GzIntensity &c, short min, short max)
{
	if (c > max) 
		c = max;
	else if (c < min)
		c = min;

	return GZ_SUCCESS;
}
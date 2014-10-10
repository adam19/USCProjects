// Application5.cpp: implementation of the Application5 class.
//
//////////////////////////////////////////////////////////////////////

/*
 * application test code for homework assignment #5
*/

#include "stdafx.h"
#include "CS580HW.h"
#include "Application6.h"
#include "Gz.h"
#include "disp.h"
#include "rend.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define INFILE  "ppot.asc"
#define OUTFILE "output.ppm"


float AAFilter[AAKERNEL_SIZE][3] =			/* X, Y, coef */
{
	-0.52f, 0.38f, 0.128f,
	0.41f, 0.56f, 0.119f,
	0.27f, 0.08f, 0.294f,
	-0.17f, -0.29f, 0.249f,
	0.58f, -0.55f, 0.104f,
	-0.31f, -0.71f, 0.106f
};

GzRender** renderers;

extern int tex_fun(float u, float v, GzColor color); /* image texture function */
extern int ptex_fun(float u, float v, GzColor color); /* procedural texture function */

void shade(GzCoord norm, GzCoord color);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Application5::Application5()
{

}

Application5::~Application5()
{
	
}

int Application5::Initialize()
{
	GzCamera	camera;  
	int		    xRes, yRes, dispClass;	/* display parameters */ 

	GzToken		nameListShader[9]; 	    /* shader attribute names */
	GzPointer   valueListShader[9];		/* shader attribute pointers */
	GzToken     nameListLights[10];		/* light info */
	GzPointer   valueListLights[10];
	int			interpStyle;
	float		specpower;
	int			status; 
 
	status = 0; 

	/* 
	 * Allocate memory for user input
	 */
	m_pUserInput = new GzInput;

	/* 
	 * initialize the display and the renderer 
	 */ 
 	m_nWidth = 512;		// frame buffer and display width
	m_nHeight = 512;    // frame buffer and display height

	status |= GzNewFrameBuffer(&m_pFrameBuffer, m_nWidth, m_nHeight);

	status |= GzNewDisplay(&m_pDisplay, GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);
	status |= GzGetDisplayParams(m_pDisplay, &xRes, &yRes, &dispClass); 	 
	status |= GzInitDisplay(m_pDisplay);  
	status |= GzNewRender(&m_pRender, GZ_Z_BUFFER_RENDER, m_pDisplay);

	renderers = new GzRender*[AAKERNEL_SIZE];
	for (int i=0; i<AAKERNEL_SIZE; i++)
	{
		GzDisplay* disp;
		status |= GzNewDisplay(&disp, GZ_RGBAZ_DISPLAY, m_nWidth, m_nHeight);
		/*status |= GzGetDisplayParams(disp, 
									(int*)&disp->xres, 
									(int*)&disp->yres, 
									&disp->dispClass); */	 
		status |= GzInitDisplay(disp);
		status |= GzNewRender(&renderers[i], GZ_Z_BUFFER_RENDER, disp);
	}

/* Translation matrix */
GzMatrix	scale = 
{ 
	3.25f,	0.0f,	0.0f,	0.0f, 
	0.0f,	3.25f,	0.0f,	-3.25f, 
	0.0f,	0.0f,	3.25f,	3.5f, 
	0.0f,	0.0f,	0.0f,	1.0f 
}; 
 
GzMatrix	rotateX = 
{ 
	1.0f,	0.0f,	0.0f,	0.0f, 
	0.0f,	.7071f,	.7071f,	0.0f, 
	0.0f,	-.7071f,	.7071f,	0.0f, 
	0.0f,	0.0f,	0.0f,	1.0f 
}; 
 
GzMatrix	rotateY = 
{ 
	.866f,	0.0f,	-0.5f,	0.0f, 
	0.0f,	1.0f,	0.0f,	0.0f, 
	0.5f,	0.0f,	.866f,	0.0f, 
	0.0f,	0.0f,	0.0f,	1.0f 
}; 

#if 1 	/* set up app-defined camera if desired, else use camera defaults */
    camera.position[X] = -3.0f;
    camera.position[Y] = -25.0f;
    camera.position[Z] = -4.0f;

    camera.lookat[X] = 7.8f;
    camera.lookat[Y] = 0.7f;
    camera.lookat[Z] = 6.5f;

    camera.worldup[X] = -0.2f;
    camera.worldup[Y] = 1.0f;
    camera.worldup[Z] = 0.0f;
	/*camera.position[X] = 0.0f;
    camera.position[Y] = 3.5f;
    camera.position[Z] = -5.0f;

    camera.lookat[X] = 0.0f;
    camera.lookat[Y] = 0.0f;
    camera.lookat[Z] = 0.0f;

    camera.worldup[X] = 0.0f;
    camera.worldup[Y] = 1.0f;
    camera.worldup[Z] = 0.0f;*/

    camera.FOV = 63.7f;              /* degrees *              /* degrees */

	status |= GzPutCamera(m_pRender, &camera);
	for (int i=0; i<AAKERNEL_SIZE; i++)
		status |= GzPutCamera(renderers[i], &camera);
#endif 

	/* Start Renderer */
	status |= GzBeginRender(m_pRender);
	for (int i=0; i<AAKERNEL_SIZE; i++)
		status |= GzBeginRender(renderers[i]);

	/* Light */
	GzLight	light1 = { {-0.7071f, 0.7071f, 0.0f}, {0.5f, 0.5f, 0.9f} };
	GzLight	light2 = { {0.0f, -0.7071f, -0.7071f}, {0.9f, 0.2f, 0.3f} };
	GzLight	light3 = { {0.7071f, 0.0f, -0.7071f}, {0.2f, 0.7f, 0.3f} };
	GzLight	ambientlight = { {0.0f, 0.0f, 0.0f}, {0.3f, 0.3f, 0.3f} };

	/* Material property */
	GzColor specularCoefficient = { 0.3f, 0.3f, 0.3f };
	GzColor ambientCoefficient = { 0.1f, 0.1f, 0.1f };
	GzColor diffuseCoefficient = {0.7f, 0.7f, 0.7f};

/* 
  renderer is ready for frame --- define lights and shader at start of frame 
*/

        /*
         * Tokens associated with light parameters
         */
        nameListLights[0] = GZ_DIRECTIONAL_LIGHT;
        valueListLights[0] = (GzPointer)&light1;
        nameListLights[1] = GZ_DIRECTIONAL_LIGHT;
        valueListLights[1] = (GzPointer)&light2;
        nameListLights[2] = GZ_DIRECTIONAL_LIGHT;
        valueListLights[2] = (GzPointer)&light3;
        status |= GzPutAttribute(m_pRender, 3, nameListLights, valueListLights);
		for (int i=0; i<AAKERNEL_SIZE; i++)
		{
			printf_s("Renderer[%i]\n", i);

			status |= GzPutAttribute(renderers[i], 3, nameListLights, valueListLights);
		}

        nameListLights[0] = GZ_AMBIENT_LIGHT;
        valueListLights[0] = (GzPointer)&ambientlight;
        status |= GzPutAttribute(m_pRender, 1, nameListLights, valueListLights);
		for (int i=0; i<AAKERNEL_SIZE; i++)
			status |= GzPutAttribute(renderers[i], 1, nameListLights, valueListLights);

        /*
         * Tokens associated with shading 
         */
        nameListShader[0]  = GZ_DIFFUSE_COEFFICIENT;
        valueListShader[0] = (GzPointer)&diffuseCoefficient;

	/* 
	* Select either GZ_COLOR or GZ_NORMALS as interpolation mode  
	*/
        nameListShader[1]  = GZ_INTERPOLATE;
        interpStyle = GZ_NORMALS;         /* Phong shading */
		//interpStyle = GZ_COLOR;
        valueListShader[1] = (GzPointer)&interpStyle;

        nameListShader[2]  = GZ_AMBIENT_COEFFICIENT;
        valueListShader[2] = (GzPointer)&ambientCoefficient;
        nameListShader[3]  = GZ_SPECULAR_COEFFICIENT;
        valueListShader[3] = (GzPointer)&specularCoefficient;
        nameListShader[4]  = GZ_DISTRIBUTION_COEFFICIENT;
        specpower = 32;
        valueListShader[4] = (GzPointer)&specpower;

        nameListShader[5]  = GZ_TEXTURE_MAP;
#if 0   /* set up null texture function or valid pointer */
        valueListShader[5] = (GzPointer)0;
#else
        valueListShader[5] = (GzPointer)(tex_fun);	/* or use ptex_fun */
		//valueListShader[5] = (GzPointer)(ptex_fun);
#endif
        status |= GzPutAttribute(m_pRender, 6, nameListShader, valueListShader);
		for (int i=0; i<AAKERNEL_SIZE; i++)
			status |= GzPutAttribute(renderers[i], 6, nameListShader, valueListShader);

		GzToken nameListShift[2] = {GZ_AASHIFTX, GZ_AASHIFTY};
		GzPointer valueListShift[2];
		for (int i=0; i<AAKERNEL_SIZE; i++)
		{
			printf_s("Renderer[%i]\n", i);

			float sx = AAFilter[i][0];
			float sy = AAFilter[i][1];
			valueListShift[0] = (GzPointer)&sx;
			valueListShift[1] = (GzPointer)&sy;
			status |= GzPutAttribute(renderers[i], 2, nameListShift, valueListShift);
		}

	status |= GzPushMatrix(m_pRender, scale);
	status |= GzPushMatrix(m_pRender, rotateY);
	status |= GzPushMatrix(m_pRender, rotateX);
	for (int i=0; i<AAKERNEL_SIZE; i++)
	{
		status |= GzPushMatrix(renderers[i], scale);
		status |= GzPushMatrix(renderers[i], rotateY);
		status |= GzPushMatrix(renderers[i], rotateX);
	}

	if (status) exit(GZ_FAILURE); 

	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Render() 
{
	GzToken			nameListTriangle[3]; 	/* vertex attribute names */
	GzPointer		valueListTriangle[3]; 	/* vertex attribute pointers */
	GzCoord			vertexList[3];	/* vertex position coordinates */ 
	GzCoord			normalList[3];	/* vertex normals */ 
	GzTextureIndex  uvList[3];		/* vertex texture map indices */ 
	char			dummy[256]; 
	int				status; 


	/* Initialize Display */
	status |= GzInitDisplay(m_pDisplay); 
	for (int i=0; i<AAKERNEL_SIZE; i++)
		status |= GzInitDisplay(renderers[i]->display);
	
	/* 
	* Tokens associated with triangle vertex values 
	*/ 
	nameListTriangle[0] = GZ_POSITION; 
	nameListTriangle[1] = GZ_NORMAL; 
	nameListTriangle[2] = GZ_TEXTURE_INDEX;  

	// I/O File open
	FILE *infile;
	if (fopen_s(&infile, INFILE, "r") != NULL)
	{
         MessageBox(NULL, L"[1]: The input file was not opened\n", L"Error", MB_OK );
		 return GZ_FAILURE;
	}

	FILE *outfile;
	if( (fopen_s(&outfile, OUTFILE , "wb" )) != NULL )
	{
         MessageBox( NULL, L"The output file was not opened\n", L"Error", MB_OK);
		 return GZ_FAILURE;
	}

	/* 
	* Walk through the list of triangles, set color 
	* and render each triangle 
	*/
	while( fscanf(infile, "%s", dummy) != EOF ) { 	/* read in tri word */
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[0][0]), &(vertexList[0][1]), &(vertexList[0][2]), 
		&(normalList[0][0]), &(normalList[0][1]), &(normalList[0][2]), 
		&(uvList[0][0]), &(uvList[0][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[1][0]), &(vertexList[1][1]), &(vertexList[1][2]), 
		&(normalList[1][0]), &(normalList[1][1]), &(normalList[1][2]), 
		&(uvList[1][0]), &(uvList[1][1]) ); 
	    fscanf(infile, "%f %f %f %f %f %f %f %f", 
		&(vertexList[2][0]), &(vertexList[2][1]), &(vertexList[2][2]), 
		&(normalList[2][0]), &(normalList[2][1]), &(normalList[2][2]), 
		&(uvList[2][0]), &(uvList[2][1]) ); 

	    /* 
	     * Set the value pointers to the first vertex of the 	
	     * triangle, then feed it to the renderer 
	     * NOTE: this sequence matches the nameList token sequence
	     */ 
	     valueListTriangle[0] = (GzPointer)vertexList; 
		 valueListTriangle[1] = (GzPointer)normalList; 
		 valueListTriangle[2] = (GzPointer)uvList; 
		 GzPutTriangle(m_pRender, 3, nameListTriangle, valueListTriangle); 
		 for (int i=0; i<AAKERNEL_SIZE; i++)
			GzPutTriangle(renderers[i], 3, nameListTriangle, valueListTriangle); 
	}

	// combine all framebuffers
	for (int j=0; j<renderers[0]->display->xres * renderers[0]->display->yres; j++)
	{
		// clear pixel in final framebuffer
		m_pDisplay->fbuf[j].red = m_pDisplay->fbuf[j].green = m_pDisplay->fbuf[j].blue = 0;
		
		// combine and weight framebuffer pixels
		GzColor color = {0.0f, 0.0f, 0.0f};
		for (int i=0; i<AAKERNEL_SIZE; i++)
		{
			/*m_pDisplay->fbuf[j].red += renderers[i]->display->fbuf[j].red * AAFilter[i][2];
			m_pDisplay->fbuf[j].green += renderers[i]->display->fbuf[j].green * AAFilter[i][2];
			m_pDisplay->fbuf[j].blue += renderers[i]->display->fbuf[j].blue * AAFilter[i][2];*/

			color[0] += (renderers[i]->display->fbuf[j].red * AAFilter[i][2]);
			color[1] += (renderers[i]->display->fbuf[j].green * AAFilter[i][2]);
			color[2] += (renderers[i]->display->fbuf[j].blue * AAFilter[i][2]);
		}

		// convert and store floats into framebuffer
		m_pDisplay->fbuf[j].red = (GzIntensity)color[0];
		m_pDisplay->fbuf[j].green = (GzIntensity)color[1];
		m_pDisplay->fbuf[j].blue = (GzIntensity)color[2];
	}

	GzFlushDisplay2File(outfile, m_pDisplay); 	/* write out or update display to file*/
	GzFlushDisplay2FrameBuffer(m_pFrameBuffer, m_pDisplay);	// write out or update display to frame buffer

	/* 
	 * Close file
	 */ 
	if( fclose( infile ) )
		MessageBox(NULL, L"[2] The input file was not closed\n", L"Error", MB_OK);

	if( fclose( outfile ) )
		MessageBox(NULL, L"The output file was not closed\n", L"Error", MB_OK);
 
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS); 
}

int Application5::Clean()
{
	/* 
	 * Clean up and exit 
	 */ 
	int	status = 0; 

	status |= GzFreeRender(m_pRender); 
	status |= GzFreeDisplay(m_pDisplay);
	for (int i=0; i<AAKERNEL_SIZE; i++)
	{
		status |= GzFreeRender(renderers[i]); 
		status |= GzFreeDisplay(renderers[i]->display);
	}
	
	if (status) 
		return(GZ_FAILURE); 
	else 
		return(GZ_SUCCESS);
}




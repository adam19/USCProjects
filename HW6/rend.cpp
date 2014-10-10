/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include	"MatrixMath.hpp"




//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay	*display)
{
/*  
- malloc a renderer struct 
- keep closed until all inits are done 
- setup Xsp and anything only done once 
- span interpolator needs pointer to display 
- check for legal class GZ_Z_BUFFER_RENDER 
- init default camera 
*/ 
	if (display == NULL)
		return GZ_FAILURE;

	*render = new GzRender;
	render[0]->open = 0;
	render[0]->display = display;
	render[0]->renderClass = renderClass;

	if (render[0]->renderClass != GZ_Z_BUFFER_RENDER)
		return GZ_FAILURE;

	// init default camera
	render[0]->camera.FOV = DEFAULT_FOV;	
	render[0]->camera.lookat[0] = 0;
	render[0]->camera.lookat[1] = 0;
	render[0]->camera.lookat[2] = 0;
	render[0]->camera.position[0] = 0;
	render[0]->camera.position[1] = 0;
	render[0]->camera.position[2] = -5;	
	render[0]->camera.worldup[0] = 0;
	render[0]->camera.worldup[1] = 1;
	render[0]->camera.worldup[2] = 0;
	
	// ambient light default
	render[0]->ambientlight.color[0] = ctoi(0.5f);
	render[0]->ambientlight.color[1] = ctoi(0.5f);
	render[0]->ambientlight.color[2] = ctoi(0.5f);
	render[0]->ambientlight.direction[0] = 0.0f;
	render[0]->ambientlight.direction[1] = 0.0f;
	render[0]->ambientlight.direction[2] = 1.0f;

	// ambient coefficient default
	render[0]->Ka[0] = ctoi(1.0f);
	render[0]->Ka[1] = ctoi(1.0f);
	render[0]->Ka[2] = ctoi(1.0f);

	// diffuse light list
	memset((GzLight*)render[0]->lights, 0, sizeof(GzLight)*10);
	render[0]->numlights = 0;

	// diffuse coefficient default
	render[0]->Kd[0] = ctoi(1.0f);
	render[0]->Kd[1] = ctoi(1.0f);
	render[0]->Kd[2] = ctoi(1.0f);

	// specular coefficient default
	render[0]->Ks[0] = ctoi(1.0f);
	render[0]->Ks[1] = ctoi(1.0f);
	render[0]->Ks[2] = ctoi(1.0f);

	// default flat color
	render[0]->flatcolor[0] = ctoi(0.0f);
	render[0]->flatcolor[1] = ctoi(1.0f);
	render[0]->flatcolor[2] = ctoi(0.0f);	
	
	render[0]->open = 1;

	return GZ_SUCCESS;
}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	render->display = NULL;
	render->tex_fun = NULL;

	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- set up for start of each frame - clear frame buffer 
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms if it want to. 
*/ 
	int status = GZ_SUCCESS;

	/*if (render->open != 0)
		return GZ_FAILURE;*/

	if (!render->display->fbuf)
		return GZ_FAILURE;

	/*****
	- set up for start of each frame - clear frame buffer 
	- compute Xiw and projection xform Xpi from camera definition 
	- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
	- now stack contains Xsw and app can push model Xforms if it want to. 
	*****/
	GzClearFrameBuffer(render->display);
	render->matlevel = -1; // set matrix stack to bottom

	IdentityMatrix(render->Ximage[0]);

	BuildXspMatrix(render->display->xres, render->display->yres, INT_MAX, 1.0f / tan((render->camera.FOV * PIOVER180) / 2.0f), render->Xsp);
	BuildXpiMatrix(render->camera.FOV, render->camera.Xpi);
	BuildXiwMatrix(&render->camera);

	status |= GzPushMatrix(render, render->Xsp);
	status |= GzPushMatrix(render, render->camera.Xpi);
	status |= GzPushMatrix(render, render->camera.Xiw);

	return status;
}

int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/
	render->camera.FOV = camera->FOV;
	memcpy((GzCoord*)render->camera.lookat, (GzCoord*)camera->lookat, sizeof(GzCoord));
	memcpy((GzCoord*)render->camera.position, (GzCoord*)camera->position, sizeof(GzCoord));
	memcpy((GzCoord*)render->camera.worldup, (GzCoord*)camera->worldup, sizeof(GzCoord));
	memcpy((GzMatrix*)render->camera.Xiw, (GzMatrix*)camera->Xiw, sizeof(GzMatrix));
	memcpy((GzMatrix*)render->camera.Xpi, (GzMatrix*)camera->Xpi, sizeof(GzMatrix));

	return GZ_SUCCESS;	
}

int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	if (render->matlevel >= MATLEVELS)
		return GZ_FAILURE;
	
	if (render->matlevel < 0) // empty stack
	{
		render->matlevel++;
		memcpy((GzMatrix*)render->Ximage[render->matlevel],
			(GzMatrix*)matrix,
			sizeof(GzMatrix));

		GzMatrix iden;
		IdentityMatrix(iden);
		memcpy((GzMatrix*)render->Xnorm[render->matlevel],
			(GzMatrix*)iden,
			sizeof(GzMatrix));

		//PrintGzMatrix(render->Xnorm[render->matlevel]);
	}
	else
	{
		// multiply and push to Ximage matrix stack
		GzMatrix newMatrix;
		MultMatrix(render->Ximage[render->matlevel], matrix, newMatrix);
		
		memcpy((GzMatrix*)render->Ximage[render->matlevel + 1],
			(GzMatrix*)newMatrix,
			sizeof(GzMatrix));

		// multiply and push to normal matrix stack
		if (render->matlevel > 0) // do not transform normal matrix with Xsp or Xpi
		{
			GzMatrix normMatrix;
			memcpy((GzMatrix*)normMatrix, (GzMatrix*)matrix, sizeof(GzMatrix));
		
			// strip translation
			normMatrix[0][3] = normMatrix[1][3] = normMatrix[2][3] = 0.0f;
		
			// normalize upper left 3x3
			GzCoord col = {normMatrix[0][0], normMatrix[1][0], normMatrix[2][0]};	
			float scale = VectorMagnitude(col);
			if (scale != 0.0f)
				scale = 1.0f / scale;

			// scale upper left 3x3
			for (int i=0; i<3; i++)
				for (int j=0; j<3; j++)
					normMatrix[i][j] *= scale;

			MultMatrix(render->Xnorm[render->matlevel], normMatrix, newMatrix);
			memcpy((GzMatrix*)render->Xnorm[render->matlevel + 1],
				(GzMatrix*)newMatrix,
				sizeof(GzMatrix));

			//PrintGzMatrix(render->Xnorm[render->matlevel+1]);
			//OutputDebugString("Transforming norm matrix\n");
			//PrintGzMatrix(normMatrix);
		}
		else // only push identity matrix for first 2 levels of Xnorm
		{
			GzMatrix iden;
			IdentityMatrix(iden);
			memcpy((GzMatrix*)render->Xnorm[render->matlevel + 1],
				(GzMatrix*)iden,
				sizeof(GzMatrix));

			//PrintGzMatrix(render->Xnorm[render->matlevel+1]);
		}
		
		render->matlevel++;
	}

	return GZ_SUCCESS;
}

int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/
	if (render->matlevel < 0)
		return GZ_FAILURE;

	render->matlevel--;

	return GZ_SUCCESS;
}


int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	for (int i=0; i<numAttributes; i++)
	{
		switch (nameList[i])
		{
			case GZ_RGB_COLOR:
			{
				float* color = (float*)valueList[i];
				render->flatcolor[0] = color[0];
				render->flatcolor[1] = color[1];
				render->flatcolor[2] = color[2];
			}break;

			case GZ_INTERPOLATE:
			{
				int* interpMode = (int*)valueList[i];			
				render->interp_mode = *interpMode;
			}break;

			case GZ_TEXTURE_MAP:
			{
				render->tex_fun = (GzTexture)valueList[i];
			}break;

			case GZ_DIRECTIONAL_LIGHT:
			{
				// GzLight
				if (render->numlights < 10)
				{
					GzLight *temp = (GzLight*)valueList[i];
				
					render->lights[render->numlights].color[0] = temp->color[0];
					render->lights[render->numlights].color[1] = temp->color[1];
					render->lights[render->numlights].color[2] = temp->color[2];
					render->lights[render->numlights].direction[0] = temp->direction[0];
					render->lights[render->numlights].direction[1] = temp->direction[1];
					render->lights[render->numlights].direction[2] = temp->direction[2];

					// normalize light direction just in case
					NormalizeVector(render->lights[render->numlights].direction,
									render->lights[render->numlights].direction);
					render->numlights++;
				}
			}break;

			case GZ_AMBIENT_LIGHT:
			{
				GzLight *temp = (GzLight*)valueList[i];

				render->ambientlight.color[0] = temp->color[0];
				render->ambientlight.color[1] = temp->color[1];
				render->ambientlight.color[2] = temp->color[2];

				// not necessary
				render->ambientlight.direction[0] = temp->direction[0];
				render->ambientlight.direction[1] = temp->direction[1];
				render->ambientlight.direction[2] = temp->direction[2];
			}break;

			case GZ_AMBIENT_COEFFICIENT:
			{
				//memcpy((GzColor*)render->Ka, (GzColor*)valueList[i], sizeof(GzColor));
				float *Ka = (float*)valueList[i];
				render->Ka[0] = Ka[0];
				render->Ka[1] = Ka[1];
				render->Ka[2] = Ka[2];
			}break;

			case GZ_DIFFUSE_COEFFICIENT:
			{
				float *Kd = (float*)valueList[i];
				render->Kd[0] = Kd[0];
				render->Kd[1] = Kd[1];
				render->Kd[2] = Kd[2];
			}break;

			case GZ_SPECULAR_COEFFICIENT:
			{
				float *Ks = (float*)valueList[i];
				render->Ks[0] = Ks[0];
				render->Ks[1] = Ks[1];
				render->Ks[2] = Ks[2];
			}break;

			case GZ_DISTRIBUTION_COEFFICIENT:
			{
				float* s = (float*)valueList[i];
				render->spec = *s;
			}break;

			case GZ_AASHIFTX:
			{
				float *x = (float*)valueList[i];
				render->pixelOffset[0] = *x * -1.0f;

				printf_s("  AASHIFTX = %f\n", render->pixelOffset[0]);
			}break;

			case GZ_AASHIFTY:
			{
				float *y = (float*)valueList[i];
				render->pixelOffset[1] = *y * -1.0f;

				printf_s("  AASHIFTY = %f\n", render->pixelOffset[1]);
			}break;
		}
	}

	return GZ_SUCCESS;
}

void CalcLight(GzRender *render, const float *normal, const float z, float *uv, GzColor &final)
{
	GzColor diff = {0.0f, 0.0f, 0.0f};
	GzColor spec = {0.0f, 0.0f, 0.0f};

	for (int l=0; l<render->numlights; l++)
	{
		GzCoord		e = {0.0f, 0.0f, -1.0f};
		GzLight		xformLight;
		GzMatrix	Xn;
		double		nDotL;
		double		nDotE;
		GzCoord		n;

		n[0] = normal[0];
		n[1] = normal[1];
		n[2] = normal[2];

		memcpy((GzCoord*)xformLight.direction, (GzCoord*)render->lights[l].direction, sizeof(GzCoord));
		memcpy((GzCoord*)xformLight.color, (GzCoord*)render->lights[l].color, sizeof(GzCoord));

		memcpy((GzMatrix*)Xn, (GzMatrix*)render->Xnorm[render->matlevel], sizeof(GzMatrix));

		nDotL = DotProduct(n, xformLight.direction);
		nDotE = DotProduct(n, e);

		if ((nDotL < 0.0 && nDotE > 0.0) || (nDotL > 0.0 && nDotE < 0.0))
		{
			continue;
		}
		else if (nDotL < 0.0 && nDotE < 0.0)
		{
			// flip normal
			n[0] *= -1.0f;
			n[1] *= -1.0f;
			n[2] *= -1.0f;
			
			nDotL = DotProduct(n, xformLight.direction);
			nDotE = DotProduct(n, e);
		}
				
		// specular - per light
		// R = 2(N.L)N - L
		// Ie * Ks(R.E)^s
		/*GzCoord r = {(2.0f * nDotL * n[0]) - xformLight.direction[0],
					(2.0f * nDotL * n[1]) - xformLight.direction[1],
					(2.0f * nDotL * n[2]) - xformLight.direction[2]};*/
		GzCoord r;
		ReflectVector(xformLight.direction, n, r);
		//NormalizeVector(r, r);

		float rDotE = DotProduct(r, e);
		if (rDotE > 1.0f)
			rDotE = 1.0f;
		if (rDotE < 0.0f)
			rDotE = 0.0f;
		float specPow = pow(rDotE, render->spec);

		spec[0] += xformLight.color[0] * specPow;
		spec[1] += xformLight.color[1] * specPow;
		spec[2] += xformLight.color[2] * specPow;

		// diffuse - per light
		// Ie * Kd(N.L)
		diff[0] += xformLight.color[0] * (float)nDotL;
		diff[1] += xformLight.color[1] * (float)nDotL;
		diff[2] += xformLight.color[2] * (float)nDotL;
	}

	if (render->tex_fun != NULL)
	{
		if (render->interp_mode == GZ_COLOR) // Gouraud
		{
			GzColor Kt;
			render->tex_fun(uv[0], uv[1], Kt);
			memcpy((GzColor*)render->Kd, (GzColor*)Kt, sizeof(GzColor));
			memcpy((GzColor*)render->Ka, (GzColor*)Kt, sizeof(GzColor));
			memcpy((GzColor*)render->Ks, (GzColor*)Kt, sizeof(GzColor));
		}
		else if (render->interp_mode == GZ_NORMALS) // Phong
		{
			//memcpy((GzColor*)specular, (GzColor*)render->Ks, sizeof(GzColor));

			GzColor Kt;
			render->tex_fun(uv[0], uv[1], Kt);
			memcpy((GzColor*)render->Kd, (GzColor*)Kt, sizeof(GzColor));
			memcpy((GzColor*)render->Ka, (GzColor*)Kt, sizeof(GzColor));
		}
	}

	final[0] = final[1] = final[2] = 0.0f;

	// ambient
	// Ia * Ka
	final[0] += (render->ambientlight.color[0] * render->Ka[0]);
	final[1] += (render->ambientlight.color[1] * render->Ka[1]);
	final[2] += (render->ambientlight.color[2] * render->Ka[2]);

	// specular component
	final[0] += (spec[0] * render->Ks[0]);
	final[1] += (spec[1] * render->Ks[1]);
	final[2] += (spec[2] * render->Ks[2]);

	// diffuse component
	final[0] += (diff[0] * render->Kd[0]);
	final[1] += (diff[1] * render->Kd[1]);
	final[2] += (diff[2] * render->Kd[2]);
}

void RenderPixel(GzRender *render, int x, int y, GzDepth dz, float *uv, float *value)
{
	/*
	 value depends on the value of render->interp_mode
	  - GZ_COLOR: color at pixel
	  - GZ_NORMALS: normal at pixel
	  - GZ_NONE or GZ_FLAT: fallback color
	*/
	GzColor final = {0.0f, 0.0f, 0.0f};

	float u = uv[0];
	float v = uv[1];

	switch (render->interp_mode)
	{
		case GZ_COLOR:
			GzColor color;
			render->tex_fun(uv[0], uv[1], color);

			final[0] = value[0] * color[0];
			final[1] = value[1] * color[1];
			final[2] = value[2] * color[2];
			break;
		case GZ_NORMALS:
			CalcLight(render, value, (float)dz, uv, final);
			break;

		case GZ_NONE:
		default:
			final[0] = render->flatcolor[0];
			final[1] = render->flatcolor[1];
			final[2] = render->flatcolor[2];
	}

	//GzPutDisplay(render->display, (int)(x + render->pixelOffset[0]), (int)(y + render->pixelOffset[1]), 
	GzPutDisplay(render->display, x, y, 
		ctoi(final[0]), ctoi(final[1]), ctoi(final[2]),
		1, 
		(GzDepth)dz);
}

void RenderPoint(GzRender *render, GzColor color, GzCoord point, int size)
{
	int x1 = (int)(point[0] - (size/2.0f));
	int x2 = (int)(point[0] + (size/2.0f));
	int y1 = (int)(point[1] - (size/2.0f));
	int y2 = (int)(point[1] + (size/2.0f));

	for (int x = x1; x < x2; x++)
	{
		for (int y = y1; y < y2; y++)
		{
			GzPutDisplay(render->display, (int)x, (int)y, 
				ctoi(color[0]), ctoi(color[1]), ctoi(color[2]),
				1, 
				0);
		}
	}
}

void FillTriangle(GzRender *render, Edge lEdge, Edge rEdge, bool fillLeft)
{
	// calculate lighting color
	if (render->interp_mode == GZ_COLOR)	// per-vertex
	{
		CalcLight(render, lEdge.v1.normal, lEdge.v1.coord[2], lEdge.v1.uv, lEdge.v1.color);
		CalcLight(render, lEdge.v2.normal, lEdge.v2.coord[2], lEdge.v2.uv, lEdge.v2.color);
		CalcLight(render, rEdge.v1.normal, rEdge.v1.coord[2], rEdge.v1.uv, rEdge.v1.color);
		CalcLight(render, rEdge.v2.normal, rEdge.v2.coord[2], rEdge.v2.uv, rEdge.v2.color);
	}

	/*int y1 = fillLeft ? floor(lEdge.v1.coord[1]) : ceil(lEdge.v1.coord[1]);
	int y2 = fillLeft ? floor(lEdge.v2.coord[1]) : ceil(lEdge.v2.coord[1]);*/

	for (float y = lEdge.v1.coord[1]; y < lEdge.v2.coord[1]; y += 1.0f)		// iterate along y-axis
	{
		Vertex startCoord, endCoord;
		lEdge.CalcPointAtY(y, startCoord);
		rEdge.CalcPointAtY(y, endCoord);

		float x1 = startCoord.coord[0], x2 = endCoord.coord[0];
		float z1 = startCoord.coord[2], z2 = endCoord.coord[2];

		if (fillLeft)
			x1 = floor(x1);
		else
			x1 = ceil(x1);

		for (float x = x1; x < x2; x++)	// iterate and fill each row
		{
			float t, dz;

			if (x2 - x1 != 0)
				t = (x-x1)/(x2-x1);
			else
				t = x1;
			dz = (t * (z2-z1)) + z1;

			if (dz > 0)
			{
				GzTextureIndex uv;
				InterpolateUV(startCoord.uv, endCoord.uv, t, uv);
				float vz = dz / ((float)INT_MAX - dz);
				uv[0] = uv[0] * (vz + 1.0f);	// to image space
				uv[1] = uv[1] * (vz + 1.0f);	// to image space

				switch (render->interp_mode)
				{
					case GZ_COLOR:		// vertex lighting
						GzColor color;
						InterpolateVector(startCoord.color, endCoord.color, t, color);
						RenderPixel(render, (int)(x + render->pixelOffset[0]), (int)(y + render->pixelOffset[1]), (GzDepth)dz, (float*)uv, color);	// passing vertex-color to interpolate
						break;
					case GZ_NORMALS:	// per-pixel lighting
						GzCoord normal;
						InterpolateVector(startCoord.normal, endCoord.normal, t, normal);
						NormalizeVector(normal, normal);
						RenderPixel(render, (int)(x + render->pixelOffset[0]), (int)(y + render->pixelOffset[1]), (GzDepth)dz, (float*)uv, normal);	// passing normal to calculate lighting
						break;
				}
			}
		}
	}
}

int GzFillTopOfTriangle(GzRender *render, Vertex *verts, bool fillLeft)
{
	Edge lEdge, rEdge;

	// sort bottom two verts by X
	if (verts[1].coord[0] > verts[2].coord[0])
		SwapVerts(verts[1], verts[2]);

	lEdge = Edge(verts[0], verts[1]);
	rEdge = Edge(verts[0], verts[2]);

	FillTriangle(render, lEdge, rEdge, fillLeft);

	return GZ_SUCCESS;
}

int GzFillBottomOfTriangle(GzRender *render, Vertex *verts, bool fillLeft)
{
	Edge lEdge, rEdge;
	
	// sort top two verts by X
	if (verts[0].coord[0] > verts[1].coord[0])
		SwapVerts(verts[0], verts[1]);

	lEdge = Edge(verts[0], verts[2]);
	rEdge = Edge(verts[1], verts[2]);

	FillTriangle(render, lEdge, rEdge, fillLeft);

	return GZ_SUCCESS;
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, 
				  GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts  
- Clip - just discard any triangle with verts behind view plane 
       - test for triangles with all three verts off-screen 
- invoke triangle rasterizer  
*/ 
	//OutputDebugString("putTriangle\n");

	Vertex	xformVerts[3];
	Edge	edge[3];
	Edge	lEdge, rEdge;
	int		longestEdgeIndex = 0;
	bool	isLeft;

	for (int i=0; i<numParts; i++)
	{
		switch(nameList[i])
		{
			case GZ_POSITION:
				GzMatrix Xsm;
				GetXsmMatrix(render, Xsm);

				GzCoord	verts[3];
				memcpy((GzCoord&)verts, (GzPointer&)valueList[i], sizeof(GzCoord)*3);

				MultMatrixVector(Xsm, verts[0], xformVerts[0].coord);
				MultMatrixVector(Xsm, verts[1], xformVerts[1].coord);
				MultMatrixVector(Xsm, verts[2], xformVerts[2].coord);
			break;

			case GZ_NORMAL:
				GzMatrix Xn;
				GzCoord normals[3];

				memcpy((GzCoord&)normals, (GzPointer&)valueList[i], sizeof(GzCoord)*3);

				// transforming each normal by stack. inefficient and needs to be altered
				memcpy((GzMatrix*)Xn, (GzMatrix*)render->Xnorm[render->matlevel], sizeof(GzMatrix));
				MultMatrixVector(Xn, normals[0], xformVerts[0].normal);
				MultMatrixVector(Xn, normals[1], xformVerts[1].normal);
				MultMatrixVector(Xn, normals[2], xformVerts[2].normal);

				NormalizeVector(xformVerts[0].normal, xformVerts[0].normal);
				NormalizeVector(xformVerts[1].normal, xformVerts[1].normal);
				NormalizeVector(xformVerts[2].normal, xformVerts[2].normal);
			break;

			case GZ_TEXTURE_INDEX:
				GzTextureIndex uv[3];

				memcpy((GzTextureIndex&)uv, (GzPointer&)valueList[i], sizeof(GzTextureIndex) * 3);

				float vz;
				vz = xformVerts[0].coord[2] / ((float)INT_MAX - xformVerts[0].coord[2]);
				xformVerts[0].uv[0] = uv[0][0] / (vz + 1.0f);	// to perspective space
				xformVerts[0].uv[1] = uv[0][1] / (vz + 1.0f);	// to perspective space
				
				vz = xformVerts[1].coord[2] / ((float)INT_MAX - xformVerts[1].coord[2]);
				xformVerts[1].uv[0] = uv[1][0] / (vz + 1.0f);	// to perspective space
				xformVerts[1].uv[1] = uv[1][1] / (vz + 1.0f);	// to perspective space
				
				vz = xformVerts[2].coord[2] / ((float)INT_MAX - xformVerts[2].coord[2]);
				xformVerts[2].uv[0] = uv[2][0] / (vz + 1.0f);	// to perspective space
				xformVerts[2].uv[1] = uv[2][1] / (vz + 1.0f);	// to perspective space
			break;

			default:
				break;
		}
	}	
				
	// sort verts by y
	for (int j=0; j<3; j++)
		for (int k=0; k<3; k++)
			if (xformVerts[j].coord[1] < xformVerts[k].coord[1])
			{
				Vertex vtemp;
				vtemp = xformVerts[k];
				xformVerts[k] = xformVerts[j];
				xformVerts[j] = vtemp;
			}

	edge[0] = Edge(xformVerts[0], xformVerts[1]);
	edge[1] = Edge(xformVerts[1], xformVerts[2]);
	edge[2] = Edge(xformVerts[2], xformVerts[0]);

	isLeft = IsLeft(xformVerts[0].coord, xformVerts[1].coord, xformVerts[2].coord);

	if (xformVerts[1].coord[1] == xformVerts[2].coord[1]) // fill just top
		GzFillTopOfTriangle(render, xformVerts, isLeft);
				
	if (xformVerts[0].coord[1] == xformVerts[1].coord[1]) // fill just bottom
		GzFillBottomOfTriangle(render, xformVerts, isLeft);

	if ((xformVerts[0].coord[1] != xformVerts[1].coord[1]) && 
		(xformVerts[1].coord[1] != xformVerts[2].coord[1]))
	{
		Vertex	vt; // temp vertex and normal along edge to y=verts[1][1]
		Vertex	v[3];
		Edge	edge;
		v[0] = xformVerts[0];	 // top vert
		v[1] = xformVerts[1];
		edge = Edge(v[0], xformVerts[2]);
		edge.CalcPointAtY(xformVerts[1].coord[1], vt);
		v[2] = vt; // split point

		if (v[1].coord[0] > v[2].coord[0])
			SwapVerts(v[1], v[2]);

		// fill top
		GzFillTopOfTriangle(render, v, isLeft);
		// fill bottom
		SwapVerts(v[0], v[2]);
		v[2] = xformVerts[2];
		GzFillBottomOfTriangle(render, v, isLeft);
	}

	return GZ_SUCCESS;
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

inline bool IsLeft(float* v1, float* v2, float* v3)
{
	return (((v2[0]-v1[0]) * (v3[1]-v1[1])) - ((v2[1]-v1[1]) * (v3[0]-v1[0]))) > 0.0f;
}

void SwapVerts(Vertex &v1, Vertex &v2)
{
	Vertex temp;
	temp = v1;
	v1 = v2;
	v2 = temp;
}

int GzClearFrameBuffer(GzDisplay *display)
{
	GzColor clearColor;
	clearColor[0] = ctoi(0.5f);
	clearColor[1] = ctoi(0.5f);
	clearColor[2] = ctoi(0.5f);

	GzPixel p;
	p.red = ctoi(0.5f);
	p.green = ctoi(0.5f);
	p.blue = ctoi(0.5f);
	p.alpha = ctoi(1.0f);
	p.z = INT_MAX;

	//memset((GzPixel*)display->fbuf, (GzPixel)p, sizeof(GzPixel));
	for (int i=0; i<display->xres * display->yres; i++)
	{
		display->fbuf[i].red = p.red;
		display->fbuf[i].green = p.green;
		display->fbuf[i].blue = p.blue;
		display->fbuf[i].alpha = p.alpha;
		display->fbuf[i].z = p.z;
	}

	return GZ_SUCCESS;
}


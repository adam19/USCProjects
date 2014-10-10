#include <math.h>

#include "Gz.h"


float Clamp(float v, float min, float max)
{
	if (v < min)
		return min;
	if (v > max)
		return max;
	return v;
}

inline void PrintGzCoord(const GzCoord v)
{
	printf_s("<%f %f %f>\n", v[0], v[1], v[2]);
}

void PrintGzMatrix(GzMatrix mat)
{
	for (int i=0; i<4; i++)
	{
		for (int j=0; j<4; j++)
		{
			printf_s("%f\t", mat[i][j]);
		}
		printf_s("\n");
	}
	printf_s("\n");
}

void IdentityMatrix(GzMatrix mat)
{
	memset((GzMatrix*)mat, 0, sizeof(GzMatrix));
	mat[0][0] = mat[1][1] = mat[2][2] = mat[3][3] = 1.0f;
}

void MultMatrix(GzMatrix a, GzMatrix b, GzMatrix &c)
{
	GzMatrix r;

	r[0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0] + a[0][3]*b[3][0];
	r[0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1] + a[0][3]*b[3][1];
	r[0][2] = a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2] + a[0][3]*b[3][2];
	r[0][3] = a[0][0]*b[0][3] + a[0][1]*b[1][3] + a[0][2]*b[2][3] + a[0][3]*b[3][3];
	
	r[1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0] + a[1][3]*b[3][0];
	r[1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1] + a[1][3]*b[3][1];
	r[1][2] = a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2] + a[1][3]*b[3][2];
	r[1][3] = a[1][0]*b[0][3] + a[1][1]*b[1][3] + a[1][2]*b[2][3] + a[1][3]*b[3][3];

	r[2][0] = a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0] + a[2][3]*b[3][0];
	r[2][1] = a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1] + a[2][3]*b[3][1];
	r[2][2] = a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2] + a[2][3]*b[3][2];
	r[2][3] = a[2][0]*b[0][3] + a[2][1]*b[1][3] + a[2][2]*b[2][3] + a[2][3]*b[3][3];
	
	r[3][0] = a[3][0]*b[0][0] + a[3][1]*b[1][0] + a[3][2]*b[2][0] + a[3][3]*b[3][0];
	r[3][1] = a[3][0]*b[0][1] + a[3][1]*b[1][1] + a[3][2]*b[2][1] + a[3][3]*b[3][1];
	r[3][2] = a[3][0]*b[0][2] + a[3][1]*b[1][2] + a[3][2]*b[2][2] + a[3][3]*b[3][2];
	r[3][3] = a[3][0]*b[0][3] + a[3][1]*b[1][3] + a[3][2]*b[2][3] + a[3][3]*b[3][3];

	memcpy((GzMatrix*)c, (GzMatrix*)r, sizeof(GzMatrix));
}

void MultMatrixVector(GzMatrix m, GzCoord v, GzCoord &res)
{
	res[0] = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3];
	res[1] = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3];
	res[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3];
	
	float w1;
	w1 = m[3][0]*v[0] + m[3][1]*v[1] + m[3][2]*v[2] + m[3][3];
	
	res[0] /= w1;
	res[1] /= w1;
	res[2] /= w1;
}

float DotProduct(const GzCoord a, const GzCoord b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void CrossProduct(const GzCoord a, const GzCoord b, GzCoord &c)
{
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}

inline float VectorMagnitude(const GzCoord vector)
{
	return sqrt(vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
}

void ReflectVector(const GzCoord i, const GzCoord n, GzCoord &r)
{
	float nDotI = DotProduct(i, n) * 2.0f;
	
	r[0] = nDotI*n[0];
	r[1] = nDotI*n[1];
	r[2] = nDotI*n[2];

	r[0] -= i[0];
	r[1] -= i[1];
	r[2] -= i[2];
}

void NormalizeVector(const GzCoord a, GzCoord &b)
{
	float mag = VectorMagnitude(a);

	// prevent divide-by-zero
	if (mag == 0.0f)
		mag = 1.0f;
	
	b[0] = a[0] / mag;
	b[1] = a[1] / mag;
	b[2] = a[2] / mag;
}

void InterpolateVector(const float* v1, const float* v2, const float t, float* result)
{
	result[0] = ((v2[0] - v1[0]) * t) + v1[0];
	result[1] = ((v2[1] - v1[1]) * t) + v1[1];
	result[2] = ((v2[2] - v1[2]) * t) + v1[2];
}

void InterpolateUV(GzTextureIndex uv1, GzTextureIndex uv2, const float t, GzTextureIndex &result)
{
	result[0] = ((uv2[0] - uv1[0]) * t) + uv1[0];
	result[1] = ((uv2[1] - uv1[1]) * t) + uv1[1];
}

void BuildXspMatrix(int screenWidth, int screenHeight, int zMax, float d, GzMatrix &xsp)
{
	IdentityMatrix(xsp);

	xsp[0][0] = (float)screenWidth / 2.0f;
	xsp[0][3] = (float)screenWidth / 2.0f;
	xsp[1][1] = (float)screenHeight / -2.0f;
	xsp[1][3] = (float)screenHeight / 2.0f;
	
	xsp[2][2] = (float)zMax / (float)d;
}

void BuildXpiMatrix(float fov, GzMatrix &xpi)
{
	IdentityMatrix(xpi);

	xpi[3][2] = tan((fov * PIOVER180) / 2.0f);
}

void BuildXiwMatrix(GzCamera *camera)
{
	IdentityMatrix(camera->Xiw);

	GzCoord right;
	GzCoord look;

	// build look-vector
	look[0] = camera->lookat[0] - camera->position[0];
	look[1] = camera->lookat[1] - camera->position[1];
	look[2] = camera->lookat[2] - camera->position[2];
	NormalizeVector(look, look);

	// build up-vector
	float upDotZ = DotProduct(camera->worldup, look);
	camera->worldup[0] = camera->worldup[0] - upDotZ * look[0];
	camera->worldup[1] = camera->worldup[1] - upDotZ * look[1];
	camera->worldup[2] = camera->worldup[2] - upDotZ * look[2];
	NormalizeVector(camera->worldup, camera->worldup);

	// build right-vector
	CrossProduct(camera->worldup, look, right);
	NormalizeVector(right, right);

	// build matrix
	camera->Xiw[0][0] = right[0];
	camera->Xiw[0][1] = right[1];
	camera->Xiw[0][2] = right[2];

	camera->Xiw[1][0] = camera->worldup[0];
	camera->Xiw[1][1] = camera->worldup[1];
	camera->Xiw[1][2] = camera->worldup[2];

	camera->Xiw[2][0] = look[0];
	camera->Xiw[2][1] = look[1];
	camera->Xiw[2][2] = look[2];

	// build upper right column
	GzCoord tRight = {-right[0], -right[1], -right[2]};
	GzCoord tUp = {-camera->worldup[0], -camera->worldup[1], -camera->worldup[2]};
	GzCoord tLook = {-look[0], -look[1], -look[2]};
	
	//up - (up . Z)Z 
	camera->Xiw[0][3] = DotProduct(tRight, camera->position);
	camera->Xiw[1][3] = DotProduct(tUp, camera->position);
	camera->Xiw[2][3] = DotProduct(tLook, camera->position);
	
	/*fprintf(outf, "Camera vectors\n");
	fprintf(outf, "  right: "); PrintGzCoord(right);
	fprintf(outf, "  up:    "); PrintGzCoord(camera->worldup);
	fprintf(outf, "  look:  "); PrintGzCoord(look);
	fprintf(outf, "  pos:   "); PrintGzCoord(camera->position);*/
}

void BuildTranspose(GzMatrix a, GzMatrix &b)
{
	GzMatrix c;

	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++)
			c[i][j] = a[j][i];
	memcpy((GzMatrix*)b, (GzMatrix*)c, sizeof(GzMatrix));
}

void GetXsmMatrix(GzRender *render, GzMatrix &Xsm)
{
	memcpy((GzMatrix*)Xsm, 
		(GzMatrix*)render->Ximage[render->matlevel], 
		sizeof(GzMatrix));
}

int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	GzMatrix m;
	float cd = cos(degree * PIOVER180);
	float sd = sin(degree * PIOVER180);

	IdentityMatrix(m);
	m[1][1] = cd;
	m[1][2] = -sd;
	m[2][1] = sd;
	m[2][2] = cd;

	memcpy((GzMatrix*)mat, (GzMatrix*)m, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value
	GzMatrix m;
	float cd = cos(degree * PIOVER180);
	float sd = sin(degree * PIOVER180);

	IdentityMatrix(m);
	m[0][0] = cd;
	m[2][0] = -sd;
	m[0][2] = sd;
	m[2][2] = cd;

	memcpy((GzMatrix*)mat, (GzMatrix*)m, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value
	GzMatrix m;
	float cd = cos(degree * PIOVER180);
	float sd = sin(degree * PIOVER180);

	IdentityMatrix(m);
	m[0][0] = cd;
	m[1][0] = sd;
	m[0][1] = -sd;
	m[1][1] = cd;

	memcpy((GzMatrix*)mat, (GzMatrix*)m, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	GzMatrix m;

	IdentityMatrix(m);
	m[0][3] = translate[0];
	m[1][3] = translate[1];
	m[2][3] = translate[2];

	memcpy((GzMatrix*)mat, (GzMatrix*)m, sizeof(GzMatrix));

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value
	GzMatrix m;

	IdentityMatrix(m);
	m[0][0] = scale[0];
	m[1][1] = scale[1];
	m[2][2] = scale[2];	

	memcpy((GzMatrix*)mat, (GzMatrix*)m, sizeof(GzMatrix));

	return GZ_SUCCESS;
}
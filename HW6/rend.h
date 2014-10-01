#include "disp.h" /* include your own disp.h file (e.g. hw1)*/

#define PIOVER180 3.1415927f / 180.0f

/* Camera defaults */
#define	DEFAULT_FOV		35.0
#define	DEFAULT_IM_Z	(-10.0)  /* world coords for image plane origin */
#define	DEFAULT_IM_Y	(5.0)    /* default look-at point = 0,0,0 */
#define	DEFAULT_IM_X	(-10.0)

#define	DEFAULT_AMBIENT	{0.1, 0.1, 0.1}
#define	DEFAULT_DIFFUSE	{0.7, 0.6, 0.5}
#define	DEFAULT_SPECULAR	{0.2, 0.3, 0.4}
#define	DEFAULT_SPEC		32

#define	MATLEVELS	100		/* how many matrix pushes allowed */
#define	MAX_LIGHTS	10		/* how many lights allowed */

/* Dummy definition : change it later */
//#ifndef GzLight
//#define GzLight		GzPointer
//#endif

//#ifndef GzTexture
//#define GzTexture	GzPointer
//#endif

#ifndef GZRENDER
#define GZRENDER
typedef struct {			/* define a renderer */
  GzRenderClass	renderClass;
  GzDisplay		*display;
  short		    open;
  GzCamera		camera;
  short		    matlevel;	        /* top of stack - current xform */
  GzMatrix		Ximage[MATLEVELS];	/* stack of xforms (Xsm) */
  GzMatrix		Xnorm[MATLEVELS];	/* xforms for norms (Xim) */
  GzMatrix		Xsp;		        /* NDC to screen (pers-to-screen) */
  GzColor		flatcolor;          /* color state for flat shaded triangles */
  int			interp_mode;
  int			numlights;
  GzLight		lights[MAX_LIGHTS];
  GzLight		ambientlight;
  GzColor		Ka, Kd, Ks;
  float		    spec;		/* specular power */
  GzTexture		tex_fun;    /* tex_fun(float u, float v, GzColor color) */
  GzCoord		pixelOffset;
}  GzRender;
#endif


class	Edge;
inline void PrintGzCoord(const GzCoord v);
void	PrintGzMatrix(GzMatrix mat);
//void	PrintStack(GzRender *render);
void	IdentityMatrix(GzMatrix mat);
void	MultMatrix(float** a, float** b, float** c);
//void	MultMatrixVector(float** m, float* v, float** res, float *w);
void	MultMatrixVector(GzMatrix m, GzCoord v, GzCoord &res);
float	DotProduct(const GzCoord a, const GzCoord b);
void	CrossProduct(const GzCoord a, const GzCoord b, GzCoord &c);
void	NormalizeVector(const GzCoord a, GzCoord &b);
void	BuildXspMatrix(int screenWidth, int screenHeight, int zMax, int d, GzMatrix &xsp);
void	BuildXpiMatrix(float fov, GzMatrix &xpi);
void	BuildXiwMatrix(GzCamera *camera);
void	BuildTranspose(GzMatrix a, GzMatrix &b);
void	PerspectiveDivide(GzCoord &v, float w);

void CalcLight(GzRender *render, const float *normal, const float z, float *uv, GzColor &final);
void RenderPixel(GzRender *render, int x, int y, GzDepth dz, const float *uv, float *value);

// Function declaration
// HW2
int		GzNewRender(GzRender **render, GzRenderClass renderClass, GzDisplay *display);
int		GzFreeRender(GzRender *render);
int		GzBeginRender(GzRender	*render);
int		GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
			GzPointer *valueList);
int		GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
			GzPointer *valueList);
void	FillTriangle(GzRender *render, Edge lEdge, Edge rEdge, bool fillLeft);
int		GzFillTopOfTriangle(GzRender *render, Vertex *verts, bool fillLeft);
int		GzFillBottomOfTriangle(GzRender *render, Vertex *verts, bool fillLeft);

// HW3
int		GzPutCamera(GzRender *render, GzCamera *camera);
int		GzPushMatrix(GzRender *render, GzMatrix	matrix);
int		GzPopMatrix(GzRender *render);

// Object Translation
int		GzRotXMat(float degree, GzMatrix mat);
int		GzRotYMat(float degree, GzMatrix mat);
int		GzRotZMat(float degree, GzMatrix mat);
int		GzTrxMat(GzCoord translate, GzMatrix mat);
int		GzScaleMat(GzCoord scale, GzMatrix mat);

inline short	ctoi(float color);
bool IsLeft(float* v1, float* v2, float* v3);
void SwapVerts(Vertex &v1, Vertex &v2);

int GzClearFrameBuffer(GzDisplay *display);

void closeFile();
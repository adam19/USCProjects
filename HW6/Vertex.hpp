#pragma once

#ifndef __VERTEX_HPP__
#define __VERTEX_HPP__

#include "Gz.h"

class Vertex
{
public:
	GzCoord			coord;
	GzCoord			normal;
	GzColor			color;
	GzTextureIndex	uv;

	Vertex()
	{
		coord[0] = coord[1] = coord[2] = 0.0f;
		normal[0] = normal[1] = normal[2] = 0.0f;
		color[0] = color[1] = color[2] = 1.0f;
		uv[0] = uv[1] = 0.0f;
	}

	Vertex(Vertex &v)
	{
		memcpy((GzCoord*)coord, (GzCoord*)v.coord, sizeof(GzCoord));
		memcpy((GzCoord*)normal, (GzCoord*)v.normal, sizeof(GzCoord));
		memcpy((GzColor*)color, (GzColor*)v.color, sizeof(GzColor));
		this->uv[0] = v.uv[0];
		this->uv[1] = v.uv[1];
	}

	~Vertex() {}

	void operator=(Vertex &v)
	{
		this->coord[0] = v.coord[0];
		this->coord[1] = v.coord[1];
		this->coord[2] = v.coord[2];
		
		this->normal[0] = v.normal[0];
		this->normal[1] = v.normal[1];
		this->normal[2] = v.normal[2];
		
		this->color[0] = v.color[0];
		this->color[1] = v.color[1];
		this->color[2] = v.color[2];
		
		this->uv[0] = v.uv[0];
		this->uv[1] = v.uv[1];
	}

	void print(FILE* outf)
	{
		fprintf(outf, "<%f %f %f> \t <%f %f %f> \t <%f %f %f> \t [%f %f]\n",
			coord[0], coord[1], coord[2],
			normal[0], normal[1], normal[2],
			color[0], color[1], color[2],
			uv[0], uv[1]);
	}
};

#endif
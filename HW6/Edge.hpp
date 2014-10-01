#pragma once

#ifndef __EDGE_HPP__
#define __EDGE_HPP__

#include "Vertex.hpp"

class Edge
{
public:
	Vertex	v1, v2;

	Edge()
	{
	}

	Edge(Vertex &v1, Vertex &v2)
	{
		this->v1 = v1;
		this->v2 = v2;
	}

	void operator=(Edge &e)
	{
		this->v1 = e.v1;
		this->v2 = e.v2;
	}

	inline float Length()
	{
		float dx, dy, dz;

		dx = v2.coord[0] - v1.coord[0];
		dy = v2.coord[1] - v1.coord[1];
		dz = v2.coord[2] - v2.coord[2];

		return abs(sqrt(dx*dx + dy*dy + dz*dz));
	}
	
	void CalcPointAtY(float y, Vertex &result)
	{		
		float yLen = abs(v2.coord[1] - v1.coord[1]);

		if (yLen == 0.0f)
		{
			result.coord[0] = v1.coord[0];
			result.coord[1] = y;
			result.coord[2] = v1.coord[2];
			
			// THIS COULD CAUSE PROBLEMS LATER
			result.normal[0] = v1.normal[0];
			result.normal[1] = v1.normal[1];
			result.normal[2] = v1.normal[2];
			return;
		}

		float t = (y - v1.coord[1]) / yLen;
		
		result.coord[0] = t * (v2.coord[0] - v1.coord[0]) + v1.coord[0];
		result.coord[1] = t * (v2.coord[1] - v1.coord[1]) + v1.coord[1];
		result.coord[2] = t * (v2.coord[2] - v1.coord[2]) + v1.coord[2];

		result.normal[0] = t * (v2.normal[0] - v1.normal[0]) + v1.normal[0];
		result.normal[1] = t * (v2.normal[1] - v1.normal[1]) + v1.normal[1];
		result.normal[2] = t * (v2.normal[2] - v1.normal[2]) + v1.normal[2];

		result.color[0] = t * (v2.color[0] - v1.color[0]) + v1.color[0];
		result.color[1] = t * (v2.color[1] - v1.color[1]) + v1.color[1];
		result.color[2] = t * (v2.color[2] - v1.color[2]) + v1.color[2];

		result.uv[0] = t * (v2.uv[0] - v1.uv[0]) + v1.uv[0];
		result.uv[1] = t * (v2.uv[1] - v1.uv[1]) + v1.uv[1];
	}

	void print(FILE* outf)
	{
		fprintf(outf, "v1 = <%f %f %f>   n1 = <%f %f %f>\nv2 = <%f %f %f>   n2 = <%f %f %f>\n", 
			v1.coord[0], v1.coord[1], v1.coord[2],
			v1.normal[0], v1.normal[1], v1.normal[2],
			v2.coord[0], v2.coord[1], v2.coord[2],
			v2.normal[0], v2.normal[1], v2.normal[2]);
	}
};

#endif
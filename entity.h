#ifndef _H_ENTITY
#define _H_ENTITY

#include "matrix.h"
#include "renderer.h"

typedef struct{
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
	Vec3 colour;

	Buffered_Mesh_Handle handle;
}Entity;

#endif

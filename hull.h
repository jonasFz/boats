#ifndef _H_HULL
#define _H_HULL

#include "mesh.h"
#include "matrix.h"

typedef struct{
	
}Rib;

typedef struct{
	float keel_length;
	float keel_width;
	float keel_depth;

	float bow_depth;
	float bow_width;
	float bow_radius;
}Hull;

typedef struct{
	
	unsigned int vertical_segments;
	unsigned int horizontal_segments;

	Vec3 *positions;
} Surface;

Surface make_surface(unsigned int hor, unsigned int vert);
void set_position(Surface *surface, int x, int y, Vec3 value);
Vec3 get_position(Surface *surface, int x, int y);

Mesh_Data mesh_from_surface(Surface surface);

void generate_bow(Hull *hull, Mesh_Data *result);
void generate_arc(Mesh_Data *result, float theta, unsigned int segments, float width, float height, float radius, Vec3 start_normal, Vec3 end_normal);
void generate_keel(Hull *hull, Mesh_Data *result);
unsigned int number_of_pieces(Hull *hull);

#endif

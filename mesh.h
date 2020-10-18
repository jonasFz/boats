#ifndef _H_MESH
#define _H_MESH

#include "matrix.h"
#include <stddef.h>

typedef struct{
	float vx;
	float vy;
	float vz;

	float nx;
	float ny;
	float nz;

	float tx;
	float ty;
}Vertex;

typedef struct{
	float *vertices;
	int vertex_count;

	float *normals;
	int normal_count;
	
	float *textures;
	int texture_count;

	unsigned int *indices;
	int index_count;
}Mesh_Data;


void free_mesh(Mesh_Data *mesh);

void flip_normals(Mesh_Data *data);
void calculate_normals(Mesh_Data *data);

Mesh_Data make_hexagon_mesh();

Mesh_Data make_plane_mesh();

Mesh_Data make_extruded_polygon(unsigned int count, float radius, float length);

void allocate_mesh(Mesh_Data *mesh, size_t vertices, size_t indices);

void normalize_mesh(Mesh_Data *md);
void move_mesh_origin(Mesh_Data *md, Vec3 change);
Mesh_Data load_mesh_data(char *filepath);
Mesh_Data load_stl_file(char *filepath);
#endif

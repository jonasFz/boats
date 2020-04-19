#ifndef _H_MESH
#define _H_MESH

#include "matrix.h"
#include <stddef.h>

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

void calculate_normals(Mesh_Data *data);

Mesh_Data make_hexagon_mesh();

Mesh_Data make_plane_mesh();

void allocate_mesh(Mesh_Data *mesh, size_t vertices, size_t indices);

void normalize_mesh(Mesh_Data *md);
void move_mesh_origin(Mesh_Data *md, Vec3 change);
Mesh_Data load_mesh_data(char *filepath);
#endif

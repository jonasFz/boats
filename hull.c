#include "hull.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "mesh.h"

Surface make_surface(unsigned int hor, unsigned int vert){
	Surface ret;
	ret.vertical_segments = vert;
	ret.horizontal_segments = hor;

	ret.positions = (Vec3 *)malloc(sizeof(Vec3) * vert * hor);

	float height = 1.0f;
	float width = 1.0f;

	float dx = width/(ret.horizontal_segments-1);
	float dy = height/(ret.vertical_segments -1);
	
	for(int i = 0; i < ret.horizontal_segments; i++){
		for(int j = 0; j < ret.vertical_segments; j++){
			ret.positions[i+j*hor] = make_vec3(dx*i - width/2, dy*j- height/2, 0.0f);
		}
	}

	return ret;
}

void set_position(Surface *surface, int x, int y, Vec3 value){
	surface->positions[x+y*surface->horizontal_segments] = value;
}

Vec3 get_position(Surface *surface, int x, int y){
	return surface->positions[x+y*surface->horizontal_segments];
}

Mesh_Data mesh_from_surface(Surface surface){
	//Lets just try this using Vec3, should work but lets note the assumption
	Mesh_Data result;
	allocate_mesh(&result, surface.vertical_segments * surface.horizontal_segments, (surface.vertical_segments-1) * (surface.horizontal_segments-1) * 2 * 3);

	memcpy(result.vertices, surface.positions, sizeof(float)*result.vertex_count);

	int tally = 0;
	for(int i = 0; i < surface.horizontal_segments  - 1; i++){
		for(int j = 0; j < surface.vertical_segments - 1; j++){
			result.indices[tally++] = i+j*surface.horizontal_segments;
			result.indices[tally++] = i+(j+1)*surface.horizontal_segments;
			result.indices[tally++] = 1+i+j*surface.horizontal_segments;

			result.indices[tally++] = 1+i+j*surface.horizontal_segments;
			result.indices[tally++] = i+(j+1)*surface.horizontal_segments;
			result.indices[tally++] = 1+i+(j+1)*surface.horizontal_segments;
		}
	}

	calculate_normals(&result);

	return result;
}

//Will actually have to do some math here later
unsigned int number_of_pieces(Hull *hull){
	return 2;
}


void generate_keel(Hull *hull, Mesh_Data *result){
	int verts_needed = 4 * 6;

	allocate_mesh(result, verts_needed, 2 * 3 * 6);

	float *v = result->vertices;
	v[0] = -hull->keel_length/2; v[1] = hull->keel_depth/2; v[2] = -hull->keel_width/2;
	v[3] = hull->keel_length/2; v[4] = hull->keel_depth/2; v[5] = -hull->keel_width/2;
	v[6] = hull->keel_length/2; v[7] = hull->keel_depth/2; v[8] = hull->keel_width/2;
	v[9] = -hull->keel_length/2; v[10] = hull->keel_depth/2; v[11] = hull->keel_width/2;

	v[12] = -hull->keel_length/2; v[13] = -hull->keel_depth/2; v[14] = -hull->keel_width/2;
	v[15] = hull->keel_length/2; v[16] = -hull->keel_depth/2; v[17] = -hull->keel_width/2;
	v[18] = hull->keel_length/2; v[19] = -hull->keel_depth/2; v[20] = hull->keel_width/2;
	v[21] = -hull->keel_length/2; v[22] = -hull->keel_depth/2; v[23] = hull->keel_width/2;

	v[24] = -hull->keel_length/2; v[25] = hull->keel_depth/2; v[26] = -hull->keel_width/2;
	v[27] = hull->keel_length/2; v[28] = hull->keel_depth/2; v[29] = -hull->keel_width/2;
	v[30] = hull->keel_length/2; v[31] = -hull->keel_depth/2; v[32] = -hull->keel_width/2;
	v[33] = -hull->keel_length/2; v[34] = -hull->keel_depth/2; v[35] = -hull->keel_width/2;

	v[36] = -hull->keel_length/2; v[37] = hull->keel_depth/2; v[38] = hull->keel_width/2;
	v[39] = hull->keel_length/2; v[40] = hull->keel_depth/2; v[41] = hull->keel_width/2;
	v[42] = hull->keel_length/2; v[43] = -hull->keel_depth/2; v[44] = hull->keel_width/2;
	v[45] = -hull->keel_length/2; v[46] = -hull->keel_depth/2; v[47] = hull->keel_width/2;

	v[48] = -hull->keel_length/2; v[49] = hull->keel_depth/2; v[50] = -hull->keel_width/2;
	v[51] = -hull->keel_length/2; v[52] = hull->keel_depth/2; v[53] = hull->keel_width/2;
	v[54] = -hull->keel_length/2; v[55] = -hull->keel_depth/2; v[56] = hull->keel_width/2;
	v[57] = -hull->keel_length/2; v[58] = -hull->keel_depth/2; v[59] = -hull->keel_width/2;

	v[60] = hull->keel_length/2; v[61] = hull->keel_depth/2; v[62] = -hull->keel_width/2;
	v[63] = hull->keel_length/2; v[64] = hull->keel_depth/2; v[65] = hull->keel_width/2;
	v[66] = hull->keel_length/2; v[67] = -hull->keel_depth/2; v[68] = hull->keel_width/2;
	v[69] = hull->keel_length/2; v[70] = -hull->keel_depth/2; v[71] = -hull->keel_width/2;


	unsigned int *in = result->indices;
	in[0] = 0; in[1] = 1; in[2] = 2;
	in[3] = 0; in[4] = 2; in[5] = 3;

	in[6] = 4; in[7] = 6; in[8] = 5;
	in[9] = 4; in[10] = 7; in[11] = 6;

	in[12] = 8; in[13] = 10; in[14] = 9;
	in[15] = 8; in[16] = 11; in[17] = 10;

	in[18] = 12; in[19] = 13; in[20] = 14;
	in[21] = 12; in[22] = 14; in[23] = 15;

	in[24] = 16; in[25] = 17; in[26] = 18;
	in[27] = 16; in[28] = 18; in[29] = 19;

	in[30] = 20; in[31] = 21; in[32] = 22;
	in[33] = 20; in[34] = 22; in[35] = 23;

	calculate_normals(result);
}

void generate_bow(Hull *hull, Mesh_Data *result){
	generate_arc(result, 1.57, 12, hull->bow_width, hull->bow_depth, hull->bow_radius, make_vec3(-1.0f, 0.0f, 0.0f), make_vec3(0.0f, 1.0f, 0.0f));
}

void generate_arc(Mesh_Data *result, float theta, unsigned int segments, float width, float height, float radius, Vec3 start_normal, Vec3 end_normal){

	int verts_needed = (segments) * 2 * 4 + 8; //2 per segment

	allocate_mesh(result, verts_needed, (segments -1) * 2 * 3 * 4 + 12);

	float angle = theta / (segments-1);

	//Finding indices into result->vertices
	float *vt = result->vertices;
	float *vb = &result->vertices[segments * 3 * 2];
	float *vl = &result->vertices[segments * 3 * 2 * 2];
	float *vr = &result->vertices[segments * 3 * 2 * 3];

	//Generating vertex data for the four non end sides of curve
	for(int i = 0; i < segments; i++){
		float here_angle = angle * i;
		float dx = sin(here_angle);
		float dy = cos(here_angle);

		float nearx = dx*(radius - height/2);
		float farx  = dx*(radius + height/2);

		float neary = radius - dy*(radius - height/2);
		float fary  = radius - dy*(radius + height/2);

		int b = i * 3 * 2;
		vt[b]   = nearx; vt[b+1] = neary; vt[b+2] = -width/2;
		vt[b+3] = nearx; vt[b+4] = neary; vt[b+5] = width/2;

		vb[b]   = farx; vb[b+1] = fary; vb[b+2] = -width/2;
		vb[b+3] = farx; vb[b+4] = fary; vb[b+5] = width/2;
	
		vl[b]   = nearx; vl[b+1] = neary; vl[b+2] = -width/2;
		vl[b+3] = farx;  vl[b+4] = fary;  vl[b+5] = -width/2;

		vr[b]   = nearx; vr[b+1] = neary; vr[b+2] = width/2;
		vr[b+3] = farx;  vr[b+4] = fary;  vr[b+5] = width/2;
	}

	//Building the triangles for the four non end sides
	unsigned int *in = result->indices;
	for(int i = 0; i < segments - 1; i++){
		int bt = i * 3 * 2;
		
		in[bt]   = i*2; in[bt+1] = i*2 + 2; in[bt+2] = i*2 + 3;
		in[bt+3] = i*2 + 3; in[bt+4] = i*2 + 1; in[bt+5] = i*2;

		int bb = (i + segments - 1) * 3 * 2;
		int _i = i + segments;
		in[bb]   = _i*2; in[bb+1] = _i*2 + 3; in[bb+2] = _i*2 + 2;
		in[bb+3] = _i*2 + 1; in[bb+4] = _i*2 + 3; in[bb+5] = _i*2;

		int bl = (i + (segments - 1) * 2) * 3 * 2;
		int __i = _i + segments;
		in[bl]   = __i*2; in[bl+1] = __i*2 + 3; in[bl+2] = __i*2 + 2;
		in[bl+3] = __i*2 + 1; in[bl+4] = __i*2 + 3; in[bl+5] = __i*2;

		int br = (i + (segments - 1) * 3) * 3 * 2;
		int ___i = __i + segments;
		in[br]   = ___i*2; in[br+1] = ___i*2 + 2; in[br+2] = ___i*2 + 3;
		in[br+3] = ___i*2 + 3; in[br+4] = ___i*2 + 1; in[br+5] = ___i*2;
	}

	//Build the first end face
	float *ve = &result->vertices[segments * 3 * 2 * 4];
	float nearx = sin(0) * (radius-height/2);
	float farx  = sin(0) * (radius+height/2);

	float neary = radius - cos(0)*(radius-height/2);
	float fary  = radius - cos(0)*(radius+height/2);

	ve[0] = nearx; ve[1] = neary; ve[2] = -width/2;
	ve[3] = nearx; ve[4] = neary; ve[5] =  width/2;

	ve[6] = farx; ve[7] = fary; ve[8] = -width/2;
	ve[9] = farx; ve[10] = fary; ve[11] = width/2;


	//Build the second end face
	nearx = sin(theta) * (radius-height/2);
	farx  = sin(theta) * (radius+height/2);

	neary = radius - cos(theta)*(radius-height/2);
	fary  = radius - cos(theta)*(radius+height/2);

	ve[12] = nearx; ve[13] = neary; ve[14] = -width/2;
	ve[15] = nearx; ve[16] = neary; ve[17] =  width/2;

	ve[18] = farx; ve[19] = fary; ve[20] = -width/2;
	ve[21] = farx; ve[22] = fary; ve[23] = width/2;



	int vi = segments*4*2;
	int b = ((segments-1) * 4) * 3 * 2; //segments -1 face, 2 triangles per face, 3 verts per triangle, 4 sides
	
	in[b]   = vi;   in[b+1] = vi+1; in[b+2] = vi+2;
	in[b+3] = vi+1; in[b+4] = vi+3; in[b+5] = vi+2;

	in[b+6] = vi+4; in[b+7]  = vi+6; in[b+8] = vi+5;
	in[b+9] = vi+5; in[b+10] = vi+6; in[b+11] = vi+7;

	calculate_normals(result);
}




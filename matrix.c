#include "matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

Vec3 make_vec3(float x, float y, float z){
	Vec3 ret;
	ret.x = x;
	ret.y = y;
	ret.z = z;

	return ret;
}


void show_vec3(Vec3 v){
	printf("v3(%f %f %f)\n", v.x, v.y, v.z);
}

Vec3 transform(Mat4 *mat, Vec3 *v){
	Vec3 ret;
	ret.x = mat->data[0]*v->x + mat->data[4]*v->y + mat->data[8]*v->z + mat->data[12]*1.0f;
	ret.y = mat->data[1]*v->x + mat->data[5]*v->y + mat->data[9]*v->z + mat->data[13]*1.0f;
	ret.z = mat->data[2]*v->x + mat->data[6]*v->y + mat->data[10]*v->z + mat->data[14]*1.0f;

	return ret;
}

float dot_product(Vec3 a, Vec3 b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

Vec3 angle_to_direction(Vec3 v){
	return make_vec3(sin(v.y), sin(v.x), -cos(v.y));	
}

Vec3 rotate_x_axis(Vec3 start, float rad){
	Mat4 rot = make_rotation_matrix(rad, 0.0f, 0.0f);
	
	return transform(&rot, &start);
}

Vec3 rotate_y_axis(Vec3 start, float rad){
	Mat4 rot = make_rotation_matrix(0.0f, rad, 0.0f);
	
	return transform(&rot, &start);
}

Vec3 scale_vec(Vec3 v, float scale){
	Vec3 ret;
	ret.x = v.x*scale;
	ret.y = v.y*scale;
	ret.z = v.z*scale;

	return ret;
}

Vec3 add_vec(Vec3 a, Vec3 b){
	Vec3 ret;
	ret.x = a.x+b.x;
	ret.y = a.y+b.y;
	ret.z = a.z+b.z;

	return ret;
}

Vec3 sub_vec(Vec3 a, Vec3 b){
	Vec3 ret;
	ret.x = a.x-b.x;
	ret.y = a.y-b.y;
	ret.z = a.z-b.z;

	return ret;
}

Mat4 make_view_matrix(Vec3 right, Vec3 up, Vec3 forward, Vec3 position){
	Mat4 ret = make_identity_matrix();
	ret.data[0] = right.x;
	ret.data[4] = right.y;
	ret.data[8] = right.z;

	ret.data[1] = up.x;
	ret.data[5] = up.y;
	ret.data[9] = up.z;

	ret.data[2]  = forward.x;
	ret.data[6]  = forward.y;
	ret.data[10] = forward.z;

	Mat4 p = make_identity_matrix();
	p.data[12] = -position.x;
	p.data[13] = -position.y;
	p.data[14] = -position.z;

	return multiply_matrix(&ret, &p);
}


Vec3 cross_product(Vec3 a, Vec3 b){
	Vec3 ret;
	//(x1 + y1 + z1) * (x2 + y2 + z2)
	//x1*x2 + x1*y2 + x1*z2 + y1*x2 + y1*y2 + y1*z2 + z1*x2 + z1*y2 + z1*z2
	//0     + z     + -y    + -z    + 0     + x     + y     + -x    + 0
	ret.x = a.y*b.z - a.z*b.y;
	ret.y = a.z*b.x - a.x*b.z;
	ret.z = a.x*b.y - a.y*b.x;

	return ret;
}
void add_to_vector(Vec3 *to, Vec3 from){
	to->x += from.x;
	to->y += from.y;
	to->z += from.z;
}

Vec3 normalize(Vec3 v){
	float mag = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	return make_vec3(v.x/mag, v.y/mag, v.z/mag);
}

void show_matrix(Mat4 *mat){
	float *data = mat->data;
	for(int i = 0; i<4; i++){
		for(int j = 0; j<4; j++){
			printf("%f ", data[j*4+i]);
		}
		printf("\n");
	}
}

Mat4 make_identity_matrix(){
	Mat4 ret;
	memset(ret.data, 0, 16*sizeof(float));
	ret.data[0] = 1.0f;
	ret.data[5] = 1.0f;
	ret.data[10] = 1.0f;
	ret.data[15] = 1.0f;


	return ret;
}

Mat4 make_zero_matrix(){
	Mat4 ret;
	memset(ret.data, 0, 16*sizeof(float));
	return ret;
}

Mat4 make_projection_matrix(float a, float fov, float zfar, float znear){
	float y_scale = (1/tan(fov/2))*a;
	float x_scale = y_scale/a;
	float zm = zfar - znear;
	float zp = zfar + znear;

	Mat4 ret;
	memset(ret.data, 0, 16*sizeof(float));

	ret.data[0] = x_scale;
	ret.data[5] = y_scale;
	ret.data[10] = -zp/zm;
	ret.data[11] = -1;
	ret.data[14] = -(2*zfar*znear)/zm;

	return ret;
}

Mat4 make_rotation_matrix(float rx, float ry, float rz){
	Mat4 x = make_zero_matrix();
	x.data[0] = 1;
	x.data[5] = cos(rx);
	x.data[6] = sin(rx);
	x.data[9] = -sin(rx);
	x.data[10] = cos(rx);
	x.data[15] = 1;

	Mat4 y = make_zero_matrix();
	y.data[0] = cos(ry);
	y.data[2] = -sin(ry);
	y.data[5] = 1;
	y.data[8] = sin(ry);
	y.data[10] = cos(ry);
	y.data[15] = 1;

	Mat4 z = make_zero_matrix();
	z.data[0] = cos(rz);
	z.data[1] = sin(rz);
	z.data[4] = -sin(rz);
	z.data[5] = cos(rz);
	z.data[10] = 1;
	z.data[15] = 1;


	Mat4 zy = multiply_matrix(&z, &y);
	return multiply_matrix(&zy, &x);
	//Mat4 xy = multiply_matrix(&y, &x);
	//return multiply_matrix(&z, &xy);
}

Mat4 make_scale_matrix(float x, float y, float z){
	Mat4 ret;
	
	float *data = ret.data;
	memset(data, 0, 16*sizeof(float));
	
	*data = x;
	*(data+5) = y;
	*(data+10) = z;
	*(data+15) = 1.0f;

	return ret;
}

Vec3 scale(Vec3 start, Vec3 stretch){
	Mat4 scale_matrix = make_scale_matrix(stretch.x, stretch.y, stretch.z);
	return transform(&scale_matrix, &start);
}

Mat4 make_translation_matrix(float x, float y, float z){
	Mat4 ret;

	float *data = ret.data;
	memset(data, 0, 16*sizeof(float));

	data[0] = 1.0f;
	data[5] = 1.0f;
	data[10] = 1.0f;
	data[15] = 1.0f;

	data[12] = x;
	data[13] = y;
	data[14] = z;

	return ret;
}

// This would be cleaner with a loop but this is faster maybe?
Mat4 multiply_matrix(Mat4 *a, Mat4 *b){
	Mat4 ret;
	float *ad = a->data;
	float *bd = b->data;
	
	ret.data[0]  = ad[0]*bd[0] + ad[1]*bd[4] + ad[2]* bd[8] + ad[3]*bd[12];
	ret.data[1]  = ad[0]*bd[1] + ad[1]*bd[5] + ad[2]* bd[9] + ad[3]*bd[13];
	ret.data[2]  = ad[0]*bd[2] + ad[1]*bd[6] + ad[2]*bd[10] + ad[3]*bd[14];
	ret.data[3]  = ad[0]*bd[3] + ad[1]*bd[7] + ad[2]*bd[11] + ad[3]*bd[15];

	ret.data[4]  = ad[4]*bd[0] + ad[5]*bd[4] + ad[6]* bd[8] + ad[7]*bd[12];
	ret.data[5]  = ad[4]*bd[1] + ad[5]*bd[5] + ad[6]* bd[9] + ad[7]*bd[13];
	ret.data[6]  = ad[4]*bd[2] + ad[5]*bd[6] + ad[6]*bd[10] + ad[7]*bd[14];
	ret.data[7]  = ad[4]*bd[3] + ad[5]*bd[7] + ad[6]*bd[11] + ad[7]*bd[15];
	
	ret.data[8]  = ad[8]*bd[0] + ad[9]*bd[4] + ad[10]* bd[8] + ad[11]*bd[12];
	ret.data[9]  = ad[8]*bd[1] + ad[9]*bd[5] + ad[10]* bd[9] + ad[11]*bd[13];
	ret.data[10] = ad[8]*bd[2] + ad[9]*bd[6] + ad[10]*bd[10] + ad[11]*bd[14];
	ret.data[11] = ad[8]*bd[3] + ad[9]*bd[7] + ad[10]*bd[11] + ad[11]*bd[15];
	
	ret.data[12] = ad[12]*bd[0] + ad[13]*bd[4] + ad[14]* bd[8] + ad[15]*bd[12];
	ret.data[13] = ad[12]*bd[1] + ad[13]*bd[5] + ad[14]* bd[9] + ad[15]*bd[13];
	ret.data[14] = ad[12]*bd[2] + ad[13]*bd[6] + ad[14]*bd[10] + ad[15]*bd[14];
	ret.data[15] = ad[12]*bd[3] + ad[13]*bd[7] + ad[14]*bd[11] + ad[15]*bd[15];

	return ret;
}

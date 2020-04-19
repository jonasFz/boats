#ifndef _H_MATRIX
#define _H_MATRIX

typedef struct{
	float data[16];
}Mat4;

typedef struct{
	float x;
	float y;
	float z;
}Vec3;

void show_vec3(Vec3 v);

Mat4 make_view_matrix(Vec3 right, Vec3 up, Vec3 forward, Vec3 position);

Mat4 multiply_matrix(Mat4 *a, Mat4 *b);

Mat4 make_identity_matrix();

void add_to_vector(Vec3 *to, Vec3 from);

Vec3 normalize(Vec3 v);

Vec3 scale_vec(Vec3 v, float scale);
Vec3 add_vec(Vec3 a, Vec3 b);
Vec3 sub_vec(Vec3 a, Vec3 b);

Vec3 rotate_x_axis(Vec3 start, float rad);
Vec3 rotate_y_axis(Vec3 start, float rad);

Vec3 cross_product(Vec3 a, Vec3 b);

float dot_product(Vec3 a, Vec3 b);
Vec3 angle_to_direction(Vec3 v);

Vec3 transform(Mat4 *mat, Vec3 *v);

Vec3 make_vec3(float x, float y, float z);
void show_matrix(Mat4 *mat);
Mat4 make_scale_matrix(float x, float y, float z);
Mat4 make_translation_matrix(float x, float y, float z);

Vec3 scale(Vec3 start, Vec3 stretch);

//Radians
Mat4 make_rotation_matrix(float x, float y, float z);
Mat4 make_projection_matrix(float a, float fov, float zfar, float znear);

#endif

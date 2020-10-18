#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <assert.h>

#include "matrix.h"
#include "mesh.h"
#include "bucket.h"

char *load_file_as_string(const char *file_path, int *length);

void allocate_mesh(Mesh_Data *mesh, size_t vertices, size_t indices){

	mesh->vertex_count = 3 * vertices;
	mesh->normal_count = 3 * vertices;
	mesh->texture_count = 2 * vertices;
	mesh->index_count = indices;

	mesh->vertices = (float *)malloc(sizeof(float) * mesh->vertex_count);
	mesh->normals = (float *)malloc(sizeof(float) * mesh->normal_count);
	mesh->textures = (float *)malloc(sizeof(float) * mesh->texture_count);
	mesh->indices = (unsigned int *)malloc(sizeof(unsigned int) * mesh->index_count);

	memset(mesh->vertices, 0, sizeof(float) * mesh->vertex_count);
	memset(mesh->normals, 0, sizeof(float) * mesh->normal_count);
	memset(mesh->textures, 0, sizeof(float) * mesh->texture_count);
}

void free_mesh(Mesh_Data *mesh){
	free(mesh->vertices);
	free(mesh->normals);
	free(mesh->textures);
	free(mesh->indices);

	mesh->vertices = NULL;
	mesh->normals = NULL;
	mesh->textures = NULL;
	mesh->indices = NULL;

	mesh->vertex_count = 0;
	mesh->normal_count = 0;
	mesh->texture_count = 0;
	mesh->index_count = 0;
}

Mesh_Data make_extruded_polygon(unsigned int count, float radius, float length){
	assert(count > 2);
	Mesh_Data mesh;
	allocate_mesh(&mesh, 2*(count+1) + count*4, 2*(count*3) + count*2*3);
	//allocate_mesh(&mesh, 2*(1+count) + count*4, (2*count + 2*count)*3);

	int vc = 0;
	int ii = 0;

	double pi = 3.14159;

	mesh.vertices[vc++] = 0.0f;
	mesh.vertices[vc++] = 0.0f;
	mesh.vertices[vc++] = 0.0f;

	for(int i = 0; i < count; i++){
		mesh.vertices[vc++] = radius*cos(i*2*pi/count);
		mesh.vertices[vc++] = 0.0f;
		mesh.vertices[vc++] = radius*sin(i*2*pi/count);
	}

	for(int i = 0; i < count; i++){
		mesh.indices[ii++] = 0;
		mesh.indices[ii++] = (i+1)%count + 1;
		mesh.indices[ii++] = i%count + 1;
	}

	mesh.vertices[vc++] = 0.0f;
	mesh.vertices[vc++] = length;
	mesh.vertices[vc++] = 0.0f;

	for(int i = 0; i < count; i++){
		mesh.vertices[vc++] = radius*cos(i*2*pi/count);
		mesh.vertices[vc++] = length;
		mesh.vertices[vc++] = radius*sin(i*2*pi/count);
	}

	for(int i = 0; i < count; i++){
		
		mesh.indices[ii++] = count+1;
		mesh.indices[ii++] = count+1+(i)%count + 1;
		mesh.indices[ii++] = count+1+(i+1)%count + 1;
	}

	for(int i = 0; i < count; i++){
		mesh.vertices[vc++] = radius*cos(i*2*pi/count);
		mesh.vertices[vc++] = 0.0f;
		mesh.vertices[vc++] = radius*sin(i*2*pi/count);

		mesh.vertices[vc++] = radius*cos(i*2*pi/count);
		mesh.vertices[vc++] = length;
		mesh.vertices[vc++] = radius*sin(i*2*pi/count);

		mesh.vertices[vc++] = radius*cos((i+1)*2*pi/count);
		mesh.vertices[vc++] = 0.0f;
		mesh.vertices[vc++] = radius*sin((i+1)*2*pi/count);

		mesh.vertices[vc++] = radius*cos((i+1)*2*pi/count);
		mesh.vertices[vc++] = length;
		mesh.vertices[vc++] = radius*sin((i+1)*2*pi/count);
	}

	int h = (count + 1) * 2;
	for(int i = 0; i < count; i++){
		mesh.indices[ii++] = h+i*4;
		mesh.indices[ii++] = h+i*4+3;
		mesh.indices[ii++] = h+i*4+1;

		mesh.indices[ii++] = h+i*4;
		mesh.indices[ii++] = h+i*4+2;
		mesh.indices[ii++] = h+i*4+3;
	}	

	calculate_normals(&mesh);

	return mesh;
}

void flip_normals(Mesh_Data *data){
	for(int i = 0; i < data->index_count/3; i++){
		unsigned int temp = data->indices[i*3];
		data->indices[i*3] = data->indices[i*3+1];
		data->indices[i*3+1] = temp;
	}
	calculate_normals(data);
}

//@maybe opengl specific
void __swap_y_and_z_for_opengl(Mesh_Data *data){
	for(int v = 0; v < data->vertex_count/3; v++){
		float temp = data->vertices[v*3+1];
		data->vertices[v*3+1] = data->vertices[v*3+2];
		data->vertices[v*3+2] = temp;
	}
}

void calculate_normals(Mesh_Data *data){
	float *v = data->vertices;
	float *n = data->normals;

	//Loop throught all the triangles in the mesh
	for(int i = 0; i < data->index_count/3; i++){
		unsigned int index0 = data->indices[i*3];
		unsigned int index1 = data->indices[i*3 + 1];
		unsigned int index2 = data->indices[i*3 + 2];

		// Pull out the vectors for those triangles;
		Vec3 v0 = make_vec3(v[index0*3], v[index0*3+1], v[index0*3+2]);
		Vec3 v1 = make_vec3(v[index1*3], v[index1*3+1], v[index1*3+2]);
		Vec3 v2 = make_vec3(v[index2*3], v[index2*3+1], v[index2*3+2]);

		Vec3 n0 = make_vec3(n[index0*3], n[index0*3+1], n[index0*3+2]);
		Vec3 n1 = make_vec3(n[index1*3], n[index1*3+1], n[index1*3+2]);
		Vec3 n2 = make_vec3(n[index2*3], n[index2*3+1], n[index2*3+2]);

		//Calculate the normal for this face
		Vec3 u = sub_vec(v1, v0);
		Vec3 v = sub_vec(v2, v1);
		Vec3 w = sub_vec(v0, v2);

		Vec3 norm = cross_product(u, v);
		norm.x = -norm.x; norm.y = -norm.y; norm.z = -norm.z;
		Vec3 normal = normalize(norm);

		//show_vec3(normal);

		// So this should be weighted based on angle
		float a102 = dot_product(w, u)/(sqrt(w.x*w.x+w.y*w.y+w.z*w.z)*sqrt(u.x*u.x+u.y*u.y+u.z*u.z));
		float a012 = dot_product(v, u)/(sqrt(v.x*v.x+v.y*v.y+v.z*v.z)*sqrt(u.x*u.x+u.y*u.y+u.z*u.z));
		float a120 = dot_product(w, v)/(sqrt(w.x*w.x+w.y*w.y+w.z*w.z)*sqrt(v.x*v.x+v.y*v.y+v.z*v.z));


		a102 = 1.0 - fabs(a102);
		a012 = 1.0 - fabs(a012);
		a120 = 1.0 - fabs(a120);


		Vec3 nn0 = add_vec(scale_vec(normal, a102), n0);
		Vec3 nn1 = add_vec(scale_vec(normal, a012), n1);
		Vec3 nn2 = add_vec(scale_vec(normal, a120), n2);
		
		// Should really just do a memcpy, cause vec3 is in the same order
		n[index0*3] = nn0.x; n[index0*3+1] = nn0.y; n[index0*3+2] = nn0.z;
		n[index1*3] = nn1.x; n[index1*3+1] = nn1.y; n[index1*3+2] = nn1.z;
		n[index2*3] = nn2.x; n[index2*3+1] = nn2.y; n[index2*3+2] = nn2.z;
	
	}
	for(int i = 0; i< data->normal_count/3; i++){
		Vec3 norm = make_vec3(n[i*3], n[i*3+1], n[i*3+2]);
		norm = normalize(norm);
		n[i*3] = norm.x; n[i*3+1] = norm.y; n[i*3+2] = norm.z;
	}
}

char *load_raw(const char *file_path, int *length){
	FILE* in_file = fopen(file_path, "rb");
	if(!in_file){
		fprintf(stderr, "Failed to load file %s\n", file_path);
		return NULL;
	}
	fseek(in_file, 0, SEEK_END);
	int file_length = ftell(in_file);
	fseek(in_file, 0, SEEK_SET);
	*length = file_length;
	char *ret = (char *)malloc(file_length);

	fread(ret, 1, file_length, in_file);
	
	fclose(in_file);
	return ret;
}

void __join_stl_triangels(Mesh_Data *md){

	int new_index_count = 0;

	for(int i = 1; i < md->index_count; i++){
		
		float avx = md->vertices[i*3];
		float avy = md->vertices[i*3+1];
		float avz = md->vertices[i*3+2];

		float anx = md->normals[i*3];
		float any = md->normals[i*3+1];
		float anz = md->normals[i*3+2];

		for(int j = 0; j < i; i++){
			float bvx = md->vertices[j*3];
			float bvy = md->vertices[j*3+1];
			float bvz = md->vertices[j*3+2];
	
			float bnx = md->normals[j*3];
			float bny = md->normals[j*3+1];
			float bnz = md->normals[j*3+2];

			//Check if they are the same, should probably actually check if they are PRETTY MUCH the same
			if( avx == bvx && avy == bvy && avz == bvz &&
				anx == bnx && any == bny && anz == bnz){
				for(int k = 0; k < md->index_count; k++){
				}
			}
		}
	}
}

Mesh_Data __load_binary_stl_file(int file_length, char *file){
	
	Mesh_Data md;

	unsigned int triangle_count = *((unsigned int *)(file + 80));

	allocate_mesh(&md, triangle_count*3, triangle_count*3);

	int file_offset = 84; //80 byte header, 4 byte triangle count
	int triangles_loaded = 0;

	unsigned int v = 0;
	unsigned int i = 0;

	while(triangles_loaded < triangle_count){
		file_offset += 4*3; //Skipping normal. We can just calculate it
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;
		md.vertices[v++] = *((float *)(file + file_offset)); file_offset+=4;

		if(!(file[file_offset] == 0 && file[file_offset+1] == 0)){
			return md;
		}
		file_offset += 2; //Skipping attribute byte count
		
		md.indices[i] = i; i++;
		md.indices[i] = i; i++;
		md.indices[i] = i; i++;

		triangles_loaded++;
	}

	__swap_y_and_z_for_opengl(&md);
	calculate_normals(&md);

	normalize_mesh(&md);
	return md;

}


Mesh_Data __load_ascii_stl_file(char *file_path){
	
	FILE *f = fopen(file_path, "r");

	Mesh_Data md;

	Bucket b = make_bucket();

	int vertex_count = 0;

	// I want to just skip the solid <name> line at the start
	// because if it contains a v it screws with loading!
	while(fgetc(f) != '\n');

	while(1){
		int input = fgetc(f);
		if(input == EOF) break;
		if(input == 'v'){ //Then we found a vector line
			float v[3];
			//Skipping the 'v' in vertex because we already pulled it out
			int lets_just_check = fscanf(f, "ertex %f %f %f", &v[0], &v[1], &v[2]);
			//int lets_just_check = sscanf(file, "vertex %f %f %f", &v[0], &v[1], &v[2]); Just leaving this here to remind not to use sscanf!
			//Does a friggen strlen every time!
			assert(lets_just_check == 3);
			write_to_bucket(&b, v, sizeof(float) * 3);

			vertex_count++;
		}
	}

	fclose(f);
	printf("Number of vetex lines loaded = %d\n", vertex_count);

	allocate_mesh(&md, vertex_count, vertex_count);
	memcpy(md.vertices, b.data, vertex_count*sizeof(float)*3);

	for(int i = 0; i < vertex_count; i++){
		md.indices[i] = i;
	}


	destroy_bucket(&b);

	__swap_y_and_z_for_opengl(&md);
	calculate_normals(&md);

	normalize_mesh(&md);
	return md;

}
Mesh_Data load_stl_file(char *filepath){

	Mesh_Data md;
	int file_length = 0;
	char *file = load_raw(filepath, &file_length);
	

	// Check if this is an ascii version or binary version
	// I would love to be able to just check for the word "solid"
	// at the beginning of the file, but I have found examples of
	// binary stl files that still have that "solid" in their
	// 80 byte headers, dumb
	int we_think_its_ascii = 0;
	
	char check[] = "facet normal";
	int length_of_check = strlen(check);
	// If its not in the first thousands bytes then I think we can assume its not in here at all
	// If someone puts a crazy long name in for the solid I guess this could be a problem!?
	for(int i = 0; (i < file_length - length_of_check) && (i < 1000) && (we_think_its_ascii == 0); i++){
		for(int j = 0; j < length_of_check; j++){
			if(file[i+j] != check[j]){
				break;
			}
			if(j == (length_of_check-1)){
				we_think_its_ascii = 1;
			}
		}
	}

	if(we_think_its_ascii == 1){
		printf("Loading %s as ascii stl file\n", filepath);
		md = __load_ascii_stl_file(filepath);
		//md = __load_ascii_stl_file(file_length, file);
	}else{
		printf("Loading %s as binary stl file\n", filepath);
		md = __load_binary_stl_file(file_length, file);
	}
	
	free(file);
	return md;
}

Mesh_Data load_mesh_data(char *filepath){
	Mesh_Data md;
	int file_length = 0;
	char *file = load_file_as_string(filepath, &file_length);

	int vertex_lines = 0;
	int	texture_lines = 0; 
	int normal_lines = 0;
	int face_lines = 0;

	char line_buffer[256];
	int index = 0;
	int line_length = 0;
	printf("Calculating needed space\n");
	while(file[index] != '\0'){
		line_length = 0;
		while(file[index] != '\n' && line_length < 255){
			line_buffer[line_length++] = file[index++];
		}
		index++;
		line_buffer[line_length] = '\0';

		if(line_buffer[0] == 'v' && line_buffer[1] == 't'){
			texture_lines++;
		}else if(line_buffer[0] == 'v' && line_buffer[1] == 'n'){
			normal_lines++;
		}else if(line_buffer[0] == 'v'){
			vertex_lines++;
		}else if(line_buffer[0] == 'f'){
			face_lines++;
		}
	}
	
	printf("#v=%d, #vt=%d, #vn=%d, #f=%d\n", vertex_lines, texture_lines, normal_lines, face_lines);

	float *v = (float *)malloc(sizeof(float) * 3 * vertex_lines);
	int vs_loaded = 0;
	
	float *t = (float *)malloc(sizeof(float) * 2 * texture_lines);
	int ts_loaded = 0;
	
	float *n = (float *)malloc(sizeof(float) * 3 * normal_lines); 
	int ns_loaded = 0;

	//vertex, texture, normal
	int *table = (int *)malloc(sizeof(int) * face_lines * 3 * 3);
	memset(table, 0xff, sizeof(int)*face_lines*3);

	unsigned int *indices = (unsigned int *)malloc(sizeof(unsigned int) * face_lines * 3);
	unsigned int indices_loaded = 0;

	int final_length = 0;

	index = 0;
	printf("Doing main load\n");
	while(file[index] != '\0'){
		line_length = 0;
		while(file[index] != '\n' && line_length < 255){
			line_buffer[line_length++] = file[index++];
		}
		index++;
		line_buffer[line_length] = '\0';

		if(line_buffer[0] == 'v' && line_buffer[1] == 't'){
			float vt1 = 0;
			float vt2 = 0;

			int scan = sscanf(line_buffer, "vt %f %f", &vt1, &vt2);
			if(scan != 2){
				printf("Failed loading obj file '%s', bad vt line format, line given = '%s'\n", filepath, line_buffer);
				exit(0);
			}
			t[ts_loaded++] = vt1;
			t[ts_loaded++] = vt2;
		}else if(line_buffer[0] == 'v' && line_buffer[1] == 'n'){
			float vn1 = 0;
			float vn2 = 0;
			float vn3 = 0;

			int scan = sscanf(line_buffer, "vn %f %f %f", &vn1, &vn2, &vn3);
			if(scan != 3){
				printf("Failed loading obj file '%s', bad vn line format, line given = '%s'\n", filepath, line_buffer);
				exit(0);
			}
			n[ns_loaded++] = vn1;
			n[ns_loaded++] = vn2;
			n[ns_loaded++] = vn3;
		}else if(line_buffer[0] == 'v'){
			float v1 = 0;
			float v2 = 0;
			float v3 = 0;

			int scan = sscanf(line_buffer, "v %f %f %f", &v1, &v2, &v3);
			if(scan != 3){
				printf("Failed loading obj file '%s', bad v line format, line given = '%s'\n", filepath, line_buffer);
				exit(0);
			}
			v[vs_loaded++] = v1;
			v[vs_loaded++] = v2;
			v[vs_loaded++] = v3;
		}else if(line_buffer[0] == 'f'){
			int is[9];

			int scan = sscanf(line_buffer, "f %d/%d/%d %d/%d/%d %d/%d/%d",
				&is[0], &is[1], &is[2],
				&is[3], &is[4], &is[5],
				&is[6], &is[7], &is[8]);

			if(scan != 9){
				printf("Failed loading obj file '%s', bad f line format, line given = '%s'\n", filepath, line_buffer);
				exit(0);
			}
			int a, b, c;
			for(int i = 0; i < 3; i++){
				a = is[i*3]   - 1;
				b = is[i*3+1] - 1;
				c = is[i*3+2] - 1;
				int found = -1;
				for(int j = final_length - 1; j >= 0; j--){
					if(a == table[j*3] && b == table[j*3+1] && c == table[j*3+2]){
						found = j;
						break;
					}
				}
				if(found == -1){
					//final_length++;
					table[final_length*3] = a;
					table[final_length*3+1] = b;
					table[final_length*3+2] = c;

					found = final_length++;
				}
				indices[indices_loaded++] = found;
			}
		}
	}
	float *final_v = (float *)malloc(sizeof(float) * 3 * final_length);
	float *final_t = (float *)malloc(sizeof(float) * 2 * final_length);
	float *final_n = (float *)malloc(sizeof(float) * 3 * final_length);
	
	//I made this into three loops because maybe cache behaviour?
	//I should try both ways and see if either is faster.
	printf("Writing final arrays\n");
	printf("Final_length = %d, Face_lines = %d\n", final_length, face_lines);
	
	for(int i = 0; i < final_length; i++){
		int vertex_index = table[i*3];
		final_v[i*3] = v[vertex_index*3];
		final_v[i*3+1] = v[vertex_index*3+1];
		final_v[i*3+2] = v[vertex_index*3+2];
	}
	
	for(int i = 0; i < final_length; i++){
		int texture_index = table[i*3+1];
		final_t[i*2] = t[texture_index*2];
		final_t[i*2+1] = t[texture_index*2+1];
	}

	for(int i = 0; i < final_length; i++){
		int normal_index = table[i*3+2];
		final_n[i*3] = n[normal_index*3];
		final_n[i*3+1] = n[normal_index*3+1];
		final_n[i*3+2] = n[normal_index*3+2];
	}

	md.vertices = final_v;
	md.vertex_count = final_length*3;

	md.normals = final_n;
	md.normal_count = final_length*3;

	md.textures = final_t;
	md.texture_count = final_length*2;

	md.indices = indices;
	md.index_count = indices_loaded;

	__swap_y_and_z_for_opengl(&md);

	calculate_normals(&md);

	free(v);
	free(n);
	free(t);
	free(table);

	free(file);

	printf("Done\n");
	return md;
}

Mesh_Data make_hexagon_mesh(){
	Mesh_Data ret;
	ret.vertex_count = 7*3;
	ret.normal_count = 7*3;
	ret.texture_count = 7*2;
	ret.index_count = 7*3;

	ret.vertices = (float *)malloc(sizeof(float) * ret.vertex_count);
	ret.normals = (float *)malloc(sizeof(float) * ret.normal_count);
	ret.textures = (float *)malloc(sizeof(float) * ret.texture_count);
	ret.indices = (unsigned int *)malloc(sizeof(unsigned int) * ret.index_count);

	ret.vertices[0] = 0.0f;
	ret.vertices[1] = 0.0f;
	ret.vertices[2] = 0.0f;

	ret.textures[0] = 0.5f;
	ret.textures[1] = 0.5f;

	int index = 3;
	int text_index = 2;
	for(float f = 0; f < 2*3.14f; f += 2*3.14159 / 6){
		ret.vertices[index] = cos(f);
		ret.vertices[index+1] = 0.0f;
		ret.vertices[index+2] = sin(f);

		ret.textures[text_index] = ret.vertices[index]/2.0f + 0.5f;
		ret.textures[text_index + 1] = ret.vertices[index+2]/2.0f + 0.5f;
		text_index += 2;

		index += 3;
	}
	index = 0;
	for(int i = 1; i < 7; i++){
		ret.indices[index] = 0;
		ret.indices[index+1] = i;
		ret.indices[index+2] = i%6 + 1;
		
		index += 3;
	}

	for(int i = 0; i < 7; i++){
		ret.normals[i*3] = 0.0f;
		ret.normals[i*3+1] = 1.0f;
		ret.normals[i*3+2] = 0.0f;
	}

	return ret;
}

Mesh_Data make_plane_mesh(){
	Mesh_Data ret;
	ret.vertex_count = 4*3;
	ret.normal_count = 4*3;
	ret.texture_count = 4*2;
	ret.index_count = 6;

	ret.vertices = (float *)malloc(sizeof(float) * ret.vertex_count);
	ret.normals = (float *)malloc(sizeof(float) * ret.normal_count);
	ret.textures = (float *)malloc(sizeof(float) * ret.texture_count);
	ret.indices = (unsigned int *)malloc(sizeof(unsigned int) * ret.index_count);

	ret.vertices[0] = 1.0f; ret.vertices[1] = 1.0f; ret.vertices[2] = 0.0f;
	ret.vertices[3] = -1.0f; ret.vertices[4] = 1.0f; ret.vertices[5] = 0.0f;
	ret.vertices[6] = -1.0f; ret.vertices[7] = -1.0f; ret.vertices[8] = 0.0f;
	ret.vertices[9] = 1.0f; ret.vertices[10] = -1.0f; ret.vertices[11] = 0.0f;

	//Blah
	memset(ret.normals, 0, sizeof(float)*ret.normal_count);
	ret.normals[2] = 1.0f;
	ret.normals[5] = 1.0f;
	ret.normals[8] = 1.0f;
	ret.normals[11] = 1.0f;

	ret.textures[0] = 1.0f; ret.textures[1] = 1.0f;
	ret.textures[2] = 0.0f; ret.textures[3] = 1.0f;
	ret.textures[4] = 0.0f; ret.textures[5] = 0.0f;
	ret.textures[6] = 1.0f; ret.textures[7] = 0.0f;

	ret.indices[0] = 0; ret.indices[1] = 1; ret.indices[2] = 2;
	ret.indices[3] = 0; ret.indices[4] = 2; ret.indices[5] = 3;

	return ret;
}

void normalize_mesh(Mesh_Data *md){
	float max_mag = 0.0f;
	for(int i = 0; i < md->vertex_count; i+=3){
		float x = *(md->vertices+i);
		float y = *(md->vertices+i+1);
		float z = *(md->vertices+i+2);

		float mag = sqrt(x*x + y*y + z*z);
		if(mag > max_mag) max_mag = mag;
	}
	for(int i = 0; i < md->vertex_count; i+=3){
		float x = md->vertices[i];
		float y = md->vertices[i+1];
		float z = md->vertices[i+2];

		float mag = sqrt(x*x + y*y + z*z);
		float ratio = mag/max_mag;
		
		x = (x/mag)*ratio;
		y = (y/mag)*ratio;
		z = (z/mag)*ratio;

		md->vertices[i] = x;
		md->vertices[i+1] = y;
		md->vertices[i+2] = z;
	}
}

void move_mesh_origin(Mesh_Data *md, Vec3 change){
	for(int i = 0; i < md->vertex_count; i+=3){
		md->vertices[i] += change.x;
		md->vertices[i+1] += change.y;
		md->vertices[i+2] += change.z;
	}
}

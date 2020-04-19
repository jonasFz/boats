#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "matrix.h"
#include "renderer.h"
#include "mesh.h"
#include "input.h"

#include "hull.h"

char *load_file_as_string(const char *file_path, int *length){
	FILE* in_file = fopen(file_path, "r");
	if(!in_file){
		fprintf(stderr, "Failed to load file %s\n", file_path);
		return NULL;
	}
	fseek(in_file, 0, SEEK_END);
	int file_length = ftell(in_file);
	fseek(in_file, 0, SEEK_SET);
	*length = file_length;
	char *ret = (char *)malloc(file_length+1);

	fread(ret, 1, file_length, in_file);
	ret[file_length] = '\0';
	
	
	fclose(in_file);
	return ret;
}

float default_texture_data[12] = {
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f
};

void make_entity(Entity *entity, Buffered_Mesh_Handle handle, unsigned int texture){
	entity->texture_id = texture;
	entity->position = make_vec3(0.0f, 0.0f, 0.0f);
	entity->scale = make_vec3(1.0f, 1.0f, 1.0f);
	entity->rotation = make_vec3(0.0f, 0.0f, 0.0f);
	entity->colour = make_vec3(1.0f, 1.0f, 1.0f);
	entity->handle = handle;
}

void move_entity(Entity *entity, float x, float y, float z){
	entity->position.x += x;
	entity->position.y += y;
	entity->position.z += z;
}

int main(){
	assert(sizeof(Vec3) == 12);

	Hull h;
	h.keel_length = 10;
	h.keel_width = 0.5f;
	h.keel_depth = 0.5f;

	h.bow_depth = 0.5f;
	h.bow_width = 0.5f;
	h.bow_radius = 3;

	Mesh_Data keel;
	Mesh_Data bow;

	generate_keel(&h, &keel);
	generate_bow(&h, &bow);

	Renderer renderer = init_display(); 

	Buffered_Mesh_Handle keel_handle = buffer_mesh_data(keel);
	Buffered_Mesh_Handle bow_handle = buffer_mesh_data(bow);

	free_mesh(&keel);
	free_mesh(&bow);

	//TODO find out why the buffer_texture call has to happen after the buffer_mesh_data call above
	unsigned int texture_handle = buffer_texture(default_texture_data, 2, 2);
	GLuint shader_program = load_glsl_program("vertex.txt", "fragment.txt");
	
	Render_Context context;
	context.projection = make_projection_matrix(4.0f/3.0f, 1.0f, 1000, 0.1);

	Surface surface = make_surface(10, 6);
	for(int i = 0; i< 10; i++){
		for(int j = 0; j < 6; j++){
			Vec3 before = get_position(&surface, i, j);
			before.x *= 2;
			//before = scale(before, make_vec3(20.0f, 1.0f, 1.0f));
			//show_vec3(before);
			
			Vec3 after = make_vec3(before.x, before.y, ((float)j/6) *  -before.x*before.x+2);
			set_position(&surface, i, j, after);
		}
	}

	Mesh_Data surface_mesh = mesh_from_surface(surface);

	Buffered_Mesh_Handle surface_handle = buffer_mesh_data(surface_mesh);

	unsigned int number_of_entities = 1;
	Entity *entities = (Entity *)malloc(sizeof(Entity) * number_of_entities);

	make_entity(entities, surface_handle, texture_handle);
/*
	unsigned int number_of_entities = number_of_pieces(&h) + 5;
	Entity *entities = (Entity *)malloc(sizeof(Entity) * number_of_entities);
	
	make_entity(entities, keel_handle, texture_handle);
	make_entity(entities+1, bow_handle, texture_handle);
	
	move_entity(entities+1, h.keel_length/2, 0, 0);

	for(int i = 0; i < 5; i++){
		Mesh_Data rib;
		generate_arc(&rib, 1.57f, 10, 0.2f, 0.5f, 3, make_vec3(-1.0f, 0.0f, 0.0f), make_vec3(0.0f, 1.0f, 0.0f));
		Buffered_Mesh_Handle mesh_handle = buffer_mesh_data(rib);
		Entity *ent = entities+2+i;
		make_entity(ent, mesh_handle, texture_handle);
		float l = h.keel_length - 0.2f;
		ent->position = make_vec3(i*l/4 - l/2, 0.0f, -h.keel_width/2);
		ent->rotation = make_vec3(0.0f, 1.57f, 0.0f);
		free_mesh(&rib);
	}
*/
	context.camera.entity.position = make_vec3(0.0f, 7.0f, 15.0f);
	context.camera.facing = make_vec3(0.0f, 0.0f, -1.0f);
	context.camera.angle = make_vec3(0.0f, 0.0f, 0.0f);
	context.shader_program = shader_program;
	
	context.light_position = make_vec3(0.0f, 10.0f, 2.0f);
	context.light_colour = make_vec3(1.0f, 1.0f, 1.0f);

	Input_State input_state;
	memset(&input_state, 0, sizeof(Input_State));

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);

	while (1) {
		maybe_update_mouse_position(&input_state, &renderer);
		Vec3 p = make_vec3(0.0f, 0.0f, -1.0f);
		Mat4 facing_rotation = make_rotation_matrix(context.camera.angle.x, context.camera.angle.y, context.camera.angle.z);

		p = transform(&facing_rotation, &p);

		Vec3 r = cross_product(p, make_vec3(0.0f, 1.0f, 0.0f));

		#define ROT_AMOUNT 0.06f

		int kc = -1;
		if((kc = get_key_press(&renderer)) != -1){
			switch(kc){
				case 38:
				r = scale_vec(r, -1.0f);
				context.camera.entity.position.x += r.x;
				context.camera.entity.position.y += r.y;
				context.camera.entity.position.z -= r.z;
				break;
				case 25:
				p = scale_vec(p, -1.0f);
				context.camera.entity.position.x += p.x;
				context.camera.entity.position.y += p.y;
				context.camera.entity.position.z -= p.z;
				break;
				case 40:
				context.camera.entity.position.x += r.x;
				context.camera.entity.position.y += r.y;
				context.camera.entity.position.z -= r.z;
				break;
				case 39:
				p = scale_vec(p, 1.0f);
				context.camera.entity.position.x += p.x;
				context.camera.entity.position.y += p.y;
				context.camera.entity.position.z -= p.z;
				break;
				case 111:
				context.camera.angle.x -= ROT_AMOUNT;
				break;
				case 114:
				context.camera.angle.y += ROT_AMOUNT;
				break;
				case 116:
				context.camera.angle.x += ROT_AMOUNT;
				break;
				case 113:
				context.camera.angle.y -= ROT_AMOUNT;
				break;
			}
		}

		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program);

		render_entities(&renderer, &context, entities, number_of_entities);

		swap_buffer(&renderer);
	}

	close_display(&renderer);

	return 0;
}

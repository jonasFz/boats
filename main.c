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
	1.0f, 0.0f, 1.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 1.0f
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


	unsigned int texture_handle = buffer_texture(default_texture_data, 2, 2);


	//unsigned int texture_handle = buffer_texture(data, 128, 128);
	Buffered_Mesh_Handle keel_handle = buffer_mesh_data(keel);
	Buffered_Mesh_Handle bow_handle = buffer_mesh_data(bow);

	free_mesh(&keel);
	free_mesh(&bow);

	//TODO find out why the buffer_texture call has to happen after the buffer_mesh_data call above
	//unsigned int texture_handle = buffer_texture(default_texture_data, 2, 2);
	printf("Texture handle = %u\n", texture_handle);
	GLuint shader_program = load_glsl_program("vertex.txt", "fragment.txt");
	
	Render_Context context;
	context.projection = make_projection_matrix(4.0f/3.0f, 1.0f, 1000, 0.1);
	

	GLuint text_shader = load_glsl_program("text_vertex.txt", "text_fragment.txt");
	context.text_shader_program = text_shader;


	float l_div = 10.0f/11;

	float dys[11] = {0.13f, 0.15f, 0.16f, 0.169f, 0.175f, 0.175f, 0.165f, 0.150f, 0.12f, 0.075f, 0.0f};

	float shape[6] = {0.0f, 0.1f, 0.6, 0.90f, 1.0f, 0.99f};

	Surface port_surface = make_surface(11, 6);
	Surface star_surface = make_surface(11, 6);
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 6; j++){
			Vec3 before = get_position(&port_surface, i, j);
			before.x = l_div*i;
			before.z = dys[i]*6.0f*shape[j];
			//before = scale(before, make_vec3(20.0f, 1.0f, 1.0f));
			//show_vec3(before);
			
			set_position(&port_surface, i, j, before);
			before.z = before.z*(-1);
			set_position(&star_surface, 10-i, j, before);
		}
	}

	Mesh_Data port_mesh = mesh_from_surface(port_surface);
	Mesh_Data star_mesh = mesh_from_surface(star_surface);
	//flip_normals(&star_mesh);

	Buffered_Mesh_Handle port_surface_handle = buffer_mesh_data(port_mesh);
	Buffered_Mesh_Handle star_surface_handle = buffer_mesh_data(star_mesh);

	Mesh_Data test = load_stl_file("res/toy_boat.stl");

	//Mesh_Data test = load_mesh_data("res/better_teapot.obj");
	Buffered_Mesh_Handle test_handle = buffer_mesh_data(test);

	unsigned int number_of_entities = 1;
	Entity *entities = (Entity *)malloc(sizeof(Entity) * number_of_entities);

	make_entity(entities, test_handle, texture_handle);

	//make_entity(entities, port_surface_handle, texture_handle);
	//make_entity(entities+1, star_surface_handle, texture_handle);

	//Billboard stuff
	Mesh_Data quad = make_plane_mesh();
	Buffered_Mesh_Handle quad_handle = buffer_mesh_data(quad);
//	free_mesh(&quad);
	//make_entity(entities+2, quad_handle, texture_handle);
	//entities[2].scale.y = 2;

	context.billboard_quad = quad_handle;
/*
	Mesh_Data polygon = make_extruded_polygon(6, 0.1f, 1.0f);
	Buffered_Mesh_Handle polygon_handle = buffer_mesh_data(polygon);
	make_entity(entities+2, polygon_handle, texture_handle);
	entities[2].position.y = 5;
*//*

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
	context.camera.entity.position = make_vec3(0.0f, 2.0f, 15.0f);
	context.camera.facing = make_vec3(0.0f, 0.0f, -1.0f);
	context.camera.angle = make_vec3(0.0f, 0.0f, 0.0f);
	context.shader_program = shader_program;
	
	context.light_position = make_vec3(0.0f, 10.0f, 0.0f);
	context.light_colour = make_vec3(1.0f, 1.0f, 1.0f);

	Input_State input_state;
	memset(&input_state, 0, sizeof(Input_State));

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);

	int running = 1;

	while (running) {
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
				case 9:
				running = 0;
				break;
				default:
				printf("Key code: %d\n", kc);
				break;
			}
		}

		glClearColor(0.8, 0.8, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program);

		render_entities(&renderer, &context, entities, number_of_entities);

		Vec3 dim = get_window_dimensions(&renderer);

		float relx = (2.0*dim.y/dim.x)/2 - 1.0f;

	//	render_billboard(&renderer, &context, texture_handle, -1, 1, relx, -1); 

		swap_buffer(&renderer);
	}

	close_display(&renderer);

	return 0;
}

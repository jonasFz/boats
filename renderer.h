#ifndef _H_RENDERER
#define _H_RENDERER

#ifdef __linux__
	#include <GL/glew.h>
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif

#include "matrix.h"
#include "mesh.h"

typedef struct{
	unsigned int pointer;
	unsigned int index_count;
	unsigned int index_buffer_pointer;
	unsigned int vertex_buffer_pointer;
	unsigned int normal_buffer_pointer;	
}Buffered_Mesh_Handle;

typedef struct{
	char *data;
	int width;
	int height;
}Texture;

typedef struct{
	Vec3 position;
	Vec3 rotation;
	Vec3 scale;
	Vec3 colour;

	Buffered_Mesh_Handle handle;
	unsigned int texture_id;
}Entity;

typedef struct{
	Entity entity;
	Vec3 facing;
	Vec3 angle;
}Camera;

typedef struct{
	Mat4 projection;
	Camera camera;
	int shader_program;

	Vec3 light_position;
	Vec3 light_colour;
}Render_Context;

#ifdef __linux__
typedef GLXContext (*proc)(Display*, GLXFBConfig, GLXContext, int, const int*);
#endif
unsigned int buffer_texture(float *data, int width, int height);

typedef struct{
	//Platform stuff
	Display *display;
	Window window;
	XEvent event;
}Renderer;


void render_entities(Renderer *renderer, Render_Context *context, Entity *entities, int entity_count);

void render_text(Renderer *renderer, const char *text, float x, float y);

int get_key_press(Renderer *renderer);
Vec3 get_mouse_position(Renderer *renderer);

void swap_buffer(Renderer* renderer);

Renderer init_display();

void close_display(Renderer* renderer);

Buffered_Mesh_Handle buffer_mesh_data(Mesh_Data mesh_data);

int load_glsl_program(const char *vertex_shader_path, const char *fragment_shader_path);

#endif

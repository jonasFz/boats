#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

char *load_file_as_string(const char *file_path, int *length);

Vec3 get_window_dimensions(Renderer *renderer){
	
	#ifdef __linux__
	Window root;
	int x;
	int y;
	unsigned int width;
	unsigned int height;
	unsigned int border_width;
	unsigned int depth;
	XGetGeometry(
		renderer->display,
		renderer->window,
		&root,
		&x,
		&y,
		&width,
		&height,
		&border_width,
		&depth);
	Vec3 ret;
	ret.x = width;
	ret.y = height;
	ret.z = 0.0f;
	
	return ret;
	#else
	return make_vec3(0.0f, 0.0f, 0.0f);
	#endif
}

Vec3 get_mouse_position(Renderer *renderer){
	#ifdef __linux__
	Window root;
	Window child;
	int root_x;
	int root_y;
	int win_x;
	int win_y;
	unsigned int mask;

	int all_good = XQueryPointer(
		renderer->display,
		renderer->window,
		&root,
		&child,
		&root_x,
		&root_y,
		&win_x,
		&win_y,
		&mask);

	if(!all_good){
		printf("GameWarning: Failed to get mousepointer in function get_mouse_position\n");
	}

	Vec3 dimensions = get_window_dimensions(renderer);

	Vec3 ret;
	ret.x = win_x/dimensions.x;
	ret.y = win_y/dimensions.y;
	ret.z = 0.0f;

	return ret;
	#else
	return make_vec3(0.0f, 0.0f, 0.0f);
	#endif
}

void swap_buffer(Renderer* renderer){
	glXSwapBuffers(renderer->display, renderer->window);
}

void close_display(Renderer* renderer){
	XCloseDisplay(renderer->display);
}


void print_gl_error(GLenum val){
	switch(val){
		case GL_NO_ERROR:
			printf("GL_NO_ERROR\n");
			break;
		case GL_INVALID_ENUM:
			printf("GL_INVALID_ENUM\n");
			break;
		case GL_INVALID_VALUE:
			printf("GL_INVALID_VALUE\n");
			break;
		case GL_INVALID_OPERATION:
			printf("GL_INVALID_OPERATION\n");
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			printf("GL_INVALID_FRAMEBUFFER_OPERATION\n");
			break;
		case GL_OUT_OF_MEMORY:
			printf("GL_OUT_OF_MEMORY\n");
			break;
		default:
			printf("%d is an unknow gl error\n", val);
	}
}

void render_entities(Renderer *renderer, Render_Context *context, Entity* entities, int entity_count){
	
	for(int i = 0; i < entity_count; i++){
		Entity entity = entities[i];
		Buffered_Mesh_Handle handle = entity.handle;
		glBindVertexArray(handle.pointer);

		Mat4 scale = make_scale_matrix(entity.scale.x, entity.scale.y, entity.scale.z);
		Mat4 trans = make_translation_matrix(entity.position.x, entity.position.y, entity.position.z);
		Mat4 rot = make_rotation_matrix(entity.rotation.x, entity.rotation.y, entity.rotation.z);
			
		Mat4 m = multiply_matrix(&rot, &scale);
		m = multiply_matrix(&m, &trans);


		Mat4 v = make_identity_matrix();
		Mat4 v_r = make_rotation_matrix(context->camera.angle.x, context->camera.angle.y, context->camera.angle.z);
		Mat4 v_t = make_translation_matrix(
			-context->camera.entity.position.x,
			-context->camera.entity.position.y,
			-context->camera.entity.position.z
		);
		v = multiply_matrix(&v_t, &v_r);

		GLuint mat_id = glGetUniformLocation(context->shader_program, "trans");
		glUniformMatrix4fv(mat_id, 1, GL_FALSE, m.data);
		GLuint projection_id = glGetUniformLocation(context->shader_program, "projection");
		glUniformMatrix4fv(projection_id, 1, GL_FALSE, context->projection.data);

		GLuint view_id = glGetUniformLocation(context->shader_program, "view");
		glUniformMatrix4fv(view_id, 1, GL_FALSE, v.data);

		GLuint col_id= glGetUniformLocation(context->shader_program, "col");
		glUniform3f(col_id, entity.colour.x, entity.colour.y, entity.colour.z); 

		GLuint light_id = glGetUniformLocation(context->shader_program, "light_position");
		glUniform3f(light_id, context->light_position.x, context->light_position.y, context->light_position.z);
		
		GLuint light_colour_id = glGetUniformLocation(context->shader_program, "light_colour");
		glUniform3f(light_colour_id, context->light_colour.x, context->light_colour.y, context->light_colour.z);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, entity.texture_id);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glDrawElements(GL_TRIANGLES, handle.index_count, GL_UNSIGNED_INT, (void *)0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		GLenum er = glGetError();
		if(er != GL_NO_ERROR){
			printf("We got some kinda error in the render loop\n");
			print_gl_error(er);
			exit(1);
		}
	}
}

void render_text(Renderer *renderer, const char *text, float x, float y){

}

int get_key_press(Renderer *renderer){
	long mask = KeyPressMask;
	XEvent input_event;
	if(XCheckWindowEvent(renderer->display, renderer->window, mask, &input_event)){
		return input_event.xkey.keycode;
	}else{
		return -1;
	}
}

Renderer init_display(){

	Renderer renderer;
	renderer.display = XOpenDisplay(NULL);
	if(renderer.display == NULL){
		printf("Failed to open the display for some reason\n");
		exit(1);
	}

	GLint glx_attributes[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_DEPTH_SIZE, 24,
		GLX_STENCIL_SIZE, 8,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_SAMPLE_BUFFERS, 0,
		GLX_SAMPLES, 4,
		None
	};

	Window root = DefaultRootWindow(renderer.display);

	int number = 0;
	GLXFBConfig *fbc = glXChooseFBConfig(renderer.display, DefaultScreen(renderer.display), glx_attributes, &number);
	
	if(number == 0){
		printf("Failed to get any GLXFBConfigs\n");
	}

	XVisualInfo *visual = glXGetVisualFromFBConfig(renderer.display, fbc[0]);
	if(!visual){
		printf("Failed to get XVisualInfo\n");
	}
	GLXContext old = glXCreateContext(renderer.display, visual, 0, GL_TRUE);
	
	glXMakeCurrent(renderer.display, 0, 0);
	glXDestroyContext(renderer.display, old);

	int context_attributes[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		None
	};

	Colormap color_map = XCreateColormap(renderer.display, root, visual->visual, AllocNone);
	
	XSetWindowAttributes swa;
	swa.colormap = color_map;
	swa.event_mask = ExposureMask | KeyPressMask;

	renderer.window = XCreateWindow(renderer.display, root, 10, 10, 800, 600, 0, visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &swa);

	XMapWindow(renderer.display, renderer.window);

	proc glXCreateContextAttribsARB = (proc)glXGetProcAddressARB((void*)"glXCreateContextAttribsARB");

	GLXContext context = glXCreateContextAttribsARB(renderer.display, fbc[0], NULL, 1, context_attributes);
	
	if(!context){
		printf("Failed to get context\n");
	}

	glXMakeCurrent(renderer.display, renderer.window, context);

	glewExperimental = 1;
	glewInit();
	
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("GL:   %s\n", glGetString(GL_VERSION));
	
	return renderer;
}


unsigned int buffer_texture(float *data, int width, int height){
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	GLenum er = glGetError();
	if(er != GL_NO_ERROR){
		printf("Error occured while buffering texture\n");
		print_gl_error(er);
		exit(1);
	}

	return texture;
}

Buffered_Mesh_Handle buffer_mesh_data(Mesh_Data mesh_data){
	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh_data.vertex_count, mesh_data.vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	GLuint normal_buffer;
	glGenBuffers(1, &normal_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh_data.normal_count, mesh_data.normals, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	GLuint texture_buffer;
	glGenBuffers(1, &texture_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, texture_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh_data.texture_count, mesh_data.textures, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	GLuint index_buffer;
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * mesh_data.index_count, mesh_data.indices, GL_STATIC_DRAW);

	Buffered_Mesh_Handle handle;
	handle.pointer = vertex_array;
	handle.index_count = mesh_data.index_count;
	handle.index_buffer_pointer = index_buffer;
	handle.vertex_buffer_pointer = vertex_buffer;
	handle.normal_buffer_pointer = normal_buffer;
	return handle;
}

int __load_and_compile_shader(const char *shader_path, unsigned int type){
	int length;
	const char* shader_source = load_file_as_string(shader_path, &length);

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &shader_source, NULL);
	glCompileShader(shader);
	
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE){
		if(type == GL_VERTEX_SHADER){
			printf("Failed to compile vertex shader\n");
		}else if(type == GL_FRAGMENT_SHADER){
			printf("Failed to compile fragment shader\n");
		}
		GLint log_size = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		char *log = (char *)malloc(log_size);
		int len;
		glGetShaderInfoLog(shader, log_size, &len, log);
		printf("%s\n", log);
		free(log);
	}

	//free(shader_source);
	if(success == GL_FALSE){
		return -1;
	}
	return shader;
}

int load_glsl_program(const char *vertex_shader_path, const char *fragment_shader_path){
	int vertex_shader 	= __load_and_compile_shader(vertex_shader_path, GL_VERTEX_SHADER);
	int fragment_shader = __load_and_compile_shader(fragment_shader_path, GL_FRAGMENT_SHADER);

	GLuint shader_program;
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	GLint success = 0;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if(!success){
		printf("Failed to link the shader\n");
		//TODO: cleanup the shaders that we generated a second ago?
		return -1;
	}
	return shader_program;
}

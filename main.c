#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

// @Platform!
#include <arpa/inet.h>

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

struct Chunk{
	unsigned int length;
	unsigned char type[4];
	void *data;
	unsigned int crc;
};

struct IHDR_Header{
	unsigned int width;
	unsigned int height;
	unsigned char bit_depth;
	unsigned char color_type;
	unsigned char compression_method;
	unsigned char filter_method;
	unsigned char interlace_method;
};

struct Bit_Dripper{
	unsigned int byte_count;
	unsigned int byte_index;
	void *bytes;
	char done;
	
	unsigned int index_in_current;
};

struct Bit_Dripper make_dripper(void *data, unsigned int length){
	assert(length > 0);
	assert(data != NULL);
	struct Bit_Dripper bd;
	bd.byte_count = length;
	bd.byte_index = 0;
	bd.bytes = data;
	bd.done = 0;
	bd.index_in_current = 0;

	return bd;
}

char drip(struct Bit_Dripper *bd){
	assert(!bd->done);

	char result = (((char *)(bd->bytes))[bd->byte_index] >> bd->index_in_current) & 1;
	bd->index_in_current++;
	if(bd->index_in_current == 8){
		bd->index_in_current = 0;
		bd->byte_index++;

		if(bd->byte_index == bd->byte_count){
			bd->done = 1;
		}
	}
	return result;
}

void deflate(char *data, int data_length){
	printf("We in here!\n");

	unsigned char CMF = data[0];

	printf("%x\n", CMF);

	unsigned char FLG = data[1];

	unsigned char compression_method = CMF&0xf;
	unsigned int compression_info = (CMF >> 4) & 0xf;

	if(compression_method != 8){
		printf("We would expect the compression method to be equal to 8 but it seems not to be\n");
	}

	printf("Compression method = %d\n", CMF&0xf);
	printf("Compression info = %d\n", (CMF >> 4) & 0xf);
	
	// 2^(compression_info + 8)
	unsigned int window_size = 1 << (compression_info + 8);
	printf("Window size = %d\n", window_size);

	assert(((FLG >> 5) & 1) == 0);

	printf("%x\n", data[2]);

	struct Bit_Dripper bd = make_dripper(data+2, data_length - 2);

	char bfinal = drip(&bd);
	char btype = drip(&bd) |(drip(&bd)<<1);

	if(btype != 2){
		printf("Compression type = %d is not yet implemented!\n", btype);
	}

	printf("bfinal = %d, btype = %d\n", bfinal, btype);
}

int main(){
	
	FILE *f = fopen("res/unnamed.png", "rb");
	assert(f != NULL);

	//Just printing out the required 8 header bytes
	for(int i = 0; i < 8; i++){
		printf("%d ", fgetc(f));
	}
	printf("\n");
	
	struct Chunk a;
	fread(&a.length, 1, 4, f);
	a.length = ntohl(a.length);
	fread(a.type, 1, 4, f);
	a.data = malloc(a.length);
	fread(a.data, 1, a.length, f);
	fread(&a.crc, 1, 4, f);

	assert(a.length == 13);
	printf("%d %c%c%c%c\n", a.length, a.type[0], a.type[1], a.type[2], a.type[3]);

	//Should probably make sure we actually have the IHDR header
	struct IHDR_Header *ihdr = (struct IHDR_Header *)a.data;
	ihdr->width = htonl(ihdr->width);
	ihdr->height = htonl(ihdr->height);

	printf("Width = %d, Height = %d, bit depth = %d, color type = %d, compression method = %d , filter method = %d, interlace method = %d\n", ihdr->width, ihdr->height, ihdr->bit_depth, ihdr->color_type, ihdr->compression_method, ihdr->filter_method, ihdr->interlace_method);

	while(1){
		struct Chunk b;
		fread(&b.length, 1, 4, f);
		b.length = ntohl(b.length);
		fread(b.type, 1, 4, f);
		b.data = malloc(b.length);
		fread(b.data, 1, b.length, f);
		fread(&b.crc, 1, 4, f);

		printf("%d %c%c%c%c\n", b.length, b.type[0], b.type[1], b.type[2], b.type[3]);

		if(b.type[0] == 'I' && b.type[1] == 'D' && b.type[2] == 'A' && b.type[3] == 'T'){
			deflate(b.data, b.length);
			//Handle the case where more IDAT follows!
			break;
		}

		if(b.type[0] == 'I' && b.type[1] == 'E' && b.type[2] == 'N' && b.type[3] == 'D'){
			break;
		}
	}

	// -----------------------------------------------
	//Don't touch bellow
	assert(sizeof(Vec3) == 12);

	Renderer renderer = init_display(); 
	unsigned int texture_handle = buffer_texture(default_texture_data, 2, 2);

	//TODO find out why the buffer_texture call has to happen after the buffer_mesh_data call above
	//unsigned int texture_handle = buffer_texture(default_texture_data, 2, 2);
	GLuint shader_program = load_glsl_program("vertex.txt", "fragment.txt");
	
	Render_Context context;
	context.projection = make_projection_matrix(4.0f/3.0f, 1.0f, 1000, 0.1);

	GLuint text_shader = load_glsl_program("text_vertex.txt", "text_fragment.txt");
	context.text_shader_program = text_shader;


	//Mesh_Data test = load_mesh_data("res/better_teapot.obj");
	Mesh_Data test = load_stl_file("res/sabrinas_pot.stl");
	Buffered_Mesh_Handle test_handle = buffer_mesh_data(test);

	unsigned int number_of_entities = 1;
	Entity *entities = (Entity *)malloc(sizeof(Entity) * number_of_entities);

	make_entity(entities, test_handle, texture_handle);

	context.camera.entity.position = make_vec3(0.0f, 0.0f, 10.0f);
	context.camera.facing = make_vec3(0.0f, 0.0f, 0.0f);
	context.camera.angle = make_vec3(0.0f, 0.0f, 0.0f);
	context.shader_program = shader_program;
	
	context.light_position = make_vec3(10.0f, 10.0f, 0.0f);
	context.light_colour = make_vec3(1.0f, 1.0f, 1.0f);

	Input_State input_state;
	memset(&input_state, 0, sizeof(Input_State));

	glEnable(GL_DEPTH_TEST);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_MULTISAMPLE);

	int running = 1;

	while (running) {
		maybe_update_mouse_position(&input_state, &renderer);
		Vec3 move = make_vec3(0.0, 0.0, -1.0);

		Vec3 cr = context.camera.angle;
		//show_vec3(cr);
		Mat4 rotation_matrix = make_rotation_matrix(cr.x, cr.y, cr.z);
		move = transform(&rotation_matrix, &move);

		Vec3 r = cross_product(move, make_vec3(0.0, 1.0, 0.0)); 
		r = normalize(r);

		//show_vec3(r);

		//show_vec3(move);

		#define ROT_AMOUNT 0.06f

		int kc = -1;
		if((kc = get_key_press(&renderer)) != -1){
			switch(kc){
				case 25:
				context.camera.entity.position.x += move.x;
				context.camera.entity.position.y += move.y;
				context.camera.entity.position.z += move.z;
				break;
				case 39:
				context.camera.entity.position.x -= move.x;
				context.camera.entity.position.y -= move.y;
				context.camera.entity.position.z -= move.z;
				break;
				case 40:
				context.camera.entity.position.x += r.x;
				context.camera.entity.position.y += r.y;
				context.camera.entity.position.z += r.z;
				break;
				case 38:
				context.camera.entity.position.x -= r.x;
				context.camera.entity.position.y -= r.y;
				context.camera.entity.position.z -= r.z;
				break;
				case 50:
				context.camera.entity.position.y += 1;
				break;
				case 37:
				context.camera.entity.position.y -= 1;
				break;
				case 111:
				context.camera.angle.x += ROT_AMOUNT;
				break;
				case 114:
				context.camera.angle.y -= ROT_AMOUNT;
				break;
				case 116:
				context.camera.angle.x -= ROT_AMOUNT;
				break;
				case 113:
				context.camera.angle.y += ROT_AMOUNT;
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

		swap_buffer(&renderer);
	}

	close_display(&renderer);

	return 0;
}

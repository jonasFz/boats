#ifndef _H_INPUT
#define _H_INPUT

#include "renderer.h"

typedef struct{
	float x;
	float y;
}Position;

typedef struct{
	Position mouse_position;
	int is_on_window;
}Input_State;

int maybe_update_mouse_position(Input_State *state, Renderer *renderer);

#endif

#include "input.h"
#include "matrix.h"

// Return 1 if we updated, return 0 otherwise
int maybe_update_mouse_position(Input_State *state, Renderer *renderer){
	Vec3 mouse_position = get_mouse_position(renderer);
	float x = mouse_position.x;
	float y = mouse_position.y;

	if(x > 1.0f || x < 0.0f || y > 1.0f || y < 0.0f){
		state->is_on_window = 0;
		return 0;
	}
	state->mouse_position.x = x;
	state->mouse_position.y = y;

	state->is_on_window = 1;
	return 1;
}

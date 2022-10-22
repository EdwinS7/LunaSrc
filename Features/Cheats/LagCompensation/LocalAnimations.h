#include "..\..\includes.hpp"
/* Lots of extra shit in here that is not used */

struct Local_data
{
	bool visualize_lag = false;

	c_baseplayeranimationstate* prediction_animstate = nullptr;
	c_baseplayeranimationstate* animstate = nullptr;

	int stored_command_number = 0;
	Vector stored_real_angles = ZERO;
	Vector real_angles = ZERO;
	Vector fake_angles = ZERO;
};

enum INVALIDATE_PHYSICS_BITS
{
	POSITION_CHANGED = 0x1,
	ANGLES_CHANGED = 0x2,
	VELOCITY_CHANGED = 0x4,
	ANIMATION_CHANGED = 0x8,
	BOUNDS_CHANGED = 0x10,
	SEQUENCE_CHANGED = 0x20
};

class local_animations : public singleton <local_animations>
{
	bool real_server_update = false;
	bool fake_server_update = false;

	float real_simulation_time = 0.0f;
	float fake_simulation_time = 0.0f;

	CBaseHandle* handle = nullptr;

	float spawntime = 0.0f;
	float tickcount = 0.0f;

	float abs_angles = 0.0f;
	float pose_parameter[24];
	AnimationLayer layers[15];

public:
	Local_data local_data;
	CUserCmd* command;

	void update_fake_animations();
	void update_local_animations();
	void run(ClientFrameStage_t stage);
	//void update_prediction_animations();
};
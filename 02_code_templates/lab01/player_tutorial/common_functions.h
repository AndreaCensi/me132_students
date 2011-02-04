#ifndef H_MY_FUNCTIONS
#define H_MY_FUNCTIONS

#include <libplayerc++/playerc++.h>

int check_robot_connection(PlayerCc::PlayerClient& robot, PlayerCc::Position2dProxy&pp, int num_attempts);

void write_error_details_and_exit(const char*program, PlayerCc::PlayerError&);

#endif
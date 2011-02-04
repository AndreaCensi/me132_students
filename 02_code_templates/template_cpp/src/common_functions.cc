#include <stdio.h>

#include "common_functions.h"
#include "cmdline_parsing.h"

using namespace PlayerCc;

int check_robot_connection(PlayerClient& robot, Position2dProxy&pp, int num_attempts) {

	// let's enable the motors first
	pp.SetMotorEnable(true);

	// now we run a quick for loop to make sure we can read from the motors (this may take some time)
	for(int i=0; i<num_attempts; i++) {
	     // set the initial odometry of the robot to be 0,0,0
	     pp.SetOdometry( 0.0, 0.0, 0.0);

	     // now try reading
	     robot.Read();

	     // can we read the motors?
	     if(pp.GetXPos()==0.0)
			return 1;

	}
	// if we maxed out our connection attempts, we'd better stop
 	fprintf(stderr, "failed to connect to robot after %d attempts.\n", num_attempts);
	return 0;
}


		
void write_error_details_and_exit(const char*program, PlayerError&e) {
	fprintf(stderr, "%s: Error while connecting to %s:%d.\n", 
			program, gHostname.c_str(), gPort );
	fprintf(stderr, "%s: Error details: %s\n", 
			program, e.GetErrorStr().c_str());
	exit(-1);
}


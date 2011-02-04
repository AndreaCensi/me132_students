#include <libplayerc++/playerc++.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>

#include "cmdline_parsing.h"
#include "common_functions.h"

// Player objects are in the "PlayerCC" namespace.
using namespace PlayerCc; 
using namespace std;

int main(int argc, char **argv)
{
    /* Calls the command line parser that puts the settings
       in global variables gXXX. */
    parse_args(argc, argv);

    try {
        // Try to connect with the given hostname and port
        PlayerClient robot(gHostname, gPort); 
        // Establish a proxy to query odometry values and send motor commands
        Position2dProxy pp(&robot, gIndex); 
		// Make sure we can communicate with the robot
		if(!check_robot_connection(robot, pp, 10)) 
			exit(-2);
			
        // Now we start the main processing loop
 		while(1) {

            // read from the proxies; YOU MUST ALWAYS HAVE THIS LINE
            robot.Read(); 

            // these next three lines illustrate how you query the 
			// positionproxy to gather the robot pose information
		    double xpos = pp.GetXPos();
		    double ypos = pp.GetYPos();
		    double thetapos = pp.GetYaw();
		
            // Let's output this data to the terminal
            printf("xpos: %f  ypos: %f  thetapos: %f\n", xpos, ypos, thetapos);

    		// The rest of your algorithm goes here;
        	// Ideally you'd want to have some logic in here that eventually
 			// outputs a robot control command.
            // In this example, we just set it to 1 m/s in speed and 1 rad/s 
			// in turnrate since we are dealing with two-wheeled non-holonomic 
			// dynamics in our robots
            double speed= 1; // m/s
            double turnrate = 1; // rad/s
        
            // control the robot by setting motion commands here
            pp.SetSpeed(speed, turnrate);
        }

    } catch(PlayerError e) {
		write_error_details_and_exit(argv[0], e);
	}
}

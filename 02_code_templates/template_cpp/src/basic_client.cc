#include <libplayerc++/playerc++.h>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <iostream>

#include "cmdline_parsing.h"
#include "my_functions.h"

using namespace std;

int main(int argc, char **argv)
{
    /* Calls the command line parser that puts the settings
       in global variables gXXX. */
    parse_args(argc, argv);
    
    // Player objects are in the "PlayerCC" namespace.
    using namespace PlayerCc; 
    
    try {
        // Try to connect with the given hostname and port
        PlayerClient robot(gHostname, gPort); 
    
        // Establish a proxy to query odometry values and send motor commands
        Position2dProxy pp(&robot, gIndex); 
    

        double xpos, ypos, thetapos;
        double speed, turnrate;

        // Now we start the main processing loop
        for(;;)
        {

            // read from the proxies; YOU MUST ALWAYS HAVE THIS LINE
            robot.Read();

            /*
            / these next three lines illustrate how you query the positionproxy to gather 
            / the robot pose information
            */
            xpos = pp.GetXPos();
            ypos = pp.GetYPos();
            thetapos = pp.GetYaw();

            // Let's output this data to the terminal
            printf("xpos: %f  ypos: %f  thetapos: %f\n", xpos, ypos, thetapos);


    	/*
    	/ The rest of your algorithm goes here;
            / Ideally you'd want to have some logic in here that eventually outputs a robot control command.
            / In this example, we just set it to 1 m/s in speed and 1 rad/s in turnrate since we are dealing 
            / with two-wheeled non-holonomic dynamics in our robots
    	*/
            turnrate = 1; // rad/s
            speed = 1; // m/s
        
            // control the robot by setting motion commands here
            pp.SetSpeed(speed, turnrate);
        }
    } catch(PlayerError e) {
        cerr << argv[0] << ": Error detected while connecting to " << 
            gHostname << ":" << gPort << "." << endl;
        cerr << argv[0] << ": Error details: " <<  e << endl;
        exit(-1);
    }


}

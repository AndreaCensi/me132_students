#include <libplayerc++/playerc++.h>
#include <stdio.h>
#include <math.h>
#include <vector>

#include "cmdline_parsing.h"
#include "common_functions.h"

using namespace PlayerCc;
using namespace std;

int main(int argc, char **argv)
{
  /* Calls the command line parser */
  parse_args(argc, argv);

  try {
  	/* Initialize connection to player */
	  PlayerClient robot(gHostname, gPort);
	  Position2dProxy pp(&robot, gIndex);
	  LaserProxy lp(&robot, gIndex); 

	  if(!check_robot_connection(robot, pp, 10)) 
	  	exit(-2);
  
	  // Now we start the main processing loop
	  while(1) {
	    // read from the proxies; YOU MUST ALWAYS HAVE THIS LINE
	    robot.Read();
        
	    // these next three lines illustrate how you query the laserproxy to gather 
	    // the laser scanner data        
	    unsigned int n = lp.GetCount();
	  	vector<double> range_data(n);
	  	vector<double> bearing_data(n);
	    for(uint i=0; i<n; i++) {
	      range_data[i] = lp.GetRange(i);
	      bearing_data[i] = lp.GetBearing(i);
	    }
    
	    // Now laser range data can be accessed as a double vector, e.g. range_data[i] 
	    // and bearing_data[i].

	    double turnrate = 1;
	    double speed = 1;
	    pp.SetSpeed(speed, turnrate);
    
	  }

  } catch(PlayerError e) {
  	write_error_details_and_exit(argv[0], e);
  }

  return 0;
  
}

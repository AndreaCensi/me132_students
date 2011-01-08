import sys
import time

try:
    from playerc import *
    
except Exception as e:
    print('Could not import module "playerc".')
    print('Make sure you compiled Player with -DBUILD_PYTHONC_BINDINGS=ON '
          'and that the environment variable PYTHONPATH is properly set.')
    sys.exit(-1)

from cmdline_parsing import parse_args
from my_functions import *

def main():
    settings = parse_args()
    
    # Create a client object
    c = playerc_client(None, settings.host, settings.port)
    # Connect it
    if c.connect() != 0:
        raise Exception(playerc_error_str())
    
    # Create a proxy for position2d:0
    pp = playerc_position2d(c, 0)
    if pp.subscribe(PLAYERC_OPEN_MODE) != 0:
        raise Exception(playerc_error_str())
    
    # Now we start the main processing loop
    while True:
        try:
            control_loop(c, pp)
            time.sleep(0.1)
        except KeyboardInterrupt:
            print('Interrupted by CTRL-C.')
            break
        except Exception as e:
            print('Something was wrong: %s' % e)
            break
    
    print('Clean up')   
    # Very important -- stop the robot
    pp.set_cmd_vel(0.0, 0.0, 0.0, 1)
    
    # Clean up connections
    pp.unsubscribe()
    c.disconnect()

def control_loop(client, pp):
    if client.read() == None:
        raise Exception('Lost connection to player: %s' % playerc_error_str())

    # Read the pose
    pose = [pp.px, pp.py, pp.pa]

    # Let's output this data to the terminal
    print("x: %fm  y: %fm  theta: %frad" % (pose[0], pose[1], pose[2]))

    # The rest of your algorithm goes here;
    # Ideally you'd want to have some logic in here that eventually outputs 
    # a robot control command.
    # In this example, we just set it to 1 m / s in speed and 1 rad / s 
    # in turnrate since we are dealing with two - wheeled non - holonomic 
    # dynamics in our robots

    turnrate = 1 # rad / s
    speed = -1 # m / s

    pp.set_cmd_vel(speed, 0.0, turnrate, 1)
    

if __name__ == '__main__':
    main()
    
    

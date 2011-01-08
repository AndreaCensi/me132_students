from optparse import OptionParser, OptionGroup

import playerc

def parse_args():
    parser = OptionParser() 

    group = OptionGroup(parser, "Player server settings")

    group.add_option("--host", default='localhost',
                      help='[= %default] Player server host.')

    group.add_option("--port", '-p', default=6665, type='int',
                     help='[= %default] Player server port.')
    
    parser.add_option_group(group)

    group = OptionGroup(parser, "Other options")

    group.add_option("--debug", default=False, action='store_true',
                      help='Activates debug mode.')
    
    group.add_option("--laser", default=False, action='store_true',
                      help='[= %default] Uses laser.')
    
    group.add_option("--set", default='*',
                      help='[= %default] Which combinations to run.')

    group.add_option("--index", "-i", default=0, type='int',
                      help='[= %default] Index.')
    group.add_option("--mode", "-m", default=playerc.PLAYER_DATAMODE_PUSH, type='int',
                      help='[= %default] Data mode.')
    group.add_option("--update", "-u", default=0, type='float',
                      help='[= %default] Update rate (Hz).')
    
    parser.add_option_group(group)

    (options, args) = parser.parse_args() #@UnusedVariable
    
    return options

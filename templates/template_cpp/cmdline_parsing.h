#include <string>

/** Variables holding the settings extracted from the command line. */

extern std::string  gHostname;
extern uint         gPort;
extern uint         gIndex;
extern uint         gDebug;
extern uint         gFrequency;
extern uint         gDataMode;
extern bool         gUseLaser;

/** Parses the cmd line, putting the data in the above variables. */
int parse_args(int argc, char** argv);

#include "cmd_options.h"

int main(int argc, const char* argv[])
{
    int myport = parse_options(argc, argv);
    return 0;
}

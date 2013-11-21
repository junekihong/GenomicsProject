#include "cmd_options.h"

int main(int argc, const char* argv[])
{
    WorkerConfiguration config = parse_options(argc, argv);

    return 0;
}

#include <cstdlib>

#include "scheduling.h"

scheduler::Problem::Problem()
{
    right = NULL;
    down = NULL;
    right_down = NULL;
    requestor = NULL;

    left = false;
    up = false;
    left_up = false;
}

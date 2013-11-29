#ifndef __PAIR_LOCATION_VALUE_H__
#define __PAIR_LOCATION_VALUE_H__

#include "location.h"

// A Pair class for a location and a value. We use this to return the best value after performing local alignment. 
class LocationValuePair
{
public:
    Location location;
    int value;
};

#endif //__PAIR_LOCATION_VALUE_H__

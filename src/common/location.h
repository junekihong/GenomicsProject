#ifndef __LOCATION_H__
#define __LOCATION_H__

#include <msgpack.hpp>

// Location class. Contains a row and column index.
class Location
{
public:
    int row;
    int column;
    
    Location()
        : row(0), column(0)
    { }
    
    Location(int r, int c)
        : row(r), column(c)
    { }
    
    MSGPACK_DEFINE(row, column);
};


#endif //__LOCATION_H__

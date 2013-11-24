#ifndef __LOCATION_H__
#define __LOCATION_H__

class Location
{
    int x;
    int y;
    
    public:
    Location()
        : x(0), y(0)
    { }
    
    Location(int _x, int _y)
        : x(_x), y(_y)
    { }
};


#endif //__LOCATION_H__


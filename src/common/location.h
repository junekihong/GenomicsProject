#ifndef __LOCATION_H__
#define __LOCATION_H__

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
};


#endif //__LOCATION_H__

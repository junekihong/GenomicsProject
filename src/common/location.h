#ifndef __LOCATION_H__
#define __LOCATION_H__

class Location
{
  int X;
  int Y;
  
 public:
  Location(){
    X = 0;
    Y = 0;
  }
  
  Location(int x, int y){
    X = x;
    Y = y;
  }
  
};


#endif //__LOCATION_H__


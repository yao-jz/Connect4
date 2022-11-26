/********************************************************
*	Point.h : 棋盘点类                                  *
*	张永锋                                              *
*	zhangyf07@gmail.com                                 *
*	2014.5                                              *
*********************************************************/

#ifndef POINT_H_
#define POINT_H_

class Point{
public:
	int x;
	int y;
	Point() = default;
	Point(int x, int y){
		this->x = x;
		this->y = y;
	}
	bool operator<(const Point& p1) const{
		if(x < p1.x){
            return true;
        } else if ( x == p1.x) {
            return y < p1.y;
        } else {
            return false;
        }
	}
};

#endif

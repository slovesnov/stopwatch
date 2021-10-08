/*
 * Point.h
 *
 *       Created on: 03.12.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef POINT_H_
#define POINT_H_

#include <string>
#include <ostream>

struct Point {
	int x, y;
	bool operator !=(const Point& p)const{
		return x!=p.x || y!=p.y;
	}
	void fromString(std::string const&s);
};

std::ostream& operator <<(std::ostream& o, const Point& p);
//std::istream& operator >>(std::istream& i, Point& p);

#endif /* POINT_H_ */

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

struct Point {
	int x, y;
	bool operator !=(const Point& p){
		return x!=p.x || y!=p.y;
	}
};

#endif /* POINT_H_ */

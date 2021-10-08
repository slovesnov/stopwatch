/*
 * Point.cpp
 *
 *  Created on: 08.10.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Point.h"
#include "aslov.h"

void Point::fromString(std::string const&s){
	sscanf(s.c_str(),"%d %d", &x, &y);
}

std::ostream& operator <<(std::ostream& o, const Point& p) {
	return o << join(p.x,p.y);
}

//std::istream& operator >>(std::istream& i, Point& p) {
//	return i >> p.width >> p.height >> p.isotropic;
//}





/*
 * DigitalFontParameters.cpp
 *
 *       Created on: 03.12.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Base.h"
#include "DigitalFontParameters.h"

std::ostream& operator <<(std::ostream& o, const DigitalFontParameters& p) {
	return o << p.toString();
}

std::istream& operator >>(std::istream& i, DigitalFontParameters& p) {
	return i >> p.width >> p.height >> p.isotropic;
}

std::string DigitalFontParameters::toString() const {
	return Base::format("%d %d %d", width, height, isotropic);
}

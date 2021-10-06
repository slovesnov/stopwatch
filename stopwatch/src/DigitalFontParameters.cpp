/*
 * DigitalFontParameters.cpp
 *
 *       Created on: 03.12.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "DigitalFontParameters.h"
#include "help.h"

std::string DigitalFontParameters::toString() const {
	return format("%d %d %d", width, height, isotropic);
}

void DigitalFontParameters::fromString(std::string const&s){
	int i;
	sscanf(s.c_str(),"%d %d %d", &width, &height, &i);
	isotropic= i!=0;
}

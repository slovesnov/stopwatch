/*
 * DigitalFontParameters.h
 *
 *       Created on: 03.12.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIGITALFONTPARAMETERS_H_
#define DIGITALFONTPARAMETERS_H_

#include "Point.h"

class DigitalFontParameters {
public:
	int width, height;
	bool isotropic;
	//std::string toString() const;
	void fromString(std::string const&s);
};

std::ostream& operator<<(std::ostream&, DigitalFontParameters const&);
//std::istream& operator>>(std::istream&, DigitalFontParameters&);

#endif /* DIGITALFONTPARAMETERS_H_ */

/*
 * DigitalFont.h
 *
 *       Created on: 21.11.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifndef DIGITALFONT_H_
#define DIGITALFONT_H_

#include "DigitalFontParameters.h"
#include <string>
#include <gtk/gtk.h>

using PDD = std::pair<double,double>;

class DigitalFont {
	//width - full width, height -  full height
	double width, height, angleMargin, symbolMargin;
	double thicknessx, thicknessy;
	static constexpr double kThickness = .35; //.125;
	static constexpr double kSymbolMargin = .2; //.25;
	static constexpr double defaultAngleMargin = 2;
	static const char digitCode[];

	void init(double sizex, double sizey, double _thicknessx,
			double _thicknessy, double _symbolMargin, double _angleMargin) {
		thicknessx = _thicknessx;
		thicknessy = _thicknessy;
		angleMargin = _angleMargin;
		symbolMargin = _symbolMargin;
		width = sizex + 2 * thicknessx;
		height = 2 * sizey + 3 * thicknessy;
	}

	void init(double sizex, double sizey, double _angleMargin) {
		init(sizex, sizey, sizex * kThickness, sizey * kThickness,
				sizex * kSymbolMargin, _angleMargin);
	}

	PDD getSize() const {
		return std::make_pair(width - 2 * thicknessx,
				(height - 3 * thicknessy) / 2);
	}

public:
	//size inner square size
	DigitalFont(double size, double _angleMargin = defaultAngleMargin) {
		init(size, size, _angleMargin);
	}

	DigitalFont(DigitalFontParameters const &p, Point const &maxSize,
			const std::string &s);

	void drawDigit(int n, cairo_t *cr);
	void drawString(const std::string &s, cairo_t *cr);

	//draw string in rectangle center vertically & horizontally
	void drawString(GdkRectangle const &r, const std::string &s, cairo_t *cr);

	static PDD getK(const std::string &s);

	//getK(s)*getSize() = full string {width,height}
	PDD getStringSize(const std::string &s) const {
		auto a = getK(s);
		auto d = getSize();
		return PDD(a.first * d.first, a.second * d.second);
	}

};

#endif /* DIGITALFONT_H_ */

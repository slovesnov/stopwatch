/*
 * DigitalFont.cpp
 *
 *       Created on: 21.11.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#include "DigitalFont.h"
#include <vector>
#include <algorithm>

const char DigitalFont::digitCode[] = { 123, 96, 55, 103, 108, 79, 95, 97, 127,
		111 };

void DigitalFont::drawDigit(int n, cairo_t *cr) {
	/*bits - in cycle
	 up horizontal
	 down horizontal
	 center horizontal
	 left-up vertical
	 left-down vertical
	 right-up vertical
	 right-down vertical

	 bits in b - array (inverse order)
	 */
	double h = height * .5;
	double x, y;
	int i, j;
	const char b = digitCode[n];
	std::vector<PairDoubleDouble> a, c;

	for (i = 0; i < 7; i++) {
		if (i == 3) {
			//left-up vertical
			a = { { 0, angleMargin }, { 0, h - thicknessy / 2 - angleMargin }, {
					thicknessx / 2, h - angleMargin / 2 }, { thicknessx, h
					- thicknessy / 2 - angleMargin }, { thicknessx, thicknessy
					+ angleMargin } };
		} else if (i == 0 || i == 2) {
			if (i == 0) {
				a = { { thicknessx + angleMargin, thicknessy },
						{ angleMargin, 0 }, };
			} else {
				a = { { thicknessx + angleMargin, h + thicknessy / 2 }, {
						thicknessx / 2 + angleMargin, h }, { thicknessx
						+ angleMargin, h - thicknessy / 2 } };
			}

			for (j = a.size() - 1; j >= 0; j--) {
				auto &p = a[j];
				a.push_back( { width - p.first, p.second });
			}

		}
		if (b >> i & 1) { //precedence ok
			//draw edge
			j = 0;
			for (auto &p : a) {
				x = p.first;
				if (i > 4) {			//symmetry x
					x = width - x;
				}
				y = p.second;
				if (i == 1 || i == 4 || i == 6) {			//symmetry y
					y = height - y;
				}
				if (j == 0) {
					cairo_move_to(cr, x, y);
				} else {
					cairo_line_to(cr, x, y);
				}
				j++;
			}
			cairo_close_path(cr);
		}
	}
	cairo_fill(cr);
}

void DigitalFont::drawString(const std::string &s, cairo_t *cr) {
	int i;
	for (char c : s) {
		if (c == ':') {
			for (i = 0; i < 2; i++) {
				cairo_save(cr);
				cairo_translate(cr, thicknessx / 2, (2 * i + 3) * height / 8);
				cairo_scale(cr, thicknessx, thicknessy);
				cairo_arc(cr, 0, 0, .5, 0, 2 * G_PI);
				cairo_restore(cr);
				cairo_fill(cr);

//				cairo_arc(cr, thicknessx / 2, (2 * i + 3) * height / 8, thicknessx / 2, 0,
//						2 * G_PI);
//				cairo_fill(cr);
			}
			cairo_translate(cr, thicknessx + symbolMargin, 0);
		} else {
			drawDigit(c - '0', cr);
			cairo_translate(cr, width + symbolMargin, 0);
		}
	}
}

DigitalFont::DigitalFont(DigitalFontParameters const &p, CPoint const &maxSize,
		const std::string &s) {
	auto a = getK(s);
	auto w = std::min(p.width, maxSize.x) / a.first;
	auto h = std::min(p.height, maxSize.y) / a.second;
	if (p.isotropic) {
		w = h = std::min(w, h);
	}
	init(w, h, defaultAngleMargin);
}

void DigitalFont::drawString(const GdkRectangle &r, const std::string &s,
		cairo_t *cr) {
	auto a = getStringSize(s);
	cairo_translate(cr, r.x + (r.width - a.first) / 2,
			r.y + (r.height - a.second) / 2);
	drawString(s, cr);
}

PairDoubleDouble DigitalFont::getK(const std::string &s) {
	/*
	 thickness=size*kt;
	 symbolMargin=size*ks;
	 width=size+2*thickness=size*(1+2*kt);
	 height=2*size+3*thickness=size*(2+3*kt);

	 r.x = (length-colon)*width+colon*thickness+(length-1)*symbolMargin <=
	 (length-colon)*size*(1+2*kt)+colon*size*kt+(length-1)*size*ks =
	 size*( (length-colon)*(1+2*kt)+colon*kt+(length-1)*ks )

	 r.y = 2*size+3*thickness <= size*(2+3*kt) <=height
	 */
	const double kt = kThickness;
	const double ks = kSymbolMargin;
	int colon = std::count(s.begin(), s.end(), ':');
	int length = s.length();
	return {
		(length - colon) * (1 + 2 * kt) + colon * kt + (length - 1) * ks,
		2 + 3 * kt};
}

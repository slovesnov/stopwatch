/*
 * help.h
 *
 *       Created on: 12.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef HELP_H_
#define HELP_H_

#include <ctime>
#include <fstream>
#include "Point.h"

#include "aslov.h"
//#define BASE_ADDONS

enum class Mode {
	STOPWATCH, MINUTE, TIME, MODE_SIZE
};

enum class DialogType {
	PARAMETERS, ERROR, MESSASE, DIALOG_TYPE_SIZE
};

typedef int64_t BeepTimeType;
typedef std::pair<int, std::string> PredefinedDateType;
typedef std::vector<PredefinedDateType> VPredefinedDateType;

const char UPCOMING[] = "upcoming";
const int BTDIVISOR = 10000;
const int MMDDDIVISOR = 10000;
const int MMDIVISOR = 100;

double secondsSince(clock_t t);

void beep(short volume);
void beep();

#ifdef BASE_ADDONS
void exploreAllChildrenRecursive(GtkWidget* w);
#endif

int getYYYYMMDD(BeepTimeType v);
int getHHMM(BeepTimeType v);

bool has(VPredefinedDateType const& v, int i);
bool comparePredefinedDate(const PredefinedDateType& a,
		const PredefinedDateType& b);
int getYYYY(int v);
int getMM(int v);
int getDD(int v);
int getMMDD(int v);

bool yesNoDialog(std::string text);

#endif /* HELP_H_ */

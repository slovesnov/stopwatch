/*
 * main.cpp
 *
 *       Created on: 01.09.2015
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2015-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Config.h"
#include "Dialog.h"
#include "Frame.h"

/* Note if define Config variable in DateTime.cpp file program hangs,
 * because Config constructor doesn't call
 * The problem is class Frame inherit from Parameters which has member DateTime setupTime;
 * DateTime class need initialize timeZone before (actually needs only one timeZone for whole program)
 * if make static member in DateTime with type of Config then constructor of Config won't be called
 * Even if define Config config in DateTime.cpp file constructor still not called
 * read https://stackoverflow.com/questions/29465211/static-objects-constructor-not-being-called
 * So the solution is defined "Config config" before "Frame frame". It guarantees initialization before
 */

Config config;
Frame frame;

int main(int argc, char *argv[]) {
	gtk_init(&argc, &argv); //do not remove
	aslovInit(argv);

	config.init();

	//load css before possible show start dialog
	loadCSS();

	if (!frame.prepare(*argv)) {
		Dialog d(DialogType::ERROR, frame.what());
		return 1;
	}

	if (argc == 1) {
		auto a = config.getArguments();
		if (a.empty()) {
			frame.upcoming();
		}
		else {
			if (!frame.parse(a)) {
				Dialog d(DialogType::ERROR, frame.what());
				return 1;
			}
		}
	}
	else {
		if (!frame.parse(argc, argv)) {
			Dialog d(DialogType::ERROR, frame.what());
			return 1;
		}
	}

	frame.show();
	gtk_main();

	return 0;
}

/*
 * Frame.h
 *
 *       Created on: 03.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifndef FRAME_H_
#define FRAME_H_

/* #ifdef SAVE_IMAGE Ctrl+i or Ctrl+I stores image to png file
 * but title of stopwatch window has ugly font
 * may be it's gtk/winapi bug
 */
//#define SAVE_IMAGE
#include "Parameters.h"
#include "DigitalFont.h"

class Frame: public Parameters {
	GtkWidget *area[3], *box, *button[2];
	CPoint maxAreaSize = { 0, 0 }; //indicator that countAreaSize isn't called
	bool timerRunning = false;
	clock_t lastTime;
	clock_t startTime; //for stopwatch mode
	std::set<BeepTimeType>::const_iterator beepIt; //for stopwatch mode
	int fs[2] = { 0 }; //font size
	cairo_surface_t *surface;
	cairo_t *cr;
	clock_t lastDrawTime = 0;
	std::string lastIconString;
	int lastIconColorIndex = -1;
public:
	GtkWidget *window = NULL;

	void startTimer();
	void stopTimer();
	void moveSizeWindow(bool b = true);

	auto getTitle() const {
		auto s = toString(StringType::FRAME);
#ifndef NDEBUG
		s += " (NDEBUG isn't defined)";
#endif
		return s;
	}

	void draw();

	void paint() {
		for (auto a : area) {
			gtk_widget_queue_draw(a);
		}
	}

	void updateParse();

	void setIcon();
	bool windowStateIconified() {
		return gdk_window_get_state(gtk_widget_get_window(window))
				& GDK_WINDOW_STATE_ICONIFIED;
	}

	int getColorIndex() const {
		return isPredefinedDate();
	}

	void setColor(cairo_t *cr);

#ifdef SAVE_IMAGE
	void saveImage();
#endif /* SAVE_IMAGE */

	DigitalFont createDigitalFont();
public:
	void show();

	~Frame();

	bool created() {
		return window != NULL;
	}

	auto getWindow() const {
		return window;
	}

	gboolean timeFunction();
	void onKeyPress(GdkEventKey *e);

	void setParameters(Parameters const &p) {
		(Parameters&) *this = p;
		updateParse();
	}
	gboolean draw(GtkWidget *widget, cairo_t *c);
	void countAreaSize(GtkWidget*);
	void buttonClicked(GtkWidget *w);

	gboolean windowStateEvent(GdkEventWindowState *e);

	void updateReload() {
		setIcon();
		paint(); //if timer not run in stopwatch mode need to redraw
	}

	bool parse(VString const &a) {
		return Parameters::parse(a);
	}

	bool parse(int argc, char *argv[]) {
		if (!Parameters::parse(argc, argv)) {
			return false;
		}
		//15oct25 fixed bug if program started with stopwatch arguments
		beepIt = beepTime.cbegin();
		return true;
	}

	bool parse(const std::string s, bool predefined = false) {
		return Parameters::parse(s, predefined);
	}

	double getTime();

#ifndef NDEBUG
	//for testing some functionality
	void test();
#endif
};

extern Frame frame;

#endif /* FRAME_H_ */

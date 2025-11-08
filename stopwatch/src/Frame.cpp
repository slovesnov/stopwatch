/*
 * Frame.cpp
 *
 *       Created on: 03.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#include "Frame.h"
#include "Dialog.h"
#include "Config.h"
#include "DigitalFont.h"
#include <clocale>

/*
 * even if timeFunction call several times during one second
 * draw calls one time only
 */
const int MILLISECONDS = 250;
const GdkRGBA COLOR[] = { { 0, 0, 0, 1 }, { 0, 0, .5, 1 } };
const char *W[] = { "00:00:00", "00:00" };

void on_key_press(GtkWidget*, GdkEventKey *e, gpointer) {
	frame.onKeyPress(e);
}

gboolean draw_callback(GtkWidget *w, cairo_t *c, gpointer) {
	return frame.draw(w, c);
}

//same name function in Dialog.cpp so make it static
static void button_clicked(GtkWidget *w, gpointer) {
	frame.buttonClicked(w);
}

gboolean window_state_event(GtkWidget*, GdkEventWindowState *e, gpointer) {
	return frame.windowStateEvent(e);
}

gboolean on_widget_deleted(GtkWindow *widget, GdkEvent *event, gpointer data) {
	config.write(); //call here while frame & config are exist
	if (!config.closeWarning) {
		return FALSE;
	}
	return !yesNoDialog("Do you really want to exit?");
}

gboolean time_function(gpointer) {
	return frame.timeFunction();
}

void Frame::show() {
	std::string s;
	int i;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	s = getTitle();
	gtk_window_set_title(GTK_WINDOW(window), s.c_str());
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), 0); //doesn't call on shutdown

	for (auto &a : area) {
		a = gtk_drawing_area_new();
	}

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	const char *bt[] = { "set parameters", UPCOMING };
	for (i = 0; i < int(G_N_ELEMENTS(bt)); i++) {
		auto w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_pack_start(GTK_BOX(w), area[i], TRUE, TRUE, 0); //stretch

		button[i] = gtk_button_new_with_label(bt[i]);

		g_signal_connect(button[i], "clicked", G_CALLBACK(button_clicked), 0);
		gtk_widget_set_can_focus(button[i], FALSE); //prevents click on button when stop stopwatch
		gtk_container_add(GTK_CONTAINER(w), button[i]);

		gtk_container_add(GTK_CONTAINER(box), w);
	}

	gtk_box_pack_start(GTK_BOX(box), area[2], TRUE, TRUE, 0); //stretch area. set size for box
	gtk_container_add(GTK_CONTAINER(window), box);

	//later parameters.mode can be changed by user so connect signal anyway, and allow CTRL+A (adjust)
	g_signal_connect(window, "key_press_event", G_CALLBACK (on_key_press), 0);
	g_signal_connect(window, "window-state-event",
			G_CALLBACK (window_state_event), 0);

	g_signal_connect(window, "delete-event", G_CALLBACK(on_widget_deleted),
			NULL);

	if (!isStopwatch()) {
		startTimer();
	}

	/* Try to avoid blinks as much as possible
	 * in case of countAreaSize was called from Dialog() areaSize!=0 and we don't need to count areaSize
	 * otherwise try to open cfg file and read areaSize if success moveSizeWindow and iconify it before
	 * showing so avoid blink
	 * if no file and countAreaSize wasn't called from Dialog() then cann't avoid blink make show_all and
	 * then countAreaSize
	 */
	if (maxAreaSize.x == 0) { //if areaSize!=0 it's already counted from Dialog() and don't need to count 2nd time
		if (config.read) {
			moveSizeWindow();
			gtk_widget_show_all(window);

			/* in case of minimize window we should call gtk_window_iconify()
			 * before gtk_widget_show_all (window)
			 * the problem is that if cfg file was moved from another computer
			 * so areaSize can became invalid
			 */
			if (minimize()) { //window iconified in moveSizeWindow() function
				//call countAreaSize() in draw() function
			} else { //window isn't iconified can check&modify areaSize right now
				auto size = maxAreaSize;
				countAreaSize(window);
				if (size != maxAreaSize) {
					//window isn't iconified & showed so just call moveSizeWindow()
					moveSizeWindow();
				}
			}
		} else {
			gtk_widget_show_all(window);
			countAreaSize(window); //after show all cann't avoid blink
			moveSizeWindow();
		}
	} else {
		//area size already counted call moveSizeWindow() and then show window
		moveSizeWindow();
		gtk_widget_show_all(window);
	}

	//after areaSize is counted
	surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, maxAreaSize.x,
			maxAreaSize.y);
	g_assert(surface != NULL);
	cr = cairo_create(surface);

	setlocale(LC_NUMERIC, "C"); //dot interpret as decimal separator for sprintf %.1lf

	for (auto a : area) {
		g_signal_connect(a, "draw", G_CALLBACK (draw_callback), 0);
	}

#ifndef NDEBUG
//	test();
#endif

}

Frame::~Frame() {
	if (!created()) {
		return;
	}
	/* in case of error or simple close Dialog() without set of parameters
	 * Frame was not created so areaSize is not initialized so
	 * don't need to write cfg file
	 */
	cairo_surface_destroy(surface);
	cairo_destroy(cr);
}

void Frame::startTimer() {
	lastTime = startTime = clock(); //start time immediately
	timerRunning = true;
	beepIt = beepTime.cbegin();
	g_timeout_add(MILLISECONDS, time_function, 0); //g_idle_add is more evenly than g_timeout_add_seconds but hard for CPU
	//g_timeout_add_seconds(1, time_function, 0);
}

void Frame::moveSizeWindow(bool b) {
	GdkMonitor *monitor = gdk_display_get_monitor(gdk_display_get_default(), 0);
	GdkRectangle wr;
	int sx, sy;
	gdk_monitor_get_workarea(monitor, &wr);

	maxAreaSize = { wr.width - config.captionsSize.x, wr.height
			- config.captionsSize.y };

	if (config.digitalMode) {
		auto df = createDigitalFont();
		auto s = W[isStopwatch()];
		auto b = df.getStringSize(s);
		sx = int(b.first);
		sy = int(b.second) + config.additionalHeight;
	} else {
		sx = sy = std::min(maxAreaSize.x, maxAreaSize.y);
	}

	gtk_widget_set_size_request(box, sx, sy);

	//gtk_window_move(GTK_WINDOW(window), 0, 0);//invalid placement on first run
	gtk_window_move(GTK_WINDOW(window),
			wr.x + (wr.width - sx - config.captionsSize.x) / 2,
			wr.y + (wr.height - sy - config.captionsSize.y) / 2);

	if (b && minimize()) { //before gtk_widget_show_all (window)
		gtk_window_iconify(GTK_WINDOW(window));
	}
}

void Frame::countAreaSize(GtkWidget *wnd) {
	//Note window should be visible
	GdkRectangle r, wr;
	int w, h;

	auto gdk_window = gtk_widget_get_window(wnd);
	auto monitor = gdk_display_get_monitor_at_window(gdk_display_get_default(),
			gdk_window);
	gdk_monitor_get_workarea(monitor, &wr);
	gdk_window_get_frame_extents(gdk_window, &r);	//with title and borders
	gdk_window_get_geometry(gdk_window, 0, 0, &w, &h);

	config.captionsSize = { r.width - w, r.height - h };
	maxAreaSize = { wr.width - config.captionsSize.x, wr.height
			- config.captionsSize.y };
}

void Frame::draw() {
	assert(!windowStateIconified());

	char b[128];
	int i, j, k;
	int width;
	int height;
	double v;
	DateTime d;
	cairo_text_extents_t e;
	std::string s;
	const int size = std::min(maxAreaSize.x, maxAreaSize.y);
	const bool stopwatch = isStopwatch();

	s = W[stopwatch];
	auto df = createDigitalFont();

	if (config.digitalMode) {
		auto sz = df.getStringSize(s);
		width = sz.first;
		height = sz.second + config.additionalHeight;
	} else {
		width = height = size;
	}
	const int MLW = 3;
	const int r = size / 2 - MLW / 2 - (MLW % 2 == 0 ? 0 : 1);

	//reset all translations
	cairo_matrix_t matrix;
	cairo_matrix_init(&matrix, 1, 0, 0, 1, 0, 0);
	cairo_set_matrix(cr, &matrix);
	cairo_set_source_rgb(cr, 0, 0, 0);

	//fill background as in css file
	auto context = gtk_widget_get_style_context(window);
	gtk_render_background(context, cr, 0, 0, width, height);

	if (stopwatch) {
		i = getTime();
		s = format("%02d:%02d", i / 60, i % 60);
	} else {
		s = d.format("%H:%M:%S");
	}
	const std::string so = s;

	//20.728
	const double FONT_HEIGHT_PIXELS = getDPI().second < 125 ? 14 : 20;
	if (stopwatch) {
		//need set for digit mode
		cairo_select_font_face(cr, "Times New Roman", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size(cr, FONT_HEIGHT_PIXELS); //height in pixels
		drawText(cr, "Press any key to start/clear stopwatch", 0, 0,
				DRAW_TEXT_BEGIN, DRAW_TEXT_BEGIN);
	}

	PangoLayout *layout;
	layout = pango_cairo_create_layout(cr);
	PangoFontDescription *desc = pango_font_description_from_string(
			"Times New Roman, 12");
	pango_font_description_set_absolute_size(desc,
			FONT_HEIGHT_PIXELS * PANGO_SCALE);
	pango_layout_set_font_description(layout, desc);

	int maxTextHeight = 0;
	for (k = 0; k < 2; k++) {
		if (k == 0) {
			s = getPredefinedTimeString();
		} else {
			s = getInfoString() + "\npress Ctrl+A to adjust window";
			pango_layout_set_alignment(layout, PANGO_ALIGN_RIGHT);
		}

		pango_layout_set_markup(layout, s.c_str(), -1);
		pango_layout_get_pixel_size(layout, &i, &j);
		if (j > maxTextHeight) {
			maxTextHeight = j;
		}
		cairo_move_to(cr, k == 0 ? 0 : width - i, height - j);
		pango_cairo_update_layout(cr, layout);
		pango_cairo_show_layout(cr, layout);
	}

	pango_font_description_free(desc);
	g_object_unref(layout);

	config.additionalHeight = maxTextHeight;
	for (i = 0; i < 2; i++) {
		config.additionalHeight += gtk_widget_get_allocated_height(area[i]);
	}

	if (config.digitalMode) {
		for (i = j = 0; i < 2; i++) {
			j += gtk_widget_get_allocated_height(area[i]);
		}
		cairo_translate(cr, 0, j);
		setColor(cr);
		df.drawString(so, cr);

		//show current date
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_select_font_face(cr, "Times New Roman", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size(cr, FONT_HEIGHT_PIXELS); //height in pixels
		s = DateTime().sformat();
		sprintf(b, s.c_str());
		cairo_text_extents(cr, b, &e);
		cairo_set_matrix(cr, &matrix);
		cairo_translate(cr, width / 2 - e.width / 2 - e.x_bearing,
				height - maxTextHeight / 2 - e.height / 2 - e.y_bearing);
		cairo_show_text(cr, b);
	} else {
		//draw clockface
		cairo_set_line_width(cr, MLW);
		cairo_translate(cr, width / 2, height / 2);

		cairo_new_path(cr);
		cairo_arc(cr, 0, 0, r, 0, 2 * G_PI);
		cairo_stroke(cr);

		const double K[] = { 0.9, 0.85, 0.8 };
		const int LW[] = { 7, 7, 14 };
		static_assert(G_N_ELEMENTS(K)==G_N_ELEMENTS(LW));

		const int FS[] = { 70, 80 };
		const int NUMBER_MARGIN = 15 * r / 500;
		double hourArrowLength = 0;

		//Note cairo draw lines (length of line) functions depends on cairo line cap
		cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
		cairo_select_font_face(cr, "Times New Roman", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_BOLD);
		for (i = 0; i < 60; i++) {
			if (i % 5 == 0) {
				j = stopwatch && i % 10 != 0 ? 1 : 2;
			} else {
				j = 0;
			}
			cairo_set_line_width(cr, LW[j] * size / 1100);
			cairo_move_to(cr, 0, -r);
			cairo_line_to(cr, 0, -r * K[j]);

			if (i % 5 == 0) {
				j = i == 0 ? 60 : i;
				if (!stopwatch) {
					j /= 5;
				}
				sprintf(b, "%d", j);
				j = stopwatch ? i % 10 == 0 : 1;
				cairo_set_font_size(cr, FS[j] * size / 700);
				cairo_text_extents(cr, b, &e);
				cairo_move_to(cr, -e.x_bearing - e.width / 2,
						-r * K[2] - e.y_bearing + NUMBER_MARGIN);
				cairo_show_text(cr, b);
				if (i == 15) {
					hourArrowLength = r * K[2] - e.height - 2 * NUMBER_MARGIN; //-k/2. in line_to
				}
			}

			cairo_rotate(cr, 2 * G_PI / 60);
		}
		cairo_stroke(cr);

		//draw arrows
		//Note e should be set
		k = LW[2] * size / 1100;
		cairo_set_line_width(cr, k);
		for (j = stopwatch ? 2 : 0; j < 3; j++) {
			if (j == 2) {
				i = stopwatch ? getTime() : d.second();
				v = i / 60.;
			} else {
				v = (d.second() / 60. + d.minute()) / 60;
				if (j == 0) {
					v += d.hour();//don't need %12, because hour+12 gives +2*pi rotation
					v /= 12;
				}
			}
			v *= 2 * G_PI;

			cairo_set_source_rgb(cr, j == 2 ? 225 / 255. : 0., 0, 0);
			cairo_move_to(cr, 0, 0);

			cairo_rotate(cr, v);
			cairo_line_to(cr, 0, -((j == 0 ? hourArrowLength : r) - k / 2.));
			cairo_stroke(cr);
			cairo_rotate(cr, -v);

		}

		cairo_select_font_face(cr, "Times New Roman", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
		setColor(cr);

		if (fs[0] == 0) {	//one time only
			//before count fs[]
			auto as = maxAreaSize;
			countAreaSize(window);
			if (as != maxAreaSize) {
				cairo_surface_destroy(surface);
				cairo_destroy(cr);
				surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
						maxAreaSize.x, maxAreaSize.y);
				cr = cairo_create(surface);
				moveSizeWindow(false);
				draw();	//redraw & recount
				return;
			}

			for (k = 0; k < 2; k++) {
				for (j = size * (k ? 0.3 : 0.17); j > 0; j--) {
					cairo_set_font_size(cr, j);
					cairo_text_extents(cr, W[k], &e);
					if (e.width <= 2 * hourArrowLength) {
						break;
					}
				}
				fs[k] = j;
			}
		}

		//show "hh:mm:ss" or "mm:ss"
		cairo_set_font_size(cr, fs[stopwatch]);
		cairo_text_extents(cr, W[stopwatch], &e);
		cairo_move_to(cr, -e.width / 2 - e.x_bearing,
				-e.height / 2 - e.y_bearing);
		cairo_show_text(cr, so.c_str());

		const double Y = e.height / 2;

		if (!stopwatch) {
			//show current date
			s = DateTime().sformat();
			for (j = size * 0.07; j > 0; j--) {
				cairo_set_font_size(cr, j);
				cairo_text_extents(cr, s.c_str(), &e);
				if (e.width <= 2 * hourArrowLength) {
					break;
				}
			}
			cairo_set_font_size(cr, j);
			cairo_text_extents(cr, s.c_str(), &e);
			cairo_move_to(cr, -e.width / 2 - e.x_bearing, Y - e.y_bearing);
			cairo_show_text(cr, s.c_str());
		}
	}	//if not digital digits

	//full draw 0.04 seconds if fs[0]=0 and 0.02-0.03 don't need to accelerate
}

gboolean Frame::draw(GtkWidget *w, cairo_t *c) {
	//window is minimized nothing to do
	if (windowStateIconified()) {
		return FALSE;
	}

	int i;

	/* do draw() only 1 time
	 * because draw functions calls from several areas
	 */
	if (lastDrawTime == 0 || timeElapse(lastDrawTime) >= MILLISECONDS / 1000.) {
		draw();
		lastDrawTime = clock();
	}

	i = 0;
	for (auto a : area) {
		if (w == a) {
			break;
		}
		i -= gtk_widget_get_allocated_height(a);
	}

	cairo_set_source_surface(c, surface, 0, i);
	cairo_rectangle(c, 0, 0, gtk_widget_get_allocated_width(w),
			gtk_widget_get_allocated_height(w));
	cairo_fill(c);

	return FALSE;
}

gboolean Frame::timeFunction() {
	setIcon();
	//function returns double so cann't use secondsSince(lastTime) > 0
	if (timeElapse(lastTime) >= 1) {
		int i;
		lastTime = clock();
		//beep
		if (isStopwatch()) {
			i = getTime();
			if (beepIt != beepTime.cend() && i >= *beepIt) {
				beepIt++;
				beep();
			}
		} else {
			/* if "stopwatch m 40" then need to store to last time not only minutes
			 * the same do for time mode
			 */
			DateTime d;
			auto v = d.toBeepTime();
			BeepTimeType a = isTime() ? v : d.toInt(false);
			if (v != lastBeepTime && hasBeepTime(a)) {
				beep();
				lastBeepTime = v;
			}

			//if when time==beepTime[beepTime.size()-1] computer was in sleep mode then
			assert(beepTime.size() > 0);
			if (isTime()) { //only for time mode
				if (v >= *beepTime.rbegin()) {
					buttonClicked(button[1]); //next update
				}
			}
		}

		paint();
	}
	return G_SOURCE_CONTINUE;
}

void Frame::onKeyPress(GdkEventKey *e) {
	if ((e->keyval == GDK_KEY_a || e->keyval == GDK_KEY_A)
			&& (e->state & GDK_CONTROL_MASK)) {
		moveSizeWindow(false);
		return;
	}

#ifndef NDEBUG
	/*
	 //for testing some functionality
	 if (e->keyval == GDK_KEY_q || e->keyval == GDK_KEY_Q) {
	 test();
	 return;
	 }
	 */
#endif /* NDEBUG */

#ifdef SAVE_IMAGE
	if ((e->keyval == GDK_KEY_i || e->keyval == GDK_KEY_I)
			&& (e->state & GDK_CONTROL_MASK)) {
		saveImage();
		return;
	}
#endif /* SAVE_IMAGE */

	//allow Ctrl+A for stopwatch mode. Note e->state & GDK_CONTROL_MASK==GDK_CONTROL_MASK
	if (e->keyval == GDK_KEY_Control_L || e->keyval == GDK_KEY_Control_R) {
		return;
	}

	//allow make screenshots
	if ((e->keyval == GDK_KEY_Alt_L || e->keyval == GDK_KEY_Alt_R)
			&& e->state == 0) {
		return;
	}

//	println("%x %x",e->keyval,e->state)

	if (isStopwatch()) {
		if (timerRunning) {
			timerRunning = false;
			draw();
			paint();
		} else {
			beep();
			startTimer();
		}
	}

}

void Frame::updateParse() {
	if (!created()) {
		return;
	}

	auto s = getTitle();
	gtk_window_set_title(GTK_WINDOW(window), s.c_str());

	if (minimize()) {
		gtk_window_iconify(GTK_WINDOW(window));
	}

	if (isStopwatch()) {
		timerRunning = false;
		draw();
		paint();
	} else { //switch to reminder or time mode
		if (!timerRunning) { //if timer is not run
			startTimer();	//start timer
		}
	}
}

void Frame::setIcon() {
	int i, j, k, pi, pj, pk;
	cairo_t *cr;
	cairo_surface_t *surface;
	std::string s, s1;
	GdkPixbuf *p;

	if (isStopwatch()) {
		i = getTime();
		//000 not 0000 to make bigger font, also do not output 0:00 for the same reason
		assert(i >= 0);
		s = format("%d%02d", i / 60, i % 60);
	} else {
		s = beepTimeFormat(getNextBeepTime(), true);
	}

	i = getColorIndex();
	//show whether window is iconified or not
	if (windowStateIconified()) {
		if (i == 1) {
			s = "<span underline='double'>" + s + "</span>";
		}
	} else {
		s = "<u>" + s + "</u>";
	}

	if (s == lastIconString && i == lastIconColorIndex) {
		return;
	}

	lastIconString = s;
	lastIconColorIndex = i;

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 32, 32);
	cr = cairo_create(surface);
	setColor(cr);
	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription *desc;

	//string can be 1234+100 so start from small k
	for (pi = 0, pj = 0, pk = 0, k = 7;; k++) {
		desc = pango_font_description_from_string(
				format("Times New Roman, %d", k).c_str());
		//pango_font_description_set_weight (desc,PANGO_WEIGHT_NORMAL);
		pango_layout_set_font_description(layout, desc);
		pango_layout_set_markup(layout, s.c_str(), -1);
		pango_layout_get_pixel_size(layout, &i, &j);
		pango_font_description_free(desc);
		if (i > 32 || j > 32) {
			break;
		}
		pi = i;
		pj = j;
		pk = k;
	}
	assert(pi != 0);	//should be at least one cycle step
//	println("%s %d %dx%d %dx%d",s.c_str(),pk,i,j,pi,pj)
	desc = pango_font_description_from_string(
			format("Times New Roman, %d", pk).c_str());
	//pango_font_description_set_weight (desc,PANGO_WEIGHT_NORMAL);
	pango_layout_set_font_description(layout, desc);
	pango_layout_set_markup(layout, s.c_str(), -1);
	cairo_move_to(cr, (32 - pi) / 2, (32 - pj) / 2);
	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);

	pango_font_description_free(desc);
	g_object_unref(layout);

	GList *l = NULL;
	p = gdk_pixbuf_get_from_surface(surface, 0, 0, 32, 32);
	l = g_list_append(l, p);

	s = getImagePath("16.png");
	p = gdk_pixbuf_new_from_file(s.c_str(), 0);
	l = g_list_append(l, p);

	gtk_window_set_icon_list(GTK_WINDOW(window), l);

}

void Frame::buttonClicked(GtkWidget *w) {
	bool sw = isStopwatch();
	if (w == button[0]) {
		Dialog d(DialogType::PARAMETERS);
		if (config.digitalMode != d.digitalMode) {
			config.digitalMode = d.digitalMode;	//changes only after dialog closed
			moveSizeWindow(false);
			paint();
		} else if (config.digitalMode && sw != isStopwatch()) {
			moveSizeWindow(false);
			paint();
		}
		config.closeWarning = d.closeWarning;
		config.setSoundVolume(d.soundVolume);
	} else {
		if (config.digitalMode && sw) {
			mode = Mode::TIME;	//need to switch mode for correct drawing
			moveSizeWindow(false);
		}
		upcoming();
		updateParse();
	}
}

#ifdef SAVE_IMAGE
void Frame::saveImage() {
	gint x,y, width, height;
	GdkRectangle wr;
	GdkPixbuf * pixbuf;

	auto gdk_window = gtk_widget_get_window(window);
	auto monitor = gdk_display_get_monitor_at_window(gdk_display_get_default(),
			gdk_window);
	gdk_monitor_get_workarea(monitor, &wr);
	x=wr.x;
	y=wr.y;
	width=wr.width;
	height=wr.height;
	println("%d %d %dx%d",wr.x,wr.y,wr.width,wr.height)
//	gdk_window_get_frame_extents(gdk_window, &r);	//with title and borders
//	gdk_window_get_geometry(gdk_window, 0, 0, &w, &h);

	gdk_window = gdk_get_default_root_window ();
	g_assert(gdk_window);

//  gdk_window_get_geometry(gdk_window, &x, &y, &width, &height);
	pixbuf = gdk_pixbuf_get_from_window (gdk_window, x, y, width, height);

	g_assert(pixbuf);

	gdk_pixbuf_save (pixbuf, "main.jpg", "jpeg", NULL, "quality", "100", NULL);
	gdk_pixbuf_save(pixbuf, "main.png", "png", NULL, NULL);
	g_object_unref(pixbuf);

}
#endif /* SAVE_IMAGE */

gboolean Frame::windowStateEvent(GdkEventWindowState *e) {
	//if timer isn't run in stopwatch mode change underlined / normal text
	if (isStopwatch() && !timerRunning) {
		setIcon();
	}
	return TRUE;
}

void Frame::setColor(cairo_t *cr) {
	gdk_cairo_set_source_rgba(cr, COLOR + getColorIndex());
}

DigitalFont Frame::createDigitalFont() {
	auto s = W[isStopwatch()];
	CPoint p(maxAreaSize);
	p.y -= config.additionalHeight;
	return DigitalFont(config.maxDigitalClockSize[isStopwatch()], p, s);
}

double Frame::getTime() {
	return timerRunning ? timeElapse(startTime) : 0;
}

#ifndef NDEBUG
void Frame::test() {
	for (int i = 1; i < 8; i++) {
		auto s = DateTime::weekdayName(i);
		println("[%s]", s.c_str())
	}
}
#endif /* NDEBUG */

/*
 * Dialog.cpp
 *
 *       Created on: 03.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#include "Dialog.h"
#include "Config.h"
#include "Frame.h"
#include "EditSettingsFileDialog.h"
#include <cassert>
#include <set>

static Dialog *dlg = 0;

const Mode DEFAULT_MODE = Mode::TIME;
const char CERROR[] = "cerror";
const char HELP_URL[] = "http://slovesnov.rf.gd/?stopwatch";
const char *DIALOG_TITLE[] =
		{
				"Please set up parameters and press enter/return key or click ok button",
				"error", "Success reload" };
const char *DIALOG_ICON[] = { "128", "error", "48" };
static_assert(SIZEI(DIALOG_TITLE)==int(DialogType::DIALOG_TYPE_SIZE));
static_assert(SIZEI(DIALOG_TITLE)==SIZEI(DIALOG_ICON));

//same name function in Frame.cpp so make it static
static void button_clicked(GtkWidget *w, int n) {
	assert(w);
	dlg->buttonClicked(w, n);
}

void entry_activated(GtkEntry*, gpointer) {
	dlg->entryActivated();
}

void entry_changed(GtkWidget*, gpointer) {
	dlg->entryChanged();
}

gboolean combo_changed(GtkComboBox *w, gpointer) {
	dlg->comboChanged(w);
	return TRUE;
}

void check_changed(GtkToggleButton *w, gpointer) {
	dlg->checkChanged(w);
}

void volume_changed(GtkScaleButton*, double v, gpointer) {
	dlg->soundVolumeChanged(v);
}

Dialog::Dialog(DialogType dt, const std::string message) {
	std::string s, q;
	int i, j, k, l;
	GtkWidget *w, *w1, *w2, *w3;

	if (dt == DialogType::PARAMETERS) {
		dlg = this;
		digitalMode = config.digitalMode;
		closeWarning = config.closeWarning;
		soundVolume = config.getSoundVolume();
	}

	dialog = gtk_dialog_new();
	if (frame.created()) {
		/* if frame.created() DialogType::MESSASE && DialogType::ERROR
		 * can be called only from click of reloadButton
		 * so for DialogType::PARAMETERS transient for frame...
		 * otherwise  transient for dialog...
		 */
		gtk_window_set_transient_for(GTK_WINDOW(dialog),
				GTK_WINDOW(
						dt == DialogType::PARAMETERS ?
								frame.getWindow() : dlg->dialog));
	} else {
		gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
	}
	gtk_window_set_resizable(GTK_WINDOW(dialog), false);

	w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	if (dt == DialogType::PARAMETERS) {
		upcomingAllButton = gtk_button_new_with_label("upcoming all");
		gtk_container_add(GTK_CONTAINER(w1), upcomingAllButton);

		minimizeCheck = gtk_check_button_new_with_label("minimize");
		gtk_container_add(GTK_CONTAINER(w1), minimizeCheck);

		comboMode = gtk_combo_box_text_new();
		for (auto a : TITLE) {
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(comboMode), a);
		}
		gtk_container_add(GTK_CONTAINER(w1), comboMode);

		entry = gtk_entry_new();
		gtk_box_pack_start(GTK_BOX(w1), entry, TRUE, TRUE, 0); //stretch
		if (frame.created()) {
			copyButton = gtk_button_new_with_label("copy");
			gtk_container_add(GTK_CONTAINER(w1), copyButton);
			g_signal_connect(copyButton, "clicked", G_CALLBACK(button_clicked),
					0);
		}

		upcomingButton = gtk_button_new_with_label(UPCOMING);
		gtk_container_add(GTK_CONTAINER(w1), upcomingButton);

		okButton = gtk_button_new_with_label("ok");
		gtk_container_add(GTK_CONTAINER(w1), okButton);

	}
	gtk_container_add(GTK_CONTAINER(w), w1);

	//	2nd row={image build} {lerror help/message}
	w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,
			dt == DialogType::PARAMETERS ? 0 : 3);

	//w2=image predefined build
	w2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	if (dt == DialogType::PARAMETERS) {
		w3 = gtk_label_new("predefined sets");
		gtk_widget_set_margin_top(w3, 3);
		gtk_container_add(GTK_CONTAINER(w2), w3);
		mPredefined = gtk_grid_new();
		gtk_container_add(GTK_CONTAINER(w2), mPredefined);
		fillPredefinedSet();
	}

	s = getImagePath(DIALOG_ICON[int(dt)] + std::string(".png"));
	auto image = gtk_image_new_from_file(s.c_str());
	gtk_box_pack_start(GTK_BOX(w2), image, TRUE, TRUE, 0); //fill

	if (dt == DialogType::PARAMETERS) {
		w3 = gtk_label_new("click on button below\nto see full online help");
		gtk_container_add(GTK_CONTAINER(w2), w3);

		s = getImagePath("help.png");
		auto image = gtk_image_new_from_file(s.c_str());
		helpButton = gtk_button_new();
		gtk_button_set_image(GTK_BUTTON(helpButton), image);
		gtk_widget_set_halign(helpButton, GTK_ALIGN_CENTER);
		gtk_container_add(GTK_CONTAINER(w2), helpButton);

		w3 = lInfo = gtk_label_new(nullptr);
		updateLabelInfo();
		addClass(w3, "small");
		//gtk_label_set_angle(GTK_LABEL(w3),90);
		gtk_label_set_xalign(GTK_LABEL(w3), 0);
		gtk_container_add(GTK_CONTAINER(w2), w3);

		editButton = gtk_button_new_with_label("edit/view settings file");
		gtk_widget_set_halign(editButton, GTK_ALIGN_CENTER);
		gtk_box_pack_start(GTK_BOX(w2), editButton, 0, 0, 2);

		reloadButton = gtk_button_new_with_label("reload settings file");
		gtk_widget_set_halign(reloadButton, GTK_ALIGN_CENTER);
		gtk_box_pack_start(GTK_BOX(w2), reloadButton, 0, 0, 2);

		digitalModeCheck = gtk_check_button_new_with_label("digital mode");
		gtk_widget_set_halign(digitalModeCheck, GTK_ALIGN_CENTER);
		gtk_container_add(GTK_CONTAINER(w2), digitalModeCheck);

		closeWarningCheck = gtk_check_button_new_with_label("close warning");
		gtk_widget_set_halign(closeWarningCheck, GTK_ALIGN_CENTER);
		gtk_container_add(GTK_CONTAINER(w2), closeWarningCheck);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(digitalModeCheck),
				digitalMode);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(closeWarningCheck),
				closeWarning);

		volume = gtk_volume_button_new();
		g_signal_connect(volume, "value-changed", G_CALLBACK(volume_changed),
				0);
		gtk_scale_button_set_value(GTK_SCALE_BUTTON(volume), soundVolume);
		//\u266A
		testSoundButton = gtk_button_new_with_label("test \u266B");
		w3 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
		gtk_box_pack_start(GTK_BOX(w3), volume, 1, 1, 2);
		gtk_box_pack_start(GTK_BOX(w3), testSoundButton, 1, 1, 2);
		gtk_container_add(GTK_CONTAINER(w2), w3);

	}
	gtk_container_add(GTK_CONTAINER(w1), w2);

	//w2=lerror help/message
	w2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	if (dt == DialogType::PARAMETERS) {
		lError = gtk_label_new(nullptr);
		gtk_container_add(GTK_CONTAINER(w2), lError);
	}

	w3 = gtk_label_new(nullptr);
	if (dt == DialogType::PARAMETERS) {
		auto f = openResourceFileAsStream("help.txt");
		s.clear();
		while (std::getline(f, q)) {
			if (q[0] == '#') {
				break;
			}
			if (!s.empty()) {
				s += "\n";
			}
			s += q;
		}
		gtk_label_set_markup(GTK_LABEL(w3), s.c_str());
	} else if (dt == DialogType::ERROR) {
		/* cann't do gtk_label_set_markup because get error for "i<=2" parsed as tag
		 * but there is no tag
		 */
		gtk_label_set_label(GTK_LABEL(w3), message.c_str());
	} else {
		gtk_label_set_markup(GTK_LABEL(w3), message.c_str());
	}

	gtk_container_add(GTK_CONTAINER(w2), w3);
	gtk_container_add(GTK_CONTAINER(w1), w2);

	gtk_container_add(GTK_CONTAINER(w), w1);

	w3 = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(w3), w);

	if (dt == DialogType::PARAMETERS) {
		/* after all widgets created set combo
		 * Note set combo after g_signal_connect to update widgets for example lError
		 */
		g_signal_connect(comboMode, "changed", G_CALLBACK(combo_changed), 0);
		gtk_combo_box_set_active(GTK_COMBO_BOX(comboMode), int(DEFAULT_MODE));

		g_signal_connect(entry, "changed", G_CALLBACK(entry_changed), 0);
		g_signal_connect(entry, "activate", G_CALLBACK(entry_activated), 0); //call on enter or return key pressed
		for (auto a : { minimizeCheck, digitalModeCheck, closeWarningCheck })
			g_signal_connect(a, "toggled", G_CALLBACK(check_changed), 0);
		for (auto a : { okButton, upcomingButton, helpButton, editButton,
				reloadButton, upcomingAllButton, testSoundButton })
			g_signal_connect(a, "clicked", G_CALLBACK(button_clicked), 0);

	}

	gtk_window_set_title(GTK_WINDOW(dialog), DIALOG_TITLE[int(dt)]);
	gtk_widget_show_all(dialog);

	if (dt == DialogType::PARAMETERS) {
		//after show_all center horizontally lError from middle of entry
		w = gtk_widget_get_toplevel(entry);
		l = gtk_widget_get_allocated_width(entry);
		gtk_widget_translate_coordinates(entry, w, l / 2, 0, &i, 0); //i=center of entry

		assert(gtk_widget_get_toplevel(lError) == w);
		gtk_widget_translate_coordinates(lError, w, 0, 0, &j, 0); //j-left of lError
		l = gtk_widget_get_allocated_width(lError);
		gtk_widget_translate_coordinates(lError, w, l, 0, &k, 0); //k-right of lError

		j = i - j;
		k = k - i;
		if (j > k) {
			gtk_widget_set_margin_start(lError, j - k);
		} else {
			gtk_widget_set_margin_end(lError, k - j);
		}

		i = gtk_widget_get_allocated_width(editButton);
		gtk_widget_set_size_request(reloadButton, i, -1);

		//countAreaSize() avoid blinks read comments in Frame.cpp file
		gtk_widget_grab_focus(entry);
		frame.countAreaSize(dialog);
	}
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void Dialog::buttonClicked(GtkWidget *w, int n) {
	if (w == okButton) {
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
		frame.setParameters(*this);
		//frame don't call destroy signal on windows shutdown so store parameters here, after frame.setParameters
		if (isTime()) {
			config.write();
		}
	} else if (w == copyButton) {
		copyAddDays(frame);
		updateParametersChanges();
		/* Note need call entryChanged();
		 * because need to recount parameters
		 * time 0:00 and parameters "+5 1720" means 5 1720
		 * and time 0:30 and parameters "+5 1720" means 35 1720
		 */
		entryChanged();
	} else if (w == upcomingButton) {
		upcoming();
		updateParametersChanges();
	} else if (w == upcomingAllButton) {
		upcomingAll();
		updateParametersChanges();
	} else if (w == helpButton) {
		gtk_show_uri_on_window(0, HELP_URL, GDK_CURRENT_TIME, NULL);
	} else if (w == editButton) {
		/* Note	gtk_show_uri_on_window(0, getPredefinedFileName().c_str(), GDK_CURRENT_TIME,NULL);
		 * hang if set default application for txt files scite so use my own editor
		 * also the same for g_spawn_async
		 */
		EditSettingsFileDialog d(dlg->dialog);
		if (d.responseCode == GTK_RESPONSE_OK) {
			//loadPredefined() ok so just update
			updateLoadPredefined();
		}
	} else if (w == reloadButton) {
		if (loadPredefined()) {
			//before show dialog, to make updates
			updateLoadPredefined();

			auto a = getPredefinedSetString() + "\n\n"
					+ getPredefinedTimeString(DialogType::MESSASE) + "\n\n"
					+ getPredefinedDateString();
			Dialog d(DialogType::MESSASE, a);
		} else {
			Dialog d(DialogType::ERROR, what());
		}
	} else if (w == testSoundButton) {
		int volume = Config::getSoundVolumeValue(soundVolume);
		beep(volume);
	} else {
		std::string s = predefinedSet[n >> 1];
		bool ok = parse(s);
		updateParametersChanges(ok);
		if ((n & 1) && ok) {
			buttonClicked(okButton);
		}
	}
}

void Dialog::updateParametersChanges(bool ok) {
	signals = false;
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboMode), getMode());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(minimizeCheck), minimize());
	auto s = toString(StringType::ENTRY);
	gtk_entry_set_text(GTK_ENTRY(entry), s.c_str());
	signals = true;
	updateEntryChanges(ok);
}

void Dialog::entryActivated() {
	if (gtk_widget_get_sensitive(okButton)) { //valid arguments
		buttonClicked(okButton);
	}
}

void Dialog::entryChanged() {
	if (!signals) {
		return;
	}
	updateEntryChanges(
			parse(gtk_combo_box_get_active(GTK_COMBO_BOX(comboMode)),
					gtk_toggle_button_get_active(
							GTK_TOGGLE_BUTTON(minimizeCheck)),
					gtk_entry_get_text(GTK_ENTRY(entry))));
}

void Dialog::updateEntryChanges(bool ok) {
	std::string s;
	if (ok) {
		s = toString(StringType::DIALOG); //output recognized values
		removeClass(entry, CERROR); //normal font, all valid
		removeClass(lError, CERROR);
	} else {
		s = what(true);
		addClass(entry, CERROR); //red font
		addClass(lError, CERROR);
	}
	gtk_label_set_text(GTK_LABEL(lError), s.c_str());
	gtk_widget_set_sensitive(okButton, ok);
}

void Dialog::updateLoadPredefined() {
	updateReload();
	frame.updateReload();
}

void Dialog::modifyMinimizeCheck() {
	auto a = Mode(gtk_combo_box_get_active(GTK_COMBO_BOX(comboMode)));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(minimizeCheck),
			a != Mode::STOPWATCH);
}

void Dialog::updateReload() {
	signals = false;
	fillPredefinedSet();
	signals = true;
	updateLabelInfo();
}

void Dialog::fillPredefinedSet() {
	clearContainer(mPredefined);
	int i = -1, j;
	for (auto &a : predefinedSet) {
		i++;
		for (j = 0; j < 2; j++) {
			auto b = gtk_button_new_with_label(j ? "." : a.c_str());
			gtk_grid_attach(GTK_GRID(mPredefined), b, j, i, 1, 1);
			g_signal_connect(b, "clicked", G_CALLBACK(button_clicked),
					GP((i<<1)|j));
		}
	}
	gtk_widget_show_all(mPredefined);
}

void Dialog::updateLabelInfo() {
	auto s = getPredefinedTimeString() + "\n\n" + getInfoString()
			+ "\nauthor aleksey slovesnov";
	gtk_label_set_text(GTK_LABEL(lInfo), s.c_str());
}

void Dialog::comboChanged(GtkComboBox *w) {
	if (!signals) {
		return;
	}

	if (w == GTK_COMBO_BOX(comboMode)) {
		modifyMinimizeCheck();
		entryChanged();
	}
}

void Dialog::checkChanged(GtkToggleButton *w) {
	if (w == GTK_TOGGLE_BUTTON(minimizeCheck)) {
		if (signals) {
			entryChanged();
		}
	} else if (w == GTK_TOGGLE_BUTTON(digitalModeCheck)) {
		digitalMode = gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(digitalModeCheck));
	} else {
		closeWarning = gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(closeWarningCheck));
	}
}

void Dialog::soundVolumeChanged(double v) {
	soundVolume = v;
}

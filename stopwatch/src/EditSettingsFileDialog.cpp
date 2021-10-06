/*
 * EditSettingsFileDialog.cpp
 *
 *       Created on: 07.05.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "EditSettingsFileDialog.h"
#include "Frame.h"
#include "Dialog.h"

/* Note if edit settings file and make some insertions Ctrl+V
 * then after close stopwatch application get gtk warnings
 * don't know why may be gtk bug
 * (stopwatch.exe:6436): Gtk-CRITICAL **: 02:22:38.250: gtk_widget_get_window: assertion 'GTK_IS_WIDGET (widget)' failed
 * (stopwatch.exe:6436): Gdk-CRITICAL **: 02:22:38.250: gdk_window_get_state: assertion 'GDK_IS_WINDOW (window)' failed
 */

static EditSettingsFileDialog*dlg;

static void mark_set(GtkTextBuffer *, GtkTextIter *, GtkTextMark *, gpointer) {
	dlg->updateInfo();
}

static void button_clicked(GtkWidget *w, gpointer) {
	assert(w);
	dlg->buttonClicked(w);
}

EditSettingsFileDialog::EditSettingsFileDialog(GtkWidget *parent) {
	GtkWidget *w, *w1;
	std::string s;
	GtkAllocation al;
	const double kl = 0.75;
	GtkTextIter start;

	dlg = this;
	dialog = gtk_dialog_new();
	view = gtk_text_view_new();
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

	auto scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled), view);

	gtk_widget_get_allocation(parent, &al);
	gtk_widget_set_size_request(scrolled, al.width * kl, al.height * kl);

	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD);

	gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));

	s = getPredefinedFileContent();
	gtk_text_buffer_set_text(buffer, s.c_str(), -1);

	button = gtk_button_new_with_label(
			"save changes, reload settings file and close dialog");
	info = gtk_label_new("");
	gtk_widget_set_halign(info, GTK_ALIGN_END);

	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(w), button);
	gtk_box_pack_start(GTK_BOX(w), info, TRUE, TRUE, 0); //fill

	w1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_container_add(GTK_CONTAINER(w1), scrolled);
	gtk_container_add(GTK_CONTAINER(w1), w);

	w = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	gtk_container_add(GTK_CONTAINER(w), w1);

	gtk_window_set_title(GTK_WINDOW(dialog), "settings file editor");

	g_signal_connect(buffer, "mark_set", G_CALLBACK(mark_set), 0);
	g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), 0);

	//place cursor to start of file
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_place_cursor(buffer, &start);
	//signal "mark_set" is connected so don't need call updateInfo();

	gtk_widget_show_all(dialog);

	responseCode = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void EditSettingsFileDialog::updateInfo() {
	GtkTextIter it;
	gtk_text_buffer_get_iter_at_mark(buffer, &it,
			gtk_text_buffer_get_insert(buffer));

	int l = gtk_text_iter_get_line(&it);
	int c = gtk_text_iter_get_line_offset(&it);
	auto s = format("line:%d column:%d", l + 1, c + 1);
	gtk_label_set_text(GTK_LABEL(info), s.c_str());
}

void EditSettingsFileDialog::buttonClicked(GtkWidget* w) {
	GtkTextIter start, end;
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);
	std::string s = gtk_text_buffer_get_text(buffer, &start, &end, TRUE);

	if (loadPredefined(s)) {
		writableFileSetContents(getPredefinedFileName(), s);
		gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	}
	else {
		Dialog d(DialogType::ERROR, what());
	}

}

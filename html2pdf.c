#include <stdio.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

//#include <mcheck.h>

int printStatus=0;

void printFailed(WebKitPrintOperation *po, gpointer err, gpointer dat) {
	printStatus=-1;
}
void printFinished(WebKitPrintOperation *po, gpointer dat) {
	gtk_main_quit();
}
void loadFinished(WebKitWebView *web, WebKitLoadEvent ev, char *pdf) {
	if(ev!=WEBKIT_LOAD_FINISHED)
		return;

	WebKitPrintOperation *po=webkit_print_operation_new(web);
	GtkPrintSettings *pts=gtk_print_settings_new();
	GtkPaperSize *ps=gtk_paper_size_new_custom("epd", "E-Paper Display", 160, 240, GTK_UNIT_MM);
	gtk_print_settings_set_number_up(pts, 1);
	gtk_print_settings_set_reverse(pts, false);
	gtk_print_settings_set_print_pages(pts, GTK_PRINT_PAGES_ALL);
	gtk_print_settings_set(pts, "output-file-format", "pdf");
	gtk_print_settings_set(pts, "output-uri", pdf);
	gtk_print_settings_set_collate(pts, false);
	gtk_print_settings_set_n_copies(pts, 1);
	gtk_print_settings_set_printer(pts, "Print to File");
	gtk_print_settings_set_orientation(pts, GTK_PAGE_ORIENTATION_PORTRAIT);
	gtk_print_settings_set_paper_size(pts, ps);
	gtk_print_settings_set_use_color(pts, true);
	gtk_print_settings_set_quality(pts, GTK_PRINT_QUALITY_HIGH);
	gtk_print_settings_set_page_set(pts, GTK_PAGE_SET_ALL);
	gtk_print_settings_set_media_type(pts, "screen-paged");
	gtk_print_settings_set_scale(pts, 100);
	GtkPageSetup *pgs=gtk_page_setup_new();
	gtk_page_setup_set_orientation(pgs, GTK_PAGE_ORIENTATION_PORTRAIT);
	gtk_page_setup_set_paper_size(pgs, ps);
	gtk_page_setup_set_top_margin(pgs, 4, GTK_UNIT_MM);
	gtk_page_setup_set_bottom_margin(pgs, 4, GTK_UNIT_MM);
	gtk_page_setup_set_left_margin(pgs, 4, GTK_UNIT_MM);
	gtk_page_setup_set_right_margin(pgs, 4, GTK_UNIT_MM);
	webkit_print_operation_set_print_settings(po, pts);
	webkit_print_operation_set_page_setup(po, pgs);
	g_signal_connect(G_OBJECT(po), "failed", G_CALLBACK(printFailed), NULL);
	g_signal_connect(G_OBJECT(po), "finished", G_CALLBACK(printFinished), NULL);

	webkit_print_operation_print(po);
	///////////////
	g_object_unref(po);
	g_object_unref(pts);
	g_object_unref(pgs);
	gtk_paper_size_free(ps);
	////
}
int main(int argc, char *argv[]) {
	//mtrace();
	gtk_init(&argc, &argv);
	if(argc!=3) {
		fprintf(stderr, "Usage: %s <HTML> <PDF>\n", argv[0]);
		return -1;
	}

	char absHtml[PATH_MAX];
	char absPdf[PATH_MAX];
	realpath(argv[1], absHtml);
	realpath(argv[2], absPdf);
	gchar *uriHtml=g_filename_to_uri(absHtml, NULL, NULL);
	gchar *uriPdf=g_filename_to_uri(absPdf, NULL, NULL);

	GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	WebKitUserContentManager *ucm=webkit_user_content_manager_new();
	WebKitUserStyleSheet *style=webkit_user_style_sheet_new(
			"p { text-align: justify; -webkit-hyphens: auto; hyphens: auto; } "
			"body { text-rendering: optimizeLegibility; -webkit-font-feature-settings: \"kern\" 1, \"dlig\" 1, \"frac\" 1, \"liga\" 1;  font-feature-settings: \"kern\" 1, \"dlig\" 1, \"frac\" 1, \"liga\" 1; font-variant-ligatures: common-ligatures; -webkit-font-variant-ligatures: common-ligatures; font-kerning: normal; }",
			WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
			WEBKIT_USER_STYLE_LEVEL_AUTHOR, //????
			NULL, NULL);
	webkit_user_content_manager_add_style_sheet(ucm, style);

	GtkWidget *web=webkit_web_view_new_with_user_content_manager(ucm);
	WebKitSettings *set=webkit_web_view_get_settings(WEBKIT_WEB_VIEW(web));
	// settings
	webkit_settings_set_monospace_font_family(set, "FreeMono");
	webkit_settings_set_sans_serif_font_family(set, "FreeSans");
	webkit_settings_set_serif_font_family(set, "FreeSerif");
	webkit_settings_set_default_font_family(set, "FreeSerif");
	webkit_settings_set_default_font_size(set, 16);
	webkit_settings_set_default_monospace_font_size(set, 16);
	webkit_settings_set_default_charset(set, "utf-8");
	///
	gtk_container_add(GTK_CONTAINER(win), web);
	gtk_window_set_default_size(GTK_WINDOW(win), 720, 960);
	//gtk_widget_show_all(win);
	g_signal_connect(web, "load-changed", G_CALLBACK(loadFinished), uriPdf);
	webkit_web_view_load_uri(WEBKIT_WEB_VIEW(web), uriHtml);
	//
	//
	//g_object_set (G_OBJECT(set), "enable-developer-extras", TRUE, NULL);
	//WebKitWebInspector *inspector = webkit_web_view_get_inspector (WEBKIT_WEB_VIEW(web));
	//webkit_web_inspector_show (WEBKIT_WEB_INSPECTOR(inspector));
	//
	gtk_main();
	gtk_widget_destroy(win);

	g_free(uriHtml);
	g_free(uriPdf);
	g_object_unref(ucm);
	webkit_user_style_sheet_unref(style);

	//muntrace();
	return printStatus;
}

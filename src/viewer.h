#ifndef VIEWER_H
#define VIEWER_H

#include <gtk/gtk.h>

GtkWidget *viewer_new(void);
void viewer_load_markdown(GtkWidget *viewer, const char *html, const char *base_uri);
void viewer_load_uri(GtkWidget *viewer, const char *uri);

#endif

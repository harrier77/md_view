#include "viewer.h"

#include <webkit/webkit.h>

#define KEY_WEBVIEW "md-view-webview"

GtkWidget *viewer_new(void)
{
  GtkWidget *scrolled = gtk_scrolled_window_new();
  gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(scrolled), TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
      GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  WebKitWebView *webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
  gtk_widget_set_vexpand(GTK_WIDGET(webview), TRUE);
  gtk_widget_set_hexpand(GTK_WIDGET(webview), TRUE);

  WebKitSettings *settings = webkit_web_view_get_settings(webview);
  webkit_settings_set_enable_javascript(settings, FALSE);


  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled),
      GTK_WIDGET(webview));

  g_object_set_data(G_OBJECT(scrolled), KEY_WEBVIEW, webview);

  return scrolled;
}

static WebKitWebView *get_webview(GtkWidget *viewer)
{
  GtkWidget *child = gtk_scrolled_window_get_child(
      GTK_SCROLLED_WINDOW(viewer));

  if (WEBKIT_IS_WEB_VIEW(child))
    return WEBKIT_WEB_VIEW(child);

  GtkWidget *first = gtk_widget_get_first_child(child);
  if (WEBKIT_IS_WEB_VIEW(first))
    return WEBKIT_WEB_VIEW(first);

  return g_object_get_data(G_OBJECT(viewer), KEY_WEBVIEW);
}

void viewer_load_markdown(GtkWidget *viewer, const char *html, const char *base_uri)
{
  WebKitWebView *webview = get_webview(viewer);
  if (!webview) {
    g_warning("viewer_load_markdown: no WebKitWebView found");
    return;
  }
  webkit_web_view_load_html(webview, html, base_uri);
}

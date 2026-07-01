#include "app.h"
#include "viewer.h"
#include "md_render.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  GtkWidget *window;
  GtkWidget *viewer;
  char *current_file;
} AppState;

static void app_open_file(AppState *s, const char *path)
{
  char *html = md_render_file(path);
  if (!html) {
    g_printerr("Failed to render: %s\n", path);
    return;
  }

  char *dir = g_path_get_dirname(path);
  char *base_uri = g_strdup_printf("file://%s/", dir);
  viewer_load_markdown(s->viewer, html, base_uri);
  g_free(base_uri);
  g_free(dir);
  g_free(html);

  free(s->current_file);
  s->current_file = strdup(path);

  char title[1024];
  snprintf(title, sizeof(title), "md_view — %s", path);
  gtk_window_set_title(GTK_WINDOW(s->window), title);
}

static void on_dialog_response(GtkNativeDialog *dialog, int response, gpointer data)
{
  AppState *s = data;

  if (response == GTK_RESPONSE_ACCEPT) {
    GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
    char *path = g_file_get_path(file);
    app_open_file(s, path);
    g_free(path);
    g_object_unref(file);
  }
  g_object_unref(dialog);
}

static void open_dialog(GtkWidget *btn, gpointer data)
{
  (void)btn;
  AppState *s = data;

  GtkFileChooserNative *dialog = gtk_file_chooser_native_new(
      "Open Markdown File", GTK_WINDOW(s->window),
      GTK_FILE_CHOOSER_ACTION_OPEN, "_Open", "_Cancel");

  GtkFileFilter *filter = gtk_file_filter_new();
  gtk_file_filter_set_name(filter, "Markdown Files (*.md)");
  gtk_file_filter_add_pattern(filter, "*.md");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

  GtkFileFilter *all = gtk_file_filter_new();
  gtk_file_filter_set_name(all, "All Files");
  gtk_file_filter_add_pattern(all, "*");
  gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), all);

  g_signal_connect(dialog, "response", G_CALLBACK(on_dialog_response), s);
  gtk_native_dialog_show(GTK_NATIVE_DIALOG(dialog));
}

static void on_open_action(GSimpleAction *action, GVariant *param, gpointer data)
{
  (void)action;
  (void)param;
  open_dialog(NULL, data);
}

static void on_quit_action(GSimpleAction *action, GVariant *param, gpointer data)
{
  (void)action;
  (void)param;
  AppState *s = data;
  gtk_window_destroy(GTK_WINDOW(s->window));
}

static GActionEntry app_entries[] = {
  { "open", on_open_action, NULL, NULL, NULL },
  { "quit", on_quit_action, NULL, NULL, NULL },
};

static GMenuModel *make_menubar_model(void)
{
  GMenu *file_menu = g_menu_new();
  g_menu_append(file_menu, "_Open", "app.open");

  GMenu *quit_section = g_menu_new();
  g_menu_append(quit_section, "_Quit", "app.quit");
  g_menu_append_section(file_menu, NULL, G_MENU_MODEL(quit_section));
  g_object_unref(quit_section);

  GMenu *menubar = g_menu_new();
  g_menu_append_submenu(menubar, "_File", G_MENU_MODEL(file_menu));
  g_object_unref(file_menu);

  return G_MENU_MODEL(menubar);
}

static void startup(GApplication *app, gpointer data)
{
  (void)data;

  gtk_application_set_accels_for_action(GTK_APPLICATION(app),
      "app.open", (const char *[]){"<Ctrl>O", NULL});
  gtk_application_set_accels_for_action(GTK_APPLICATION(app),
      "app.quit", (const char *[]){"<Ctrl>Q", NULL});
}

static void activate(GApplication *app, gpointer data)
{
  AppState *s = data;
  if (s->window) {
    gtk_window_present(GTK_WINDOW(s->window));
    return;
  }

  s->window = gtk_application_window_new(GTK_APPLICATION(app));
  gtk_window_set_title(GTK_WINDOW(s->window), "md_view");
  gtk_window_set_default_size(GTK_WINDOW(s->window), 900, 700);

  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_window_set_child(GTK_WINDOW(s->window), box);

  GMenuModel *model = make_menubar_model();
  GtkWidget *menubar = gtk_popover_menu_bar_new_from_model(model);
  g_object_unref(model);
  gtk_box_append(GTK_BOX(box), menubar);

  s->viewer = viewer_new();
  gtk_widget_set_vexpand(s->viewer, TRUE);
  gtk_box_append(GTK_BOX(box), s->viewer);

  gtk_window_present(GTK_WINDOW(s->window));
}

static void on_open_files(GApplication *app, GFile **files,
    int n, gchar *hint, gpointer data)
{
  (void)app;
  (void)hint;
  AppState *s = data;
  activate(G_APPLICATION(app), s);
  if (n > 0) {
    char *path = g_file_get_path(files[0]);
    app_open_file(s, path);
    g_free(path);
  }
}

static void on_app_destroy(gpointer data, GObject *obj)
{
  (void)obj;
  free(data);
}

GtkApplication *app_new(void)
{
  AppState *s = calloc(1, sizeof(AppState));

  GtkApplication *app = gtk_application_new(APP_ID, G_APPLICATION_HANDLES_OPEN);
  g_signal_connect(app, "startup", G_CALLBACK(startup), s);
  g_signal_connect(app, "activate", G_CALLBACK(activate), s);
  g_signal_connect(app, "open", G_CALLBACK(on_open_files), s);

  g_action_map_add_action_entries(G_ACTION_MAP(app),
      app_entries, G_N_ELEMENTS(app_entries), s);

  g_object_weak_ref(G_OBJECT(app), on_app_destroy, s);

  return app;
}

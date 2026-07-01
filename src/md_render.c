#include "md_render.h"
#include "resources.h"

#include <cmark.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path, size_t *out_len)
{
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;

  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (len < 0) { fclose(f); return NULL; }

  char *buf = malloc((size_t)len + 1);
  if (!buf) { fclose(f); return NULL; }

  size_t n = fread(buf, 1, (size_t)len, f);
  fclose(f);
  buf[n] = '\0';

  if (out_len) *out_len = n;
  return buf;
}

char *md_render_file(const char *path)
{
  size_t md_len;
  char *md = read_file(path, &md_len);
  if (!md) return NULL;

  char *html_body = cmark_markdown_to_html(md, md_len, CMARK_OPT_DEFAULT);
  free(md);
  if (!html_body) return NULL;

  GResource *res = gresource_get_resource();
  GBytes *css_bytes = g_resource_lookup_data(res,
      "/com/mdview/style.css", G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

  const char *css = "";
  gsize css_len = 0;
  if (css_bytes) {
    css = g_bytes_get_data(css_bytes, &css_len);
  }

  GString *html = g_string_new(
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
      "<meta name=\"color-scheme\" content=\"light dark\">"
      "<style>");
  g_string_append_len(html, css, css_len);
  g_string_append(html, "</style></head><body>");
  g_string_append(html, html_body);
  g_string_append(html, "</body></html>");

  free(html_body);
  if (css_bytes) g_bytes_unref(css_bytes);

  return g_string_free(html, FALSE);
}

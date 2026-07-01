# md_view — Markdown Viewer with GTK4 and WebKitGTK

Reads Markdown (`.md`) files and displays them in a GTK4 window using
WebKitGTK for rendering, with markdown→HTML conversion via
`libcmark` (CommonMark).

## Dependencies

- `libgtk-4-dev` ≥ 4.6
- `libwebkitgtk-6.0-dev` ≥ 2.40
- `libcmark-dev` ≥ 0.30
- `meson` ≥ 0.60, `ninja`

### Installation on Ubuntu 22.04

```bash
sudo apt install libgtk-4-dev libwebkitgtk-6.0-dev libcmark-dev meson ninja-build
```

> **Note**: `libwebkitgtk-6.0-dev` is only available from
> `jammy-updates`/`jammy-security`. If you can't find it,
> update the package list with `sudo apt update`.

## Build

```bash
meson setup builddir
ninja -C builddir
```

## Install

Installs to `/opt/md_view` (default) and creates a symlink at
`/usr/local/bin/md_view`:

```bash
sudo ./install.sh
```

To install to a different path:

```bash
sudo ./install.sh /usr/local
```

Uninstall:

```bash
sudo ./uninstall.sh
```

## Usage

```bash
# With a file passed as argument
./builddir/md_view helloworld.md

# Without arguments — empty window, open file with Ctrl+O
./builddir/md_view
```

## Architecture

```
src/
├── main.c          # Entry point, creates GtkApplication
├── app.h / app.c   # GtkApplication, Open/Quit actions, menu bar
├── viewer.h/c      # WebKitWebView wrapped in GtkScrolledWindow
├── md_render.h/c   # cmark → HTML + CSS embedded via GResource
└── resources/
    ├── style.css   # Light/dark auto theme (prefers-color-scheme)
    └── gresource.xml
```

### Flow

1. **`main.c`** starts `GtkApplication` with flag `G_APPLICATION_HANDLES_OPEN`
2. If passed via CLI, GApplication emits `::open` → `on_open_files()`
3. **`md_render.c`** reads the file, passes it to `cmark_markdown_to_html()`,
   wraps the result in a full HTML document with `<style>` from the embedded CSS
4. **`viewer.c`** loads the HTML with `webkit_web_view_load_html()`
5. The window has a `GtkPopoverMenuBar` at the top with **File > Open**
   (Ctrl+O) and **File > Quit** (Ctrl+Q) above the viewer
6. Without arguments, `Ctrl+O` or menu → `GtkFileChooserNative` filtered
   for `*.md`

## Bug fixes

### Vertical layout with always-visible menu bar

**Problem**: The menu set with `gtk_application_set_menubar()` is only
shown in the titlebar of CSD windows. On systems without CSD or with
window managers that don't support it, the menu bar is invisible and the
WebKit widget fills the entire window.

**Fix**: Vertical layout in `activate()` (`app.c`): the window contains a
`GtkBox` with a `GtkPopoverMenuBar` (created from `GMenuModel`) at the top
and the viewer below with `vexpand: TRUE`. Works regardless of CSD support.

### Missing `quit` action

**Problem**: `app.quit` was registered as an accelerator (`<Ctrl>Q`) but
did not exist in `GActionEntry` — the shortcut did nothing.

**Fix**: Added `quit` action with callback `on_quit_action()` that calls
`gtk_window_destroy()`.

### Invalid cast `GtkViewport → WebKitWebView`

**Problem**: `gtk_scrolled_window_get_child()` in GTK4 does not always
return the widget you passed to `set_child()`. If the child does not
implement `GtkScrollable` (like `WebKitWebView`), GTK4 automatically
inserts an intermediate `GtkViewport`. `get_child()` returns the viewport,
not the webview, causing:

```
invalid cast from 'GtkViewport' to 'WebKitWebView'
```

**Fix**: `get_webview()` function in `viewer.c` that tries in order:
1. Direct cast of `get_child()` result
2. `gtk_widget_get_first_child()` on the viewport
3. Fallback to `g_object_set_data()` saved at creation time

Handles both the automatic viewport case and future versions where
WebKitWebView might become scrollable.

### Missing `base_uri`

**Problem**: `webkit_web_view_load_html()` called with `base_uri = NULL`.
Some WebKitGTK versions may refuse to load or have issues with relative
resources.

**Fix**: Compute `base_uri` as `file:///directory/of/file/` in
`app_open_file()` and pass it to `viewer_load_markdown()`.

### CSS embedded via GResource

The `style.css` stylesheet is compiled into the binary with
`gnome.compile_resources()` (meson). Automatic light/dark themes
via `@media (prefers-color-scheme: dark)`.

## Technical dependencies resolved

On Ubuntu 22.04, `libwebkit2gtk-4.1-dev` is pinned to version 2.36.0
(while the runtime library is 2.50.4), causing `apt` conflicts.
Solution: use `libwebkitgtk-6.0-dev` (API 6.0 based on GTK4), available
in an updated version from `jammy-updates`/`jammy-security`.

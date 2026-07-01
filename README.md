# md_view — Visualizzatore Markdown con GTK4 e WebKitGTK

Legge file Markdown (`.md`) e li mostra in una finestra GTK4 usando
WebKitGTK per il rendering, con conversione markdown→HTML tramite
`libcmark` (CommonMark).

## Dipendenze

- `libgtk-4-dev` ≥ 4.6
- `libwebkitgtk-6.0-dev` ≥ 2.40
- `libcmark-dev` ≥ 0.30
- `meson` ≥ 0.60, `ninja`

### Installazione su Ubuntu 22.04

```bash
sudo apt install libgtk-4-dev libwebkitgtk-6.0-dev libcmark-dev meson ninja-build
```

> **Nota**: `libwebkitgtk-6.0-dev` è disponibile solo da
> `jammy-updates`/`jammy-security`. Se non lo trovi,
> aggiorna la lista pacchetti con `sudo apt update`.

## Build

```bash
meson setup builddir
ninja -C builddir
```

## Install

Installa in `/opt/md_view` (default) e crea il symlink
`/usr/local/bin/md_view`:

```bash
sudo ./install.sh
```

Per installare in un percorso diverso:

```bash
sudo ./install.sh /usr/local
```

Disinstallazione:

```bash
sudo ./uninstall.sh
```

## Esecuzione

```bash
# Con file passato come argomento
./builddir/md_view helloworld.md

# Senza argomenti — finestra vuota, apri file con Ctrl+O
./builddir/md_view
```

## Architettura

```
src/
├── main.c          # Entry point, crea GtkApplication
├── app.h / app.c   # GtkApplication, azioni Open/Quit, barra menu
├── viewer.h/c      # WebKitWebView incapsulato in GtkScrolledWindow
├── md_render.h/c   # cmark → HTML + CSS embedded via GResource
└── resources/
    ├── style.css   # Tema chiaro/scuro automatico (prefers-color-scheme)
    └── gresource.xml
```

### Flusso

1. **`main.c`** avvia `GtkApplication` con flag `G_APPLICATION_HANDLES_OPEN`
2. Se passato via CLI, GApplication emette `::open` → `on_open_files()`
3. **`md_render.c`** legge il file, lo passa a `cmark_markdown_to_html()`,
   avvolge il risultato in un HTML completo con `<style>` dal CSS embedded
4. **`viewer.c`** carica l'HTML con `webkit_web_view_load_html()`
5. La finestra contiene una `GtkPopoverMenuBar` in alto con **File > Open**
   (Ctrl+O) e **File > Quit** (Ctrl+Q) sopra il viewer
6. In assenza di argomenti, `Ctrl+O` o menu → `GtkFileChooserNative` filtrato
   per `*.md`

## Bugfix applicati

### Layout verticale con menu visibile

**Problema**: Il menu impostato con `gtk_application_set_menubar()` viene
mostrato solo nel titlebar delle finestre con CSD. Su sistemi senza CSD o con
gestori di finestre che non lo supportano, la barra menu è invisibile e il
widget WebKit occupa l'intera finestra.

**Fix**: Layout verticale in `activate()` (`app.c`): la finestra contiene un
`GtkBox` con una `GtkPopoverMenuBar` (creata da `GMenuModel`) in alto e il
viewer sotto con `vexpand: TRUE`. Funziona sempre, indipendentemente dal
supporto CSD.

### Azione `quit` mancante

**Problema**: `app.quit` era registrato come acceleratore (`<Ctrl>Q`) ma
non esisteva nelle `GActionEntry` — la scorciatoia non faceva nulla.

**Fix**: Aggiunta azione `quit` con callback `on_quit_action()` che chiama
`gtk_window_destroy()`.

### Cast errato `GtkViewport → WebKitWebView`

**Problema**: `gtk_scrolled_window_get_child()` in GTK4 non restituisce
sempre il widget che hai passato a `set_child()`. Se il child non implementa
`GtkScrollable` (come `WebKitWebView`), GTK4 inserisce automaticamente un
`GtkViewport` intermedio. `get_child()` restituisce il viewport, non il
webview, causando:

```
invalid cast from 'GtkViewport' to 'WebKitWebView'
```

**Fix**: Funzione `get_webview()` in `viewer.c` che tenta in ordine:
1. Cast diretto del risultato di `get_child()`
2. `gtk_widget_get_first_child()` sul viewport
3. Fallback a `g_object_set_data()` salvato in fase di creazione

Serve a gestire sia il caso con viewport automatico, sia versioni future
in cui WebKitWebView potrebbe diventare scrollable.

### `base_uri` non impostato

**Problema**: `webkit_web_view_load_html()` chiamato con `base_uri = NULL`.
Alcune versioni di WebKitGTK possono rifiutare il caricamento o avere
problemi con risorse relative.

**Fix**: Calcolo del `base_uri` come `file:///directory/del/file/` in
`app_open_file()` e passaggio a `viewer_load_markdown()`.

### CSS embedded via GResource

Il foglio di stile `style.css` viene compilato nel binario con
`gnome.compile_resources()` (meson). Temi chiaro/scuro automatici
tramite `@media (prefers-color-scheme: dark)`.

## Dipendenze tecniche risolte

Su Ubuntu 22.04, `libwebkit2gtk-4.1-dev` è bloccato alla versione 2.36.0
(mentre la libreria runtime è 2.50.4), causando conflitti `apt`.
Soluzione: usare `libwebkitgtk-6.0-dev` (API 6.0 basata su GTK4), disponibile
in versione aggiornata dai repos `jammy-updates`/`jammy-security`.

project('md_view', 'c',
  version: '0.1.0',
  meson_version: '>= 0.60.0')

gnome = import('gnome')

gtk4   = dependency('gtk4', version: '>= 4.0')
webkit = dependency('webkitgtk-6.0', version: '>= 2.40')
cmark  = dependency('libcmark', version: '>= 0.30')

resources = gnome.compile_resources('resources',
  'src/resources/gresource.xml',
  source_dir: 'src/resources')

executable('md_view',
  'src/main.c', 'src/app.c', 'src/viewer.c', 'src/md_render.c',
  resources,
  dependencies: [gtk4, webkit, cmark],
  install: true)

install_data('com.mdview.app.desktop',
  install_dir: get_option('datadir') / 'applications')

install_data('src/resources/com.mdview.app.svg',
  install_dir: get_option('datadir') / 'icons/hicolor/scalable/apps')

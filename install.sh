#!/bin/sh
set -e

PREFIX="${1:-/opt/md_view}"

echo "==> Configure build directory (prefix: $PREFIX)"
meson setup builddir --prefix="$PREFIX" --reconfigure 2>/dev/null || \
  meson setup builddir --prefix="$PREFIX"

echo ""
echo "==> Build"
ninja -C builddir

echo ""
echo "==> Install (sudo)"
sudo meson install -C builddir

echo ""
echo "==> Create symlink /usr/local/bin/md_view"
sudo ln -sf "$PREFIX/bin/md_view" /usr/local/bin/md_view

echo ""
echo "Done.  Run 'md_view' to start."

#!/bin/sh
set -e

PREFIX="${1:-/opt/md_view}"

echo "==> Remove symlink"
sudo rm -f /usr/local/bin/md_view

echo "==> Remove installed files"
sudo rm -rf "$PREFIX"

echo "Done."

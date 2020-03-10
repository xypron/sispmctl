#!/bin/sh

set -e

test -f configure.ac || {
  echo "Please, run this script in the top level project directory."
  exit
}

libtoolize --force --copy
aclocal -I m4
autoconf
autoheader
automake --add-missing --force --copy

echo "For installation instructions, please, refer to file INSTALL."

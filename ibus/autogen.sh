#! /bin/sh
cd `dirname $0`
aclocal
autoheader
automake --add-missing
autoconf

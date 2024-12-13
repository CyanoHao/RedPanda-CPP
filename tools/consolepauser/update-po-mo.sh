#!/bin/bash

set -e

xgettext --c++ --from-code=UTF-8 --keyword=_ --no-location --omit-header --output=po/unified.pot \
	main.unix.cpp \
	main.windows.cpp

for lang in zh_CN zh_TW
do
	if [[ ! -f po/$lang.po ]]
	then
		msginit --locale=$lang --no-translator --input=po/unified.pot --output=po/$lang.po
	else
		msgmerge --update --sort-output po/$lang.po po/unified.pot
	fi
	msgfmt -o mo/$lang.mo po/$lang.po
done

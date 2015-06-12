#!/bin/sh

while [ true ] ; do
	fukkit=`cat cnr.sh`
	big_rebuild=`cat Makefile *h cnr.sh`
	little_rebuild=`cat *.c`

	if [ "$last_fukkit" = "" ] || [ "$last_fukkit" = "$fukkit" ] ; then
		last_fukkit="$fukkit"
	else
		exec $0
	fi

	if [ "$last_big_rebuild" != "$big_rebuild" ] ; then
		make clean;
		last_big_rebuild="$big_rebuild"
		unset last_little_rebuild
	fi

	if [ "$last_little_rebuild" != "$little_rebuild" ] ; then
		clear
		make
		last_little_rebuild="$little_rebuild"
		date
	fi
	sleep 1
done

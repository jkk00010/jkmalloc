#!/bin/sh
if [ $# -eq 0 ]; then
	printf '%s: missing operands\n' "$0" >&2
	exit 1
fi

LD_PRELOAD=$(dirname $0)/../lib/libjkmalloc.so "$@"

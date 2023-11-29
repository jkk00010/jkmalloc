#!/bin/sh
DIR=$(dirname $0)

printf '\033[0m0-Byte Allocation:\n\033[31m'
${DIR}/jktest-dynamic -z

printf '\033[0mDouble Free:\n\033[31m'
${DIR}/jktest-dynamic -d

printf '\033[0mInvalid Free:\n\033[31m'
${DIR}/jktest-dynamic -f

printf '\033[0mInvalid Reallocation:\n\033[31m'
${DIR}/jktest-dynamic -r

printf '\033[0mOverflow (1 Byte):\n\033[31m'
${DIR}/jktest-dynamic -o 1

printf '\033[0mOverflow (1 Page):\n\033[31m'
${DIR}/jktest-dynamic -o $(getconf PAGE_SIZE)

printf '\033[0mUnderflow (1 Byte):\n\033[31m'
${DIR}/jktest-dynamic -u 1

printf '\033[0mUnderflow (1 Page):\n\033[31m'
${DIR}/jktest-dynamic -u $(getconf PAGE_SIZE)

printf '\033[0mUse Afer Free:\n\033[31m'
${DIR}/jktest-dynamic -a

printf '\033[0mNULL Dereference:\n\033[31m'
${DIR}/jktest-dynamic -n

printf '\033[0m'

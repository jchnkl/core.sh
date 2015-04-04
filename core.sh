#!/bin/bash

if [ -z "${EVAGG}" ]; then
    echo "Please set EVAGG"
    exit 1
elif [ ! -x "$(which ${EVAGG} 2>/dev/null)" ]; then
    echo "Not an executable: ${EVAGG}"
    exit 1
fi

if [ -z "${SRCS_D}" ]; then
    echo "Please set SRCS_D"
    exit 1
elif [ ! -d ${SRCS_D} ]; then
    echo "No such directory: ${SRCS_D}"
    exit 1
fi

if [ -z "${SINKS_D}" ]; then
    echo "Please set SINKS_D"
    exit 1
elif [ ! -d ${SINKS_D} ]; then
    echo "No such directory: ${SINKS_D}"
    exit 1
fi

coproc {
    declare -a srcs
    for src in $(echo ${SRCS_D}/*.sh); do
        if [ -x ${src} ]; then
            srcs+=(${src})
        fi
    done

    ${EVAGG} ${srcs[@]}
}

trap "kill ${COPROC_PID}" EXIT

while read -u ${COPROC[0]}; do
    for sink in $(echo ${SINKS_D}/*.sh); do
        if [ -x ${sink} ]; then
            ${sink} ${REPLY}
        fi
    done
done

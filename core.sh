#!/bin/bash

declare -a events
declare -A handler

eventsSource=${HOME}/.xinitrc.d/events.sh
handlerSource=${HOME}/.xinitrc.d/handler.sh

trap "stopEvents; exit 0" SIGTERM SIGINT
trap "stopEvents; startEvents" SIGUSR1

function startEvents () {
    source ${eventsSource}
    source ${handlerSource}

    if [ ${#events} = 0 ]; then kill $(jobs -p); exit 1; fi

    for e in ${events[@]}; do run $e & done
}

function stopEvents () {
    kill -HUP $(jobs -pr)
}

function run () {
    coproc { eval $1; }

    trap "pkill -P ${COPROC_PID}; exit 0" SIGHUP

    while read -u ${COPROC[0]}; do

        source ${handlerSource}

        for h in ${handler[$1]}; do
            eval $h $REPLY
        done

    done
}

startEvents

while true; do wait; continue; done

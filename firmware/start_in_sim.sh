#!/bin/bash
mcu=${2}
if [ "${2}" == "None" ]; then
mcu="atmega328p"
fi

speed=${3}
if [ "${3}" == "None" ]; then
speed="16000000L"
fi

LD_LIBRARY_PATH=${1}/tool-simavr/lib/ \
${1}/tool-simavr/bin/simavr \
-m \
${mcu} \
-f \
${speed} \
${4}

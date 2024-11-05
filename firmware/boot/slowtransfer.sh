#!/bin/bash

DEVICE=$1
FILE=$2

while IFS= read -r -n1 CHAR; do
    echo -n "${CHAR}" > "${DEVICE}"
    sleep 0.02s
done < "${FILE}"


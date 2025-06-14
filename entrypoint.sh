#!/bin/bash
set -e

# Inizializza solo se non esiste già
if [ ! -d "zephyrproject" ]; then
    echo ">> Initializing Zephyr workspace..."
    west init zephyrproject
    cd zephyrproject
    west update
    west zephyr-export
    pip3 install -r zephyr/scripts/requirements.txt
else
    cd zephyrproject
fi

exec bash
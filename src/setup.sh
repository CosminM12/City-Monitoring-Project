#!/bin/bash

echo "===1. Compiling programs==="

gcc -Wall city_manager.c -o city_manager
gcc -Wall monitor_reports.c -o monitor_reports
gcc -Wall scorer.c -o scorer
gcc -Wall city_hub.c -o city_hub

echo "Compiled successfully"
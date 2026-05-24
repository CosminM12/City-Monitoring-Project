#!/bin/bash

echo "=== 1. Compiling Phase 3 Programs ==="

gcc -Wall city_manager.c -o city_manager
gcc -Wall monitor_reports.c -o monitor_reports
gcc -Wall scorer.c -o scorer
gcc -Wall city_hub.c -o city_hub

echo "Compiled successfully"

echo "=== 2. Populating Districts with Data ==="
# --- DOWNTOWN ---
./city_manager --role manager --user alice --add downtown <<EOF
12.5
21.8
road
2
new pothole reported
EOF

./city_manager --role manager --user alice --add downtown <<EOF
13.1
22.0
flooding
3
major intersection flooded
EOF

# --- UPTOWN ---
./city_manager --role manager --user alice --add uptown <<EOF
10.9
11.2
lighting
1
broken street light
EOF

./city_manager --role inspector --user bob --add uptown <<EOF
11.5
12.1
road
2
cracked pavement
EOF

# --- INDUSTRIAL ---
./city_manager --role inspector --user bob --add industrial <<EOF
88.8
99.9
other
3
chemical spill
EOF

./city_manager --role manager --user charlie --add industrial <<EOF
89.1
98.5
lighting
1
flickering security light
EOF


echo "=== Setup Complete! ==="
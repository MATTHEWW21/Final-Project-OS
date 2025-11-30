#!/bin/bash

OUT="report.html"
BASELINE="/var/log/perf_baseline.txt"

CPU=$(top -bn1 | grep "Cpu(s)" | awk '{print $2 + $4}')
RAM=$(free -m | awk '/Mem:/ {print $3 "/" $2 " MB (" $3/$2*100 "%)"}')
DISK_IO=$(iostat -dx | grep -E 'sda|nvme|vda')
NET=$(ifconfig | grep "RX packets")

echo "<html><body>" > $OUT
echo "<h1>System Performance Report</h1>" >> $OUT
echo "<p><b>Date:</b> $(date)</p>" >> $OUT

echo "<h2>CPU Usage</h2><p>$CPU %</p>" >> $OUT
echo "<h2>RAM Usage</h2><p>$RAM</p>" >> $OUT

echo "<h2>Disk I/O</h2><pre>$DISK_IO</pre>" >> $OUT
echo "<h2>Network Stats</h2><pre>$NET</pre>" >> $OUT

# Baseline comparison
echo "<h2>Baseline Comparison</h2>" >> $OUT

if [ -f "$BASELINE" ]; then
    echo "<pre>$(cat $BASELINE)</pre>" >> $OUT
else
    echo "<p>No baseline file, creating one...</p>" >> $OUT
    echo "CPU=$CPU" > $BASELINE
    echo "RAM=$RAM" >> $BASELINE
    echo "Baseline created." >> $OUT
fi

echo "</body></html>" >> $OUT

echo "Report generated: $OUT"

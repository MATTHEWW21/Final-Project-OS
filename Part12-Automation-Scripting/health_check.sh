#!/bin/bash

LOG="/var/log/storage_health.log"
ALERT_EMAIL="youremail@example.com"

echo "======== HEALTH CHECK ========" >> $LOG
date >> $LOG

# RAID Status
echo "[*] Checking RAID..." | tee -a $LOG
if mdadm --detail --scan >/dev/null 2>&1; then
    mdadm --detail --scan >> $LOG
else
    echo "[ALERT] No RAID arrays found!" | tee -a $LOG
    echo "RAID issue detected" | mail -s "RAID ALERT" $ALERT_EMAIL
fi

# LVM Status
echo "[*] Checking LVM..." | tee -a $LOG
vgdisplay >> $LOG 2>&1
lvdisplay >> $LOG 2>&1

# Filesystem health
echo "[*] Checking Filesystem (read-only fsck)..." | tee -a $LOG
for dev in $(lsblk -lnpo NAME,TYPE | grep disk | awk '{print $1}'); do
    fsck -n $dev >> $LOG 2>&1
done

# Disk Space
echo "[*] Checking disk usage..." | tee -a $LOG
df -h >> $LOG

CRIT=$(df -h | awk '$5+0 > 90 {print $1,$5}')
if [ ! -z "$CRIT" ]; then
    echo "[ALERT] Disk above 90%!" | tee -a $LOG
    echo "Disk usage critical: $CRIT" | mail -s "DISK ALERT" $ALERT_EMAIL
fi

echo "Health check completed." | tee -a $LOG
echo -e "\n" >> $LOG

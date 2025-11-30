#!/bin/bash

SRC="/home"
DEST="/home/backup"
LOG="/var/log/backup.log"

DATE=$(date +%Y-%m-%d)
FULL="$DEST/full_$DATE.tar.gz"
INCR="$DEST/incr_$DATE.tar.gz"
SNAPSHOT="$DEST/snapshot.snar"

mkdir -p $DEST

echo "===== BACKUP START =====" >> $LOG
date >> $LOG

# Full backup (first of the day)
if [ ! -f "$SNAPSHOT" ]; then
    echo "[*] Creating FULL backup..." | tee -a $LOG
    tar --listed-incremental=$SNAPSHOT -czf $FULL $SRC >> $LOG 2>&1
else
    echo "[*] Creating Incremental backup..." | tee -a $LOG
    tar --listed-incremental=$SNAPSHOT -czf $INCR $SRC >> $LOG 2>&1
fi

# Cleanup backups older than 7 days
echo "[*] Cleaning old backups..." | tee -a $LOG
find $DEST -type f -mtime +7 -delete

echo "Backup completed." | tee -a $LOG
echo -e "\n" >> $LOG

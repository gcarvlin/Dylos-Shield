#!/bin/bash
#ifconfig wlan0 up
python "/root/CAU1.py"
cat /dev/null > "/root/temp_data.txt"
#ifconfig wlan0 down
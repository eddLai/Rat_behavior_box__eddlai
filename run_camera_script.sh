#!/bin/bash

# 設定虛擬環境的 Python 路徑
PYTHON_PATH="/home/yylab-win7/桌面/Action Behavior Box_camera_detect/venv/bin/python"
SCRIPT_PATH="/home/yylab-win7/桌面/Action Behavior Box_camera_detect/two_camera_videos_diff_v3_Uart.py"

echo "whether to update the baseline? type 1 for yes, else no"
read choice

if [ "$choice" -eq 1 ]; then
	# 使用 sudo 運行腳本
	echo "update"
	sudo rm -rf ./baseline
	rm -f ./baseline.pkl
	sudo "$PYTHON_PATH" "$SCRIPT_PATH" 1
else
	echo "no update"
	sudo "$PYTHON_PATH" "$SCRIPT_PATH" 0
fi

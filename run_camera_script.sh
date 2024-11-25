#!/bin/bash

# 設定虛擬環境的 Python 路徑
PYTHON_PATH="/home/ntk-win7/Desktop/Rat_behavior_box__eddlai/venv/bin/python"
SCRIPT_PATH="/home/ntk-win7/Desktop/Rat_behavior_box__eddlai/python_to_detect_rat_at_point/two_camera_videos_diff_Uart_blah.py"

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

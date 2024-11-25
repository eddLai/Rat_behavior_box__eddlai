cd ./Rat_behavior_box__eddlai

# create virtual environment
python3 -m venv venv

# enter venv
source Rat_behavior_box__eddlai/venv/bin/activate
pip3 install opencv-python3
pip3 install pyserial

# Set execute permission on your script to run it as a program
chmod +x Rat_behavior_box__eddlai/run_camera_script.sh


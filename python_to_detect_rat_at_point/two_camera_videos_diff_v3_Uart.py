# -*- coding: utf-8 -*-
import cv2
import os
import numpy as np
import serial
import time
import pickle

PORT = 'COM6'
show = True
camera_number1 = 1
camera_number2 = 2
update = 0 #about baseline
#path
now_path = os.getcwd()
baseline_save_path = now_path + "/baseline"
baseline_pkl_save_path = now_path + "/baseline.pkl"
circles_pkl_save_path = "circles_posi.pkl"

# 初始化串口
def send_signal_to_arduino(port, baudrate, message):
    try:
        # 初始化Serial物件
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Connected to {port} at {baudrate} baud")

        # 發送訊號
        ser.write(message.encode())
        print(f"Sent message: {message}")

        # 延遲以確保訊號傳輸完成
        time.sleep(0.02)

        # 關閉Serial通訊
        ser.close()
        print("Connection closed")

    except serial.SerialException as e:
        print(f"Error: {e}")

def take_baseline():
    baseline_save_path = "baseline"
    "======================================="
    baseline_save_path = now_path+r'/{}'.format(baseline_save_path)
    # baseline_save_path= "/home/yylab/Desktop/camera_detect/baseline"


    try:
        os.chdir(baseline_save_path)
        print("資料夾已存在:", baseline_save_path)
    except FileNotFoundError:
        try:
            os.makedirs(baseline_save_path)
            os.chdir(baseline_save_path)
            print("成功創建資料夾:", baseline_save_path)
        except Exception as e:
            print("無法創建或訪問資料夾:", e)
            
    checkpoint_x_1_cam1 = 400
    checkpoint_y_1_cam1 = 0
    checkpoint_x_1_cam2 = 400
    checkpoint_y_1_cam2 = cut_stop

    cap1 = cv2.VideoCapture(camera_number1)
    print("0號相機啟動")
    cap2 = cv2.VideoCapture(camera_number2)
    print("1號相機啟動")

    now = 0
    while now < 30:
        ret0, frame1 = cap1.read()
        ret1, frame2 = cap2.read()
        now = now + 1

    now = 0
    while now < 120:
        ret0, frame1 = cap1.read()
        ret1, frame2 = cap2.read()
        
        combine_videos = np.concatenate((frame2[cut_start:cut_stop,:,:], frame1), axis=0)
        cv2.imwrite(baseline_save_path + r"/" + str(now) + ".jpg", combine_videos)
        cv2.imshow('combine', combine_videos)
        now = now + 1
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        
    cap1.release()
    cap2.release()
    cv2.destroyAllWindows()

def get_mouse_coord(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        print("at ({},{})".format(x,y))
        mouse_x, mouse_y = x, y
        circle_pos_list[index][1] = mouse_x
        circle_pos_list[index][2] = mouse_y
        
def printCircles(frame, circle_list):
    cv2.circle(frame, (circle_list[0][1], circle_list[0][2]), 25, (0, 0, 255), -1)
    cv2.circle(frame, (circle_list[1][1], circle_list[1][2]), 15, (255, 0, 0), -1)
    cv2.circle(frame, (circle_list[2][1], circle_list[2][2]), 15, (0, 255, 0), -1)
    cv2.circle(frame, (circle_list[3][1], circle_list[3][2]), 15, (0, 0, 0), -1)
    cv2.circle(frame, (circle_list[4][1], circle_list[4][2]), 15, (0, 0, 0), -1)
    cv2.circle(frame, (circle_list[5][1], circle_list[5][2]), 15, (0, 0, 0), -1)

def pin_output_ifs(x, y, distance_from_target):
    if np.abs(x - circle_pos_list[0][1]) < distance_from_target and np.abs(y - circle_pos_list[0][2]) < distance_from_target:
        send_signal_to_arduino(PORT, 115200, 'M')
        print("Middle cue")
            
    elif np.abs(x - circle_pos_list[1][1]) < distance_from_target and np.abs(y - circle_pos_list[1][2]) < distance_from_target:
        send_signal_to_arduino(PORT, 115200, 'L')
        print("Left cue")
    
    elif np.abs(x - circle_pos_list[2][1]) < distance_from_target and np.abs(y -circle_pos_list[2][2]) < distance_from_target:
        send_signal_to_arduino(PORT, 115200, 'R')
        print("Right cue")
    elif np.abs(x - circle_pos_list[5][1]) < distance_from_target and np.abs(y - circle_pos_list[5][2]) < distance_from_target:
        send_signal_to_arduino(PORT, 115200, 'D')
        print("RD")
    
    elif np.abs(x - circle_pos_list[4][1]) < distance_from_target and np.abs(y - circle_pos_list[4][2]) < distance_from_target:
        send_signal_to_arduino(PORT, 115200, 'D')
        print("LD")
            
    elif np.abs(x - circle_pos_list[3][1]) < distance_from_target and np.abs(y - circle_pos_list[3][2]) < distance_from_target:
        send_signal_to_arduino(PORT, 115200, 'MD')
        print("MD")
    else:
        pass
        
def cal_baseline(file_path):
    baseline_file = os.listdir(file_path)
    combine_baseline = np.array(np.zeros((480 + cut_stop, 640, 3)))
    count = 0
    print("計算baseline平均")
    for i in baseline_file:
        print(file_path + "/" + i)
        img = cv2.imread(file_path + "/" + i)
        combine_baseline = combine_baseline + img
        count = count + 1

    baseline_avg = np.array((combine_baseline/count), dtype=np.uint8)
    baseline_img_gray = cv2.cvtColor(baseline_avg, cv2.COLOR_BGR2GRAY)
    return baseline_img_gray

"========================================="

global circle_pos_list
try:
    with open(circles_pkl_save_path, "rb") as file:
        circle_pos_list = pickle.load(file)
except FileNotFoundError:
    print("no recoreded circles' positions, use default setting")
    target_M_x = 50 
    target_M_y = 50
    target_L_x = 100
    target_L_y = 100
    target_R_x = 150
    target_R_y = 150
    target_MD_x = 200
    target_MD_y = 200
    target_LD_x = 250
    target_LD_y = 250
    target_RD_x = 300
    target_RD_y = 300
    circle_pos_list = [["M", target_M_x, target_M_y], ["L", target_L_x, target_L_y], ["R", target_R_x, target_R_y], ["MD", target_MD_x, target_MD_y], ["LD", target_LD_x, target_LD_y], ["RD", target_RD_x, target_RD_y]]

cut_start = 0
cut_stop = 435

try:
    with open(baseline_pkl_save_path, "rb") as file:
        print(baseline_pkl_save_path)
        baseline_img_gray = pickle.load(file)
        exist = True
except FileNotFoundError:
    exist = False
    
if not exist or update:
    with open(baseline_pkl_save_path, "wb") as file:
        take_baseline()
        print(baseline_save_path)
        baseline_img_gray = cal_baseline(baseline_save_path)
        pickle.dump(baseline_img_gray, file)
        print("new baseline saved")
else:
    print("old baseline loaded")

kernel = np.ones((7, 7), np.uint8)

cap1 = cv2.VideoCapture(camera_number1)
print("0號相機啟動")
cap2 = cv2.VideoCapture(camera_number2)
print("1號相機啟動")

cv2.namedWindow("combine", cv2.WINDOW_NORMAL)
cv2.resizeWindow("combine", 500, 700)
cv2.setMouseCallback("combine", get_mouse_coord)
now = 0
while now < 30:
    ret0, frame1 = cap1.read()
    ret1, frame2 = cap2.read()
    now = now + 1

index=0
while True:
    ret0, frame1 = cap1.read()
    ret1, frame2 = cap2.read()
    
    combine_videos = np.concatenate((frame2[cut_start:cut_stop,:,:], frame1), axis=0)
    video_without_circles = np.copy(combine_videos)
    [cv2.circle(video_without_circles, (circle[1], circle[2]), 0, (0, 0, 0), -1) for circle in circle_pos_list]
    cv2.imshow('thresh', video_without_circles)
    
    printCircles(combine_videos, circle_pos_list)
    
    img_gray = cv2.cvtColor(video_without_circles, cv2.COLOR_BGR2GRAY)
    result = cv2.absdiff(baseline_img_gray, img_gray)
    erosion = cv2.erode(result, kernel, iterations = 3)
    ret, thresh = cv2.threshold(erosion, 50, 255, 0)
    ind = np.where(thresh == 255)
    if ind[0].size == 0 or ind[1].size == 0:
        cv2.putText(combine_videos, f"Can not detect any item", (200, 400), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
    else:
        x = int(np.mean(ind[1]))
        y = int(np.mean(ind[0]))
        cv2.circle(combine_videos, (x, y), 25, (0, 255, 255), 3)
        pin_output_ifs(x, y, 40)
    
    if show:
        circle_name = circle_pos_list[index][0]
        cv2.putText(combine_videos, f"Selected Color: {circle_name}", (20, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255,255,255), 2)
        cv2.imshow('combine', combine_videos)
        cv2.imshow('thresh', thresh)
    
    key = cv2.waitKey(1)
    if key & 0xFF == ord('q'):
        break
    elif key == ord("a"):
        index = (index + 1) % len(circle_pos_list)
    elif key == ord("d"):
        index = (index - 1) % len(circle_pos_list)
    
with open(circles_pkl_save_path, "wb") as file:
    print(circle_pos_list)
    pickle.dump(circle_pos_list, file)
    print("circles positions saved")

cv2.destroyAllWindows()

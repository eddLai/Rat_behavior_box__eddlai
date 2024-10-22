# -*- coding: utf-8 -*-
"""
Created on Tue Aug 16 16:25:13 2022

@author: Skull
"""

import cv2
import os
import numpy as np
import RPi.GPIO as GPIO

def get_mouse_coord(event, x, y, flags, param):
    if event == cv2.EVENT_LBUTTONDOWN:
        print("at ({},{})".format(x,y))

now_path = os.getcwd()
"========================================="
"camera"
camera_number1 = 0
camera_number2 = 2

"distance from target"
distance_from_target = 40

"target area water" #red
target_water_x = 10
target_water_y = 10

"target area LED1" #blue
target_LED1_x = 100
target_LED1_y = 100

"target area LED2" #green
target_LED2_x = 200
target_LED2_y = 200

"camera2 cut off"
cut_start = 0
cut_stop = 435

"baseline save path"
baseline_save_path = "baseline"

"show or not"
show = True
"========================================="


baseline_save_path = now_path + "/" + baseline_save_path

baseline_file = os.listdir(baseline_save_path)

combine_baseline = np.array(np.zeros((480 + cut_stop, 640, 3)))
count = 0

print("計算baseline平均")
for i in baseline_file:
    img = cv2.imread(baseline_save_path + "/" + i)
    combine_baseline = combine_baseline + img
    count = count + 1

baseline_avg = np.array((combine_baseline/count), dtype=np.uint8)
baseline_img_gray = cv2.cvtColor(baseline_avg, cv2.COLOR_BGR2GRAY)

# 侵蝕核心大小
kernel = np.ones((7, 7), np.uint8)

cap1 = cv2.VideoCapture(camera_number1)
print("0號相機啟動")
cap2 = cv2.VideoCapture(camera_number2)
print("1號相機啟動")

cv2.namedWindow("combine")
cv2.setMouseCallback("combine", get_mouse_coord)
# 光源變化影響紀錄品質，讓相機先啟動30個frame，讓光圈自動調節
now = 0
while now < 30:
    ret0, frame1 = cap1.read()
    ret1, frame2 = cap2.read()
    now = now + 1

# 開始偵測

while True:
    ret0, frame1 = cap1.read()
    ret1, frame2 = cap2.read()
    
    #frame2 = cv2.flip(frame2, 0)
    #frame2 = cv2.flip(frame2, 1)
    
    combine_videos = np.concatenate((frame2[cut_start:cut_stop,:,:], frame1), axis=0)
    
    cv2.circle(combine_videos, (target_water_x, target_water_y), 15, (0, 0, 255), -1)
    cv2.circle(combine_videos, (target_LED1_x, target_LED1_y), 15, (255, 0, 0), -1)
    cv2.circle(combine_videos, (target_LED2_x, target_LED2_y), 15, (0, 255, 0), -1)
    
    img_gray = cv2.cvtColor(combine_videos, cv2.COLOR_BGR2GRAY)
    result = cv2.absdiff(baseline_img_gray, img_gray)
    erosion = cv2.erode(result, kernel, iterations = 3)
    ret, thresh = cv2.threshold(erosion, 50, 255, 0)
    
    ind = np.where(thresh==255)
    
    if len(ind[0]) > 0:
        x = int(np.mean(ind[1]))
        y = int(np.mean(ind[0]))
        cv2.circle(combine_videos, (x, y), 25, (0, 255, 255), 3)
        
        if np.abs(x - target_water_x) < distance_from_target:
            if np.abs(y - target_water_y) < distance_from_target:
                print("給水")
                
        elif np.abs(x - target_LED1_x) < distance_from_target:
            if np.abs(y - target_LED1_y) < distance_from_target:
                print("LED1")
                
        elif np.abs(x - target_LED2_x) < distance_from_target:
            if np.abs(y -target_LED2_y ) < distance_from_target:
                print("LED2")
        
        else:
            print("未經過目標區域")
        
    else:
        print("未檢測到物體")
    
    
    if show:
        cv2.imshow('combine', combine_videos)
        cv2.imshow('thresh', thresh)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

cv2.destroyAllWindows()

#
#now = 0
#
#for i in range(1, len(videos_file_cam1) + 1):
#    img1 = cv2.imread(videos_path + "\\" + "cam1" + "\\" + str(i) + ".jpg")
#    img2 = cv2.imread(videos_path + "\\" + "cam2" + "\\" + str(i) + ".jpg")
#    
#    img2 = cv2.flip(img2, 1)
#    
#    combine_videos = np.concatenate((img2[cut_start:cut_stop,:,:], img1), axis=0)
#    
#    img_gray = cv2.cvtColor(combine_videos, cv2.COLOR_BGR2GRAY)
#    result = cv2.absdiff(baseline_img_gray, img_gray)
#    
#    erosion = cv2.erode(result, kernel, iterations = 3)
#    
#    ret, thresh = cv2.threshold(erosion, 50, 255, 0)
#    
#    cv2.circle(combine_videos, (target_x, target_y), 10, (255, 0, 255), -1)
#    
#    ind = np.where(thresh==255)
#    if len(ind[0]) > 0:
#        x = int(np.mean(ind[1]))
#        y = int(np.mean(ind[0]))
#        cv2.circle(combine_videos, (x, y), 25, (0, 255, 255), 3)
#        if np.abs(x - target_x) < 40:
#            if np.abs(y - target_y) < 40:
#                print("給水")
#            else:
#                print("尚未經過目標區域")
#        else:
#            print("尚未經過目標區域")
#    else:
#        print("未檢測到物體")
#    cv2.namedWindow('video', cv2.WINDOW_NORMAL)
#    cv2.imshow('video', thresh)
#    if cv2.waitKey(1) & 0xFF == ord('q'):
#        break
#    now = now + 1
#
#cv2.destroyAllWindows()


#baseline_img1 = cv2.imread(baseline_path + "\\" + "baseline1" + "\\" + baseline_file_cam1[0])
#baseline_img2 = cv2.imread(baseline_path + "\\" + "baseline2" + "\\" + baseline_file_cam2[0])
#
#videos_img1 = cv2.imread(videos_path + "\\" + "cam1" + "\\" + videos_file_cam1[283])
#videos_img2 = cv2.imread(videos_path + "\\" + "cam2" + "\\" + videos_file_cam2[283])
#
#videos_img2 = cv2.flip(videos_img2, 1)
#
#cv2.imshow("baseline1", videos_img1)
#cv2.imshow("baseline2", videos_img2)
#
#combine_baseline = np.array(np.zeros((480*2, 640, 3)))
#combine_videos = np.array(np.zeros((480*2, 640, 3)))
#
#combine_videos = np.concatenate((videos_img2[cut_start:cut_stop,:,:], videos_img1), axis=0)
#
#cv2.imshow("combine", combine_videos)

#cv2.waitKey(0)
#cv2.destroyAllWindows()







#baseline_file = os.listdir(baseline_path)
#videos_file = os.listdir(videos_path)


#kernel = np.ones((3,3), np.uint8)
#
#baseline_avg = np.zeros((360, 640, 3))
#count = 0
#for i in baseline_file:
#    img = cv2.imread(baseline_path + "\\" + i)
#    baseline_avg = baseline_avg + img
#    count = count + 1
#    
#baseline_avg = np.array((baseline_avg/count), dtype=np.uint8)
#
#baseline_img_gray = cv2.cvtColor(baseline_avg, cv2.COLOR_BGR2GRAY)
#
#
#now = 0
#
#
#
#while(True):
#    frame , img = cap.read()
#    if frame == False:
#        break
#    if now > 0:
#        img_gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
#        
#        result = cv2.absdiff(baseline_img_gray, img_gray)
#        
#        erosion = cv2.erode(result, kernel, iterations = 3)
#        
#        ret, thresh = cv2.threshold(erosion, 50, 255, 0)
#        
#        cv2.circle(img, (target_x, target_y), 10, (255, 0, 255), -1)
#        
#        ind = np.where(thresh==255)
#        if len(ind[0]) > 0:
#            x = int(np.mean(ind[1]))
#            y = int(np.mean(ind[0]))
#            cv2.circle(img, (x, y), 25, (0, 255, 255), 3)
#            if np.abs(x - target_x) < 40:
#                if np.abs(y - target_y) < 40:
#                    print("給水")
#                else:
#                    print("尚未經過目標區域")
#            else:
#                print("尚未經過目標區域")
#        else:
#            print("未檢測到物體")
#        cv2.namedWindow('video', cv2.WINDOW_NORMAL)
#        cv2.imshow('video', img)
#        if cv2.waitKey(30) & 0xFF == ord('q'):
#            break
#    now = now + 1
#cap.release()
#cv2.destroyAllWindows()

















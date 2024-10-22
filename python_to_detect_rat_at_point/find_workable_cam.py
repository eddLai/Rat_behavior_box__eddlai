# lsusb, guvcview
import cv2

def list_available_cameras():
    camera_indices = []
    
    for index in range(10):  # 假設攝像頭索引範圍在 0 到 9 之間
        cap = cv2.VideoCapture(index)
        if cap.isOpened():
            print(f"Camera at index {index} is available")
            camera_indices.append(index)
            cap.release()
    
    return camera_indices

if __name__ == "__main__":
    available_cameras = list_available_cameras()
    if len(available_cameras) == 0:
        print("No available cameras found.")
    else:
        print("Available camera indices:", available_cameras)

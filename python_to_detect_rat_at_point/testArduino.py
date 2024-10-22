#  Use WiringPi command in cmd, E.G. GPIO readall
import RPi.GPIO as GPIO
import time

# 设置 GPIO 引脚编号模式为 BCM 模式
GPIO.setmode(GPIO.BCM)

# 配置输出引脚
output_pin = 27
GPIO.setup(output_pin, GPIO.OUT, initial= GPIO.LOW)

try:
    while True:
        # 切换输出引脚状态
        current_state = int(input())
        print(f"Input state: {current_state}")
        
        if current_state == 1:
            temp = GPIO.output(output_pin, GPIO.HIGH)
            print("changed to HIGH")
        else:
            GPIO.output(output_pin, GPIO.LOW)

except KeyboardInterrupt:
    print("程序被用户中断.")

finally:
    # 清理资源
    GPIO.cleanup()


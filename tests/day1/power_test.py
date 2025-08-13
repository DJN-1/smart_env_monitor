#!/usr/bin/env python3
import subprocess
import time

def get_cpu_temp():
    try:
        temp = subprocess.check_output(['vcgencmd', 'measure_temp'])
        return temp.decode().strip()
    except:
        return "측정 불가"

def get_voltage():
    try:
        voltage = subprocess.check_output(['vcgencmd', 'measure_volts', 'core'])
        return voltage.decode().strip()
    except:
        return "측정 불가"

print("전원 안정성 모니터링 (30초간)")
for i in range(6):
    temp = get_cpu_temp()
    voltage = get_voltage()
    print(f"{i*5:2d}초: {temp}, {voltage}")
    time.sleep(5)

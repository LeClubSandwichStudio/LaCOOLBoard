#!/usr/bin/env python3
import serial
import time
import subprocess
import threading
import thread
import sys
import datetime
import RPi.GPIO as GPIO

#serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10) # MAC
serialBus = serial.Serial('/dev/ttyUSB0', 115200, timeout=10) # Raspberry pi 3 B+

timestampValue = datetime.datetime.now().strftime("%y-%m-%d")
listPinJetpack = [10,22,17,27,18,24,23,25]
pinPWM = 13
GPIO.setmode(GPIO.BCM)
GPIO.setup(pinPWM, GPIO.OUT)
levelBattery = GPIO.PWM(pinPWM, 10000)

def quit(fn_name):
    print('{0} took too long'.format(fn_name), file==sys.stderr)
    sys.stderr.flush()
    thread.interrupt_main()


def exit_after(s):
    def outer(fn):
        def inner(*args, **kwargs):
            timer = threading.Timer(s, quit, args=[fn.__name__])
            timer.start()
            try:
                result = fn(*args, **kwargs)
            finally:
                timer.cancel()
            return result
        return inner
    return outer


def step_failed():
    p = subprocess.call(["behave", "-f", "plain", "-T", " ------------------    FAILED"])


def hotspotActivated():
    p = subprocess.Popen(["sudo", "service", "hostapd", "status"], stdout=subprocess.PIPE)
    (hostapd_status, err) = p.communicate()
    if hostapd_status.find("running") == -1:
        subprocess.call(["sudo", "service", "hostapd", "start"])


def hotspotDesactivated():
    p = subprocess.Popen(["sudo", "service", "hostapd", "status"], stdout=subprocess.PIPE)
    (hostapd_status, err) = p.communicate()
    if hostapd_status.find("running") != -1:
        subprocess.call(["sudo", "service", "hostapd", "stop"])


def wifiRegistered():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:57] == "INFO:    Wifi count  = "):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def wifiRetry():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:58] == "WARN:  Wifi status: idle"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def saveFileSpiffs():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:69] == "DEBUG: Saved file data name:  /log/"
                or lineReturned[33:68] == "DEBUG: Saved file data name:  /log/"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def messageSend():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:66] == "INFO:  Sending saved log number:"
                or lineReturned[33:65] == "INFO:  Sending saved log number:"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def updateOTA():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:64] == "INFO:  MQTT publish successful"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def updateOTAFailed():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:80] == "ERROR: MQTT publish failed, kept log on SPIFFS"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def updateOTAWrongLink():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:66] == "ERROR: HTTP Update failed, code:"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def updateJsonFile():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(line)
            if (lineReturned[33:60] == "DEBUG: Saved JSON config to" 
                or lineReturned[34:61] == "DEBUG: Saved JSON config to"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def firmwareUpdate():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:61] == "INFO:  Firmware version is:"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def wifiConnectionSucceed():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(line)
            if (lineReturned[34:63] == "INFO:  Wifi status: connected"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def wifiConnectionFailed():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(line)
            if (lineReturned[34:60] == "WARN:  Network unreachable"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def endLoop():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:60] == "INFO:  Going to sleep for:"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def batteryLowLevel():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:62] == "WARN:  Battery Power is low!"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def sleepAgain():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[34:68] == "INFO:  And need to sleep again for"):
                print(lineReturned[34:])
                return False
            else:
                print(line)


def changeLevelBattery(dutyCycle):
    levelBattery.start(dutyCycle)
    time.sleep(20)


def cleanPWM():
    levelBattery.stop()
    GPIO.cleanup()


def timestamp():
    if serialBus.isOpen():
        while (True):
            line = serialBus.readline()
            lineReturned = str(serialBus.readline())
            if (lineReturned[1:11] == "20" + timestampValue):
                print(lineReturned[0:])
                return False
            else:
                print(lineReturned[1:11])


def actuator_1_activated():
    if (GPIO.input(listPinJetpack[0]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_2_activated():
    if (GPIO.input(listPinJetpack[1]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_3_activated():
    if (GPIO.input(listPinJetpack[2]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_4_activated():
    if (GPIO.input(listPinJetpack[3]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_5_activated():
    if (GPIO.input(listPinJetpack[4]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_6_activated():
    if (GPIO.input(listPinJetpack[5]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_7_activated():
    if (GPIO.input(listPinJetpack[6]) == 1):
        return False
    else:
        return KeyboardInterrupt


def actuator_8_activated():
    if (GPIO.input(listPinJetpack[7]) == 1):
        return False
    else:
        return KeyboardInterrupt
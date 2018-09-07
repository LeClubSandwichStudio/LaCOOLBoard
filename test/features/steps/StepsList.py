#!/usr/bin/env python3
import behave
import StepsFunctions
import serial
import SerialFunctions
import AwsMessage
import time
import boto3
import subprocess
import Jetpack

#serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10) # MAC
serialBus = serial.Serial('/dev/ttyUSB0', 115200, timeout=10) # Raspberry pi 3 B+

# ----------- GIVEN -----------
@given('the CoolBoard is ON')
def step_impl(context):
    if(serialBus):
        pass

@given('the CoolBoard is connected to a WiFi network')
def step_impl(context):
    StepsFunctions.wifiConnectionSucceed()

@given('I reset the CoolBoard')
def step_impl(context):
    SerialFunctions.resetBoard()

@given('I initialize the JetPack')
def step_impl(context):
    Jetpack.initJetpack()

@given('I activate the JetPack')
def step_impl(context):
    AwsMessage.sendMessageJetPack()

# ----------- WHEN -----------
@when('I reset the CoolBoard')
def step_impl(context):
    SerialFunctions.resetBoard()

@when('it has a WiFi registered')
def step_impl(context):
    StepsFunctions.wifiRegistered()

@when('the firmware is updated by OTA')
def step_impl(context):
    while (AwsMessage.sendMessageFirmwareVersion()):
        StepsFunctions.updateOTA()

@when('the firmware is updated by OTA but failed')
def step_impl(context):
    while (AwsMessage.sendMessageFirmwareVersion()):
        StepsFunctions.updateOTAFailed()

@when('the firmware is updated by OTA with a wrong link')
def step_impl(context):
    while (AwsMessage.sendMessageWrongFirmware()):
        StepsFunctions.updateOTAWrongLink()

@when('the loginterval is updated by OTA (less than 3600)')
def step_impl(context):
    while (AwsMessage.sendMessageLogIntervalLower()):
        StepsFunctions.updateOTA()

@when('the logInterval is updated by OTA (more than 3600)')
def step_impl(context):
    while (AwsMessage.sendMessageLogIntervalHigher()):
        StepsFunctions.updateOTA()

@when('it does not connect to the Wifi')
def step_impl(context):
    StepsFunctions.hotspotDesactivated()
    StepsFunctions.wifiConnectionFailed()

@when('it connects to the WiFi')
def step_impl(context):
    StepsFunctions.hotspotActivated()
    StepsFunctions.wifiConnectionSucceed()

@when('I choose the state of the actuators') 
def step_impl(context):
    AwsMessage.sendMessageAct0()
    AwsMessage.sendMessageAct1()
    AwsMessage.sendMessageAct2()
    AwsMessage.sendMessageAct3()
    AwsMessage.sendMessageAct4()
    AwsMessage.sendMessageAct5()
    AwsMessage.sendMessageAct6()
    AwsMessage.sendMessageAct7()
    time.sleep(60)

@when('the battery of the CoolBoard reaches a low level')
def step_impl(context):
    StepsFunctions.changeLevelBattery(50)
    time.sleep(20)
    SerialFunctions.resetBoard()
    StepsFunctions.batteryLowLevel()

@when('I replug the CoolBoard on DC')
def step_impl(context):
    time.sleep(20)
    StepsFunctions.changeLevelBattery(100)
    SerialFunctions.resetBoard()
    StepsFunctions.cleanPWM()

# ----------- THEN -----------
@then('it connects to the WiFi')
@StepsFunctions.exit_after(60)
def step_impl(context):
    StepsFunctions.hotspotActivated()
    try:
        StepsFunctions.wifiConnectionSucceed()
    except KeyboardInterrupt:
        StepsFunctions.step_failed()

@then('a message is saved as JSON on SPIFFS')
@StepsFunctions.exit_after(50)
def step_impl(context):
    try:
        StepsFunctions.saveFileSpiffs()
    except KeyboardInterrupt:
        StepsFunctions.step_failed()

@then('the json file is changed')
def step_impl(context):
    StepsFunctions.updateJsonFile()

@then('it tells me the new version of the firmware')
@StepsFunctions.exit_after(190)
def step_impl(context):
    try:
        time.sleep(130)
        SerialFunctions.resetBoard()
        if (SerialFunctions.resetBoard()):
            StepsFunctions.firmwareUpdate()
    except KeyboardInterrupt:
        StepsFunctions.step_failed()

@then('the CoolBoard is working normally')
def step_impl(context):
    StepsFunctions.endLoop()

@then('it retries several time to connect to the Wifi')
def step_impl(context): 
    StepsFunctions.wifiRetry()

@then('it needs to sleep again')
def step_impl(context): 
    StepsFunctions.sleepAgain()

@then ('I wait for the synchro')
def step_impl(context): 
    time.sleep(60)

@then('messages saved on SPIFFS are sended')
@StepsFunctions.exit_after(60)
def step_impl(context):
    try:
        StepsFunctions.messageSend()
    except KeyboardInterrupt:
        StepsFunctions.step_failed()

@then('the timestamp is true')
@StepsFunctions.exit_after(60)
def step_impl(context):
    try:
        StepsFunctions.timestamp()
    except KeyboardInterrupt:
        StepsFunctions.step_failed()
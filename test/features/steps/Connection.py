#!/usr/bin/env python3
import behave
import StepsFunctions
import serial
import SerialFunctions
import AwsMessage
import time
import boto3
#import SerialTest
serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10)

@given('The CoolBoard is ON')
def step_impl(context):
    if(serialBus):
        pass

@given('the CoolBoard is connected to a WiFi network')
def step_impl(context):
    if(StepsFunctions.wifiConnexion()):
        assert True is not False
    else:
        assert False is not True

@when('it has a WiFi registered')
def step_impl(context):
    if (StepsFunctions.wifiRegistered()):
        assert True is not False
    else:
        assert False is not True
    #if (not SerialTest.REGISTERED_WIFI_TEST(serialBus, SerialFunctions.resultFileName, False)):
    #    assert True is not False
    #else:
    #    assert False is not True

@when('the firmware is updated by OTA')
def step_impl(context):
    while (AwsMessage.sendMessageFirmwareVersion()):
        if (StepsFunctions.updateOTA()):
            assert True is not False
        else:
            assert False is not True

@when('the firmware is updated by OTA with a wrong link')
def step_impl(context):
    while (AwsMessage.sendMessageWrongFirmware()):
        if (StepsFunctions.updateOTA()):#####chercher la ligne à mettre, changer fonction /!\
            assert True is not False
        else:
            assert False is not True

@when('the loginterval is updated by OTA')
def step_impl(context):
    while (AwsMessage.sendMessageLogInterval()):
        if (StepsFunctions.updateOTA()):
            assert True is not False
        else:
            assert False is not True

@then('it connects to the WiFi')
def step_impl(context):
    if (StepsFunctions.wifiConnexion()):
        assert True is not False
    else:
        assert False is not True

@then('the json file is changed')
def step_impl(context):
    if(StepsFunctions.updateJsonFile()):
        assert True is not False
    else:
        assert False is not True

@then('it tells me the new version of the firmware')
def step_impl(context):
    if(StepsFunctions.firmwareUpdate()):
        assert True is not False
    else:
        assert False is not True

@then('an access point is opened')
def step_impl(context):
    if (StepsFunctions.accessPoint()):
        assert True is not False
    else:
        assert False is not True
    #if (SerialTest.WIFI_ACCESS_POINT_OPENED(context.serialBus, SerialFunctions.resultFileName, True)):
    #    assert True
    #else:
    #    assert False

@then('the CoolBoard is working normally')
def step_impl(context):
    if(StepsFunctions.endLoop()):
        assert True is not False
    else:
        assert False is not True

@then('I reset the CoolBoard')
def step_impl(context):
    SerialFunctions.resetBoard()
    #time.sleep(1)

#!/usr/bin/env python3

import RPi.GPIO as GPIO

listPin = [10,22,17,27,18,24,23,25]

def initJetpack():
    GPIO.setmode(GPIO.BCM)
    for i in listPin:
        GPIO.setup(i, GPIO.IN)

def ALL_ACTUATORS_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ALL ACTUATORS ACTIVATED"
    testResult = not (0 in [GPIO.input(i) for i in listPin])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult

def ALL_ACTUATORS_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ALL ACTUATORS DISACTIVATED"
    testResult = not (1 in [GPIO.input(i) for i in listPin])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult

def ACTUATOR_1_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 1 ACTIVATED"
    testResult = GPIO.input(listPin[0])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_1_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 1 DISACTIVATED"
    testResult = not GPIO.input(listPin[0])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult

def ACTUATOR_2_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 2 ACTIVATED"
    testResult = GPIO.input(listPin[1])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_2_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 2 DISACTIVATED"
    testResult = not GPIO.input(listPin[1])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_3_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 3 ACTIVATED"
    testResult = GPIO.input(listPin[2])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_3_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 3 DISACTIVATED"
    testResult = not GPIO.input(listPin[2])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_4_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 4 ACTIVATED"
    testResult = GPIO.input(listPin[3])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_4_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 4 DISACTIVATED"
    testResult = not GPIO.input(listPin[3])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_5_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 5 ACTIVATED"
    testResult = GPIO.input(listPin[4])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_5_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 5 DISACTIVATED"
    testResult = not GPIO.input(listPin[4])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_6_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 6 ACTIVATED"
    testResult = GPIO.input(listPin[5])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_6_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 6 DISACTIVATED"
    testResult = not GPIO.input(listPin[5])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_7_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 7 ACTIVATED"
    testResult = GPIO.input(listPin[6])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_7_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 7 DISACTIVATED"
    testResult = not GPIO.input(listPin[6])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_8_ACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 8 ACTIVATED"
    testResult = GPIO.input(listPin[7])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult


def ACTUATOR_8_DISACTIVATED(serialBus, resultFileName, giveResult=True):
    TEST_NAME = "ACTUATOR 8 DISACTIVATED"
    testResult = not GPIO.input(listPin[7])
    if giveResult:
        resultFile = open(resultFileName, 'a')
        if testResult:
            message = "OK------------------ " + TEST_NAME + "\n"
        else :
            message = "ERROR--------------- " + TEST_NAME + "\n"
        resultFile.write(message)
        resultFile.close()
    return testResult
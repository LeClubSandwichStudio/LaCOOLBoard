#!/usr/bin/env python3
import json
import boto3

macAddress = '2C3AE84FBF4F'
valueLogInterval = 230
Act0 = True
Act1 = True
linkFirmware = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/feature/test-bench/debug-v0.2.7-14-gdc7c463.bin"
versionFirmware = "v0.2.7-14-gdc7c463"

logInterval = {"state":
                {"desired":
                    {"CoolBoard":
                        {"logInterval": valueLogInterval
                        }
                    }
                }
            }
jsonLogInterval = json.dumps(logInterval)

actuator_0 = {"state":
                {"desired":
                    {"Act0": Act0
                    }
                }
            }
jsonActuator_0 = json.dumps(actuator_0)

actuator_1 = {"state":
                {"desired":
                    {"Act1": Act1
                    }
                }
            }
jsonActuator_1 = json.dumps(actuator_1)

firmware = {"state":
                {"desired":
                    {"CoolBoard":
                        {"firmwareUpdate":
                            {"firmwareUrlFingerprint": "BC5445C5CE60988076FFFE5C83555949810370A1",
                            "firmwareUrl": linkFirmware,
                            "firmwareVersion": versionFirmware
                            }
                        }
                    }
                }
            }
jsonFirmware = json.dumps(firmware)

client = boto3.client('iot-data', region_name='eu-west-1')

def printjson(jsonmessage):
    jsonmessage.split()
    nbTab = 0
    printableMessage= ""
    for i in jsonmessage:
        if(i not in ['{', '}', ',']):
            printableMessage += i
        elif ("}" == i):
            nbTab = nbTab - 1
            printableMessage += i + "\n" + "  " * nbTab
        elif("{" == i):
            nbTab = nbTab + 1
            printableMessage += i
        else:
            printableMessage += i + "\n" + "  " * nbTab
    print(printableMessage)


def sendMessageLogInterval():
    global jsonLogInterval
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonLogInterval
    )


def sendMessageAct0():
    global jsonActuator_0
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_0
    )


def sendMessageAct1():
    global jsonActuator_1
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_1
    )


def sendMessageFirmwareVersion():
    global jsonFirmware
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonFirmware
        )


def getShadow():
    global client
    response = client.get_thing_shadow(thingName=macAddress)
    streamingBody = response["payload"]
    rawDataBytes = streamingBody.read()
    rawDataString = rawDataBytes.decode('utf-8')
    jsonState = json.loads(rawDataString)
    return(jsonState)

# sendMessageLogInterval()
# sendMessageAct0()
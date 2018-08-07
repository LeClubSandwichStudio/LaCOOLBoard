#!/usr/bin/env python3
import json
import boto3

macAddress = '2C3AE84FBF4F'
valueLogInterval = 300
Act0 = True
Act1 = True
#linkFirmware_1 = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/feature/test-bench/debug-v0.2.7-14-gdc7c463.bin"
linkFirmware_1 = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/feature/test-bench/debug-v0.2.7-14-g07e5da3.bin"
#versionFirmware_1 = "v0.2.7-14-gdc7c463"
versionFirmware_1 = "v0.2.7-14-g07e5da3"

linkFirmware_2 = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/feature/test-bench/debug-v0.2.7-14-gdc7c464.bin"
versionFirmware_2 = "v0.2.7-14-gdc7c463"

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

firmware_1 = {"state":
                {"desired":
                    {"CoolBoard":
                        {"firmwareUpdate":
                            {"firmwareUrlFingerprint": "BC5445C5CE60988076FFFE5C83555949810370A1",
                            "firmwareUrl": linkFirmware_1,
                            "firmwareVersion": versionFirmware_1
                            }
                        }
                    }
                }
            }
jsonFirmware_1 = json.dumps(firmware_1)

firmware_2 = {"state":
                {"desired":
                    {"CoolBoard":
                        {"firmwareUpdate":
                            {"firmwareUrlFingerprint": "BC5445C5CE60988076FFFE5C83555949810370A1",
                            "firmwareUrl": linkFirmware_2,
                            "firmwareVersion": versionFirmware_2
                            }
                        }
                    }
                }
            }
jsonFirmware_2 = json.dumps(firmware_2)

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
    global jsonFirmware_1
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonFirmware_1
        )


def sendMessageWrongFirmware():
    global jsonFirmware_2
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonFirmware_2
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

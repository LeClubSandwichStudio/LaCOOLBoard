#!/usr/bin/env python3
import json
import boto3

macAddress = '2C3AE84FBF4F'
valueLogInterval = 320
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


def testAWS() :
    getShadow()
    sendMessageLogInterval()
    # sendMessageFirmwareVersion()
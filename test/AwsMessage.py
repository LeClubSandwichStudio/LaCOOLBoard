#!/usr/bin/env python3
import json
import boto3
MacAddress = '2C3AE84FBF4F'


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


def sendMessageLogInterval()
    client.update_thing_shadow(
        thingName=MacAddress,
        payload=b"""'{"state":
                        {"desired":
                            {"CoolBoard":
                                {"logInterval":306
                                }
                            }
                        }
                    }'"""
    )


#def sendMessageFirmwareVersion():
#    client.update_thing_shadow(
#        thingName=MacAddress,
#        payload=b"""'{
#                    "state": {
#                        "desired": {
#                            "CoolBoard": {
#                                "firmwareUpdate": {
#                                    "firmwareUrlFingerprint" : "BC5445C5CE60988076FFFE5C83555949810370A1",
#                                    "firmwareUrl" : "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/feature/test-bench/debug-v0.2.7-14-gdc7c463.bin",
#                                    "firmwareVersion" : "v0.2.7-14-gdc7c463"
#                                }
#                            }
#                        }
#                    }
#                }'"""
#        )


client = boto3.client('iot-data', region_name='eu-west-1')
response = client.get_thing_shadow(thingName=MacAddress)
streamingBody = response["payload"]
rawDataBytes = streamingBody.read()
rawDataString = rawDataBytes.decode('utf-8')
jsonState = json.loads(rawDataString)
sendMessageLogInterval()
# sendMessageFirmwareVersion()

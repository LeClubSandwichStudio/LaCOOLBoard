#!/usr/bin/env python3
import json
import boto3

macAddress = '2C3AE84FBF4F'
valueLogInterval = 315
Act0 = True
Act1 = True
Act2 = True
Act3 = True
Act4 = True
Act5 = True
Act6 = True
Act7 = True
fingerPrint = "1C8ABE2E0203E4093085DAD910D1265C0E07A9B4"
#linkFirmware_1 = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/master/debug-v0.2.7-34-g701ca6e.bin"
linkFirmware_1 = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/master/debug-v0.2.7-36-ga65e3ed.bin"
#versionFirmware_1 = "v0.2.7-34-g701ca6e"
versionFirmware_1 = "v0.2.7-36-ga65e3ed"

linkFirmware_2 = "https://s3-eu-west-1.amazonaws.com/cool-firmware-releases/feature/test-bench/debug-v0.2.7-14-gdc7c463.bin"
versionFirmware_2 = "v0.2.7-14-gdc7c464"

logInterval = {"state":
                {"desired":
                    {"CoolBoard":
                        {"logInterval": valueLogInterval
                        }
                    }
                }
            }
jsonLogInterval = json.dumps(logInterval)

jetPack = {"state":
                {"desired":
                    {"CoolBoard":
                        {"jetpackActive": True
                        }
                    }
                }
            }
jsonJetPack = json.dumps(jetPack)

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

actuator_2 = {"state":
                {"desired":
                    {"Act2": Act2
                    }
                }
            }
jsonActuator_2 = json.dumps(actuator_2)

actuator_3 = {"state":
                {"desired":
                    {"Act2": Act3
                    }
                }
            }
jsonActuator_3 = json.dumps(actuator_3)

actuator_4 = {"state":
                {"desired":
                    {"Act4": Act4
                    }
                }
            }
jsonActuator_4 = json.dumps(actuator_4)

actuator_5 = {"state":
                {"desired":
                    {"Act5": Act5
                    }
                }
            }
jsonActuator_5 = json.dumps(actuator_5)

actuator_6 = {"state":
                {"desired":
                    {"Act6": Act6
                    }
                }
            }
jsonActuator_6 = json.dumps(actuator_6)

actuator_7 = {"state":
                {"desired":
                    {"Act7": Act7
                    }
                }
            }
jsonActuator_7 = json.dumps(actuator_7)

firmware_1 = {"state":
                {"desired":
                    {"CoolBoard":
                        {"firmwareUpdate":
                            {"firmwareUrlFingerprint": fingerPrint,
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
                            {"firmwareUrlFingerprint": fingerPrint,
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


def sendMessageJetPack():
    global jsonLogInterval
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonJetPack
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


def sendMessageAct2():
    global jsonActuator_2
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_2
    )


def sendMessageAct3():
    global jsonActuator_3
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_3
    )


def sendMessageAct4():
    global jsonActuator_4
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_4
    )


def sendMessageAct5():
    global jsonActuator_5
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_5
    )


def sendMessageAct6():
    global jsonActuator_6
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_6
    )


def sendMessageAct7():
    global jsonActuator_7
    global macAddress
    client.update_thing_shadow(
        thingName=macAddress,
        payload=jsonActuator_7
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
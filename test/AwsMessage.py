#!/usr/bin/env python3
import boto3
import json

def printjson( jsonmessage ):
    jsonmessage.split()
    nbTab = 0 
    for i in jsonmessage: 
        if( "{" in i ):
            nbTab = nbTab +1
            print( "\n"+ "    "*nbTab + i , end="" )
        elif ("}" in i ): 
            nbTab = nbTab -1
            print(i + "\n" + "    "*nbTab ,end="" )
        elif ("," in i ): 
            print(i + "\n" + "    "*nbTab ,end="" ) 
        else: 
            print(i ,end="" )

client = boto3.client('iot-data', region_name='eu-west-1')
response = client.get_thing_shadow(thingName='2C3AE84FBF4F')
streamingBody = response["payload"]
rawDataBytes = streamingBody.read()
rawDataString = rawDataBytes.decode('utf-8')
jsonState = json.loads(rawDataString)
#!/usr/bin/env python3
import boto3
import json 

client = boto3.client('iot-data', region_name='eu-west-1')
response = client.get_thing_shadow(thingName='2C3AE84FBF4F')
streamingBody = response["payload"]
rawDataBytes = streamingBody.read()
rawDataString = rawDataBytes.decode('utf-8')
jsonState = json.loads(rawDataString)

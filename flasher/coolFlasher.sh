#!/bin/bash
if [ ${#} -lt 2 ]
then
    echo $'Usage : ./coolFlasher.sh -e [env] -s [macAddress] (optionnal) for more information read flasher/README.md\n\n  -e [...]        choose compilation environnement [minified, prod, debug, trace]\n  -s [...]        flash SPIFFS and send configuration to aws [yourMacAddress]'
else
    if [ ${1} = "-e" ]
    then
        pio run -e $2 -t upload
    fi
    if [ ${#} -eq 4 ]
    then
        if [ ${3} = "-s" ]
        then
            start='{"state":{"reported":{"config":'
            end='}}}'

            general=$(<examples/WeatherStation/data/general.json)
            aws iot-data update-thing-shadow --thing-name ${4} --payload "${start}${general}${end}" general
            rm general

            sensors=$(<examples/WeatherStation/data/sensors.json)
            aws iot-data update-thing-shadow --thing-name ${4} --payload "${start}${sensors}${end}" sensors
            rm sensors

            if [ -r examples/WeatherStation/data/actuators.json ]
            then
                actuators=$(<examples/WeatherStation/data/actuators.json)
                aws iot-data update-thing-shadow --thing-name ${4} --payload "${start}${actuators}${end}" actuators
                rm actuators
            fi
            pio run -e ${2} -t uploadfs
        fi
    fi
fi
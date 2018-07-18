import serial
import time
import os
import datetime
import time
import RPi.GPIO as GPIO


#global variable 
lineNumber=0
lineBuffer=[]

def absolutFlushBuffer():
    global lineBuffer
    global lineNumber
    lineBuffer=[]
    lineNumber=0
    serialBus.flushInput()
    return

def flushBuffer():
    global lineBuffer
    global lineNumber
    lineBuffer=lineBuffer[lineNumber:]
    lineNumber=0
    return

def resetBuffer():
    global lineNumber
    lineNumber=0
    return

def readLine(serialBus):
    global lineBuffer
    global lineNumber
    if (len(lineBuffer)==lineNumber):
        lineReturned=str(serialBus.readline())
        if lineReturned[2]=="[":
            lineNumber+=1
            if lineReturned[35]=="]":
                lineBuffer.append(lineReturned[36:-5])
                return lineReturned[36:-5]
            else :
                lineBuffer.append(lineReturned[35:-5])
                return lineReturned[35:-5]
        elif lineReturned[2]=="I" or lineReturned[2]=="W" or lineReturned[2]=="E" or lineReturned[2]=="D" :
            lineNumber+=1 
            lineBuffer.append(lineReturned[2:-5])
            return lineReturned[2:-5]
        elif lineReturned[2]=="*":
            lineNumber+=1
            lineBuffer.append(lineReturned[6:])
            return lineReturned[6:]
        
        else :
            return readLine(serialBus)
    else:
        lineReturned=lineBuffer[lineNumber]
        lineNumber+=1
        return lineReturned

def resetBoard() :
    GPIO.output(4,True)  #GPIO.LOW)
    time.sleep(2)
    GPIO.output(4,False) #GPIO.HIGH)
    absolutFlushBuffer()
    
    
def writeResetLine(Line, resultFileName) :
    resultFile = open(resultFileName,'a')
    print("\n Reset at line : ",Line)
    resultFile.write("\nRESET at line:                          "+ Line + "\n") 
    resultFile.close()
    
    
def waitLine(resetLineList,lineNumber, resultFileName):
    if lineNumber >= len(resetLineList):
        return True
    else :
        writeResetLine(resetLineList[lineNumber], resultFileName)
        resetLineLen=len(resetLineList[lineNumber])
        while (resetLineList[lineNumber]!=readLine(serialBus)[: resetLineLen] ):
            continue
        resetBoard()
        return False

##########################TEST FONCTIONS############################
    


def MAC_STATUS_TEST(serialBus, resultFileName, giveResult=True):
    test_name="MAC TEST"
    begin=""
    error=""
    success="INFO:  MAC address is:"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def FIRMWARE_VERSION_TEST(serialBus, resultFileName, giveResult=True):
    test_name="FIRMWARE VERSION"
    begin=""
    error=""
    success="INFO:  Firmware version"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def REGISTERED_WIFI_TEST(serialBus, resultFileName, giveResult=False):
    test_name="WIFI REGISTERED"
    begin="INFO:  Wifi configuration loaded"
    error="INFO:    Wifi count  = 0"
    success="INFO:    Wifi count"# à remplir avec une regular expression
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name="RETRY WIFI CONNECTION"
    begin="WARN:  Network unreachable"
    error=""# à remplir
    success="INFO:  Wifi status: connected"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def RETRY_MQTT_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name="RETRY MQTT CONNECTION"
    begin="INFO:  Wifi connecting..."
    error="INFO:  Subscribed to MQTT input topic"
    success="WARN:  MQTT connection failed, retrying"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def WIFI_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name="WIFI CONNECTION"
    begin="INFO:  Connecting..."   
    error="WARN:  Network unreachable"
    success="INFO:  Wifi status: connected"
    readFromBeginning=False
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def WIFI_ACCESS_POINT_OPENED(serialBus, resultFileName, giveResult=True):
    test_name="WIFI ACCESS POINT OPENED"
    begin="INFO:  Wifi configuration loaded"   
    error="DEBUG: Reading configuration file as JSON: /wifiConfig.json"
    success="INFO:  Starting Wifi access point and configuration portal"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def MQTT_CONNECTION(serialBus, resultFileName, giveResult=True):
    test_name="MQTT CONNECTION"
    begin=""
    error="WARN:  MQTT connection failed"
    success="INFO:  Subscribed to MQTT input topic"
    readFromBeginning=False
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def RTC_SYNCHRO(serialBus, resultFileName, giveResult=True):
    test_name="RTC_SYNCHRO"
    begin="INFO:  Synchronizing RTC..."
    error="WARN:  NTP failed, falling back to RTC"
    success="INFO:  RTC ISO8601 timestamp:"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def SEND_LOG( serialBus , resultFileName,giveResult=True ):
    test_name="SEND LOG"
    begin="INFO:  Collecting board and sensor data..."
    error="WARN:  Log not sent" 
    success="INFO:  MQTT publish successful"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def SAVE_LOG(serialBus , resultFileName,giveResult=True ):
    test_name="SAVE LOG"
    begin="INFO:  Sending log over MQTT..."
    error="" #à remplir
    success="WARN:  Log not sent, saved on SPIFFS"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def MAIN_CONFIGURATION(serialBus, resultFileName, giveResult=True):
    test_name="READ MAIN CONFIGURATION"
    begin=""
    error="ERROR:  Failed to parse main configuration" 
    success="INFO:  Main configuration loaded"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def SPIFFS(serialBus, resultFileName, giveResult=True):
    test_name="READ ALL SPIFFS"
    begin=""
    error="ERROR:  Filesystem failed, please contact support!" 
    success="DEBUG: Configuring MQTT"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult


def SLEEPMODE_ACTIVATE(serialBus, resultFileName, giveResult=True):
    test_name="SLEEPMODE ACTIVATE"
    begin="DEBUG: Reading configuration file as JSON: /coolBoardConfig.json"
    error="INFO:    Sleep active            = 0"
    success="INFO:    Sleep active            = 1"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def DATA_SAVING_JSON(serialBus, resultFileName, giveResult=True):
    test_name="SAVING DATA IN JSON"
    begin="WARN:  Log not sent, saved on SPIFFS"
    error=""
    success="DEBUG: Saved file data name:  /log/"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def DATA_DELETING_JSON(serialBus, resultFileName, giveResult=True):
    test_name="DELETING DATA SAVED IN JSON"
    begin="DEBUG: Message to log:"
    error="INFO:  Going to sleep for seconds:" 
    success="INFO:  Deleted log file: /log/"
    readFromBeginning=False##
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def DATA_SAVED_JSON(serialBus, resultFileName, giveResult=True):
    test_name="DATA SAVED IN JSON"
    begin="DEBUG: Message to log:"
    error="INFO:  Going to sleep for seconds:"
    success="DEBUG: Saved file data name:  /log/"
    readFromBeginning=False
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def EXTERNAL_SENSOR_ACTIVATED(serialBus, resultFileName, giveResult=True):
    test_name="EXTERNAL SENSOR ACTIVATED"
    begin="DEBUG: Reading configuration file as JSON: /coolBoardConfig.json"
    error="INFO:    External sensors active = 0"
    success="INFO:    External sensors active = 1"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def EXTERNAL_SENSOR_DATA_SEND(serialBus, resultFileName, giveResult=True):
    test_name="EXTERNAL SENSOR DATA SEND"
    begin="DEBUG: Reading configuration file as JSON: /externalSensorsConfig.json"
    error=""
    success="INFO:  Number of external sensors ="
    readFromBeginning=False
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def OTA_RECEIPTED(serialBus, resultFileName, giveResult=True):
    test_name="MESSAGE RECEIPTED BY OTA"
    begin="INFO:  Listening to update messages..."
    error="INFO:  Going to sleep for seconds:"
    success="INFO:  MQTT publish successful"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

def OTA_PUBLISHED(serialBus, resultFileName, giveResult=True):
    test_name="MESSAGE RECEIPTED BY OTA PUBLISHED"
    begin="INFO:  Received new MQTT message"
    error="ERROR:  Failed to update configuration file:"
    success="DEBUG: Successfully updated configuration file:"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult

"""def OTA_UPDATE_FIRMWARE(serialBus, resultFileName, giveResult=True):
#voir avec Khalil la commande dans AWS pour avoir la ligne de succès dans la sérial
    test_name="FIRMWARE UPDATED BY OTA"
    begin=""###
    error="ERROR:  HTTP Update failed, code:"
    success="HTTP update succeeded!"
    readFromBeginning=True
    maxLinesRead=500
    testResult = template_test(serialBus, resultFileName, maxLinesRead, test_name, begin, error, success, readFromBeginning, giveResult)
    return testResult"""

###############################TEMPLATE TEST FONCTIONS#################


def template_test(serialBus,resultFileName,maxLinesRead,TEST_NAME,BEGIN,ERROR,SUCCESS,READFROMBEGINNING,GIVERESULT) :
    #TEST_NAME  give name for your test
    #BEGIN      give the first line before the test should begin, let it empty if there is none  
    #ERROR      give the line that appear (or the first part) if Coolboard fails, let it empty if there is none
    #SUCCESS    give the line that appear (or the first part) if Coolboard behave normally, let it empty if there is none
        #there should be at least one of ERROR and SUCCESS not empty 
    #READFROMBEGINNING      if you want the test to check from the first line that arrive on the serial monitor. 
    #GIVERESULT if true, write in the result file the meaningfull line of the test.

    beginLen=len(BEGIN)
    errorLen=len(ERROR)
    successLen=len(SUCCESS)
    resultFile = open(resultFileName,'a')
    listWARN=[]
    if (READFROMBEGINNING) :
        resetBuffer()
        
    if (errorLen==0 and successLen==0 ):
        print(TEST_NAME, "impossible to assert : no ERROR or SUCCESS line given ")
        resultFile.write("IMPOSSIBLE TO ASSERT " + TEST_NAME + ": \n")
        resultFile.close()
        return False

    if (beginLen>0 and READFROMBEGINNING):
        waitBegin=0
        while(readLine(serialBus)[:beginLen]!= BEGIN and waitBegin<maxLinesRead):
            waitBegin+=1
            if waitBegin==maxLinesRead:
                resultFile.write("ERROR reached limit- " + TEST_NAME + "\n")
                print("--------------------",TEST_NAME , ": ERROR reached limit\n")
                resetBuffer()
                resultFile.close()
                return False
        
    print("\nBegin " ,TEST_NAME , "------------------------")

    waitEndTest = 0
    testLine=[]
    while(not waitEndTest>maxLinesRead ):##
        testLine=readLine(serialBus)
        print(testLine)
        
        if(successLen!=0 and testLine[:successLen]==SUCCESS):
            print("--------------------------------" , TEST_NAME ,": OK")
            if(GIVERESULT):
                OKmessage = "OK------------------ " + TEST_NAME
                resultFile.write(OKmessage)
                resultFile.write(" "*(40-len(TEST_NAME)) + testLine + "\n")
                for i in listWARN:
                    resultFile.write(" "*65 + i + "\n")
            resultFile.close()
            return True
        
        if( (errorLen!=0 and testLine[:errorLen]==ERROR ) or testLine[:34]== "INFO:  Going to sleep for seconds:"):
            if(GIVERESULT):
                print("-------------------------------" , TEST_NAME , ": ERROR")
                ERRORmessage="ERROR--------------- " + TEST_NAME 
                resultFile.write(ERRORmessage)
                resultFile.write( " "*(40-len(TEST_NAME))+ testLine + "\n")
                for i in listWARN:
                    resultFile.write(" "*65 + i + "\n")
            resultFile.close()
            return False
        
        if(testLine[0:5]== "ERROR" or testLine[0:4]=="WARN"):
            listWARN.append(testLine)
        waitEndTest=1
    
    resultFile.write("ERROR reached limit- " + TEST_NAME + "\n")
    print("-------------------------------" , TEST_NAME , ": ERROR reached limit")
    resetBuffer();
    resultFile.close()
    return False

####################### principal Programm ##########################

#serialBus = serial.Serial('/dev/cu.usbserial-DN02PRAQ', 115200, timeout=10)
#serialBus = serial.Serial('/COM7', 115200, timeout=10)
serialBus = serial.Serial('/dev/ttyUSB0', 115200, timeout=10)

resultFileName='CoolBoard-Test-' + datetime.datetime.now().strftime("%y-%m-%d-%H-%M")+".txt"

GPIO.setmode(GPIO.BCM)
GPIO.setup(4,GPIO.OUT, initial=GPIO.LOW)#, #initial=GPIO.HIGH)
#GPIO.output(4, 0)

endOfTest=False
resetLineList=[]
#"INFO:  Builtin actuator configuration loaded","INFO:  Wifi connecting...","INFO:  MQTT connecting...","INFO:  Listening to update messages..."]
resetLineNumber=0
resetBoard()

while not endOfTest:
#puts in the good order all the test functions
    SPIFFS(serialBus, resultFileName, True)
    MAIN_CONFIGURATION(serialBus, resultFileName, True)
    MAC_STATUS_TEST(serialBus, resultFileName, True)
    FIRMWARE_VERSION_TEST(serialBus, resultFileName, True)

    if (not REGISTERED_WIFI_TEST(serialBus, resultFileName, False)):
         WIFI_ACCESS_POINT_OPENED(serialBus, resultFileName, True)
         
    
    elif (WIFI_CONNECTION(serialBus, resultFileName,True)):
             if (not MQTT_CONNECTION(serialBus, resultFileName,True)):
                     SLEEPMODE_ACTIVATE(serialBus, resultFileName, True)
                     RETRY_MQTT_CONNECTION(serialBus, resultFileName, True)
             else:
                     SEND_LOG(serialBus, resultFileName,True)
                     RTC_SYNCHRO(serialBus, resultFileName,True)
                     while(DATA_SAVED_JSON(serialBus, resultFileName, False)):
                         DATA_DELETING_JSON(serialBus, resultFileName, True)
             if(EXTERNAL_SENSOR_ACTIVATED(serialBus, resultFileName, False)):
                EXTERNAL_SENSOR_DATA_SEND(serialBus, resultFileName, True)

             if (OTA_RECEIPTED(serialBus, resultFileName, False)):
                OTA_PUBLISHED(serialBus, resultFileName, True)
                #if(OTA_UPDATE_FIRMWARE(serialBus, resultFileName, True)):
                #FIRMWARE_VERSION_TEST(serialBus, resultFileName, True)
    else:
        #RETRY_WIFI_CONNECTION(serialBus, resultFileName, giveResult=True) #passera à la suite que si succès
        SAVE_LOG(serialBus, resultFileName,True)
        DATA_SAVING_JSON(serialBus, resultFileName, True)

    time.sleep(1)    
    resetBoard()
    time.sleep(1)
    endOfTest = waitLine(resetLineList,resetLineNumber, resultFileName)
    resetLineNumber = resetLineNumber +1
        
print("\n\n--------------------|END OF THE TEST|------------------------")

serialBus.close()
GPIO.cleanup()






#regler problème de com
#pbm en cas de mauvais formatage du serial




    


#!/usr/bin/env python3

import SerialParameters

# --------------------------TEMPLATE TEST FONCTIONS-------------------------- #


def runner(
        serialBus, resultFileName, maxLinesRead,
        TEST_NAME, BEGIN, ERROR, SUCCESS,
        READFROMBEGINNING, GIVERESULT):
    # TEST_NAME: give name for your test
    # BEGIN: give the first line before the test should begin,
    #        let it empty if there is none
    # ERROR: give the line that appear (or the first part) if Coolboard fails,
    #        let it empty if there is none
    # SUCCESS: give the line that appear (or the first part)
    #          if Coolboard behave normally, let it empty if there is none
        # /!\ there should be at least one of ERROR and SUCCESS not empty
    # READFROMBEGINNING: if you want the test to check from the first line
    #                    that arrive on the serial monitor
    # GIVERESULT: if true,
    #             write in the result file the meaningfull line of the test

    beginLen = len(BEGIN)
    errorLen = len(ERROR)
    successLen = len(SUCCESS)
    resultFile = open(resultFileName, 'a')
    listWARN = []
    if (READFROMBEGINNING):
        Config.resetBuffer()
    if (errorLen == 0 and successLen == 0):
        print(TEST_NAME,
              "impossible to assert : no ERROR or SUCCESS line given")
        resultFile.write("IMPOSSIBLE TO ASSERT " + TEST_NAME + ": \n")
        resultFile.close()
        return False
    if (beginLen > 0 and READFROMBEGINNING):
        waitBegin = 0
        while(Config.readLine(serialBus)[:beginLen] !=
                BEGIN and waitBegin < maxLinesRead):
            waitBegin += 1
            if waitBegin == maxLinesRead:
                resultFile.write("ERROR reached limit- " + TEST_NAME + "\n")
                print("--------------------", TEST_NAME,
                      ": ERROR reached limit\n")
                Config.resetBuffer()
                resultFile.close()
                return False
    print("\nBegin ", TEST_NAME, "------------------------")
    waitEndTest = 0
    testLine = []
    while(not waitEndTest > maxLinesRead):
        testLine = Config.readLine(serialBus)
        print(testLine)
        if(successLen != 0 and testLine[:successLen] == SUCCESS):
            print("--------------------------------", TEST_NAME, ": OK")
            if(GIVERESULT):
                OKmessage = "OK------------------ " + TEST_NAME
                resultFile.write(OKmessage)
                resultFile.write(" " * (40 - len(TEST_NAME)) + testLine + "\n")
                for i in listWARN:
                    resultFile.write(" " * 65 + i + "\n")
            resultFile.close()
            return True
        if((errorLen != 0 and testLine[:errorLen] == ERROR) or
           testLine[:34] == "INFO:  Going to sleep for seconds:"):
            if(GIVERESULT):
                print("-------------------------------", TEST_NAME, ": ERROR")
                ERRORmessage = "ERROR--------------- " + TEST_NAME
                resultFile.write(ERRORmessage)
                resultFile.write(" " * (40 - len(TEST_NAME)) + testLine + "\n")
                for i in listWARN:
                    resultFile.write(" " * 65 + i + "\n")
            resultFile.close()
            return False
        if(testLine[0:5] == "ERROR" or testLine[0:4] == "WARN"):
            listWARN.append(testLine)
        waitEndTest = 1

    resultFile.write("ERROR reached limit- " + TEST_NAME + "\n")
    print("-------------------------------",
          TEST_NAME, ": ERROR reached limit")
    Config.resetBuffer()
    resultFile.close()
    return False
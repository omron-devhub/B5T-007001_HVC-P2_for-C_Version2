----------------------------------------------------
 B5T-007001 Sample Code
----------------------------------------------------

(1)About this material
  It provides the sample code of B5T-007001(HVC-P2).
    1-1) Sample code to transmit command (execute functions) with B5T-007001
    1-2) Source code of library(STBLib) stabilizing HVC result

(2) Contents
The setting / acquisition of each setting value of B5T-007001 and the following functions are executed,
and outputs the result in standard output.

   1 : Detection/Estimation                       Executes the 9 function of detections / estimates except recognition.
   2 : Recognition(Identify)                      Executes recognition(Identify) function.
   3 : Recognition(Verify)                        Executes recognition(Verify) function.
   4 : Register data                              Executes the face recognition registration function.
   5 : Delete specified data                      Deletes one of the specified face recognition data.
   6 : Delete specified user                      Deletes the face recognition data of the specified user.
   7 : Delete all data                            Delete the face recognition data of all users.
   8 : Save Album                                 Save the album data to the host machine.
   9 : Load Album                                 Read the album data saved on the host machine.
  10 : Save Album on Flash ROM                    Write the album data to the flash ROM.
  11 : Reformat Flash ROM                         Reformat the album data storage area of the flash ROM.
  12 : Set Number of registered people in album   Set the maximum number of user registrations for an album.
  13 : Get Number of registered people in album   Get the maximum number of user registrations for an album 

  * This sample executes stabilization for gender/age estimation and recognition(Identify).
    (In start-up argument, it is possible to select to use STBLib or not.)
  * Functions 3, 12, and 13 can only be performed in version 1.2.3 and later. 

(3) Directory Structure
    bin/                            Output directory for building
    import/                         Import directory to use STBLib
    platform/                       Building environment
        Windows/                        For Windows
        Linux/                          For Linux
    src/
        HVCApi/                     B5T-007001 interface function
            HVCApi.c                    API function
            HVCApi.h                    API function definition
            HVCDef.h                    Struct definition
            HVCExtraUartFunc.h          Definition for external functions called from API function
        STBApi/                     STBLib interface function
            STBWrap.c                   STBLib wrapper function
            STBWrap.h                   STB Lib wrapper function definition
        bmp/                        Function to save bitmap file
            bitmap_windows.c            Saving image function for Windows
            bitmap_linux.c              Saving image function for Linux
        uart/                       UART interface function
            uart_windows.c              UART function for Windows
            uart_linux.c                UART function for Linux
            uart.h                      UART function definition
        Album/                      Album file save/read function
            Album.c                     Function to I/O album obtained from B5T-007001
        Sample/                     Detection process sample
            main.c                      Sample code for detection process
    STBLib/                         STBLib kit
        doc/                            Documents set of STBLib
        bin/                            STBLib binary file
        platform/                       STBLib's building environment
            Windows/                        For Windows
            Linux/                          For Linux
        src/                            STBLib source code

(4) Building method for sample code
  * For Windows
  1. The sample code is built to be operated on Windows 10/11.
     It can be compiled with VC10 (Visual Studio 2010 C ++) or later.
  2. The exe file is generated under bin/Windows/ after compilation.
     And STBLib DLL file is needed on the same directory with the exe file.
     (In advance, STB.dll is stored in that directory.)

  NOTE: You need the MFC library when you build STBLib. Install it in advance.
        If you will customize STBLib, it is necessary that STBLib is recompiled.
        After that, update STB.dll in the same directory with the exe file and files in import/ directory.

  * For Linux
  1. Run "build.sh" that exists in lib/platform/Linux/ to build STBLib.
  2. After step 1, store the files of [STB.a, libSTB.so] in import/lib/ directory.
     (Those files are created in lib/bin/Linux directory.)
  3. It can be compiled and linked by running "build.sh" in the platform/Sample/ directory.

(5) Method for executing sample code
  When executing this sample code, it is necessary to specify as following in start-up argument.

    Usage: sample.exe <com_port> <baudrate> [use_stb]
        com_port:  COM port
        baudrate: UART baudrate
        use_stb:   Using flag for STB Library (STB_ON or STB_OFF)
                   * If skipped this argument, working as STB_ON.

  (Examples about execution) 
  * For Windows
   sample.exe 1 921600 STB_ON

  * For Linux
   - Start-up arguments are the same as those cases of Windows.
     To execute this sample code, it is necessary that B5T-007001 is recognized as "/dev/ttyACM0" by host machine.
     About the details of execution, refer to shell-scripts bin/Linux/Sample.sh.

   - Example of sample (6th line of Sample.sh)
      ./Sample 0 921600 STB_ON
     * In this case, first argument "0" is ignored in Linux.
       So, working as "921600bps" and "Use STBLib".


[NOTES ON USAGE]
* This sample code and documentation are copyrighted property of OMRON Corporation
* This sample code does not guarantee proper operation
* This sample code is distributed in the Apache License 2.0.
* STBLib is specialized for B5T-007001.
  It will be assumed as agreeing the relevant products "Terms of Use and Disclaimer" to use this sample code.

----
OMRON Corporation
Copyright(C) 2014-2025 OMRON Corporation, All Rights Reserved.
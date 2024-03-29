readme.txt file  for DC Machine Control: (Project name DC_machine_control)
Copyright 2022.

====================================================================================


Table of Contents
-----------------

This file contains the following sections:

* Windows Installation Instructions
* Mac (OSX) Installation Instructions
* Linux (64b) Installation Instructions
* Links to Additional Information

-------------------------------------

**Windows Installation Instructions**
-------------------------------------
A shortcut to your application is installed in the Windows Start menu under Texas Instruments.

You can alternatively start the application by running the win32_start.bat batch file that is located
 in the application folder.  This is typically installed in the guicomposer folder that is in your Users folder. e.g:
        c:\Users\<myUserName>\guicomposer\runtime\gcruntime.v3\DC_machine_control\launcher.exe
	

---------------------------------------

**Mac (OSX) Installation Instructions**  
---------------------------------------
To run your application:

  * open a terminal window
  * cd into the designer folder that is in the guicomposer runtime folder path you
specified when installing your application.

The following commands may be required to be executed from the Terminal app with this folder as the working directory
in order to enable proper execution of the application:

    sudo chmod -R a+rwx ../..

Then run the application by entering the following command

    ./launcher.osx

or by double-clicking on mac_start.app


--------------------------------------------

**Linux (64 bit) Installation Instructions**     
--------------------------------------------

To install all prerequisites for your application:

  * open a terminal window
  * cd into the the DC_machine_control folder that is in the guicomposer runtime folder path you
specified when installing your application.

The following commands need to be executed from the Terminal app with this folder as the working directory in order
to enable proper execution of the application:

     sudo ln -sf /lib/x86_64-linux-gnu/libudev.so.1 /lib/x86_64-linux-gnu/libudev.so.0
     sudo apt-get install lib32stdc++6 libc6-i386 libusb-1.0-0-dev:i386
     sudo chmod -R a+rwx ../..
     cd linux/TICloudAgentHostApp
     sudo ./install.sh --install

To run your application, cd into the DC_machine_control folder and enter the following command:

     ./launcher.lnx


If the application needs to access to your PC's serial ports, you will need to configure permissions to access
the serial ports by adding yourself to the group that owns the /dev/usbPortName
To find out what group the serial port ttyACM0 is owned by, do

     ls -l /dev/ttyACM0

Assuming the group is named dialout, add yourself to that group by using the sudo adduser command:

     sudo adduser my_user_name dialout

You will likely also need to change the permissions on the USB port to permit read / write access for anyone.

     sudo chmod 666 /dev/ttyACM0

For more information, please see
http://askubuntu.com/questions/112568/how-do-i-allow-a-non-default-user-to-use-serial-device-ttyusb0

In order to get permission to use USB-HID ports, you will need to create a file in /etc/udev/rules.d named
usbhid.rules that contains the following:

    ATTRS(idVendor)=="2047", MODE="0666"

If your PC is configured to use a proxy, you will need to ensure that there is no proxy for localhost.  To do this,
edit your .bashrc file

     gedit ~/.bashrc

and add the following two lines:

     no_proxy=localhost,127.0.0.1,local.home
     export no_proxy

-------------------------------

Links to Additional Information
-------------------------------

Please visit http://dev.ti.com for more info about GUIComposer2

For help, please see https://dev.ti.com/gc/designer/help/Tutorials/GettingStarted/index.html




Last updated: Sat Feb 26 2022

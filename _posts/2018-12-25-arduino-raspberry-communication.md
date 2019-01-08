---
layout: default
title: "Arduino and Raspberry communication"
---

# Serial communication between Arduino and Raspberry pi
## Notes from video
Notes taken from [this Youtube video](https://www.youtube.com/watch?v=M2-nXbi3qmk)

### Arduino set up
The communication is achieved using the serial port, so the serial cable connected to Arduino is inserted in the USB of the Raspberry.

### Raspberry set up
In order to find the device connected to the Raspberry the command
```
sudo ls /dev
```
(the backtick is done with alt+9 on mac otherwise alt+96) showing all the devices connected to the pi. To find the port at which the arduino is connected, the following command is executed:
```
sudo ls /dev >> dev.txt
```
which creates a file containing all the connected devices. If nothing is connected, it will be shown the default devices. Then we connect the Arduino, then run:
```
sudo ls /dev >> dev2.txt
diff dev.txt dev2.txt
```
In this way the differences will be shown and it is possible to find the device id of the Arduino.
Then it is possible to communicate with the raspberry using a python script using this script:
```python
import serial

# Create serial master to send command to the slave
ser = serial.Serial('<device_id>', <baud_rate>)

# Send data to Arduino
ser.write('<Data_to_send>')

# Read data from Arduino
# ser.read(<variable_where_to_store_data>)
```
## Server connection
The server communication is achieved using a socket, this is the code for the server connection:
```python
import socket
import serial
import time
import subprocess

# Create a process to check Arduino device id
dev = subprocess.check_output('ls /dev/ttyACM*',shell=True)

# Print the arduino device id
print dev

try:
    # It is used the strip function to remove the '/' character from the device id
    ser = serial.Serial(dev.strip(),9600)
    print "Arduino Connected"
except:
    print "Arduino not connected"

# Function
def server():

    # Declaring ser as global allows to have access to it from outside the function
    global ser
    while True:

        # conn is used for communicate with the client
        # Addr is used for saving socket address
        (conn, addr) = s.accept()
        print 'Connection address:', addr

        # Receive data from client and save it
        # BUFFER_SIZE is the default data size 1024 KB but here it is changed
        data = conn.recv(BUFFER_SIZE)
        if not data: continue
        print "received data:", data
        if data == '1':

            # Send data to client
            conn.send("Blue light Glowing")

            # Close connection with the client
            conn.close()

            # Send data to Arduino to light up led 1
            ser.write('1')

            # Sleep for 1 second
            time.sleep(1)

        elif data == '7':
            conn.send('bye bye')

            # Disconnect from client
            conn.close()

            # Disconnect Serial connection
            ser.close()
            exit(0)


        else :
            # Send data to the Arduino
            ser.write(data)

            # Read arduino reply
            aa =  ser.readline()
            time.sleep(0.1)
            print aa
            conn.send(aa)
            conn.close()


TCP_IP = '192.168.2.100'
TCP_PORT = 5005
BUFFER_SIZE = 20  # Normally 1024, but we want fast response

# Create socket and put it in listen
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
s.bind((TCP_IP, TCP_PORT))
s.listen(5)

print 'server started'
server()  
```

## Personal implementation
The goal is to light up the Arduino built-in led through a command sent from a pc connected through a socket to the Raspberry pi which is connected to the Arduino through the serial cable. The pc is the client which sends the data to the raspberry that is used as server. The client is implemented using the unix server/client code adapted to not echo.

The Raspberry pi will use the server code written in python and then in c++ to communicate with both the pc as TCP server and with the Arduino as serial master.

Arduino will receive and execute the command lighting up the led.

### Arduino setup
This is the code used with Arduino to light up the led with the serial command.
```c++
int LEDPIN = 13;          // Led pin
char receivedChar = "";   // Character received through serial
String command = "";      // Command string

void setup() {
  // Set the led pin as output  
  pinMode(LEDPIN, OUTPUT);

  // Set up serial communication
  Serial.begin(9600);
}

void loop() {

  // Check for available serial data
  if (Serial.available())
  {
    // Read serial payload
    receivedChar = Serial.read();

    // Store received command character
    command.concat(receivedChar);
  }

  // Check for string terminator
  if (receivedChar == '\n' && command != "")
  {
    // Check for On command
    if (command == "On\n")
    {
      // Light up the led
      digitalWrite(LEDPIN, true);

      // Reset command string
      command = "";
    }

    // Check for Off command
    else if (command == "Off\n")
    {
      // Light off the led
      digitalWrite(LEDPIN, false);

      // Reset command string
      command = "";
    }
    else
    {
      // Command unknown
      Serial.print("Command Unknown\n");

      // Reset command string
      command = "";
    }
  }
```

Sending "On\n" the led will light up, while sending "Off\n" the led is light off.
If an unknown command is sent, Arduino is set up to return an error string.

### Raspberry setup
First of all, the Arduino device id is found as `/dev/ttyACM0`.

As first try, the server is implemented using the python script.

The device this time has been found using the python serial API. 

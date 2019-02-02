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

All code files are available inside the `_code` folder of this repository under the homonym folder.

### Arduino setup
This is the code used with Arduino to light up the led with the serial command.
>It can be found as SerialCommand.ino inside the project folder.

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

#### Python server

> The implemented code can be found as ArduinoSerialComm.py inside the project folder.

The device this time has been found using the python serial API. To do so, a new process checking the device connected to the Pi is launched in order to check for the Arduino connection. This is done looking for the device with the `ttyACM*` name as:

```python
# Create a process to check Arduino device id
# *.decode("utf-8") is used for cast the device name output in byte to string
dev = subprocess.check_output('ls /dev/ttyACM*',shell=True).decode("utf-8")
```
If the device is found, the Arduino is connected and the device serial connection is opened using the API provided by the serial module:
```python
# The strip function remove the '/n' character from the device id
ser = serial.Serial(dev.strip(), 9600)
```
Now the server is started. The server is created in order to take the commands received from the user and, after checking if they are compliant with the Arduino ones, it sent them to Arduino.

It is a TCP/IP server, so the socket is set up considering all the relevant parameters as:
```python
TCP_IP = "192.168.1.17"
TCP_PORT = 2000

# Create socket and wait for client
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
sock.bind((TCP_IP, TCP_PORT))
sock.listen(10)
```

The TCP address is a constant because the Raspberry pi has a static IP set.

The server itself is a muticlient server which creates a new thread on which run each of the client connected to it.
So the new connection is waited through:
```python
# Blocking call, waits to accept a connection
conn, addr = sock.accept()
```
Once the client is connected, it is sent to a new thread. To do so, the `threading.Thread` module has been used:
```python
# Start client target
  Thread(target = client_thread, args = (conn, addr, ser)).start()
```
The thread has been provided with the socket address, here passed as `conn`, the address `addr` comprehending both the IP address and the socket number (this data is just for visualization purposes) and the serial port on which connect with Arduino, passed as `ser`.

On the executed thread the commands received command is evaluated and sent to the Arduino if compliant. Before sending it down through the serial port, it is checked if the resource is available. Indeed Arduino is resource shared among all the threads so it can't be accessed by more than one client. So a mutex is used. It allows to lock the resource while it is accessed by a thread.

The implementation is quite simple, the `threading.Lock` module must be imported and only one lock in the main part of the program must be created. It is important not to create a lock inside the thread otherwise it won't provide any protection. The lock is created as:
```python
# Create lock object
lock = Lock()
```
Now to prevent from resource access, the `acquire` function is called, locking the resource right before accessing it as:
```python
# Lock mutex to prevent from multiple access to Arduino
if (lock.acquire()):
.
.
.
```
The function return if the lock has worked.

Then all the operation on the resource are done and after writing down on the serial port the command, the resource are released again as:
```python
# Unlock mutex
lock.release()
```
Now the data sent to Arduino, or the wrong command message, is sent back to the client. If the disconnect message has been received, the client is disconnected and thread is closed.

Doing so, a client, on the same network, connected through the TCP socket can access Arduino and command the LED.

#### C++ Server
> The implemented code can be found as RaspberryServerMulticlient.cpp inside the project folder.

##### Serial port
The Serial port in C++ is not managed by libraries as it was for the python server. Here the port management should be implmeneted otherwise it won't be possible to write and read from the port.

This has be done building a C++ class named SerialPortManage which is able to handle the port activation and communication.
> The implementation of the port could be find looking for the [utility repository](https://github.com/alby177/utilities) in which the serial port folder contains the code.

The hardest part is to correctly configure the port. Here the configuration is not made so flexible, the communication baud rate is 9600 and the default communication is made as 8n1. Everything can be changed but here it has been implemented specifically for this application so no customization was allowed. The interfaces for the communication are:
```C++
// Constructor for setting the port name
SerialPortManage port("<port_name>");

// Write buffer on the serial bus
port.SerialWrite(<pointer_to_buffer>);

// Read from serial port
std::string text = port.SerialRead();
```

**It is really important to remember that some time is needed for sent character down the serial bus, so between the SerialWrite and the SerialRead some time must be elapsed otherwise when the read is performed the write will not be finished.**

Another quite important thing is that the SerialRead and the SerialWrite are non-blocking so, if no character is received, the application won't wait for some characters on the serial bus but it will go with the execution.

##### Server
The server has been again implemented as a multithread object which create a new thread for every client connected to it.

So first of all the serial port is opened as:
```C++
SerialPortManage serial("/dev/ttyACM0");
```
Then the socket is created and addressed as a TCP one. Then bound and got ready to listen for a new client connecting. In the `while` loop of the client management, the server wait for a client and once it got a new one, it creates a thread and provide it all the data through a custom made structure in which there is:
```C++
struct threadData
{
    struct sockaddr_in clientData;      // IP client data
    int clientSock;                     // Client socket address
    SerialPortManage serialPort;        // Serial port address
};
```
In this way it is possible to communicate with the serial port and to visualize clearly which client is connected to the server.

The structure is passed as a pointer to void when the thread is created
```C++
// Create thread
int rc = pthread_create(&tClient[clientConnected], nullptr, ClientHandle, (void *)dataToSend);
```
This is the only way to send something to a specific thread, but it is important to remember that the generated threads and the main process share the same memory stack. The first parameter of the function is the array of thread objects created in order to manage them. Then the second one is a parameter for setting the kind of thread opened and the third one is the function that the thread must execute.

As shown in the picture, as first thing the data structure is reconverted to its original type:
```C++
struct threadData *data = (struct threadData*)clientData;
```
Then it waits for the data received by the client through the `recv` function and compare it with the standard messages to either know if terminate the thread or it's time to write something to Arduino or if the command sent is invalid. If the server receives the special message `"Muori"`, the server is shutdown disconnecting every client.

It is important to underline that only a limited number of clients can be connected to the server. The number of connected clients is managed through a variable protected by a mutex in order to allow every client created on a thread to decrement automatically the number of clients connected in case of disconnection. It is fundamental to lock this resource otherwise it would be managed in un undefined way, obtaining wrong behaviors.
```C++
// Decrease running threads number
pthread_mutex_lock(&lock);

clientConnected--;

pthread_mutex_unlock(&lock);
```

It is also important to destroy all the lock and running thread (if there are some) when the application is terminated.   

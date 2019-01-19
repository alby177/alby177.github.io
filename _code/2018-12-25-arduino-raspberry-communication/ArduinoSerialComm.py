import sys, socket, serial, subprocess
from threading import Thread, Lock

# Thread function to handle client connection
def client_thread(conn, addr, ser):

    while True:

        # Save decoded received data
        data = conn.recv(1024).decode("utf-8")

        # Check for disconnect command
        if data == "Disconnect":

            # Send disconnect message to the client
            conn.sendall(("Disconnection...").encode("utf-8"))
            break

        # Check for commands
        elif (data == "On" or data == "Off"):

            # Lock mutex to prevent from multiple access to Arduino
            if (lock.acquire()):

                # Send data to Arduino
                ser.write((data + "\n").encode("ascii"))

            # Unlock mutex
            lock.release()

            # Save data to send client
            reply = ("Data sent to Arduino:" + data)
        else:
            reply = ("Wrong command sent")

        # Send data to client
        conn.sendall(reply.encode("utf-8"))

    # Close connection
    print("Closing connection with: " + addr[0] + ":" + str(addr[1]))
    conn.close()



# Create lock object
lock = Lock()

try:
    # Create a process to check Arduino device id
    # *.decode("utf-8") is used for cast the device name output in byte to string
    dev = subprocess.check_output('ls /dev/ttyACM*',shell=True).decode("utf-8")
except:
    sys.exit("No device found")

try:
    # The strip function remove the '/n' character from the device id
    ser = serial.Serial(dev.strip(), 9600)
    print ("Arduino Connected as ", dev.strip())
except:
    sys.exit("Arduino not connected")

TCP_IP = "192.168.1.17"
TCP_PORT = 2000

# Create socket and wait for client
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
sock.bind((TCP_IP, TCP_PORT))
sock.listen(10)

while True:
    # blocking call, waits to accept a connection
    conn, addr = sock.accept()
    print("[-] Connected to " + addr[0] + ":" + str(addr[1]))

    # Start client target
    Thread(target = client_thread, args = (conn, addr, ser)).start()

# Close socket connection
sock.close()

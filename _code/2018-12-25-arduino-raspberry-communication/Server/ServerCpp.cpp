#include "../utilities/SerialPort/SerialPortManage.h"
#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#define MAX_BUFFER		128
#define MAX_CLIENTS     5
#define PORT	5001

// Global variable declaration
pthread_mutex_t lock;           // Mutex for number of client computation
pthread_mutex_t lock2;          // Mutex for Arduino access
int clientConnected = 0;        // Number of connected clients

// Data passed to the thread
struct threadData
{
    struct sockaddr_in clientData;      // IP client data
    int clientSock;                     // Client socket address
    SerialPortManage serialPort;        // Serial port address
};

// Function performed by the thread
void *ClientHandle (void *clientData)
{
    // Convert struct passed to thread to its original data type
    struct threadData *data = (struct threadData*)clientData;
    char ip[INET_ADDRSTRLEN];

    // Convert ip address form data structure and save it in ip
    inet_ntop(AF_INET, &(data->clientData.sin_addr.s_addr), ip, INET_ADDRSTRLEN);
    std::cout << "Client connected at address: " << ip << std::endl;

    char msgRcv[100];
    bool closeConnection = false;

    // Go on until terminating message is received
    while (closeConnection != true)
    {
        // Reset message received string
        memset(msgRcv, 0, sizeof(msgRcv));

        // Received data from client
        ssize_t received = recv(data->clientSock, msgRcv, sizeof(msgRcv), 0);

        // Check for received data
        if (received > 0)
        {
            std::cout << "Received Message: " << msgRcv << std::endl;
            char msgSending[120];

            // Check for commands
            if ((strcmp(msgRcv, "On") == 0) || (strcmp(msgRcv, "Off") == 0))
            {
                // Save received message
                strcpy(msgSending, msgRcv);

                // Lock Arduino access
                pthread_mutex_lock(&lock2);

                // Write data to serial port
                data->serialPort.SerialWrite(strcat(msgSending, "\n"));

                // Unlock Arduino access
                pthread_mutex_unlock(&lock2);
            }

            // Check for disconnect command
            else if (strcmp(msgRcv, "Disconnect") == 0)
            {
                // Save message
                strcpy(msgSending, "Disconnecting...");

                // Set loop break command
                closeConnection = true;
            }
            else
                strcpy(msgSending, "Wrong command sent");

            // Send message to client
            size_t byteSent = send(data->clientSock, msgSending, strlen(msgSending) + 1, 0);

            // Check for message sent
            if (byteSent != strlen(msgSending) + 1)
            {
                std::cout << "Message send failed, size " << byteSent << std::endl;
                close(data->clientSock);
            }
        }
    }

    std::cout << "Disconnecting client: " << ip  << std::endl;

    // Decrease running threads number
    pthread_mutex_lock(&lock);

    clientConnected--;

    pthread_mutex_unlock(&lock);

    // Close socket connection
    close(data->clientSock);

    // Release data structure memory
    free(clientData);

    // Close thread execution
    pthread_exit(nullptr);
 }


int main(void) {
    pthread_t tClient[MAX_CLIENTS];
	int serverFd, connectionFd;
	socklen_t clilen;
	struct threadData *dataToSend;
	struct sockaddr_in servaddr, cliaddr;

	// Open the serial port communication
	SerialPortManage serial("/dev/ttyACM0");

	// Create socket
	if ((serverFd = socket( AF_INET, SOCK_STREAM, 0)) < 0)
    {
		printf("Error creating socket\n");
		return 1;
	}

	// Set memory structure for TCP/IP connection
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	clilen = sizeof(cliaddr);

	// Bind created socket
	if (bind(serverFd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		printf("Error binding\n");
		return 1;
	}

	// Set socket to listen
	listen(serverFd, MAX_CLIENTS);

	while (1)
    {
        // Check for maximum number of clients
        if (clientConnected < MAX_CLIENTS)
        {
            std::cout << "Waiting for client connection...." << std::endl;

            // Accept client connection
            connectionFd = accept(serverFd, (struct sockaddr *) &cliaddr, &clilen);

            // Check for client connection error
            if( connectionFd < 0)
            {
                printf( "Error accepting\n" );
                break;
            }
            else
            {
                // Allocate space for data to send to thread
                dataToSend = (struct threadData *)malloc(sizeof(struct threadData));

                // Save data to send to the threads
                dataToSend->clientData = cliaddr;
                dataToSend->clientSock = connectionFd;
                dataToSend->serialPort = serial;

                // Create thread
                int rc = pthread_create(&tClient[clientConnected], nullptr, ClientHandle, (void *)dataToSend);

                // Check for error while creating threads
                if (rc != 0)
                    std::cout << "Failed to create thread" << std::endl;
                else

                    // Increase connected thread number
                    clientConnected++;
            }
        }
	}

    // Destroy mutex
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&lock2);

	// Close all running threads
	pthread_exit(nullptr);

	// Close socket
	close(serverFd);
	return 0;
}


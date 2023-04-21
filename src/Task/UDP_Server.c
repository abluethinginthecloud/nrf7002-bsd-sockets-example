/*! @file UDP_Server.c
 * @brief Implementation of a UDP Server
 *
 * @author A BLUE THING IN THE CLOUD S.L.U.
 *    ===  When the technology becomes art ===
 *
 * http://abluethinginthecloud.com
 * j.longares@abluethinginthecloud
 *
 * (c) A BLUE THING IN THE CLOUD S.L.U.
 *
 *
 *
        ██████████████    ██    ██    ██  ██████    ██████████████
        ██          ██      ████████████████  ██    ██          ██
        ██  ██████  ██  ██████  ██    ██        ██  ██  ██████  ██
        ██  ██████  ██    ██████    ██      ██      ██  ██████  ██
        ██  ██████  ██      ██      ████  ██████    ██  ██████  ██
        ██          ██    ██      ██████    ████    ██          ██
        ██████████████  ██  ██  ██  ██  ██  ██  ██  ██████████████
                        ██████  ████  ██████  ████
        ██████  ██████████  ████    ████████      ████      ██
        ██  ████  ██    ██  ████        ████    ████████  ██    ██
            ██  ██  ████  ██      ██      ██      ██  ████  ██████
        ████  ████    ██      ██          ████  ██  ██        ██
            ██████████          ██      ██    ██  ████    ██  ████
          ██  ████    ██      ██████    ██  ██████████    ██    ██
        ██  ████  ████████████████  ██    ██        ████████  ████
                ████        ██  ██████  ██████████      ████  ██
        ██████  ████████████████    ████  ██    ██████    ██  ████
            ████████  ██████  ██    ██████      ██        ████  ██
        ██    ██  ████████  ██    ██        ██    ██          ████
          ████  ████          ██      ████████████  ██  ████  ██
        ██  ██████  ████  ██    ██      ████    ██████████
                        ██    ██████    ██      ██      ██  ██████
        ██████████████  ██  ██████  ██  ████  ████  ██  ████  ████
        ██          ██  ██      ████████  ██    ██      ████  ████
        ██  ██████  ██  ████  ██    ██████      ██████████    ████
        ██  ██████  ██    ██████    ██  ██  ████      ████  ██████
        ██  ██████  ██  ████      ██    ████  ██        ████    ██
        ██          ██  ██    ██      ██████████████  ██      ██
        ██████████████  ██████  ██        ██  ████    ██████  ████



*/
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sta, CONFIG_LOG_DEFAULT_LEVEL);

#include <zephyr/kernel.h>
#include <errno.h>
#include <stdio.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <zephyr/net/socket.h>

#include "UDP_Server.h"
#include "deviceInformation.h"

//! Stack size for the UDP_SERVER thread
#define UDP_SERVER_STACK_SIZE 2*1024
//! UDP_SERVER thread priority level
#define UDP_SERVER_PRIORITY 7

//!UDP Server stack definition
K_THREAD_STACK_DEFINE(UDP_SERVER_STACK, UDP_SERVER_STACK_SIZE);
//! Variable to identify the UDP Server thread
static struct k_thread udpServerThread;
//! Receiver buffer
static uint8_t receiverBuffer[128];

/*! UDP_Server implements the UDP Client task.
* @brief UDP_Server uses a BSD socket to receive UDP connections from 
*        different clients, and echo their messages. 
*        When the clients send messages to the already known board IP 
*        address, the UDP_Server thread will re-send them their messages.  
*        This function is used on an independent thread.
*/
void UDP_Server(void) {   
    int udpServerSocket;
	struct sockaddr_in bindingAddress;
	int bindingResult;     
    struct sockaddr clientAddress;
    socklen_t clientAddressLength;
    int receivedBytes;
    int sentBytes = 0;
    uint8_t *pTransmitterBuffer;

    // Starve the thread until a DHCP IP is assigned to the board 
    while( !context.connected ){
		k_msleep( UDP_SERVER_SLEEP_TIME_MS );
	}

    // Server socket creation 
	( void )memset( &bindingAddress, 0, sizeof( bindingAddress ));
	bindingAddress.sin_family = AF_INET;
	bindingAddress.sin_port = htons( UDP_SERVER_PORT );

    udpServerSocket = socket(                                               \
                            bindingAddress.sin_family,                      \
                            SOCK_DGRAM,                                     \
                            IPPROTO_UDP);
    if ( udpServerSocket < 0 ) {
		LOG_ERR( "UDP Server error: socket: %d\n", errno );
		k_sleep( K_FOREVER );
	}

    // Binding 
    bindingResult = bind(                                                   \
                        udpServerSocket,                                    \
                        ( struct sockaddr * )&bindingAddress,               \
                        sizeof( bindingAddress ));
	if ( bindingResult < 0 ) {
		LOG_ERR( "UDP Server error: bind: %d\n", errno );
		k_sleep( K_FOREVER );
	}

    // Sending/receiving loop 
	while ( 1 ) {
		
        LOG_INF( "UDP Server waiting for UDP packets on port %d...",        \
            UDP_SERVER_PORT );

        do {
            // Receive 
            clientAddressLength = sizeof( clientAddress );
            receivedBytes = recvfrom(                                       \
                                    udpServerSocket,                        \
                                    receiverBuffer,                         \
                                    sizeof( receiverBuffer ),               \
                                    0,                                      \
                                    &clientAddress,                         \
                                    &clientAddressLength );                 \

            if ( receivedBytes <= 0 ) {
                if( receivedBytes < 0 ){
                    LOG_ERR( "UDP Server: Connection error %d", errno );
                    sentBytes = -errno;
                }
                break;
            } 

            // Send 
            pTransmitterBuffer = receiverBuffer;
            do{
                sentBytes = sendto(                                         \
                                    udpServerSocket,                        \
                                    receiverBuffer,                         \
                                    sizeof( receiverBuffer ),               \
                                    0,                                      \
                                    &clientAddress,                         \
                                    clientAddressLength );                  \
                if ( sentBytes < 0 ) {
                    LOG_ERR( "UDP Server: Failed to send %d", errno );
                    sentBytes = -errno;
                    break;
                
                } else {
                    pTransmitterBuffer += sentBytes;
                    receivedBytes -= sentBytes;
                }

                if ( sentBytes % 1000 == 0U ) {
                    LOG_INF( "UDP Server: Sent %u packets", sentBytes );
                }

                LOG_INF( "UDP Server mode: Received and replied with %d "   \
                        "bytes", sentBytes );
            } while ( receivedBytes );

	    } while ( true );
        if ( sentBytes < 0 ) {
            k_sleep( K_FOREVER );
        }
	}
}

/*! Task_UDP_Server_Init initializes the task UDP Server
*
* @brief UDP Server initialization
*/
void Task_UDP_Server_Init( void ){
	k_thread_create	(														\
					&udpServerThread,										\
					UDP_SERVER_STACK,										\
					UDP_SERVER_STACK_SIZE,									\
					(k_thread_entry_t)UDP_Server,							\
					NULL,													\
					NULL,													\
					NULL,													\
					UDP_SERVER_PRIORITY,									\
					0,														\
					K_NO_WAIT);	

	 k_thread_name_set(&udpServerThread, "udpServer");
	 k_thread_start(&udpServerThread);
}

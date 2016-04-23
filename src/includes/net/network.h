/*******************************************************************************
 File:network.h
 Description:Definitions of Socket Communication

*******************************************************************************/
#ifndef	__NETWORK_H__
#define	__NETWORK_H__

/*
================================================================================

	Prototype Statements

================================================================================
*/


/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_NET_MAX_HOST_NAME		1024

/*
================================================================================

	Management

================================================================================
*/

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:connectServer
	Input		:const char* hostname
				 < hostname to connect >
				 char *port_num
				 < port_num:if NULL, default port number is 443 >
	Output		:void
	Return		:int
				 < socket file descriptor >
	Description	:connect a server
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int connectServer( const char* hostname, char *port_num );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:sendMessage
	Input		:int soc
				 < socket descriptor >
				 const void *msg
				 < message to be sent >
				 int size
				 < size of a message >
	Output		:void
	Return		:int
				 < bytes to be sent >
	Description	:send a message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int sendMessage( int soc, const void *msg, int size );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:recvMessage
	Input		:int soc
				 < socket file descriptor >
				 void *buf
				 < buffer to be stored message >
				 int size
				 < size of a message to be received >
	Output		:void *buf
				 < received message >
	Return		:int
				 < size of received message >
	Description	:recive a message 
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int recvMessage( int soc, void *buf, int size );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:reconnectServer
	Input		:int sock_old
				 < socket file descriptor to reconnect >
	Output		:void
	Return		:int
				 < reconnected socket file descriptor >
	Description	:reconnect a server
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int reconnectServer( int sock_old );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:disconnectServer
	Input		:int soc
				 < socket discriptor to close >
	Output		:void
	Return		:int
	Description	:close socket and disconnect
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int disconnectServer( int soc );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:isServerConnected
	Input		:int soc
				 < socket discriptor to test >
	Output		:void
	Return		:int
				 < 0:connected -1:disconnectd >
	Description	:test whether connected to sever or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int isServerConnected( int soc );

#endif	//__NETWORK_H__

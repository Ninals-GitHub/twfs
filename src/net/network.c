/*******************************************************************************
 File:network.c
 Description:Procedures of Socket Communication

*******************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "net/network.h"

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
#define	DEF_NET_SEND_RETRY			3

/*
================================================================================

	Management

================================================================================
*/
struct reconnect_info
{
	int				ai_family;
	int				ai_socktype;
	int				ai_protocol;
	socklen_t		ai_addrlen;
	struct sockaddr	ai_addr;
};

static struct reconnect_info reconnect_info;

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
int connectServer( const char* hostname, char *port_num )
{
	int		result;
	int		soc			= -1;
	bool	spec_port	= false;

	struct addrinfo		hints, *res, *p;
	char				ipstr[ INET6_ADDRSTRLEN ];
	//char				def_port[ ] = "443";		// https
	char				def_port[ ] = "https";		// https

	/* ------------------------------------------------------------------------ */
	/* if port number is specified												*/
	/* ------------------------------------------------------------------------ */
	if( port_num )
	{
		//printf( "specified port number\n" );
		if( isdigit( port_num[ 0 ] ) )
		{
			if( ( sizeof( def_port ) - 1 ) < strlen( port_num ) )
			{
				/* nothing to do;use default port number			*/
			}
			else
			{
				/* whether specified port number can be converted	*/
				if( atoi( port_num ) <= 0 )
				{
					/* nothing to do; user default port number		*/
				}
				else
				{
					spec_port = true;
				}
			}
		}
		else
		{
			/* nothing to do;use default port number				*/
		}
	}

	/* ------------------------------------------------------------------------ */
	/* get address information													*/
	/* ------------------------------------------------------------------------ */
	memset( &hints, 0x00, sizeof( hints ) );
	hints.ai_family		= AF_UNSPEC;		// AF_INET or AF_INT6
	hints.ai_socktype	= SOCK_STREAM;


	if( spec_port )
	{
		/* get address information of specified port number		*/
		result = getaddrinfo( hostname, port_num, &hints, &res );
		//printf( "port_number:%s\n", port_num );
	}
	else
	{
		/* get address information of default port number		*/
		result = getaddrinfo( hostname, def_port, &hints, &res );
		//printf( "port_number:%s\n", def_port);
	}
	
	if( result != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( result ) );
		return( -1 );
	}

	//printf( "%s\n", hostname );

	/* ------------------------------------------------------------------------ */
	/* try to connect each server												*/
	/* ------------------------------------------------------------------------ */
	for( p = res ; p != NULL ; p = p->ai_next )
	{

		void	*addr;
		char	*ipver;

		/* -------------------------------------------------------------------- */
		/* get the pointer to the address itself								*/
		/* -------------------------------------------------------------------- */
		if( p->ai_family == AF_INET )
		{
			/* for ipv4					*/
			struct sockaddr_in	*ipv4 = ( struct sockaddr_in* )p->ai_addr;
			addr	= &( ipv4->sin_addr );
			ipver	= "IPv4";
			//printf( "connected port=%d\n", ntohs( ipv4->sin_port ) );
		}
		else
		{
			/* for ipv6					*/
			struct sockaddr_in6	*ipv6 = ( struct sockaddr_in6* )p->ai_addr;
			addr	= &( ipv6->sin6_addr );
			ipver	= "IPv6";
		}
		/* -------------------------------------------------------------------- */
		/* convert the ip to a string and print it								*/
		/* -------------------------------------------------------------------- */
		inet_ntop( p->ai_family, addr, ipstr, sizeof( ipstr ) );
		//printf( "  %s: %s\n", ipver, ipstr );

		/* -------------------------------------------------------------------- */
		/* create a socket														*/
		/* -------------------------------------------------------------------- */
		soc = socket( p->ai_family, p->ai_socktype, p->ai_protocol );

		if( soc < 0 )
		{
			if( !p->ai_next )
			{
				perror( "socket" );
				freeaddrinfo( res );
				return( soc );
			}

			//printf( "soc:try next\n" );
			/* ---------------------------------------------------------------- */
			/* try next server													*/
			/* ---------------------------------------------------------------- */
			continue;
		}

		//printf( "sock fd = %d\n", soc );

		/* -------------------------------------------------------------------- */
		/* connect to a server													*/
		/* -------------------------------------------------------------------- */
		result = connect( soc, p->ai_addr, p->ai_addrlen );
		if( result < 0 )
		{
			close( soc );

			if( !p->ai_next )
			{
				perror( "connect" );
				freeaddrinfo( res );
				return( result );
			}
			/* ---------------------------------------------------------------- */
			/* try next server													*/
			/* ---------------------------------------------------------------- */
			//printf( "connect:try next\n" );
			continue;
		}

		//printf( "connection complete\n" );

		/* -------------------------------------------------------------------- */
		/* save reconnection information										*/
		/* -------------------------------------------------------------------- */
		reconnect_info.ai_family 	= p->ai_family;
		reconnect_info.ai_socktype	= p->ai_socktype;
		reconnect_info.ai_protocol	= p->ai_protocol;
		reconnect_info.ai_addrlen	= p->ai_addrlen;

		memcpy( ( void* )&reconnect_info.ai_addr,
				( void* )p->ai_addr,
				sizeof( struct sockaddr ) );

		/* -------------------------------------------------------------------- */
		/* connection completed													*/
		/* -------------------------------------------------------------------- */
		break;
	}

	freeaddrinfo( res );

	return( soc );
}

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
int reconnectServer( int sock_old )
{
	int		result;
	int		new_soc;

#if 1

	new_soc = socket( reconnect_info.ai_family,
					   reconnect_info.ai_socktype,
					   reconnect_info.ai_protocol );
	
	if( new_soc < 0 )
	{
		return( new_soc );
	}
	result = connect( new_soc,
					  &reconnect_info.ai_addr,
					  reconnect_info.ai_addrlen );
#endif
	
	if( result < 0 )
	{
		return( result );
	}

	return( new_soc );
}

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
int sendMessage( int soc, const void *msg, int size )
{
	int		length;
	int		retry;
	int		org_size;
	char	rbuf[ 1 ];

	org_size = size;

#if 1
	/* ------------------------------------------------------------------------ */
	/* check whether connection is disconnected or not							*/
	/* ------------------------------------------------------------------------ */
	length = recv( soc, rbuf, sizeof( rbuf ), MSG_DONTWAIT );

	/* ------------------------------------------------------------------------ */
	/* connection is maintained													*/
	/* ------------------------------------------------------------------------ */
	if( length == -1 && errno == EAGAIN )
	{
		length = 0;

		/* -------------------------------------------------------------------- */
		/* send and retry														*/
		/* -------------------------------------------------------------------- */
		for( retry = 0 ; retry < DEF_NET_SEND_RETRY ; retry++ )
		{
			length = send( soc, &msg[ length ], size, 0 );

			if( length < 0 )
			{
				perror( "send" );

				return( length );
			}

			if( length == size )
			{
				return( org_size );
			}

			size = size - length;
		}

		return( -1 );
	}
	/* ------------------------------------------------------------------------ */
	/* connection is disconnected												*/
	/* ------------------------------------------------------------------------ */
	else
	{
		perror( "send:connection is disconnected" );

		return( -1 );
	}

	return( length );

#elif
	return(1);
#endif

}

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
int recvMessage( int soc, void *buf, int size )
{
	int		length;

	//printf( "--------------\n" );

	length = recv( soc, buf, size, 0 );

	//printf( "OK\n" );

	if( length < 0 )
	{
		perror( "recv" );
	}

	return( length );
}

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
int disconnectServer( int soc )
{
	if( soc != 0 )
	{
		return( close( soc ) );
	}

	return( 0 );
}

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
int isServerConnected( int soc )
{
	int		result;
	char	rbuf[ 1 ];

	result = recv( soc, rbuf, sizeof( rbuf ), MSG_DONTWAIT );

	if( result == -1 && errno == EAGAIN )
	{
		return( 0 );
	}

	return( -1 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
================================================================================
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:void
================================================================================
*/

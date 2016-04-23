/*******************************************************************************
 File:twitter_api.h
 Description:Definitions of API for Twitter

*******************************************************************************/
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lib/ascii.h"
#include "net/ssl.h"
#include "net/http.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int skipSpace( unsigned char *buffer, int *index, int max_size );
static int skipToSpace( unsigned char *buffer, int *index, int max_size );
void analyzeStatusCode( struct http_ctx *hctx, unsigned char *buffer );


/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_HTTP_MAX_LINE		1024

/*
================================================================================

	Management

================================================================================
*/
typedef enum
{
	E_HIS_HTTP_VER,
	E_HIS_HTTP_HEADER,
}E_HTTP_INTERP_STATE;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initHttpContext
	Input		:struct http_ctx *hctx
				 < initializee >
	Output		:struct http_ctx *hctx
				 < initializee >
	Return		:int
				 < status >
	Description	:intialize http context
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initHttpContext( struct http_ctx *hctx )
{
	if( !hctx )
	{
		return( -1 );
	}
	hctx->status_code	= 0;
	hctx->content_length= 0;

	return( 0 );
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:recvHttpHeader
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < context of http received data >
	Output		:struct http_ctx *hctx
				 < first body data block may be stored >
	Return		:int
				 < status >
	Description	:receive headers of http and interpret them
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int recvHttpHeader( struct ssl_session *session, struct http_ctx *hctx )
{
	int					result;
	int					index;
	int					len;
	bool				finish;
	unsigned char		head_buffer[ DEF_HTTP_MAX_LINE ];
	E_HTTP_INTERP_STATE	state;

	state			= E_HIS_HTTP_VER;
	finish			= false;

	while( 1 )
	{
		len = 0;

		while( len < DEF_HTTP_MAX_LINE )
		{
			result = recvSSLMessage( session, &head_buffer[ len ], 1 );

			if( result < 0 )
			{
				if( E_HIS_HTTP_HEADER <= state )
				{
					return( 0 );
				}
				printf("unexpected header:%d %d\n", len, result );
				return( -1 );
			}

			if( ( sizeof( DEF_HTTPH_DELIMITOR ) - 1 - 1 ) <= len )
			{
				if( ( head_buffer[ len     ] == '\n' ) &&
					( head_buffer[ len - 1 ] == '\r' ) )
				{
					/* -------------------------------------------------------- */
					/* we found double \r\n										*/
					/* -------------------------------------------------------- */
					if( finish )
					{
						/* reading headers is finished							*/
						return( 0 );
					}
					/* set null terminator										*/
					head_buffer[ len - 1 ] = '\0';
					/* set flag to secoud \r\n									*/
					finish = true;
					break;
				}
				else
				{
					if( head_buffer[ len ] != 'r' )
					{
						finish = false;
					}
				}
			}
			len++;
		}

		printf( "%s\r\n", head_buffer );

		if( DEF_HTTP_MAX_LINE <= len )
		{
			/* too large headers												*/
			return( -1 );
		}

		/* -------------------------------------------------------------------- */
		/* analyze headers														*/
		/* -------------------------------------------------------------------- */
		switch( state )
		{
		/* -------------------------------------------------------------------- */
		/* interpret "HTTP/1.1 status code"										*/
		/* -------------------------------------------------------------------- */
		case	E_HIS_HTTP_VER:
			if( strnCaseCmp( DEF_HTTPH_HTTP, ( const char* )head_buffer,
							 sizeof( DEF_HTTPH_HTTP ) - 1 ) == 0 )
			{
				index = sizeof( DEF_HTTPH_HTTP ) - 1;
				result = skipToSpace( head_buffer, &index, len );
				if( result < 0 )
				{
					/* ilegal headers											*/
					return( -1 );
				}

				result = skipSpace( head_buffer, &index, len );
				if( result < 0 )
				{
					/* ilegal headers											*/
					return( -1 );
				}

				analyzeStatusCode( hctx, &head_buffer[ index ] );
				
				if( hctx->status_code == DEF_HTTPH_STATUS_CONTINUE )
				{
					break;
				}
				else
				{
					state = E_HIS_HTTP_HEADER;
				}
			}
			else
			{
				/* ilegal header												*/
				return( -1 );
			}
			break;
		/* -------------------------------------------------------------------- */
		/* interpret Content-Length												*/
		/* -------------------------------------------------------------------- */
		case	E_HIS_HTTP_HEADER:
			if( strnCaseCmp( DEF_HTTPH_CONTENT_LENGTH, ( const char* )head_buffer,
							 sizeof( DEF_HTTPH_CONTENT_LENGTH ) - 1 ) == 0 )
			{
				index = sizeof( DEF_HTTPH_CONTENT_LENGTH );
				result = skipSpace( head_buffer, &index, len );
				if( result < 0 )
				{
					/* ilegal headers											*/
					printf( "ilegal 4\n" );
					return( -1 );
				}
				hctx->content_length = atoi( ( const char* )&head_buffer[ index ] );
			}
			break;
		}
	}
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
	Function	:analyzeStatusCode
	Input		:unsigned char *buffer
				 < buffer to be analized >
	Output		:struct http_ctx *hctx
				 < updated status code of hctx >
	Return		:void
	Description	:analyze http status code
================================================================================
*/
void analyzeStatusCode( struct http_ctx *hctx, unsigned char *buffer )
{
	char	first, second, third;

	first	= buffer[ 0 ];
	second	= buffer[ 1 ];
	third	= buffer[ 2 ];

	switch( first )
	{
	case	'1':
		if( second != '0' )
		{
			hctx->status_code = 100;
			break;
		}
		switch( third )
		{
		case	'0':
			hctx->status_code = 100;
			break;
		case	'1':
			hctx->status_code = 101;
			break;
		default:
			hctx->status_code = 100;
			break;
		}
		break;
	case	'2':
		if( second != '0' )
		{
			hctx->status_code = 200;
		}
		switch( third )
		{
		case	'0': hctx->status_code = 200; break;
		case	'1': hctx->status_code = 201; break;
		case	'2': hctx->status_code = 202; break;
		case	'3': hctx->status_code = 203; break;
		case	'4': hctx->status_code = 204; break;
		case	'5': hctx->status_code = 205; break;
		case	'6': hctx->status_code = 206; break;
		default:     hctx->status_code = 200; break;
		}
		break;
	case	'3':
		if( second != '0' )
		{
			hctx->status_code = 300;
			break;
		}
		switch( third )
		{
		case	'0': hctx->status_code = 300; break;
		case	'1': hctx->status_code = 301; break;
		case	'2': hctx->status_code = 302; break;
		case	'3': hctx->status_code = 303; break;
		case	'4': hctx->status_code = 304; break;
		case	'5': hctx->status_code = 305; break;
		case	'6': hctx->status_code = 306; break;
		case	'7': hctx->status_code = 307; break;
		default:     hctx->status_code = 300; break;
		}
		break;
	case	'4':
		if( second != '0' && second != '1' )
		{
			hctx->status_code = 400;
			break;
		}

		if( second == '0' )
		{
			switch( third )
			{
			case	'0': hctx->status_code = 400; break;
			case	'1': hctx->status_code = 401; break;
			case	'2': hctx->status_code = 402; break;
			case	'3': hctx->status_code = 403; break;
			case	'4': hctx->status_code = 404; break;
			case	'5': hctx->status_code = 405; break;
			case	'6': hctx->status_code = 406; break;
			case	'7': hctx->status_code = 407; break;
			case	'8': hctx->status_code = 408; break;
			case	'9': hctx->status_code = 409; break;
			default:     hctx->status_code = 400; break;
			}
		}
		else if( second == '1' )
		{
			switch( third )
			{
			case	'0': hctx->status_code = 410; break;
			case	'1': hctx->status_code = 411; break;
			case	'2': hctx->status_code = 412; break;
			case	'3': hctx->status_code = 413; break;
			case	'4': hctx->status_code = 414; break;
			case	'5': hctx->status_code = 415; break;
			case	'6': hctx->status_code = 416; break;
			case	'7': hctx->status_code = 417; break;
			default:     hctx->status_code = 400; break;
			}
		}
		break;
	case	'5':
		if( second != '0' )
		{
			hctx->status_code = 500;
			break;
		}
		switch( third )
		{
		case	'0': hctx->status_code = 500; break;
		case	'1': hctx->status_code = 501; break;
		case	'2': hctx->status_code = 502; break;
		case	'3': hctx->status_code = 503; break;
		case	'4': hctx->status_code = 504; break;
		case	'5': hctx->status_code = 505; break;
		default:     hctx->status_code = 500; break;
		break;
		}
	default:
		hctx->status_code = 0;
		break;
	}
}
/*
================================================================================
	Function	:skipSpace
	Input		:unsigned char *buffer
				 < buffer to skip space >
				 int *index
				 < current index of buffer >
				 int max_size
				 < max size of buffer >
	Output		:int *index
				 < updated buffer index >
	Return		:int
				 < -1:reach to the end of buffer 0: within a buffer >
	Description	:skip a space
================================================================================
*/
static int skipSpace( unsigned char *buffer, int *index, int max_size )
{
	while( 1 )
	{
		if( max_size <= *index )
		{
			*index = 0;
			return( -1 );
		}
		if( buffer[ *index ] == ' ' )
		{
			( *index )++;
			continue;
		}

		break;
	}

	return( 0 );
}

/*
================================================================================
	Function	:skipToSpace
	Input		:unsigned char *buffer
				 < buffer to skip space >
				 int *index
				 < current index of buffer >
				 int max_size
				 < max size of buffer >
	Output		:int *index
				 < updated buffer index >
	Return		:int
				 < -1:reach to the end of buffer 0: within a buffer >
	Description	:skip until found a space
================================================================================
*/
static int skipToSpace( unsigned char *buffer, int *index, int max_size )
{
	while( 1 )
	{
		if( max_size <= *index )
		{
			*index = 0;
			return( -1 );
		}
		if( buffer[ *index ] != ' ' )
		{
			( *index )++;
			continue;
		}

		break;
	}

	return( 0 );
}

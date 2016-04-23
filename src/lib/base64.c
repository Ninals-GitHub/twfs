/*******************************************************************************
 File:twitter_api.h
 Description:Definitions of API for Twitter

*******************************************************************************/
#include <stdbool.h>

#include "lib/ascii.h"

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

/*
================================================================================

	Management

================================================================================
*/
static char base64_table[ ] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
	'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b',
	'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/',
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeBase64
	Input		:const char* org
				 < original message >
				 int size
				 < size of original message >
				 char *buf
				 < encoded message >
	Output		:char *buf
				 < encoded message >
	Return		:int
				 < length of enceoded message >
	Description	:encode base 64
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeBase64( const unsigned char* org, int size, char *buf )
{
	int		i;
	int		bit_length;
	int		bit_index;
	int		buf_i;
	int		org_i;

	if( !org || !buf )
	{
		return( -1 );
	}

	if( size <= 0 )
	{
		return( -1 );
	}

	bit_index	= 0;
	bit_length	= ( size - 1 ) * 8;

	if( bit_length % 6 )
	{
		bit_length += 6 - ( bit_length % 6 );
	}

	buf_i		= 0;
	org_i		= 0;

	while( bit_index < bit_length )
	{
		buf[ buf_i++ ] = ( org[ org_i++ ] >> 2 ) & 0x3F;
		bit_index += 6;
		if( bit_index >= bit_length )
		{
			buf[ buf_i++ ] = ( org[ org_i - 1 ] << 4 ) & 0x3F;
			break;
		}
		buf[ buf_i ] = ( org[ org_i - 1 ] << 4 ) & 0x3F;
		buf[ buf_i++ ] |= ( org[ org_i ] >> 4 ) & 0x3F;
		bit_index += 6;
		
		buf[ buf_i ] = ( org[ org_i++ ] << 2 ) & 0x3F;
		if( bit_index >= bit_length )
		{
			buf_i++;
			break;
		}
		buf[ buf_i++ ] |= ( org[ org_i ] >> 6 ) & 0x3F;
		bit_index += 6;

		buf[ buf_i++ ] = org[ org_i++ ] & 0x3F;
		bit_index += 6;
		if( bit_index >= bit_length )
		{
			break;
		}
	}

	for( i = 0 ; i < buf_i ; i++ )
	{
		buf[ i ] = base64_table[ ( int )buf[ i ] ];
	}

	if( buf_i % 4 )
	{
		for( i = buf_i ; i < buf_i + ( 4 - ( buf_i % 4 ) ) ; i++ )
		{
			buf[ i ] = '=';
		}

		buf_i += 4 - ( buf_i % 4 );
	}

	buf[ buf_i ] = 0x00;

	return( buf_i );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:decodeBase64
	Input		: char* base64
				 < base64 message >
				 int size
				 < size of base64 message >
				 char *buf
				 < decoded message >
	Output		:char *buf
				 < decoded message >
	Return		:int
				 < length of decoded message >
	Description	:decode base 64
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int decodeBase64( char* base64, int size )
{
	int				i;
	int				d_pos;		// write position of decoded message
	int				bit_len;
	unsigned char	*decoded;	// decoded message

	if( !base64 )
	{
		return( 0 );
	}

	if( size <= 0 )
	{
		base64[ 0 ] = '\0';
		return( 0 );
	}

	i		= 0;
	d_pos	= 0;
	bit_len	= 0;
	decoded	= ( unsigned char* )base64;

	while( size-- )
	{
		unsigned char new;

		if( ( base64[ i ] == '=' ) || ( base64[ i ] == '\0' ) )
		{
			break;
		}

		if( 'A' <= base64[ i ] && base64[ i ] <= 'Z' )
		{
			new = base64[ i ] - 'A';
		}
		else if( 'a' <= base64[ i ] && base64[ i ] <= 'z' )
		{
			new = base64[ i ] - 'a' + ( 'Z' - 'A' + 1 );
		}
		else if( '0' <= base64[ i ] && base64[ i ] <= '9' )
		{
			new = base64[ i ] - '0' + ( 'Z' - 'A' + 1 ) * 2;
		}
		else if( base64[ i ] == '+' )
		{
			new = 0x3E;
		}
		else if( base64[ i ] == '/' )
		{
			new = 0x3F;
		}
		else
		{
			new = 0xFF;
		}

		i++;
		bit_len += 6;

		if( bit_len <= 6 )
		{
			decoded[ d_pos ] = new << 2;
		}
		else if( bit_len <= 12 )
		{
			decoded[ d_pos ] |= new >> 4;
			d_pos++;
			decoded[ d_pos ] = ( new & 0x0F ) << 4;
		}
		else if( bit_len <= 18 )
		{
			decoded[ d_pos ] |= ( new >> 2 );
			d_pos++;
			decoded[ d_pos ] = ( new & 0x03 ) << 6;
		}
		else
		{
			decoded[ d_pos ] |= new;
			d_pos++;
			bit_len = 0;
		}
	}

	decoded[ d_pos ] = '\0';

	return( d_pos );
}
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

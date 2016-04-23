/*******************************************************************************
 File:md5.c
 Description:Operations of MD5 algorightm

*******************************************************************************/
#include <stdio.h>

#include "lib/md5.h"
#include "lib/ascii.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static inline unsigned int
md5_F( unsigned int x, unsigned int y, unsigned int z );
static inline unsigned int
md5_G( unsigned int x, unsigned int y, unsigned int z );
static inline unsigned int
md5_H( unsigned int x, unsigned int y, unsigned int z );
static inline unsigned int
md5_I( unsigned int x, unsigned int y, unsigned int z );
static inline unsigned int circularShift( int bit_num, unsigned int word );
static void encodeWord( const unsigned char *m_word,
						unsigned int *H0,
						unsigned int *H1,
						unsigned int *H2,
						unsigned int *H3 );


/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_MD5_MESSAGE_LENGTH_BYTES	8
#define	DEF_MD5_EXPAND_TH				( DEF_MD5_UNIT_OF_ENCODE -				\
										  DEF_MD5_MESSAGE_LENGTH_BYTES )
#define	DEF_MD5_DECODE_LENGTH			16


/*
================================================================================

	Management

================================================================================
*/

const unsigned int k[ ] =
{
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

const unsigned int r[ ] =
{
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initMd5
	Input		:struct md5_ctx *context
				 < context of md5 >
	Output		:struct md5_ctx *context
				 < initialized context >
	Return		:void
	Description	:initialize hash parameters of md5
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initMd5( struct md5_ctx *context )
{
	context->digest[ 0 ] = 0x67452301;
	context->digest[ 1 ] = 0xEFCDAB89;
	context->digest[ 2 ] = 0x98BADCFE;
	context->digest[ 3 ] = 0x10325476;

	context->size = 0;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:updateMd5
	Input		:struct md5_ctx *context
				 < context of md5 >
				 const unsigned char *message
				 < original message >
				 int size
				 < size of originial message >
	Output		:strcut md5_ctx *context
				 < updated contex >
	Return		:int
				 < result of parameter check >
	Description	:update digests
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int updateMd5( struct md5_ctx *context,
			   const unsigned char *message,
			   int size )
{
	int		i, j;
	int		index;
	int		part_len;

	if( !message )
	{
		return( -1 );
	}

	if( size <= 0 )
	{
		return( -1 );
	}

	if( !context )
	{
		return( -1 );
	}

	index		= context->size % DEF_MD5_UNIT_OF_ENCODE;
	part_len	= DEF_MD5_UNIT_OF_ENCODE - index;

	if( part_len <= size )
	{
		for( i = 0 ; i < part_len ; i++ )
		{
			context->buffer[ index + i ] = message[ i ];
		}

		encodeWord( context->buffer,
					&context->digest[ 0 ],
					&context->digest[ 1 ],
					&context->digest[ 2 ],
					&context->digest[ 3 ] );

		for( i = part_len ;
			 ( i + ( DEF_MD5_UNIT_OF_ENCODE - 1 ) ) < size ;
			 i += DEF_MD5_UNIT_OF_ENCODE )
		{
			encodeWord( &message[ i ],
						&context->digest[ 0 ],
						&context->digest[ 1 ],
						&context->digest[ 2 ],
						&context->digest[ 3 ] );
		}

		index = 0;
	}
	else
	{
		i = 0;
	}

	for( j = 0 ; j < ( size - i ) ; j++ )
	{
		context->buffer[ index + j ] = message[ i + j ];
	}

	context->size += size;

	return( 0 );
#if 0
	/* ------------------------------------------------------------------------ */
	/* encode a word except for last block										*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ; i < ( size / DEF_MD5_UNIT_OF_ENCODE ) ; i++ )
	{
		encodeWord( &message[ i * DEF_MD5_UNIT_OF_ENCODE ],
					&i_digest[ 0 ],
					&i_digest[ 1 ],
					&i_digest[ 2 ],
					&i_digest[ 3 ] );
	}

	mod_msg = size % DEF_MD5_UNIT_OF_ENCODE;

	/* ------------------------------------------------------------------------ */
	/* prepare for expand message												*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ; i < mod_msg ; i++ )
	{
		last_block[ i ] = message[ size - mod_msg + i ];
	}

	/* set 0x80 as last index				*/
	last_index = i;
	if( mod_msg < DEF_MD5_EXPAND_TH )
	{
		last_block[ last_index++ ] = 0x80;
	}

	for( i = last_index ; i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
	{
		last_block[ i ] = 0x00;
	}

	/* for expand							*/
	if( DEF_MD5_EXPAND_TH <= mod_msg )
	{
		last_block[ mod_msg ] = 0x80;
		/* encode last block - 1			*/
		encodeWord( last_block,
					&i_digest[ 0 ],
					&i_digest[ 1 ],
					&i_digest[ 2 ],
					&i_digest[ 3 ] );

		/* for actual last block			*/
		for( i = 0 ; i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
		{
			last_block[ i ] = 0x00;
		}
	}

	/* calculate bit size					*/
	size *= 8;

	last_block[ sizeof( last_block ) - 8 ] = ( unsigned char )( size >>  0 );
	last_block[ sizeof( last_block ) - 7 ] = ( unsigned char )( size >>  8 );
	last_block[ sizeof( last_block ) - 6 ] = ( unsigned char )( size >> 16 );
	last_block[ sizeof( last_block ) - 5 ] = ( unsigned char )( size >> 24 );

	/* ------------------------------------------------------------------------ */
	/* encode last block														*/
	/* ------------------------------------------------------------------------ */
	encodeWord( last_block,
				&i_digest[ 0 ],
				&i_digest[ 1 ],
				&i_digest[ 2 ],
				&i_digest[ 3 ] );
	
	return( 0 );
#endif
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:finishMd5
	Input		:struct md5_ctx *context
				 < context of md5 >
				 unsigned char *hash
				 < buffer of digest >
	Output		:unsigned char *hash
				 < buffer to be stored digest >
	Return		:int
				 < lingth of the digest >
	Description	:output the result of digest
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int finishMd5( struct md5_ctx *context, unsigned char *hash )
{
	int				i;
	int				last_index;
	int				mod_msg;
	unsigned int	size;

	last_index	= context->size % DEF_MD5_UNIT_OF_ENCODE;
	mod_msg		= last_index;

	/* set 0x80 at last index				*/
	if( last_index < DEF_MD5_EXPAND_TH )
	{
		context->buffer[ last_index++ ] = 0x80;
	}

	for( i = last_index ;i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
	{
		context->buffer[ i ] = 0x00;
	}

	/* for expand							*/
	if( DEF_MD5_EXPAND_TH <= mod_msg )
	{
		context->buffer[ mod_msg ] = 0x80;
		/* encode last block - 1			*/
		encodeWord( context->buffer,
					&context->digest[ 0 ],
					&context->digest[ 1 ],
					&context->digest[ 2 ],
					&context->digest[ 3 ] );

		/* for actual last block			*/
		for( i = 0 ; i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
		{
			context->buffer[ i ] = 0x00;
		}
	}

	/* calculate bit size					*/
	size = context->size * 8;

	context->buffer[ sizeof( context->buffer ) - 8 ] = ( unsigned char )( size >>  0 );
	context->buffer[ sizeof( context->buffer ) - 7 ] = ( unsigned char )( size >>  8 );
	context->buffer[ sizeof( context->buffer ) - 6 ] = ( unsigned char )( size >> 16 );
	context->buffer[ sizeof( context->buffer ) - 5 ] = ( unsigned char )( size >> 24 );

	/* ------------------------------------------------------------------------ */
	/* encode last block														*/
	/* ------------------------------------------------------------------------ */
	encodeWord( context->buffer,
				&context->digest[ 0 ],
				&context->digest[ 1 ],
				&context->digest[ 2 ],
				&context->digest[ 3 ] );

	for( i = 0 ; i < DEF_MD5_DIGEST_LENGTH ; i += 4 )
	{
		hash[ i + 0 ] = ( context->digest[ i >> 2 ] & 0x000000FF ) >>  0;
		hash[ i + 1 ] = ( context->digest[ i >> 2 ] & 0x0000FF00 ) >>  8;
		hash[ i + 2 ] = ( context->digest[ i >> 2 ] & 0x00FF0000 ) >> 16;
		hash[ i + 3 ] = ( context->digest[ i >> 2 ] & 0xFF000000 ) >> 24;
	}

	return( DEF_MD5_DIGEST_LENGTH );
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeMd5
	Input		:const char *message
				 < original message >
				 int size
				 < size of original message >
				 unsigned char *hash
				 < sha1 hash value string buffer >
	Output		:unsinged char *hash
				 < result of sha1 hash value string >
	Return		:int
				 < length of hash >
	Description	:encode one message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeMd5( const unsigned char *message, int size, unsigned char *hash )
{
	struct md5_ctx	context;

	if( !message )
	{
		return( -1 );
	}

	if( !hash )
	{
		return( -1 );
	}

	if( size <= 0 )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* intialize sha1															*/
	/* ------------------------------------------------------------------------ */
	initMd5( &context );


	/* ------------------------------------------------------------------------ */
	/* encode sha1																*/
	/* ------------------------------------------------------------------------ */
	if( updateMd5( &context, message, size ) < 0 )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* save result																*/
	/* ------------------------------------------------------------------------ */
	return( finishMd5( &context, hash ) );
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
	Function	:md5_F
	Input		:unsigned int x
				 unsigned int y
				 unsigned int z
	Output		:void
	Return		:unsigned int
	Description	:md5 supplement function F
================================================================================
*/
static inline unsigned int
md5_F( unsigned int x, unsigned int y, unsigned int z )
{
	return( ( x & y ) | ( ~x & z ) );
}

/*
================================================================================
	Function	:md5_G
	Input		:unsigned int x
				 unsigned int y
				 unsigned int z
	Output		:void
	Return		:unsigned int
	Description	:md5 supplement function G
================================================================================
*/
static inline unsigned int
md5_G( unsigned int x, unsigned int y, unsigned int z )
{
	return( ( x & z ) | ( y & ~z ) );
}

/*
================================================================================
	Function	:md5_H
	Input		:unsigned int x
				 unsigned int y
				 unsigned int z
	Output		:void
	Return		:unsigned int
	Description	:md5 supplement function H
================================================================================
*/
static inline unsigned int
md5_H( unsigned int x, unsigned int y, unsigned int z )
{
	return( x ^ y ^ z );
}

/*
================================================================================
	Function	:md5_I
	Input		:unsigned int x
				 unsigned int y
				 unsigned int z
	Output		:void
	Return		:unsigned int
	Description	:md5 supplement function I
================================================================================
*/
static inline unsigned int
md5_I( unsigned int x, unsigned int y, unsigned int z )
{
	return( y ^ ( x | ~z ) );
}

/*
================================================================================
	Function	:circularShift
	Input		:int bit_num
				 < bit number >
				 unsigned int
				 < 32 bit integer to be shifted >
	Output		:void
	Return		:unsigned int
				 < cirular shifted 32 bit value >
	Description	:circlur shift 32 bit value
================================================================================
*/
static inline unsigned int circularShift( int bit_num, unsigned int word )
{
	return( ( word << bit_num ) | ( word >> ( 32 - bit_num ) ) );
}
/*
================================================================================
	Function	:encodeWord
	Input		:const unsigned char *m_word
				 < 64 byte word of message >
				 unsigned int *A
				 < intermediate hash value >
				 unsigned int *B
				 < intermediate hash value >
				 unsigned int *C
				 < intermediate hash value >
				 unsigned int *D
				 < intermediate hash value >
	Output		:unsigned int *A, *B, *C, *D
	Return		:void
	Description	:encode a 64 byte word of message
================================================================================
*/
static void encodeWord( const unsigned char *m_word,
						unsigned int *A,
						unsigned int *B,
						unsigned int *C,
						unsigned int *D )
{
	unsigned int	rA, rB, rC, rD, f;
	unsigned int	TEMP;
	unsigned int	X[ DEF_MD5_DECODE_LENGTH ];
	int				i, g;
	int				index;

	rA = *A; rB = *B; rC = *C; rD = *D;

	/* ------------------------------------------------------------------------ */
	/* decode a block of message												*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ;
		 i < DEF_MD5_UNIT_OF_ENCODE ;
		 i += ( DEF_MD5_UNIT_OF_ENCODE / DEF_MD5_DECODE_LENGTH ) )
	{
		index = i / ( DEF_MD5_UNIT_OF_ENCODE / DEF_MD5_DECODE_LENGTH );
		X[ index ]  = ( ( unsigned int )m_word[ i + 0 ] <<  0 ) & 0x000000FF;
		X[ index ] |= ( ( unsigned int )m_word[ i + 1 ] <<  8 ) & 0x0000FF00;
		X[ index ] |= ( ( unsigned int )m_word[ i + 2 ] << 16 ) & 0x00FF0000;
		X[ index ] |= ( ( unsigned int )m_word[ i + 3 ] << 24 ) & 0xFF000000;
	}

	/* ------------------------------------------------------------------------ */
	/* encode																	*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ; i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
	{
		if( i < 16 )
		{
			f = md5_F( rB, rC, rD );
			g = i;
		}
		else if( i < 32 )
		{
			f = md5_G( rB, rC, rD );
			g = ( 5 * i + 1 ) % 16;
		}
		else if( i < 48 )
		{
			f = md5_H( rB, rC, rD );
			g = ( 3 * i + 5 ) % 16;
		}
		else
		{
			f = md5_I( rB, rC, rD );
			g = ( 7 * i ) % 16;
		}

		TEMP = rD;
		rD = rC;
		rC = rB;
		rB = rB + circularShift( r[ i ], rA + f + k[ i ] + X[ g ] );
		rA = TEMP;
	}

	*A += rA; *B += rB; *C += rC; *D += rD;

}
/*
================================================================================
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:void
================================================================================
*/

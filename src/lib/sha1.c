/*******************************************************************************
 File:twitter_api.h
 Description:Definitions of API for Twitter

*******************************************************************************/
#include <stdio.h>

#include "lib/sha1.h"
#include "lib/ascii.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static inline unsigned int
ft00_t19( unsigned int B, unsigned int C, unsigned int D );
static inline unsigned int
ft20_t39( unsigned int B, unsigned int C, unsigned int D );
static inline unsigned int
ft40_t59( unsigned int B, unsigned int C, unsigned int D );
static inline unsigned int
ft60_t79( unsigned int B, unsigned int C, unsigned int D );
static inline unsigned int circularShift( int bit_num, unsigned int word );
static void encodeWord( const unsigned char *m_word,
						unsigned int *H0,
						unsigned int *H1,
						unsigned int *H2,
						unsigned int *H3,
						unsigned int *H4 );


/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_SHA1_MESSAGE_LENGTH_BYTES	8
#define	DEF_SHA1_EXPAND_TH				( DEF_SHA1_UNIT_OF_ENCODE -				\
										  DEF_SHA1_MESSAGE_LENGTH_BYTES )

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
	Function	:initSha1
	Input		:struct sha1_ctx *context
	Output		:void
	Return		:void
	Description	:initialize hash parameters of sha1
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initSha1( struct sha1_ctx *context )
{
	context->digest[ 0 ] = 0x67452301;
	context->digest[ 1 ] = 0xEFCDAB89;
	context->digest[ 2 ] = 0x98BADCFE;
	context->digest[ 3 ] = 0x10325476;
	context->digest[ 4 ] = 0xC3D2E1F0;

	context->size = 0;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:updateSha1
	Input		:struct sha1_ctx *context
				 < sha1 context >
				 const unsigned char *message
				 < original message >
				 int size
				 < size of originial message >
	Output		:void
	Return		:int
				 < result of parameter check >
	Description	:update digests
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int updateSha1( struct sha1_ctx *context,
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

	index		=  context->size % DEF_SHA1_UNIT_OF_ENCODE;
	part_len	= DEF_SHA1_UNIT_OF_ENCODE - index;
	
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
					&context->digest[ 3 ],
					&context->digest[ 4 ] );

		for( i = part_len ;
			 ( i + ( DEF_SHA1_UNIT_OF_ENCODE - 1 ) ) < size ;
			 i += DEF_SHA1_UNIT_OF_ENCODE )
		{
			encodeWord( &message[ i ],
						&context->digest[ 0 ],
						&context->digest[ 1 ],
						&context->digest[ 2 ],
						&context->digest[ 3 ],
						&context->digest[ 4 ] );
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
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:finishSha1
	Input		:struct sta1_ctx *context
				 < context of sha1 >
				 unsigned char *hash
				 < buffer of digest >
	Output		:unsigned char *hash
				 < buffer to be stored digest >
	Return		:int
				 < lingth of the digest >
	Description	:output the result of digest
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int finishSha1( struct sha1_ctx *context, unsigned char *hash )
{
	int				i;
	int				last_index;
	int				mod_msg;
	unsigned int	size;

	last_index	= context->size % DEF_SHA1_UNIT_OF_ENCODE;
	mod_msg		= last_index;

	/* set 0x80 at last index				*/
	if( last_index < DEF_SHA1_EXPAND_TH )
	{
		context->buffer[ last_index++ ] = 0x80;
	}

	for( i = last_index ; i < DEF_SHA1_UNIT_OF_ENCODE ; i++ )
	{
		context->buffer[ i ] = 0x00;
	}


	/* for expand							*/
	if( DEF_SHA1_EXPAND_TH <= mod_msg )
	{
		context->buffer[ mod_msg ] = 0x80;
		/* encode last block - 1			*/
		encodeWord( context->buffer,
					&context->digest[ 0 ],
					&context->digest[ 1 ],
					&context->digest[ 2 ],
					&context->digest[ 3 ],
					&context->digest[ 4 ] );

		/* for actual last block			*/
		for( i = 0 ; i < DEF_SHA1_UNIT_OF_ENCODE ; i++ )
		{
			context->buffer[ i ] = 0x00;
		}
	}

	/* calculate bit size					*/
	size = context->size * 8;

	context->buffer[ sizeof( context->buffer ) - 4 ] = ( unsigned char )( size >> 24 );
	context->buffer[ sizeof( context->buffer ) - 3 ] = ( unsigned char )( size >> 16 );
	context->buffer[ sizeof( context->buffer ) - 2 ] = ( unsigned char )( size >>  8 );
	context->buffer[ sizeof( context->buffer ) - 1 ] = ( unsigned char )( size >>  0 );

	/* ------------------------------------------------------------------------ */
	/* encode last block														*/
	/* ------------------------------------------------------------------------ */
	encodeWord( context->buffer,
				&context->digest[ 0 ],
				&context->digest[ 1 ],
				&context->digest[ 2 ],
				&context->digest[ 3 ],
				&context->digest[ 4 ] );

	for( i = 0 ; i < DEF_SHA1_DIGEST_LENGTH ; i++ )
	{
		hash[ i ] = context->digest[ i >> 2 ] >> 8 * ( 3 - ( i & 0x03 ) );
	}
	
	return( DEF_SHA1_DIGEST_LENGTH );
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeSha1
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
int encodeSha1( const unsigned char *message, int size, unsigned char *hash )
{
	struct sha1_ctx		context;

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
	initSha1( &context );

	/* ------------------------------------------------------------------------ */
	/* encode sha1																*/
	/* ------------------------------------------------------------------------ */
	updateSha1( &context, message, size );

	/* ------------------------------------------------------------------------ */
	/* save result																*/
	/* ------------------------------------------------------------------------ */
	return( finishSha1( &context, hash ) );
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
	Function	:ft00_t19
	Input		:unsigned int B
				 unsigned int C
				 unsigned int D
	Output		:void
	Return		:unsigned int
	Description	:logical fanction sequence ( 0 <= t <= 19 )
================================================================================
*/
static inline unsigned int
ft00_t19( unsigned int B, unsigned int C, unsigned int D )
{
	return( ( B & C ) | ( ( ~B ) & D ) );
}

/*
================================================================================
	Function	:ft20_t39
	Input		:unsigned int B
				 unsigned int C
				 unsigned int D
	Output		:void
	Return		:unsigned int
	Description	:logical fanction sequence ( 20 <= t <= 39 )
================================================================================
*/
static inline unsigned int
ft20_t39( unsigned int B, unsigned int C, unsigned int D )
{
	return( B ^ C ^ D );
}

/*
================================================================================
	Function	:ft40_t59
	Input		:unsigned int B
				 unsigned int C
				 unsigned int D
	Output		:void
	Return		:unsigned int
	Description	:logical fanction sequence ( 40 <= t <= 59 )
================================================================================
*/
static inline unsigned int
ft40_t59( unsigned int B, unsigned int C, unsigned int D )
{
	return( ( B & C ) | ( B & D ) | ( C & D ) );
}

/*
================================================================================
	Function	:ft60_t79
	Input		:unsigned int B
				 unsigned int C
				 unsigned int D
	Output		:void
	Return		:unsigned int
	Description	:logical fanction sequence ( 60 <= t <= 79 )
================================================================================
*/
static inline unsigned int
ft60_t79( unsigned int B, unsigned int C, unsigned int D )
{
	return( B ^ C ^ D );
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
				 unsigned int *H0
				 < intermediate hash value >
				 unsigned int *H1
				 < intermediate hash value >
				 unsigned int *H2
				 < intermediate hash value >
				 unsigned int *H3
				 < intermediate hash value >
				 unsigned int *H4
				 < intermediate hash value >
	Output		:unsigned int *H0, *H1, *H2, *H3, *H4
	Return		:void
	Description	:encode a 64 byte word of message
================================================================================
*/
static void encodeWord( const unsigned char *m_word,
						unsigned int *H0,
						unsigned int *H1,
						unsigned int *H2,
						unsigned int *H3,
						unsigned int *H4)
{
	unsigned int	A, B, C, D, E;
	unsigned int	W[ 80 ];
	unsigned int	TEMP;
	int				t;

	/* ------------------------------------------------------------------------ */
	/* initialize W[ 0 ] - W[ 15 ]												*/
	/* ------------------------------------------------------------------------ */
	for( t = 0 ; t < 16 ; t++ )
	{
		W[ t ]	 = m_word[ ( t * 4 ) + 0 ] << 24;
		W[ t ]	|= m_word[ ( t * 4 ) + 1 ] << 16;
		W[ t ]	|= m_word[ ( t * 4 ) + 2 ] <<  8;
		W[ t ]	|= m_word[ ( t * 4 ) + 3 ] <<  0;
	}
	
	/* ------------------------------------------------------------------------ */
	/* initialize W[ 16 ] - W[ 79 ]												*/
	/* ------------------------------------------------------------------------ */
	for( t = 16 ; t < 80 ; t++ )
	{
		W[ t ] =
			circularShift( 1, W[ t - 3 ] ^ W[ t - 8 ] ^ W[ t - 14 ] ^ W[ t -16 ] );
	}

	/* ------------------------------------------------------------------------ */
	/* initialize A - E															*/
	/* ------------------------------------------------------------------------ */
	A = *H0;
	B = *H1;
	C = *H2;
	D = *H3;
	E = *H4;

	/* ------------------------------------------------------------------------ */
	/* encoding																	*/
	/* ------------------------------------------------------------------------ */
	#define	DEF_SHA1_K0					0x5A827999
	#define	DEF_SHA1_K1					0x6ED9EBA1
	#define	DEF_SHA1_K2					0x8F1BBCDC
	#define	DEF_SHA1_K3					0xCA62C1D6
	for( t = 0 ; t < 20 ; t++ )
	{
		TEMP =
			circularShift( 5, A ) + ft00_t19( B, C, D ) + E + W[ t ] + DEF_SHA1_K0;
		E = D;
		D = C;
		C = circularShift( 30, B );
		B = A;
		A = TEMP;
	}

	for( t = 20 ; t < 40 ; t++ )
	{
		TEMP =
			circularShift( 5, A ) + ft20_t39( B, C, D ) + E + W[ t ] + DEF_SHA1_K1;
		E = D;
		D = C;
		C = circularShift( 30, B );
		B = A;
		A = TEMP;
	}

	for( t = 40 ; t < 60 ; t++ )
	{
		TEMP =
			circularShift( 5, A ) + ft40_t59( B, C, D ) + E + W[ t ] + DEF_SHA1_K2;
		E = D;
		D = C;
		C = circularShift( 30, B );
		B = A;
		A = TEMP;
	}

	for( t = 60 ; t < 80 ; t++ )
	{
		TEMP =
			circularShift( 5, A ) + ft60_t79( B, C, D ) + E + W[ t ] + DEF_SHA1_K3;
		E = D;
		D = C;
		C = circularShift( 30, B );
		B = A;
		A = TEMP;
	}

	*H0 += A;
	*H1 += B;
	*H2 += C;
	*H3 += D;
	*H4 += E;
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

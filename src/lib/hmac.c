/*******************************************************************************
 File:hmac.c
 Description:Procedure of HMAC hash function

*******************************************************************************/
#include <stdio.h>

#include "lib/sha1.h"
#include "lib/md5.h"
#include "lib/hmac.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int getTotalKeySize( int vec_num, struct key_vec *key_vec );


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

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeHmacSha1Vector
	Input		:char* message
				 < message to be encoded >
				 int size
				 < size of message >
				 int vec_num
				 < number of element of vector >
				 struct key_vec* key_vec[ ]
				 < vector of key and key size>
				 char *hash
				 < buffer of hmac result >
	Output		:char *hash
				 < hmac result >
	Return		:int
				 < size of the hash value >
	Description	:encode a message by hmac-sha1 with key vector
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeHmacSha1Vector( const unsigned char* message,
						  int size,
						  int vec_num,
						  struct key_vec *key_vec,
						  unsigned char *hash )
{
	unsigned char	hmac_key[ DEF_SHA1_UNIT_OF_ENCODE ];
	unsigned char	ipad[ DEF_SHA1_UNIT_OF_ENCODE ];
	unsigned char	opad[ DEF_SHA1_UNIT_OF_ENCODE ];
	int				i, j;
	int				index;
	int				hmac_key_size;
	int				total_key_size;
	struct sha1_ctx	context;

	/* ------------------------------------------------------------------------ */
	/* chech arguments															*/
	/* ------------------------------------------------------------------------ */
	if( !message || !hash )
	{
		return( -1 );
	}

	total_key_size = getTotalKeySize( vec_num, key_vec );

	if( total_key_size <= DEF_SHA1_UNIT_OF_ENCODE )
	{
		for( i = 0, index = 0 ; i < vec_num ; i++ )
		{
			for( j = 0 ; j < key_vec[ i ].key_size ; j++ )
			{
				hmac_key[ index++ ] = key_vec[ i ].key[ j ];
			}
		}

		hmac_key_size = total_key_size;
	}
	/* ------------------------------------------------------------------------ */
	/* large key 																*/
	/* ------------------------------------------------------------------------ */
	//else if( DEF_SHA1_UNIT_OF_ENCODE < key_size )
	else
	{
		initSha1( &context );
		for( i = 0 ; i < vec_num ; i++ )
		{
			updateSha1( &context, key_vec[ i ].key, key_vec[ i ].key_size );
		}
		hmac_key_size = finishSha1( &context, hmac_key );
	}

	/* ------------------------------------------------------------------------ */
	/* initialize keys															*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ; i < DEF_SHA1_UNIT_OF_ENCODE ; i++ )
	{
		if( i < hmac_key_size )
		{
			ipad[ i ] = hmac_key[ i ] ^ 0x36;
			opad[ i ] = hmac_key[ i ] ^ 0x5C;
		}
		else
		{
			ipad[ i ] = 0x36;
			opad[ i ] = 0x5C;
		}
	}

	/* ------------------------------------------------------------------------ */
	/* encode ipad																*/
	/* ------------------------------------------------------------------------ */
	initSha1( &context );
	updateSha1( &context, ipad, sizeof( ipad ) );
	updateSha1( &context, message, size );
	hmac_key_size = finishSha1( &context, hmac_key );

	/* ------------------------------------------------------------------------ */
	/* encode opad																*/
	/* ------------------------------------------------------------------------ */
	initSha1( &context );
	updateSha1( &context, opad, sizeof( opad ) );
	updateSha1( &context, hmac_key, hmac_key_size );
	hmac_key_size = finishSha1( &context, hmac_key );
	
	if( hmac_key_size < 0 )
	{
		fprintf( stderr, "cannot caliculate hash( ( k ^ ipad ) || m )\n" );
		return( -1 );
	}

	for( i = 0 ; i < DEF_SHA1_DIGEST_LENGTH ; i++ )
	{
		hash[ i ] = hmac_key[ i ];
	}

	return( hmac_key_size );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeHmacSha1
	Input		:char* message
				 < message to be encoded >
				 int size
				 < size of message >
				 const char* key
				 < key of encode >
				 int key_size
				 < size of key >
				 char *hash
				 < buffer of hmac result >
	Output		:char *hash
				 < hmac result >
	Return		:int
				 < size of the hash value >
	Description	:encode a message by hmac-sha1
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeHmacSha1( const unsigned char* message,
					int size,
					const unsigned char* key,
					int key_size,
					unsigned char *hash )
{
	struct key_vec key_vec;

	key_vec.key			= ( unsigned char* )key;
	key_vec.key_size	= key_size;

	return( encodeHmacSha1Vector( message, size, 1, &key_vec, hash ) );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeHmacMd5Vector
	Input		:char* message
				 < message to be encoded >
				 int size
				 < size of message >
				 int vec_num
				 < number of element of vector >
				 struct key_vec *key_vec
				 < vector of keys and key size >
				 char *hash
				 < buffer of hmac result >
	Output		:char *hash
				 < hmac result >
	Return		:int
				 < size of the hash value >
	Description	:encode a message by hmac-md5 with key vector
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeHmacMd5Vector( const unsigned char* message,
						 int size,
						 int vec_num,
						 struct key_vec *key_vec,
						 unsigned char *hash )
{
	unsigned char	hmac_key[ DEF_MD5_UNIT_OF_ENCODE ];
	unsigned char	ipad[ DEF_MD5_UNIT_OF_ENCODE ];
	unsigned char	opad[ DEF_MD5_UNIT_OF_ENCODE ];
	int				i, j;
	int				index;
	int				hmac_key_size;
	int				total_key_size;
	struct md5_ctx	context;

	/* ------------------------------------------------------------------------ */
	/* chech arguments															*/
	/* ------------------------------------------------------------------------ */
	if( !message || !hash )
	{
		return( -1 );
	}

	total_key_size = getTotalKeySize( vec_num, key_vec );

	if( total_key_size <= DEF_MD5_UNIT_OF_ENCODE )
	{
		for( i = 0, index = 0 ; i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
		{
			for( j = 0 ; j < key_vec[ i ].key_size ; j++ )
			{
				hmac_key[ index++ ] = key_vec[ i ].key[ j ];
			}
		}

		hmac_key_size = total_key_size;
	}
	/* ------------------------------------------------------------------------ */
	/* large key 																*/
	/* ------------------------------------------------------------------------ */
	else
	{
		initMd5( &context );
		for( i = 0 ; i < vec_num ; i++ )
		{
			updateMd5( &context, key_vec[ i ].key, key_vec[ i ].key_size );
		}
		hmac_key_size = finishMd5( &context, hmac_key );

		printf( "hmac_key_size:%d\n", hmac_key_size );
	}

	/* ------------------------------------------------------------------------ */
	/* initialize keys															*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ; i < DEF_MD5_UNIT_OF_ENCODE ; i++ )
	{
		if( i < hmac_key_size )
		{
			ipad[ i ] = hmac_key[ i ] ^ 0x36;
			opad[ i ] = hmac_key[ i ] ^ 0x5C;
		}
		else
		{
			ipad[ i ] = 0x36;
			opad[ i ] = 0x5C;
		}
	}

	/* ------------------------------------------------------------------------ */
	/* encode ipad																*/
	/* ------------------------------------------------------------------------ */
	initMd5( &context );
	updateMd5( &context, ipad, sizeof( ipad ) );
	updateMd5( &context, message, size );
	hmac_key_size = finishMd5( &context, hmac_key );

	/* ------------------------------------------------------------------------ */
	/* encode opad																*/
	/* ------------------------------------------------------------------------ */
	initMd5( &context );
	updateMd5( &context, opad, sizeof( opad ) );
	updateMd5( &context, hmac_key, hmac_key_size );
	hmac_key_size = finishMd5( &context, hmac_key );
	
	if( hmac_key_size < 0 )
	{
		fprintf( stderr, "cannot caliculate hash( ( k ^ ipad ) || m )\n" );
		return( -1 );
	}

	for( i = 0 ; i < DEF_MD5_DIGEST_LENGTH ; i++ )
	{
		hash[ i ] = hmac_key[ i ];
	}

	return( hmac_key_size );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeHmacMd5
	Input		:char* message
				 < message to be encoded >
				 int size
				 < size of message >
				 const char* key
				 < key of encode >
				 int key_size
				 < size of key >
				 char *hash
				 < buffer of hmac result >
	Output		:char *hash
				 < hmac result >
	Return		:int
				 < size of the hash value >
	Description	:encode a message by hmac-md5
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeHmacMd5( const unsigned char* message,
					int size,
					const unsigned char* key,
					int key_size,
					unsigned char *hash )
{
	struct key_vec key_vec;

	key_vec.key			= ( unsigned char* )key;
	key_vec.key_size	= key_size;

	return( encodeHmacMd5Vector( message, size, 1, &key_vec, hash ) );
}
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
================================================================================
	Function	:getTotalKeySize
	Input		:int vec_num
				 < number of element of vector >
				 struct key_vec *key_vec
				 < key vector >
	Output		:void
	Return		:int
				 < total size of keys >
	Description	:calculate total size of key
================================================================================
*/
int getTotalKeySize( int vec_num, struct key_vec *key_vec )
{
	int		i;
	int		total;

	for( i = 0, total = 0 ; i < vec_num ; i++ )
	{
		total += key_vec[ i ].key_size;
	}

	return( total );
}


/*******************************************************************************
 File:hmac.h
 Description:Definitions of HMAC hash function

*******************************************************************************/
#ifndef	__HMAC_H__
#define	__HMAC_H__

#include "lib/sha1.h"
#include "lib/md5.h"

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
struct key_vec
{
	unsigned char	*key;
	int				key_size;
};


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
				 < number of vector >
				 const char* key_vec[ ]
				 < key of encode >
				 int key_size_vec[ ]
				 < size of key >
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
						  unsigned char *hash );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encodeHmacSha1
	Input		:const char* message
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
					unsigned char *hash );

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
						 unsigned char *hash );

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
					unsigned char *hash );

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
#endif	//__HMAC_H__

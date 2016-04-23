/*******************************************************************************
 File:md5.h
 Description:Definitions of md5 function

*******************************************************************************/
#ifndef	__MD5_H__
#define	__MD5_H__

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
#define	DEF_MD5_DIGEST_LENGTH			16
#define	DEF_MD5_UNIT_OF_ENCODE			64
#define	DEF_MD5_NUM_DIGEST				4


/*
================================================================================

	Management

================================================================================
*/
struct md5_ctx
{
	unsigned int	digest[ DEF_MD5_NUM_DIGEST ];
	unsigned int	size;
	unsigned char	buffer[ DEF_MD5_UNIT_OF_ENCODE ];
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
void initMd5( struct md5_ctx *context );


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
			   int size );

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
int finishMd5( struct md5_ctx *context, unsigned char *hash );

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
int encodeMd5( const unsigned char *message, int size, unsigned char *hash );


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
#endif	//__MD5_H__

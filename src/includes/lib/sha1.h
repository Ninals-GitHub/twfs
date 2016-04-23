/*******************************************************************************
 File:sha1.h
 Description:Definitions of sha1 function

*******************************************************************************/
#ifndef	__SHA1_H__
#define	__SHA1_H__

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
#define	DEF_SHA1_DIGEST_LENGTH			20
#define	DEF_SHA1_UNIT_OF_ENCODE			64
#define	DEF_SHA1_NUM_DIGEST				5

/*
================================================================================

	Management

================================================================================
*/
struct sha1_ctx
{
	unsigned int	digest[ DEF_SHA1_NUM_DIGEST ];
	unsigned int	size;
	unsigned char	buffer[ DEF_SHA1_UNIT_OF_ENCODE ];
};

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
void initSha1( struct sha1_ctx *context );

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
				int size );

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
int finishSha1( struct sha1_ctx *context, unsigned char *hash );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:void
	Input		:const unsigned char *message
				 < original message to be expanded >
				 int size
				 < size of original message >
				 unsgined char *hash
				 < sha1 hash value string buffer >
	Output		:unsinged char *hash
				 < result of sha1 hash value string >
	Return		:int
				 < length of hash >
	Description	:encode sha1
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int encodeSha1( const unsigned char *org_msg, int size, unsigned char *hash );

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
#endif	//__SHA1_H__

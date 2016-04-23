/*******************************************************************************
 File:crypt.h
 Description:Definitions of encription of a message

*******************************************************************************/
#ifndef	__CRYPT_H__
#define	__CRYPT_H__

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

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:encryptMessage3Des
	Input		:const unsigned char *message
				 < message to encrypt >
				 long length
				 < length of message >
				 unsigned char *enc_message
				 < encrypted message >
	Output		:unsigned char *enc_message
				 < encrypted message >
	Return		:void
	Description	:encrypt a message simply by 3des encryption method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void encryptMessage3Des( const unsigned char *message,
						 long length,
						 unsigned char *enc_message );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:decryptMessage3Des
	Input		:const unsigned char *message
				 < message to decrypt >
				 long length
				 < length of message >
				 unsigned char *dec_message
				 < decrypted message >
	Output		:unsigned char *dec_message
				 < decrypted message >
	Return		:void
	Description	:decrypt a message simply by 3des encryption method
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void decryptMessage3Des( const unsigned char *message,
						 long length,
						 unsigned char *dec_message );

#endif	//__CRYPT_H__

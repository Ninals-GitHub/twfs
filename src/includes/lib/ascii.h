/*******************************************************************************
 File:ascii.h
 Description:Definitions of ascii operations

*******************************************************************************/
#ifndef	__ASCII_H__
#define	__ASCII_H__

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
	Function	:hex2Ascii
	Input		:unsigned char hex
				 < hex to be translated to ascii >
	Output		:void
	Return		:unsigned char
				 < translated ascii >
	Description	:translate hex to ascii
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
char hex2Ascii( char hex );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:hex2HexString
	Input		:const char *org
				 < message to convert ( must be hex value )>
				 int size
				 < size of message >
				 char *dst
				 < converted message buffer (converted to ascii) >
	Output		:char *dst
				 < converted message >
	Return		:int
				 < converted message size >
	Description	:convert a message to hes ascii string
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int hex2HexString( const char *org, int size, char *dst );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:strnCaseCmp
	Input		:const char *ref
				 < referer >
				 const char *cmp
				 < comparator >
				 int length
				 < length to compare exclude null terminator >
	Output		:void
	Return		:int
				 < 0:equal 1:ref>cmp -1:ref<cmp >
	Description	:convert strings to lower case, then compare them
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int strnCaseCmp( const char *ref, const char *cmp, int length );

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
#endif	//__ASCII_H__

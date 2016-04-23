/*******************************************************************************
 File:utf.h
 Description:Definitions of UTF

*******************************************************************************/
#ifndef	__UTF_H__
#define	__UTF_H__

#include <stdint.h>

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
--------------------------------------------------------------------------------

	UTF-16

--------------------------------------------------------------------------------
*/
#define	DEF_UTF16_NORMAL_LOW_LOW		0x0000
#define	DEF_UTF16_NORMAL_LOW_HIGH		0xD7FF
#define	DEF_UTF16_NORMAL_HIGH_LOW		0xE000
#define	DEF_UTF16_NORMAL_HIGH_HIGH		0xFFFF

#define	DEF_UTF16_SURR_LEAD_LOW			0xD800
#define	DEF_UTF16_SURR_LEAD_HIGH		0xDBFF
#define	DEF_UTF16_SURR_TRAIL_LOW		0xDC00
#define	DEF_UTF16_SURR_TRAIL_HIGH		0xDFFF


typedef enum
{
	E_UTF16_NORMAL,				// U+0000 to U+D7FF and U+E000 to U+FFFF
	E_UTF16_LEAD_SURROGATE,		// U+D800 to U+DBFF
	E_UTF16_TRAIL_SURROGATE,	// U+DC00 to U+DFFF
} E_UTF16_CODE_POINT;

/*
--------------------------------------------------------------------------------

	UTF-8

--------------------------------------------------------------------------------
*/
/* utf8 max coding size															*/
#define	DEF_UTF8_MAX_SIZE					4
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
	Function	:whichUtf16CodePoint
	Input		:const uint8_t *byte_order
				 < utf16 data. this pointer must be point to first byte >
	Output		:void
	Return		:E_UTF16_CODE_POINT
				 < data type of utf16 >
	Description	:determine which code point of utf16
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
E_UTF16_CODE_POINT whichUtf16CodePoint( const uint8_t *byte_order );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:convertUnicodeFromUtf16Surr
	Input		:const uint8_t *lead_byte_order
				 < utf16 lead surrogate.
				    this pointer must be point to first byte >
				 const uint8_t *trail_byte_order
				 < utf16 trail surrogate.
				    this pointer must be point to first byte >
	Output		:void
	Return		:uint32_t
				 < unicode >
	Description	:convert utf16 surrogate pair to unicode data
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
uint32_t convertUnicodeFromUtf16Surr( const uint8_t *lead_byte_order,
									  const uint8_t *trail_byte_order );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:convertUtf16ToUtf8
	Input		:uint8_t *utf16
				 < 2byte utf16 data >
				 uint8_t *output
				 < outbut buffer >
	Output		:uint8_t *output
				 < output of utf8 >
	Return		:int
				 < lnegth of data of utf8 >
	Description	:convert 2byte utf16 to utf8
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int convertUtf16ToUtf8( uint8_t *utf16, uint8_t *output );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:convertSurrUtf16ToUtf8
	Input		:uint8_t *lead_byte_order
				 < utf16 lead surrogate >
				 uint8_t *trail_byte_order
				 < utf16 trail surrogate >
	Output		:uint8_t *output
				 < output of utf8 >
	Return		:int
				 < length of data of utf8 >
	Description	:convert surrogate paris of utf16 to utf8
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int convertSurrUtf16ToUtf8( uint8_t *lead_byte_order,
							uint8_t *trail_byte_order,
							uint8_t *output );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:utf8StrnLen
	Input		:const uint8_t *str
				 < utf-8 encoded string >
				 int size
				 < size of string buffer >
	Output		:void
	Return		:int
				 < length of utf-8 encoded string >
	Description	:count string of utf-8
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int utf8StrnLen( const uint8_t *str, int size );

#endif	//__UTF_H__

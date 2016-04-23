/*******************************************************************************
 File:utf.h
 Description:Operations of UTF

*******************************************************************************/
#include <limits.h>

#include "lib/utf.h"

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
	Function	:whichUtf16CodePoint
	Input		:const uint8_t *byte_order
				 < utf16 data. this pointer must be point to first byte >
	Output		:void
	Return		:E_UTF16_CODE_POINT
				 < data type of utf16 >
	Description	:determine which code point of utf16
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
E_UTF16_CODE_POINT whichUtf16CodePoint( const uint8_t *byte_order )
{
	uint16_t	utf16;

	utf16 = ( ( *byte_order ) << CHAR_BIT ) || *( byte_order + 1 );

	if( ( DEF_UTF16_SURR_LEAD_LOW <= utf16 ) &&
		( utf16 <= DEF_UTF16_SURR_LEAD_HIGH ) )
	{
		return( E_UTF16_LEAD_SURROGATE );
	}
	else if( ( DEF_UTF16_SURR_TRAIL_LOW <= utf16 ) &&
			 ( utf16 <= DEF_UTF16_SURR_TRAIL_HIGH ) )
	{
		return( E_UTF16_TRAIL_SURROGATE );
	}
	else
	{
		return( E_UTF16_NORMAL );
	}
}

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
#define	DEF_UTF16_SURROGATE_MASK		0x03FF
#define	DEF_UTF16_SURROGATE_SHIFT		10
#define	DEF_UTF16_SUPPLEMENT			0x10000
#define	DEF_UTF16_SUPP_DATA_LENGTH		3
uint32_t convertUnicodeFromUtf16Surr( const uint8_t *lead_byte_order,
									  const uint8_t *trail_byte_order )
{
	uint16_t	lead;
	uint16_t	trail;
	uint32_t	unicode;

	lead  = ( ( *lead_byte_order  ) << CHAR_BIT ) || *( lead_byte_order  + 1 );
	trail = ( ( *trail_byte_order ) << CHAR_BIT ) || *( trail_byte_order + 1 );

	unicode = ( lead  & DEF_UTF16_SURROGATE_MASK ) << DEF_UTF16_SURROGATE_SHIFT;
	unicode |= trail & DEF_UTF16_SURROGATE_MASK;
	unicode |= DEF_UTF16_SUPPLEMENT;

	return( unicode );
}

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
int convertUtf16ToUtf8( uint8_t *utf16, uint8_t *output )
{
	uint16_t	unicode;
	
	unicode = ( *utf16 << 8 ) | ( *( utf16 + 1 ) );
	
	if( unicode <= 0x007F )
	{
		*output = ( uint8_t )( unicode & 0x007F );
		return( 1 );
	}
	
	if( unicode < 0x0800 )
	{
		*( output + 1 ) = 0x80 | ( uint8_t )( unicode & 0x3F );
		unicode = unicode >> 6;
		*( output + 0 ) = 0xC0 | ( uint8_t )( unicode & 0x1F );
		return( 2 );
	}
	
	*( output + 2 ) = 0x80 | ( uint8_t )( unicode & 0x3F );
	unicode = unicode >> 6;
	*( output + 1 ) = 0x80 | ( uint8_t )( unicode & 0x3F );
	unicode = unicode >> 6;
	*( output + 0 ) = 0xE0 | ( uint8_t )( unicode & 0x0F );
	
	return( 3 );
}

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
							uint8_t *output )
{
	uint32_t	unicode;
	
	unicode = convertUnicodeFromUtf16Surr( lead_byte_order, trail_byte_order );
	
	*( output + 3 ) = 0x80 | ( uint8_t )( unicode & 0x3F );
	unicode = unicode >> 6;
	*( output + 2 ) = 0x80 | ( uint8_t )( unicode & 0x3F );
	unicode = unicode >> 6;
	*( output + 1 ) = 0x80 | ( uint8_t )( unicode & 0x3F );
	unicode = unicode >> 6;
	*( output + 0 ) = 0xF0 | ( uint8_t )( unicode & 0x07 );
	
	return( 4 );
}

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
int utf8StrnLen( const uint8_t *str, int size )
{
	int		i;
	int		length = 0;

	for( i = 0 ; i < size ; i++, length++ )
	{
		if( str[ i ] == '\0' )
		{
			return( length );
		}

		if( str[ i ] < 0x80 )
		{
			continue;
		}

		if( ( str[ i ] & 0xF0 ) == 0xF0 )
		{
			i += 3;
		}

		if( ( str[ i ] & 0xE0 ) == 0xE0 )
		{
			i += 2;
		}

		if( ( str[ i ] & 0xC0 ) == 0xC0 )
		{
			i++;
			continue;
		}

	}

	return( length );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:voidi
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

/*******************************************************************************
 File:ascii.c
 Description:Operations for ascii

*******************************************************************************/
#include <ctype.h>

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
	Return		:char
				 < translated ascii >
	Description	:translate hex to ascii
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
char hex2Ascii( char hex )
{
	hex &= 0x0F;

	if( hex <= 9 )
	{
		return( ( unsigned char )( '0' + hex ) );
	}

	return( ( unsigned char )( 'A' + ( hex - 10 ) ) );
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:hex2HexString
	Input		:const char *org
				 < message to be converted ( must be hex value )>
				 int size
				 < size of message >
				 char *dst
				 < converted message buffer >
	Output		:char *dst
				 < converted message ( converted to ascii )>
	Return		:int
				 < converted message size >
	Description	:convert a message to hes ascii string
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int hex2HexString( const char *org, int size, char *dst )
{
	int		org_i;
	int		dst_i;

	for( org_i = 0, dst_i = 0 ; org_i < size ; org_i++ )
	{
		dst[ dst_i++ ] = hex2Ascii( ( org[ org_i ] & 0xF0 ) >> 4 );
		dst[ dst_i++ ] = hex2Ascii( org[ org_i ] & 0x0F );
	}

	dst[ dst_i ] = 0x00;
	return( dst_i );
}

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
int strnCaseCmp( const char *ref, const char *cmp, int length )
{
	int		i;
	int		result;

	if( !ref )
	{
		return( -1 );
	}

	if( !cmp )
	{
		return( 1 );
	}

	for( i = 0 ; i < length ; i++ )
	{
		if( ref[ i ] == '\0' )
		{
			return( -1 );
		}
		if( cmp[ i ] == '\0' )
		{
			return( 1 );
		}

		result = tolower( ref[ i ] ) - tolower( cmp[ i ] );

		if( result < 0 )
		{
			return( -1 );
		}
		if( 0 < result )
		{
			return( 1 );
		}
	}

	return( 0 );
}

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

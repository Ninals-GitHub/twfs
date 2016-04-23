/*******************************************************************************
 File:log.c
 Description:Operations of logging

*******************************************************************************/
#include <stdio.h>
#include <stdarg.h>

#include "lib/log.h"

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
static FILE *log_filep;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initLogging
	Input		:void
	Output		:void
	Return		:int
				 < status >
	Description	:initialize a logging operations
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initLogging( void )
{
#ifdef	DEF_LOG_DEBUG
	log_filep = fopen( DEF_LOG_FILE_NAME, "w" );

	if( !log_filep )
	{
		printf( "cannot open log file\n" );
		return( -1 );
	}

	setvbuf( log_filep, NULL, _IOLBF, 0 );
#endif
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:doLogMessage
	Input		:void
	Output		:void
	Return		:void
	Description	:logging a message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void doLogMessage( const char *format, ... )
{
#ifdef	DEF_LOG_DEBUG
	va_list		ap;
	
	va_start( ap, format );

	vfprintf( log_filep, format, ap );
#endif
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyLogging
	Input		:void
	Output		:void
	Return		:void
	Description	:release a logging resources
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroyLogging( void )
{
#ifdef	DEF_LOG_DEBUG
	fclose( log_filep );
#endif
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

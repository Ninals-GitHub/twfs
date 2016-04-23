/*******************************************************************************
 File:log.h
 Description:Definitions of logging operations

*******************************************************************************/
#ifndef	__LOG_H__
#define	__LOG_H__


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
//#define	DEF_LOG_DEBUG

#ifdef	DEF_LOG_DEBUG
#define	logMessage			doLogMessage
#else
#define	logMessage( fmt, ... )
#endif

#define	DEF_LOG_FILE_NAME		"twfs.log"

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
	Function	:initLogging
	Input		:void
	Output		:void
	Return		:int
				 < status >
	Description	:initialize a logging operations
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initLogging( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:doLogMessage
	Input		:void
	Output		:void
	Return		:void
	Description	:logging a message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void doLogMessage( const char *format, ... );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyLogging
	Input		:void
	Output		:void
	Return		:void
	Description	:release a logging resources
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroyLogging( void );

#endif	// __LOG_H__
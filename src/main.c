/*******************************************************************************
 File:FileSystem@tw.c
 Description:main process of twitter psuedo file system

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>

#include "twfs.h"
#include "twitter_operation.h"
#include "lib/utils.h"
#include "lib/log.h"
#include "lib/json.h"
#include "lib/utf.h"
#include "lib/base64.h"
#include "lib/crypt.h"
#include "net/twitter_api.h"
#include "net/network.h"
#include "net/ssl.h"
#include "net/oauth.h"



/*
================================================================================

	Prototype Statements

================================================================================
*/
static void decryptKeys( void );
static int readConfigurations( const char *user_name );
static int initFileSystemTw( int argc, char *argv[ ],
							 struct ssl_session *sesseion,
							 const char *hostname,
							 const char *screen_name );
static void destroyFileSystemTw( void );


/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_TWFS_CONFIG_DIR			".twfs"

/*
================================================================================

	Management

================================================================================
*/
static char
	oauth_consumer_key[ 64 ];
static char
	oauth_consumer_secret[ 64 ];
//static cahr
//	oauth_access_token[ 64 ],	// raw data( is not percent encoding )
//	oauth_access_secret[ 64 ];	// raw data( is not percent encoding )

const static unsigned char
	secret_consumer_key[ ]		= { 0x02, 0xE4, 0x7C, 0xF6, 0xCE, 0xE0, 0xBF, 0xC1,
									0x53, 0x77, 0x4C, 0xBB, 0xF6, 0xFA, 0xCF, 0xC6,
									0x0D, 0x20, 0x64, 0x5B, 0xF6, 0xFA, 0xBD, 0xC6,
									0x00 };
const static unsigned char
	secret_consumer_secret[ ]	= { 0x3A, 0x42, 0xCB, 0x4A, 0x4D, 0x6A, 0x71, 0xAF,
									0xB8, 0x2E, 0x42, 0x24, 0x71, 0xA0, 0x3D, 0x7A,
									0x14, 0x1E, 0x3C, 0xE9, 0x79, 0xF1, 0x3A, 0x48,
									0x71, 0x5B, 0x4C, 0x3E, 0x23, 0x94, 0x41, 0x89,
									0xB1, 0xF0, 0x53, 0xDE, 0xD4, 0xBA, 0x48, 0x7F,
									0xF0, 0x86, 0x4E, 0xED, 0x71, 0x83, 0xDA, 0xCE,
									0x00 };


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getConfigurationFilePath
	Input		:const char *id
				 < id for identifing a configuration file >
				 char *path
				 < output buffer of file path >
	Output		:const char *path
				 < configuration file path >
	Return		:int
				 < status >
	Description	:get configuration file path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getConfigurationFilePath( const char *id, char *path )
{
	char		home[ 256 ];
	struct stat	file_stat = { 0 };
	int			result;

	strcpy( home, getenv( "HOME" ) );

	if( !home )
	{
		return( -1 );
	}

	snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s", home, DEF_TWFS_CONFIG_DIR, id );

	if( ( result = stat( path, &file_stat ) ) == 0 )
	{
		if( S_ISDIR( file_stat.st_mode ) )
		{
			printf( "Cannot create a configuration file : %s\n", path );
			return( -1 );
		}
	}

	return( 0 );
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:main
	Input		:int argc
				 < number of arguments >
				 char *argv[ ]
				 < aruguments >
	Output		:void
	Return		:int
				 < return code >
	Description	:main of twitter psuedo file system
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int main( int argc, char *argv[ ] )
{
	int					result;
	struct ssl_session	session;

	if( argc != 4 )
	{
		printf( "twfs screen_name root_directory mount_point\n" );
		return( -1 );
	}
	
	/* ------------------------------------------------------------------------ */
	/* decrypt oauth keys														*/
	/* ------------------------------------------------------------------------ */
	decryptKeys( );

#if 1
	/* ------------------------------------------------------------------------ */
	/* prepare for logging														*/
	/* ------------------------------------------------------------------------ */
	initLogging( );

	/* ------------------------------------------------------------------------ */
	/* initialize																*/
	/* ------------------------------------------------------------------------ */
	if( ( result = initFileSystemTw( argc, argv, &session,
									 DEF_TWTR_HTTPH_HOST_NAME, argv[ 1 ] ) ) < 0 )
	{
		return( result );
	}
#if 1
	//printf( "%d\n", argc);
	//for( int i = 0 ; i < argc ; i++ )
	//{
	//	printf( "%s\n", argv[ i ] );
	//}

	startTwfs( argc, argv );

	/* ------------------------------------------------------------------------ */
	/* release																	*/
	/* ------------------------------------------------------------------------ */
	destroyFileSystemTw( );
#endif
#endif
	return( 0 );
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/

/*
================================================================================
	Function	:decryptKeys
	Input		:void
	Output		:void
	Return		:int
				 < status >
	Description	:descrypt oauth keys
================================================================================
*/
static void decryptKeys( void )
{
	memset( oauth_consumer_secret, 0x00, sizeof( oauth_consumer_secret ) );
	decryptMessage3Des( ( unsigned char*)secret_consumer_secret,
						sizeof( secret_consumer_secret ),
						( unsigned char* )oauth_consumer_secret );
	//printf( "\ndecripted[key] : %s\n", ( unsigned char* )oauth_consumer_secret );
	memset( oauth_consumer_key, 0x00, sizeof( oauth_consumer_key ) );
	decryptMessage3Des( ( unsigned char*)secret_consumer_key,
						sizeof( secret_consumer_secret ),
						( unsigned char* )oauth_consumer_key );
	//printf( "\ndecripted : %s\n", oauth_consumer_key );
}
/*
================================================================================
	Function	:readConfigurations
	Input		:const char *user_name
				 < user name for reading configurations >
	Output		:void
	Return		:int
				 < status >
	Description	:read configuration information
================================================================================
*/
static int readConfigurations( const char *user_name )
{
	int		result;
	int		len;
	char	id[ DEF_TWAPI_MAX_USER_ID_LEN ];
	char	screen_name[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char	access_token[ 256 ];
	char	access_secret[ 256 ];

	/* ------------------------------------------------------------------------ */
	/* read conigurations														*/
	/* ------------------------------------------------------------------------ */
	if( user_name )
	{
		len = strlen( user_name );
		if( DEF_TWAPI_MAX_USER_ID_LEN < len )
		{
			return( -1 );
		}
		strcpy( screen_name, user_name );
	}
	else
	{
		screen_name[ 0 ] = '\0';
	}

	result = readTwapiConfigurations( id, sizeof( id ),
									  screen_name, sizeof( screen_name ),
									  access_token, sizeof( access_token ),
									  access_secret, sizeof( access_secret ) );

	if( result < 0 )
	{
		/* -------------------------------------------------------------------- */
		/* register oauth information											*/
		/* -------------------------------------------------------------------- */
		result = registerOauthInfo( DEF_TWTR_HTTPH_HOST_NAME,
									DEF_TWTR_HTTPH_AUTH_CALLBACK_URL,
									oauth_consumer_key,
									oauth_consumer_secret,
									NULL, NULL );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* set conigurations														*/
	/* ------------------------------------------------------------------------ */
	setTwapiUserId( id, strlen( id ) );
	setTwapiScreenName( screen_name, strlen( screen_name ) );

	/* ------------------------------------------------------------------------ */
	/* register oauth information												*/
	/* ------------------------------------------------------------------------ */
	result = registerOauthInfo( DEF_TWTR_HTTPH_HOST_NAME,
								DEF_TWTR_HTTPH_AUTH_CALLBACK_URL,
								oauth_consumer_key,
								oauth_consumer_secret,
								access_token,
								access_secret );

	return( result );
}

/*
================================================================================
	Function	:initFileSystemTw
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *hostname
				 < hostname to connect https >
				 const char *screen_name
				 < screen name of twitter >
	Output		:void
	Return		:int
				 < status >
	Description	:initialize FileSystem@tw
================================================================================
*/
int initFileSystemTw( int argc, char *argv[ ],
					  struct ssl_session *session,
					  const char *hostname,
					  const char *screen_name )
{
	int		result;
	char	home[ DEF_TWFS_PATH_MAX ];
	char	dir_path[ DEF_TWFS_PATH_MAX ];

	initTwitterOperation( );

	session->socket			= 0;
	session->ssl_handle		= NULL;
	session->ssl_context	= NULL;


	/* ------------------------------------------------------------------------ */
	/* make directory for configuration files									*/
	/* ------------------------------------------------------------------------ */
	strcpy( home, getenv( "HOME" ) );

	if( !home )
	{
		return( -1 );
	}

	sprintf( dir_path, "%s/%s", home, DEF_TWFS_CONFIG_DIR );

	if( ( result = makeDirectory( ( const char* )dir_path, 0700, false ) ) < 0 )
	{
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* try connect																*/
	/* ------------------------------------------------------------------------ */
	if( ( session->socket = connectServer( hostname, NULL ) ) < 0 )
	{
		perror( "cannot connect a server" );
		return( session->socket );
	}

	/* ------------------------------------------------------------------------ */
	/* allocate heap memory														*/
	/* ------------------------------------------------------------------------ */
#if 0
	result = initSmalloc( );

	if( result < 0 )
	{
		disconnectServer( session->socket );
		destroySmalloc( );
		fprintf( stderr, "cannot allocate heap memory\n" );
		return( -1 );
	}
#endif

	/* ------------------------------------------------------------------------ */
	/* read and set configurations												*/
	/* ------------------------------------------------------------------------ */
	if( ( result = readConfigurations( screen_name ) ) < 0 )
	{
		disconnectServer( session->socket );
		//destroySmalloc( );
		fprintf( stderr, "cannot register oauth information\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* intialize a ssl layer													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = initSSL( ) ) < 0 )
	{
		disconnectServer( session->socket );
		unregisterOauthInfo( );
		//destroySmalloc( );
		fprintf( stderr, "cannot initialize ssl.\n" );
		return( -1 );
	}

	if( ( result = openSSLSession( session ) ) < 0 )
	{
		disconnectServer( session->socket );
		unregisterOauthInfo( );
		//destroySmalloc( );
		fprintf( stderr, "cannot open ssl.\n" );
		return( session->socket );
	}
	/* ------------------------------------------------------------------------ */
	/* try to open a ssl session												*/
	/* ------------------------------------------------------------------------ */
	if( needAuthorization( ) )
	{
		/* -------------------------------------------------------------------- */
		/* request authoriazation												*/
		/* -------------------------------------------------------------------- */
		if( ( result = requestTwapiOauth( session ) ) < 0 )
		{
			closeSSLSession( session );
			disconnectServer( session->socket );
			unregisterOauthInfo( );
			//destroySmalloc( );
			fprintf( stderr, "authorization refused by twitter api .\n" );
			return( -1 );
		}

		//closeSSLSession( session );
		//disconnectSSLServer( session );
	}
	else
	{
		//disconnectSSLServer( session );
	}

	printf( "starting Twitter Filesystem.\n" );
	printf( "Welcome! %s.\n", argv[ 1 ] );


	/* ------------------------------------------------------------------------ */
	/* set root directory														*/
	/* ------------------------------------------------------------------------ */
	setRootDirPath( argv[ 2 ] );
	setMountDirPath( argv[ 3 ] );

	/* ------------------------------------------------------------------------ */
	/* initialize twfs															*/
	/* ------------------------------------------------------------------------ */
	return( initTwitterFileSystem( destroyFileSystemTw ) );
	//printf( "%s %s\n", argv[ 2 ], argv[ 3 ] );
	//return( 0 );
}

/*
================================================================================
	Function	:destroyFileSystemTw
	Input		:void
	Output		:void
	Return		:void
	Description	:deallocate all resource
================================================================================
*/
static void destroyFileSystemTw( void )
{
	/* ------------------------------------------------------------------------ */
	/* unregister oauth information												*/
	/* ------------------------------------------------------------------------ */
	unregisterOauthInfo( );
	/* ------------------------------------------------------------------------ */
	/* destroy ssl resources													*/
	/* ------------------------------------------------------------------------ */
	destroySSLResources( );
	/* ------------------------------------------------------------------------ */
	/* free heap memory															*/
	/* ------------------------------------------------------------------------ */
	//destroySmalloc( );
	/* ------------------------------------------------------------------------ */
	/* free logging resource													*/
	/* ------------------------------------------------------------------------ */
	destroyLogging( );

}

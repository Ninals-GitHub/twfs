/*******************************************************************************
 File:twitter_operaiont.c
 Description:Operations of Twitter

*******************************************************************************/
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include "twfs.h"
#include "twitter_operation.h"
#include "lib/json.h"
#include "lib/log.h"
#include "lib/utils.h"
#include "lib/base64.h"
#include "lib/crypt.h"
#include "net/twitter_api.h"
#include "net/twitter_json.h"
#include "net/oauth.h"
#include "net/ssl.h"
#include "net/http.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int
twopeRecvHeaders( struct ssl_session *session, struct http_ctx *hctx );

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
struct user
{
	char	user_id[ DEF_TWAPI_MAX_USER_ID_LEN + 1 ];
	char	screen_name[ DEF_TWAPI_MAX_SCREEN_NAME_LEN + 1 ];
};

static struct user user;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initTwitterOperation
	Input		:void
	Output		:void
	Return		:void
	Description	:initialize twitter operation
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initTwitterOperation( void )
{
	int	i;

	for( i = 0 ; i < DEF_TWAPI_MAX_USER_ID_LEN ; i++ )
	{
		user.user_id[ i ] = '\0';
	}

	for( i = 0 ; i < DEF_TWAPI_MAX_SCREEN_NAME_LEN ; i++ )
	{
		user.screen_name[ i ] = '\0';
	}

	initOauth( );
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setTwapiUserId
	Input		:const char *set_user_id
				 < user id >
				 int size
				 < size of user id >
	Output		:void
	Return		:void
	Description	:set user id
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void setTwapiUserId( const char *set_user_id, int size )
{
	if( DEF_TWAPI_MAX_USER_ID_LEN < size )
	{
		size = DEF_TWAPI_MAX_USER_ID_LEN;
	}

	strncpy( user.user_id, set_user_id, size );
	user.user_id[ size ] = '\0';
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setTwapiScreenName
	Input		:const char *set_screen_name
				 < screen name >
				 int size
				 < size of screen name >
	Output		:void
	Return		:void
	Description	:set screen name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void setTwapiScreenName( const char *set_screen_name, int size )
{
	if( DEF_TWAPI_MAX_SCREEN_NAME_LEN < size )
	{
		size = DEF_TWAPI_MAX_SCREEN_NAME_LEN;
	}

	strncpy( user.screen_name, set_screen_name, size );
	user.screen_name[ size ] = '\0';
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiUserId
	Input		:void
	Output		:void
	Return		:const char*
				 < user id >
	Description	:get user id
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getTwapiUserId( void )
{
	return( ( const char* )( user.user_id ) );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiScreenName
	Input		:void
	Output		:void
	Return		:const char*
				 < screen name >
	Description	:get screen name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getTwapiScreenName( void )
{
	return( ( const char* )( user.screen_name ) );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:writeTwapiConfigurations
	Input		:const char *id
				 < id of twitter >
				 const char *screen_name
				 < screen name of twitter >
				 const char *access_token
				 < access token >
				 const char *access_secret
				 < access secret >
	Output		:void
	Return		:int
				 < status >
	Description	:write keys of twitter
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int writeTwapiConfigurations( const char *id,
							  const char *screen_name,
							  const char *access_token,
							  const char *access_secret )
{
	char	f_path[ DEF_TWFS_PATH_MAX ];
	char	enc_path[ 128 ];
	char	b64_enc_path[ 128 ];
	char	file_contents[ 512 ];
	char	enc_file_contents[ 512 ];
	int		fc_length;
	FILE	*file_p;
	int		result;
	int		sname_len;

	if( !id || !access_token || !access_secret )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* encrypt ~/.tsfs/[screen_name] path name									*/
	/* ------------------------------------------------------------------------ */
	memset( enc_path, 0x00, DEF_TWFS_PATH_MAX );

	sname_len = strnlen( screen_name, DEF_TWAPI_MAX_SCREEN_NAME_LEN );

	encryptMessage3Des( ( const unsigned char* )screen_name,
						sname_len,
						( unsigned char *)enc_path );
	
	memset( b64_enc_path, 0x00, sizeof( b64_enc_path ) );

	encodeBase64( enc_path, sname_len, ( char* )b64_enc_path );

	for( int i = 0 ; i < sname_len ; i++ )
	{
		if( b64_enc_path[ i ] == '/' )
		{
			b64_enc_path[ i ] = '@';
		}
	}

	/* ------------------------------------------------------------------------ */
	/* for user id file name													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = getConfigurationFilePath( b64_enc_path, f_path ) ) < 0 )
	{
		return( result );
	}

	if( ( file_p = fopen( f_path, "w" ) ) == NULL )
	{
		printf( "Cannot create a configuration file : %s\n", f_path );
		return( -1 );
	}

#if 0
	fprintf( file_p, "%s\n", id );
	fprintf( file_p, "%s\n", screen_name );
	fprintf( file_p, "%s\n", access_token );
	fprintf( file_p, "%s\n", access_secret );
#endif

	memset( file_contents, 0x00, sizeof( file_contents ) );

	fc_length = 0;
	fc_length = snprintf( &file_contents[ fc_length ],
						  sizeof( file_contents ),
						  "%s\n", id );
	fc_length += snprintf( &file_contents[ fc_length ],
						   sizeof( file_contents ) - fc_length + 1,
						   "%s\n", screen_name );
	fc_length += snprintf( &file_contents[ fc_length ],
						   sizeof( file_contents ) - fc_length + 1,
						   "%s\n", access_token );
	fc_length += snprintf( &file_contents[ fc_length ],
						   sizeof( file_contents ) - fc_length + 1,
						   "%s\n", access_secret );
	fc_length++;
	memset( enc_file_contents, 0x00, sizeof( enc_file_contents ) );
	encryptMessage3Des( ( const unsigned char* )file_contents,
						fc_length,
						( unsigned char* )enc_file_contents );

	memset( file_contents, 0x00, sizeof( file_contents ) );

	encodeBase64( enc_file_contents, fc_length, file_contents );

	fprintf( file_p, "%s", file_contents );

	fclose( file_p );

	/* ------------------------------------------------------------------------ */
	/* encrypt ~/.tsfs/[screen_name] path name									*/
	/* ------------------------------------------------------------------------ */
	memset( enc_path, 0x00, DEF_TWFS_PATH_MAX );

	encryptMessage3Des( ( const unsigned char* )"last",
						sizeof( "last" ) - 1,
						( unsigned char* )enc_path );

	memset( b64_enc_path, 0x00, sizeof( b64_enc_path ) );

	encodeBase64( enc_path, sizeof( "last" ) - 1, b64_enc_path );

	for( int i = 0 ; i < sizeof( "last" ) - 1 ; i++ )
	{
		if( b64_enc_path[ i ] == '/' )
		{
			b64_enc_path[ i ] = '@';
		}
	}

	/* ------------------------------------------------------------------------ */
	/* for last login user file													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = getConfigurationFilePath( b64_enc_path, f_path ) ) < 0 )
	{
		return( result );
	}

	if( ( file_p = fopen( f_path, "w" ) ) == NULL )
	{
		printf( "Cannot create a configuration file : %s\n", f_path );
		return( -1 );
	}

#if 0
	fprintf( file_p, "%s\n", id );
	fprintf( file_p, "%s\n", screen_name );
	fprintf( file_p, "%s\n", access_token );
	fprintf( file_p, "%s\n", access_secret );
#endif
	fprintf( file_p, "%s", file_contents );

	fclose( file_p );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readTwapiConfigurations
	Input		:char *id
				 < id of twitter >
				 int id_len
				 < length of id >
				 char *screen_name
				 < screen name of twitter >
				 int screen_name_len
				 < length of screen name >
				 char *access_token
				 < access token >
				 int access_token_len
				 < length of access token >
				 char *access_secret
				 < access secret >
				 int access_secret_len
				 < length of access_secret >
	Output		:char *id
				 char *screen_name
				 char *access_token
				 cahr *access_secret
	Return		:int
				 < status >
	Description	:read keys of twitter
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readTwapiConfigurations( char *id, int id_len,
							 char *screen_name, int screen_name_len,
							 char *access_token, int access_token_len,
							 char *access_secret, int access_secret_len )
{
	char	f_path[ DEF_TWFS_PATH_MAX ];
	char	f_contents[ 512 ];
	char	enc_f_contents[ 512 ];
	char	enc_path[ 128 ];
	char	b64_enc_path[ 128 ];
	int		result;
	int		sname_len;
	int		fd;
	int		f_index;
	int		i;

	if( !id || !screen_name || !access_token || !access_secret )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* if screen name is not designated, last file is read						*/
	/* ------------------------------------------------------------------------ */
	if( screen_name[ 0 ] != '\0' )
	{
		sname_len = strnlen( screen_name, DEF_TWAPI_MAX_SCREEN_NAME_LEN );
		encryptMessage3Des( ( const unsigned char* )screen_name,
							sname_len,
							( unsigned char *)enc_path );
	}
	else
	{
		sname_len = sizeof( "last" ) - 1;
		encryptMessage3Des( ( const unsigned char* )"last",
							sname_len,
							( unsigned char *)enc_path );
	}

	memset( b64_enc_path, 0x00, sizeof( b64_enc_path ) );
	encodeBase64( enc_path, sname_len, b64_enc_path );
	for( i = 0 ; i < sname_len ; i++ )
	{
		if( b64_enc_path[ i ] == '/' )
		{
			b64_enc_path[ i ] = '@';
		}
	}
	result = getConfigurationFilePath( b64_enc_path, f_path );

	//printf( "reading : %s\n", f_path );

	if( result < 0 )
	{
		return( -1 );
	}

	if( ( fd = openFile( f_path, O_RDONLY, 0660 ) ) < 0 )
	{
		return( -1 );
	}

	memset( enc_f_contents, 0x00, sizeof( enc_f_contents ) );
	if( ( result = readFile( fd,
							 enc_f_contents,
							 sizeof( enc_f_contents ) ) ) < 0 )
	{
		closeFile( fd );
		return( -1 );
	}

	closeFile( fd );

	result = decodeBase64( enc_f_contents, result );
	memset( enc_f_contents + result, 0x00, sizeof( enc_f_contents ) - result + 1);
	memset( f_contents, 0x00, sizeof( f_contents ) );
	decryptMessage3Des( ( const unsigned char* )enc_f_contents,
						result,
						( unsigned char* )f_contents );
	

	//printf( "file decrypt[%d]:%s\n", result, f_contents );

#if 0
	if( ( file_p = fopen( f_path, "r" ) ) == NULL )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* read id																	*/
	/* ------------------------------------------------------------------------ */
	if( fgets( temp, sizeof( temp ), file_p ) == NULL )
	{
		return( -1 );
	}
	p = strchr( f_contents, '\n' );
	if( p )
	{
		*p = '\0';
		strcpy( id, f_contents );
	}
	else
	{
		//strncpy( id, f_contents, id_len );
		//id[ id_len ] = '\0';
		return( -1 );
	}
#endif
	f_index = 0;

	for( i = 0 ; i < ( DEF_TWAPI_MAX_USER_ID_LEN + 1 ) ; i++ )
	{
		if( f_contents[ f_index ] == '\n' )
		{
			f_index++;	// skip '\n'
			id[ i ] = '\0';
			break;
		}
		else
		{
			id[ i ] = f_contents[ f_index ];
			f_index++;
		}
	}

	if( result < f_index )
	{
		return( -1 );
	}

	//printf( "id : %s\n", id );

#if 0
	/* ------------------------------------------------------------------------ */
	/* read screen name															*/
	/* ------------------------------------------------------------------------ */
	if( fgets( temp, sizeof( temp ), file_p ) == NULL )
	{
		return( -1 );
	}
	
	p = strchr( f_contents, '\n' );
	if( p )
	{
		*p = '\0';
		strcpy( screen_name, f_contents );
	}
	else
	{
		//strncpy( screen_name, temp, screen_name_len );
		//screen_name[ screen_name_len ] = '\0';
		return( -1 );
	}
#endif

	for( i = 0 ; i < ( DEF_TWAPI_MAX_SCREEN_NAME_LEN + 1 ); i++ )
	{
		if( f_contents[ f_index ] == '\n' )
		{
			f_index++;	// skip '\n'
			screen_name[ i ] = '\0';
			break;
		}
		else
		{
			screen_name[ i ] = f_contents[ f_index ];
			f_index++;
		}
	}

	if( result < f_index )
	{
		return( -1 );
	}

	//printf( "screen name : %s\n", screen_name );

#if 0
	/* ------------------------------------------------------------------------ */
	/* read access token														*/
	/* ------------------------------------------------------------------------ */
	if( fgets( temp, sizeof( temp ), file_p ) == NULL )
	{
		return( -1 );
	}
	
	p = strchr( f_contents, '\n' );
	if( p )
	{
		*p = '\0';
		strcpy( access_token, f_contents );
	}
	else
	{
		return( -1 );
		//strncpy( access_token, temp, access_token_len );
		//screen_name[ access_token_len ] = '\0';
	}
#endif

	for( i = 0 ; i < ( access_token_len + 1 ); i++ )
	{
		if( f_contents[ f_index ] == '\n' )
		{
			f_index++;	// skip '\n'
			access_token[ i ] = '\0';
			break;
		}
		else
		{
			access_token[ i ] = f_contents[ f_index ];
			f_index++;
		}
	}

	if( result < f_index )
	{
		return( -1 );
	}

	//printf( "acccess token : %s\n", access_token );

#if 0
	/* ------------------------------------------------------------------------ */
	/* read access secret														*/
	/* ------------------------------------------------------------------------ */
	fgets( temp, sizeof( temp ), file_p );
	
	p = strchr( f_contents, '\n' );
	if( p )
	{
		*p = '\0';
		strcpy( access_secret, f_contents );
	}
	else
	{
		return( -1 );
	}
#endif

	for( i = 0 ; i < ( access_secret_len + 1 ); i++ )
	{
		if( f_contents[ f_index ] == '\n' )
		{
			f_index++;	// skip '\n'
			access_secret[ i ] = '\0';
			break;
		}
		else
		{
			access_secret[ i ] = f_contents[ f_index ];
			f_index++;
		}
	}

	if( result < f_index )
	{
		return( -1 );
	}

	//printf( "acccess secret %s\n", access_secret );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:clipScreeNameFromMsg
	Input		:const char *msg
				 < message to which screen name belongs >
				 char *screen_name
				 < screen name to clip >
	Output		:char *screen_name
				 < clipped screen name >
	Return		:int
				 < size of screnn_name ( includes '@' ) >
	Description	:clip a screen name from a message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int clipTwopeScreenNameFromMsg( const char *msg, char *screen_name )
{
	int		i;

	if( msg[ 0 ] != '@' )
	{
		return( 0 );
	}

	for( i = 1 ; i < ( DEF_TWAPI_MAX_SCREEN_NAME_LEN + 1 ) ; i++ )
	{
		if( msg[ i ] == ' ' )
		{
			break;
		}
		screen_name[ i - 1 ] = msg[ i ];
	}

	screen_name[ i - 1 ] = '\0';

	return( i );
}


/*
================================================================================

	Timelines

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getHomeTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *last
				 < last tweet id >
	Output		:void
	Return		:int
				 < status >
	Description	:getHomeTimeLine
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getHomeTimeLine( struct ssl_session *session,
					 struct http_ctx *hctx,
					 const char *last )
{
	int		result;

	result = getStatusesHomeTimeLine( session,
									  DEF_TWOPE_MAX_TWEET_COUNT,
									  last, NULL,
									  false, false, false, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );


	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getMentionsTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *last
				 < last tweet id >
	Output		:void
	Return		:int
				 < status >
	Description	:get mentions timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getMentionsTimeLine( struct ssl_session *session,
						 struct http_ctx *hctx,
						 const char *last )
{
	int		result;

	result = getStatusesMentionsTimeLine( session,
										  DEF_TWOPE_MAX_TWEET_COUNT,
										  last, NULL,
										  false, false, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getUserTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name to get his timelin >
				 const char *last
				 < last tweet id >
	Output		:void
	Return		:int
				 < status >
	Description	:get user timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getUserTimeLine( struct ssl_session *session,
					 struct http_ctx *hctx,
					 const char *screen_name,
					 const char *last )
{
	int		result;

	result = getStatusesUserTimeLine( session,
									  NULL,
									  screen_name,
									  DEF_TWOPE_MAX_TWEET_COUNT,
									  last, NULL,
									  false, false, false, true );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getRetweetOfMeTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *last
				 < last tweet id >
	Output		:void
	Return		:int
				 < status >
	Description	:get retweet of me timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getRetweetOfMeTimeLine( struct ssl_session *session,
							struct http_ctx *hctx,
							const char *last )
{
	int		result;

	result = getStatusesRetweetOfMe( session,
									 DEF_TWOPE_MAX_TWEET_COUNT,
									 last,
									 false, false, false, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
================================================================================

	Tweets

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:tweetTwapi
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *mention
				 < mention to tweet >
				 int size
				 < size of mention >
	Output		:void
	Return		:int
				 < status >
	Description	:simple tweet
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int tweetTwapi( struct ssl_session *session, const char *mention,  int size )
{
	int		result;

	if( DEF_REST_TWEETS_MAX_LENGTH < size )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* tweet																	*/
	/* ------------------------------------------------------------------------ */
	result = postStatusesUpdate( session, mention,
								 NULL, NULL, NULL, NULL,
								 false, true );

	//disconnectSSLServer( session );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:removeTweet
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < tweet id >
	Output		:void
	Return		:int
				 < status >
	Description	:remove tweet
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int removeTweet( struct ssl_session *session, const char *id )
{
	int		result;

	/* ------------------------------------------------------------------------ */
	/* remove tweet																*/
	/* ------------------------------------------------------------------------ */
	result = postStatusesDestroyId( session, id, false );

	//disconnectSSLServer( session );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:retweet
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *id
				 < tweet id to retweet >
	Output		:void
	Return		:int
				 < status >
	Description	:retweet
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int retweet( struct ssl_session *session,
			 struct http_ctx *hctx,
			 const char *id )
{
	int		result;

	result = postStatusesRetweetId( session, id, true );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
================================================================================

	Friends & Followers

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestFollow
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *follow
				 < screen name to follow >
	Output		:void
	Return		:int
				 < status >
	Description	:follow
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestFollow( struct ssl_session *session, const char *follow )
{
	int		result;

	result = postFriendshipCreate( session, NULL, follow, true );

	//disconnectSSLServer( session );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestUnfollow
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *unfollow
				 < screen name to unfollow >
	Output		:void
	Return		:int
				 < status >
	Description	:unfollow
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestUnfollow( struct ssl_session *session, const char *unfollow )
{
	int		result;

	result = postFriendshipDestroy( session, NULL, unfollow );

	//disconnectSSLServer( session );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getFollowingList
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name to get a following list >
				 const char *cursor
				 < cursor in the list >
	Output		:void
	Return		:int
				 < status >
	Description	:get a list of following users
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getFollowingList( struct ssl_session *session,
					  struct http_ctx *hctx,
					  const char *screen_name,
					  const char *cursor )
{
	int		result;

	result = getTwapiFriendsList( session,
								  NULL,
								  screen_name,
								  cursor,
								  DEF_TWOPE_MAX_FF_COUNT,
								  false, false );
	
	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getFollowerList
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name to get a followers list >
				 const char *cursor
				 < cursor in the list >
	Output		:void
	Return		:int
				 < status >
	Description	:get a list of followers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getFollowerList( struct ssl_session *session,
					 struct http_ctx *hctx,
					 const char *screen_name,
					 const char *cursor )
{
	int		result;

	result = getTwapiFollowersList( session,
									NULL,
									screen_name,
									cursor,
									DEF_TWOPE_MAX_FF_COUNT,
									false, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );
	
	return( result );
}


/*
================================================================================

	Users

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getBocksList
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name to get a followers list >
				 const char *cursor
				 < cursor in the list >
	Output		:void
	Return		:int
				 < status >
	Description	:get a list of blocks
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getBlockList( struct ssl_session *session,
				  struct http_ctx *hctx,
				  const char *cursor )
{
	int		result;

	result = getTwapiBlocksList( session,
									cursor,
									true, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestBlock
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *blockee
				 < screen name to block >
	Output		:void
	Return		:int
				 < status >
	Description	:block a specified user
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestBlock( struct ssl_session *session, const char *blockee )
{
	int		result;

	result = postTwapiBlocksCreate( session, NULL, blockee, true, false );

	//disconnectSSLServer( session );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestUnblock
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *unblockee
				 < screen name to unblock >
	Output		:void
	Return		:int
				 < status >
	Description	:unblock a specified user
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestUnblock( struct ssl_session *session, const char *unblockee )
{
	int		result;

	result = postTwapiBlocksDestroy( session, NULL, unblockee, true, false );

	//disconnectSSLServer( session );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getUserProfile
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name of user to read profile >
	Output		:void
	Return		:int
				 < status >
	Description	:get user profile
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getUserProfile( struct ssl_session *session,
					struct http_ctx *hctx,
					const char *screen_name )
{
	int		result;

	result = getTwapiUsersShow( session,
								 NULL,
								 screen_name,
								 false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}
/*
================================================================================

	Direct Message

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:sendDirectMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *to
				 < screen name to which send a direct message >
				 const char *message
				 < message to send >
				 int size
				 < size of a message >
	Output		:void
	Return		:int
				 < status >
	Description	:send a direct message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int sendDirectMessage( struct ssl_session *session,
					   const char *to,
					   const char *message,
					   int size )
{
	int		result;

	if( DEF_REST_TWEETS_MAX_LENGTH < size )
	{
		return( -1 );
	}

	result = postDirectMessagesNew( session, NULL, to, message );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getDirectMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *last
				 < latest tweet id which already has been got >
	Output		:void
	Return		:int
				 < status >
	Description	:get direct messages
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getDirectMessages( struct ssl_session *session,
					   struct http_ctx *hctx,
					   const char *last )
{
	int		result;

	result = getTwapiDirectMessages( session,
									 DEF_TWOPE_MAX_DM_COUNT,
									 last, NULL,
									 false, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getDirectMessageSent
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *last
				 < latest tweet id which already has been got >
	Output		:void
	Return		:int
				 < status >
	Description	:get direct messages sent
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getDirectMessagesSent( struct ssl_session *session,
						   struct http_ctx *hctx,
						   const char *last )
{
	int		result;

	result = getTwapiDirectMessagesSent( session,
									 DEF_TWOPE_MAX_DM_COUNT,
									 last, NULL,
									 0,
									 false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:removeDirectMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < latest direct message id to remove >
	Output		:void
	Return		:int
				 < status >
	Description	:remove a direct message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int removeDirectMessage( struct ssl_session *session, const char *id )
{
	int		result;

	result = postDirectMessagesDestroy( session, id, false );

	return( result );
}
/*
================================================================================

	Favorites

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getFavoritesList
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx,
				 < http context >
				 const char *last
				 < latest tweet id which already has been received >
	Output		:void
	Return		:int
				 < status >
	Description	:get favorites list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int	getFavoritesList( struct ssl_session *session,
					  struct http_ctx *hctx,
					  const char *last )
{
	int		result;

	result = getTwapiFavoritesList( session,
									DEF_TWOPE_MAX_FAV_COUNT,
									last, NULL,
									false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestFavorite
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < tweet id to favorite >
	Output		:void
	Return		:int
				 < status >
	Description	:do favorite
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestFavorite( struct ssl_session *session, const char *id )
{
	int		result;

	result = postFavoritesCreate( session, id, false );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestUnFavorite
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < tweet id to unfavorite >
	Output		:void
	Return		:int
				 < status >
	Description	:do unfavorite
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestUnFavorite( struct ssl_session *session, const char *id )
{
	int		result;

	result = postFavoritesDestroy( session, id, false );

	return( result );
}

/*
================================================================================

	Lists

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getListsTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *slug
				 < slug for list >
				 const char *owner
				 < screen name for owner >
				 const char *last
				 < last tweet id >
	Output		:void
	Return		:int
				 < status >
	Description	:get timeline of lists
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getListsTimeLine( struct ssl_session *session,
					  struct http_ctx *hctx,
					  const char *slug,
					  const char *owner,
					  const char *last )
{
	int		result;

	logMessage( "slug:%s\n", slug );
	logMessage( "owner:%s\n", owner );
	logMessage( "last:%s\n", last );

	result = getTwapiListsStatuses( session,
									NULL, slug,
									owner, NULL,
									last, NULL,
									DEF_TWOPE_MAX_LISTS_TWEET_COUNT,
									false, false );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );


	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getListsOwnerships
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < cursor of list >
	Output		:void
	Return		:int
				 < status >
	Description	:get list of ownerships
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getListsOwnerships( struct ssl_session *session,
						struct http_ctx *hctx,
						const char *screen_name,
						const char *cursor )
{
	int		result;

	result = getTwapiListsOwnerships( session,
									  NULL,
									  screen_name,
									  DEF_TWOPE_MAX_LISTS_COUNT,
									  cursor );
	
	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getListsSubscriptions
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < cursor of list >
	Output		:void
	Return		:int
				 < status >
	Description	:get list of subscriptions
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getListsSubscriptions( struct ssl_session *session,
						   struct http_ctx *hctx,
						   const char *screen_name,
						   const char *cursor )
{
	int		result;

	result = getTwapiListsSubscriptions( session,
										 NULL,
										 screen_name,
										 DEF_TWOPE_MAX_LISTS_COUNT,
										 cursor );
	
	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getListsMemberships
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < cursor of list >
	Output		:void
	Return		:int
				 < status >
	Description	:get list of memberships
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getListsMemberships( struct ssl_session *session,
						 struct http_ctx *hctx,
						 const char *screen_name,
						 const char *cursor )
{
	int		result;

	result = getTwapiListsMemberships( session,
										NULL,
										screen_name,
										DEF_TWOPE_MAX_LISTS_COUNT,
										cursor,
										false );
	
	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getListsMembers
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *slug
				 < slug of list >
				 const char *owner_name
				 < screen name of list owner >
				 const char *cursor
				 < cursor in the list >
	Output		:void
	Return		:int
				 < status >
	Description	:get a list of members
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getListsMembers( struct ssl_session *session,
					 struct http_ctx *hctx,
					 const char *slug,
					 const char *owner_name,
					 const char *cursor )
{
	int		result;

	result = getTwapiListsMembers( session,
								   NULL,
								   slug,
								   owner_name,
								   NULL,
								   cursor,
								   false,
								   true );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getListsSubscribers
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *slug
				 < slug of list >
				 const char *owner_name
				 < screen name of list owner >
				 const char *cursor
				 < cursor in the list >
	Output		:void
	Return		:int
				 < status >
	Description	:get a list of subscribers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getListsSubscribers( struct ssl_session *session,
						 struct http_ctx *hctx,
						 const char *slug,
						 const char *owner_name,
						 const char *cursor )
{
	int		result;

	result = getTwapiListsSubscribers( session,
									   NULL,
									   slug,
									   owner_name,
									   NULL,
									   cursor,
									   false,
									   true );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:createListsMembers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *slug
				 < slug of list to create a member >
				 const char *owner_screen_name
				 < owner of the list >
				 const char *screen_name
				 < screen name to create >
	Output		:void
	Return		:int
				 < status >
	Description	:create a member of list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int createListsMembers( struct ssl_session *session,
						const char *slug,
						const char *owner_screen_name,
						const char *screen_name )
{
	int		result;

	result = postTwapiListsCreateMembers( session,
										  NULL,
										  slug,
										  NULL,
										  screen_name,
										  owner_screen_name,
										  NULL );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyListsMembers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *slug
				 < slug of list to destroy a member >
				 const char *owner_screen_name
				 < owner of the list >
				 const char *screen_name
				 < screen name to destroy >
	Output		:void
	Return		:int
				 < status >
	Description	:destroy a member of list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int destroyListsMembers( struct ssl_session *session,
						 const char *slug,
						 const char *owner_screen_name,
						 const char *screen_name )
{
	int		result;

	result = postTwapiListsDestroyMembers( session,
										   NULL,
										   slug,
										   NULL,
										   screen_name,
										   owner_screen_name,
										   NULL );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:subscribeLists
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *slug
				 < slug of list to subscribe >
				 const char *owner_screen_name
				 < owner of the list >
	Output		:void
	Return		:int
				 < status >
	Description	:create a subsribers of list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int subscribeLists( struct ssl_session *session,
					const char *slug,
					const char *owner_screen_name )
{
	int		result;

	result = postTwapiListsCreateSubscribers( session,
											  NULL,
											  slug,
											  owner_screen_name,
											  NULL );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:stopSubscriptionLists
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *slug
				 < slug of list to stop subscription >
				 const char *owner_screen_name
				 < owner of the list >
	Output		:void
	Return		:int
				 < status >
	Description	:destroy a subsribers of list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int stopSubscribeLists( struct ssl_session *session,
						const char *slug,
						const char *owner_screen_name )
{
	int		result;

	result = postTwapiListsDestroySubscribers( session,
											   NULL,
											   slug,
											   owner_screen_name,
											   NULL );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:createLists
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *name
				 < name of list to create >
				 const char *description
				 < description of the list >
	Output		:void
	Return		:int
				 < status >
	Description	:create a list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int createLists( struct ssl_session *session,
				 struct http_ctx *hctx,
				 const char *name,
				 const char *description )
{
	int		result;

	result = postListsCreate( session,
							  name,
							  false,
							  description );

	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:deleteLists
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *owner_screen_name
				 < owner of list >
				 const char *slug
				 < slug of list >
	Output		:void
	Return		:int
				 < status >
	Description	:delete a list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int deleteLists( struct ssl_session *session,
				 const char *owner_screen_name,
				 const char *slug )
{
	int		result;

	result = postListsDestroy( session,
							   owner_screen_name,
							   NULL,
							   NULL,
							   slug );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:showLists
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 const char *owner_screen_name
				 < owner of list >
				 const char *slug
				 < slug of list >
	Output		:void
	Return		:int
				 < status >
	Description	:show a list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int showLists( struct ssl_session *session,
			   struct http_ctx *hctx,
			   const char *owner_screen_name,
			   const char *slug )
{
	int		result;

	result = getTwapiListsShow( session,
								owner_screen_name,
								NULL,
								NULL,
								slug );
	
	if( result < 0 )
	{
		return( result );
	}

	result = twopeRecvHeaders( session, hctx );

	return( result );
}

/*
================================================================================

	OAuth

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:requestTwapiOauth
	Input		:struct ssl_session *session
				 < ssl session >
	Output		:void
	Return		:int
				 < status >
	Description	:request authorization to get access token and secret
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int requestTwapiOauth( struct ssl_session *session )
{
	int		result;

	/* ------------------------------------------------------------------------ */
	/* check whether need autorization											*/
	/* ------------------------------------------------------------------------ */
	if( !needAuthorization( ) )
	{
		return( 0 );
	}
	/* ------------------------------------------------------------------------ */
	/* get oauth token and secret												*/
	/* ------------------------------------------------------------------------ */
	if( ( result = postOauthRequestToken( session ) ) < 0 )
	{
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* authorize or authenticate												*/
	/* ------------------------------------------------------------------------ */
	if( strncmp( getOauthCallBackUrl( ), DEF_TWTR_HTTPH_AUTH_CALLBACK_URL,
				 sizeof( DEF_TWTR_HTTPH_AUTH_CALLBACK_URL ) - 1 ) == 0 )
	{
		int		pin_code_len;
		char	pin_code[ DEF_TWTR_PIN_CODE_MAX_SIZE ];

		printf( "Please input the following url to your browser, login twitter," );
		printf( " and take down a \"PIN CODE\" displayed on the browser.\n" );

		printf( "\n" );
		printf( "https://%s/%s/%s?%s=%s\n", getOauthHostName( ),
											DEF_TWTR_API_GRP_OAUTH,
											DEF_REST_OAUTH_AUTHORIZE,
											DEF_HTTPH_AUTH_TOKEN,
											getOauthAccessToken( ) );
		printf( "\n" );

		printf( "Then, enter the PIN CODE and the return key.\n" );
		printf( "-> " );
		fgets( pin_code, sizeof( pin_code ), stdin );
		pin_code_len = strlen( pin_code ) - 1;		// except for '\r'
		pin_code[ pin_code_len ] = '\0';			// remove '\r'
		fflush( stdin );

		setOauthVerifier( ( const char* )pin_code, pin_code_len );

		printf( "%s\n", getOauthVerifier( ) );

	}
	else
	{
		/* in the future, need to implement oauth/authenticate					*/
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get authorized access token and secret									*/
	/* ------------------------------------------------------------------------ */
	if( ( result = postOauthAccessToken( session ) ) < 0 )
	{
		printf( "Cannot be authorized.\n" );
		return( -1 );
	}

	//disconnectSSLServer( session );

	/* ------------------------------------------------------------------------ */
	/* save configurations														*/
	/* ------------------------------------------------------------------------ */
	result = writeTwapiConfigurations( getTwapiUserId( ),
									   getTwapiScreenName( ),
									   getOauthAccessToken( ),
									   getOauthAccessSecret( ) );

	return( 0 );
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
================================================================================
	Function	:twopeCommonRecvTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
	Output		:void
	Return		:int
				 < status >
	Description	:receive http headers
================================================================================
*/
static int
twopeRecvHeaders( struct ssl_session *session, struct http_ctx *hctx )
{
	int		result;
	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( hctx );

	result = recvHttpHeader( session, hctx );

	if( result < 0 || hctx->status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		logMessage( "status code : %d\n", hctx->status_code );
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx->status_code );
	logMessage( "content length : %d\n", hctx->content_length );
	logMessage( "------------------------------------\n" );

	return( result );
}

/*
================================================================================
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:void
================================================================================
*/

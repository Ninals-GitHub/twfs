/*******************************************************************************
 File:oauth.c
 Description:Oauth Procedures

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>

#include "lib/ascii.h"
#include "lib/sha1.h"
#include "lib/base64.h"
#include "lib/hmac.h"
#include "lib/log.h"
#include "net/network.h"
#include "net/twitter_api.h"
#include "net/ssl.h"
#include "net/oauth.h"
#include "net/http.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static void setOauthTimeStamp( char *timestamp, int size );
static void setOauthNonce( char *nonce, int size );
static int
encodePercent( const char *org, char *dst, int size, bool to_big );

/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_OAUTH_PARAM_BUF_SIZE	1000

/*
================================================================================

	Management

================================================================================
*/
#define	DEF_TIMESTAMP_SIZE				20

struct oauth_info
{
	char	*host_name;
	char	*call_back_url;
	char	*consumer_key;
	char	*consumer_secret;
	char	*access_token;
	char	*access_secret;
	char	*verifier;
};

static struct oauth_info oauth_info;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initOauth
	Input		:void
	Output		:void
	Return		:void
	Description	:initialize oauth
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initOauth( void )
{
	memset( ( void* )&oauth_info, 0x00, sizeof( oauth_info ) );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:registerOauthInfo
	Input		:const char *host_name
				 < host_name to which we request >
				 const char *call_back_url
				 < call back url when requesting request_token >
				 const char *consumer_key
				 < consumer key for oauth >
				 const char *consumer_secret
				 < consumer secret key for oauth >
				 const char *access_token
				 < access token for oauth >
				 const char *access_secret
				 < access secret key for oauth >
	Output		:void
	Return		:int
				 < status >
	Description	:register oauth information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int registerOauthInfo( const char *host_name,
					   const char *call_back_url,
					   const char *consumer_key,
					   const char *consumer_secret,
					   const char *access_token,
					   const char *access_secret )
{
	int		size;

	if( !host_name || !consumer_key || !consumer_secret )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* save host name															*/
	/* ------------------------------------------------------------------------ */
	size = strnlen( host_name, DEF_NET_MAX_HOST_NAME );

	oauth_info.host_name = ( char* )malloc( size + 1 );

	if( !oauth_info.host_name )
	{
		return( -1 );
	}

	memcpy( ( void* )oauth_info.host_name, ( void* )host_name, size + 1 );

	/* ------------------------------------------------------------------------ */
	/* save host name															*/
	/* ------------------------------------------------------------------------ */
	if( !call_back_url )
	{
		oauth_info.call_back_url = NULL;
	}
	else
	{
		size = strnlen( call_back_url, DEF_OAUTH_MAX_CALL_BACK_LEN );

		oauth_info.call_back_url = ( char* )malloc( size + 1 );

		if( !oauth_info.call_back_url )
		{
			unregisterOauthInfo( );
			return( -1 );
		}
		memcpy( ( void* )oauth_info.call_back_url,
				( void* )call_back_url,
				size + 1 );
	}

	/* ------------------------------------------------------------------------ */
	/* save consumer key														*/
	/* ------------------------------------------------------------------------ */
	size = strnlen( consumer_key, DEF_OAUTH_MAX_CONSUMER_KEY  );
	
	oauth_info.consumer_key	= ( char* )malloc( size + 1 );

	if( !oauth_info.consumer_key )
	{
		unregisterOauthInfo( );
		return( -1 );
	}

	memcpy( ( void* )oauth_info.consumer_key, ( void* )consumer_key, size + 1 );

	/* ------------------------------------------------------------------------ */
	/* save consumer secret														*/
	/* ------------------------------------------------------------------------ */
	size = strnlen( consumer_secret, DEF_OAUTH_MAX_CONSUMER_SECRET );

	oauth_info.consumer_secret = ( char* )malloc( size + 1 );

	if( !oauth_info.consumer_secret )
	{
		unregisterOauthInfo( );
		return( -1 );
	}

	memcpy( ( void* )oauth_info.consumer_secret,
			( void* )consumer_secret,
			size + 1 );

	/* ------------------------------------------------------------------------ */
	/* save access token														*/
	/* ------------------------------------------------------------------------ */
	if( !access_token || !access_secret )
	{
		/* if null, oauth_request should be issued								*/
		oauth_info.access_token		= NULL;
		oauth_info.access_secret	= NULL;
		return( 0 );
	}

	size = strnlen( access_token, DEF_OAUTH_MAX_ACCESS_TOKEN );
	
	oauth_info.access_token	= ( char* )malloc( size + 1 );

	if( !oauth_info.access_token )
	{
		unregisterOauthInfo( );
		return( -1 );
	}

	memcpy( ( void* )oauth_info.access_token, ( void* )access_token, size + 1 );

	/* ------------------------------------------------------------------------ */
	/* save access secret														*/
	/* ------------------------------------------------------------------------ */
	size = strnlen( access_secret, DEF_OAUTH_MAX_ACCESS_SECRET );
	oauth_info.access_secret= ( char* )malloc( size + 1 );
	
	if( !oauth_info.access_secret )
	{
		unregisterOauthInfo( );
		return( -1 );
	}

	memcpy( ( void* )oauth_info.access_secret,
			( void* )access_secret,
			size + 1 );
	
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:unregisterOauthInfo
	Input		:void
	Output		:void
	Return		:void
	Description	:unregister oauth information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void unregisterOauthInfo( void )
{
	if( oauth_info.host_name )
	{
		free( ( void* )oauth_info.host_name );
		oauth_info.host_name = NULL;
	}

	if( oauth_info.call_back_url )
	{
		free( ( void* )oauth_info.call_back_url );
		oauth_info.call_back_url = NULL;
	}

	if( oauth_info.consumer_key )
	{
		free( ( void* )oauth_info.consumer_key );
		oauth_info.consumer_key = NULL;
	}

	if( oauth_info.consumer_secret )
	{
		free( ( void* )oauth_info.consumer_secret );
		oauth_info.consumer_secret = NULL;
	}

	if( oauth_info.verifier )
	{
		free( ( void* )oauth_info.verifier );
		oauth_info.verifier = NULL;
	}

	destroyAccessKey( );

	return;
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyAccessKey
	Input		:void
	Output		:void
	Return		:void
	Description	:destroy access key
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroyAccessKey( void )
{
	if( oauth_info.access_token )
	{
		free( ( void* )oauth_info.access_token );
		oauth_info.access_token = NULL;
	}

	if( oauth_info.access_secret )
	{
		free( ( void* )oauth_info.access_secret );
		oauth_info.access_secret = NULL;
	}

	if( oauth_info.verifier )
	{
		free( ( void* )oauth_info.verifier );
		oauth_info.verifier = NULL;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setOauthAccessToken
	Input		:const char *token
				 < access token >
				 int length
				 < length of access token >
	Output		:void
	Return		:int
				 < status >
	Description	:set oauth access token
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int setOauthAccessToken( const char *token, int length )
{
	oauth_info.access_token = ( char* )malloc( length + 1 );
	if( !oauth_info.access_token )
	{
		return( -1 );
	}
	memcpy( oauth_info.access_token, token, length );
	oauth_info.access_token[ length ] = '\0';

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setOauthAccessSecret
	Input		:const char *secret
				 < access token >
				 int length
				 < length of access secret >
	Output		:void
	Return		:int
				 < status >
	Description	:set oauth access secret
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int setOauthAccessSecret( const char *secret, int length )
{
	oauth_info.access_secret = ( char* )malloc( length + 1 );
	if( !oauth_info.access_secret )
	{
		return( -1 );
	}
	memcpy( oauth_info.access_secret, secret, length );
	oauth_info.access_secret[ length ] = '\0';

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setOauthVerifier
	Input		:const char *verifier
				 < verifier >
				 int length
				 < length of verifier >
	Output		:void
	Return		:int
				 < status >
	Description	:set oauth verifier
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int setOauthVerifier( const char *verifier, int length )
{
	oauth_info.verifier = ( char* )malloc( length + 1 );
	if( !oauth_info.verifier )
	{
		return( -1 );
	}
	memcpy( oauth_info.verifier, verifier, length );
	oauth_info.verifier[ length ] = '\0';

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getOauthAccessToken
	Input		:void
	Output		:void
	Return		:const char*
				 < access token >
	Description	:get oauth access token
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getOauthAccessToken( void )
{
	return( ( const char* )oauth_info.access_token );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getOauthAccessSecret
	Input		:void
	Output		:void
	Return		:const char*
				 < access secret >
	Description	:get oauth access secret
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getOauthAccessSecret( void )
{
	return( ( const char* )oauth_info.access_secret );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getOauthHostName
	Input		:void
	Output		:void
	Return		:const char*
				 < host name >
	Description	:get oauth host name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getOauthHostName( void )
{
	return( ( const char* )oauth_info.host_name );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getOauthCallBackUrl
	Input		:void
	Output		:void
	Return		:const char*
				 < callback url >
	Description	:get oauth callback url
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getOauthCallBackUrl( void )
{
	return( ( const char* )oauth_info.call_back_url );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getOauthVerifier
	Input		:void
	Output		:void
	Return		:const char*
				 < verifier >
	Description	:get oauth verifier
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getOauthVerifier( void )
{
	return( ( const char* )oauth_info.verifier );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:needAuthorization
	Input		:void
	Output		:void
	Return		:bool
				 < true:need to authorize >
	Description	:test whether we have access token and secret
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
bool needAuthorization( void )
{
	if( oauth_info.access_token && oauth_info.access_secret )
	{
		return( false );
	}
	return( true );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:sendOauthMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *http_com
				 < http requet command >
				 const char *api_grp
				 < api gropu for requesting >
				 const char *request
				 < api of the service >
				 struct req_param *req_param
				 < array of parameters of api >
				 int n_param
				 < number of array of parameters of api >
				 bool connection
				 < true:keep alive false:close >
	Output		:void
	Return		:int
				 < result >
	Description	:make oauth message and send it
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int sendOauthMessage( struct ssl_session *session,
					  const char *http_com,
					  const char *api_grp,
					  const char *request,
					  struct req_param *req_param,
					  int n_param,
					  bool connection )
{
	char	oauth_nonce[ 32 + 1 ],
			oauth_timestamp[ DEF_TIMESTAMP_SIZE + 1 ],
			buffer[ 2000 ];
	char	*signing_key;
	char	*hash_base64;
	char	*oauth_signature;
	char	*param;
	int		skey_len;
	int		length;
	int		com_len;
	int		i;
	int		result;
	int		param_index;
	int		oauth_size;
	int		xoauth_size;
	int		param_len;

	unsigned char	hash[ DEF_SHA1_DIGEST_LENGTH ];

	if( !session || !http_com || !request )
	{
		logMessage( "invalid arguments\n" );
		return( -1 );
	}

	if( ( result = isSSLConnected( session ) ) < 0 )
	{
		logMessage( "recconect at oauth\n" );
		if( ( result = reopenSSLSession( session ) ) < 0 )
		{
			logMessage("cannnot reconnect\n" );
			return( result );
		}
	}

	logMessage("send oauth\n" );


	/* ------------------------------------------------------------------------ */
	/* allocate temporary buffer												*/
	/* ------------------------------------------------------------------------ */
	if( !( param = malloc( DEF_OAUTH_PARAM_BUF_SIZE ) ) )
	{
		logMessage( "cannot allocate param\n" );
	}

	oauth_size	= strlen( "oauth_" );
	xoauth_size	= strlen( "xoauth_" );

	param_index = 0;

	/* ------------------------------------------------------------------------ */
	/* make signature base														*/
	/* ------------------------------------------------------------------------ */
	length = snprintf( buffer, sizeof( buffer ), "%s&", 	http_com );
	param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						  //"https://%s%s", oauth_info.host_name, api_grp );
						  "https://%s%s%s",
						  oauth_info.host_name, api_grp, request );

	length += encodePercent( param, &buffer[ length ], param_len, false );
	length += snprintf( &buffer[ length ], sizeof( buffer ) - length ,
						//"%s&", request );
						"&" );

	/* insert befor oauth_callback						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_CALLBACK, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_CALLBACK ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
							   "%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ],
									 param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}
	/* oauth_callback									*/
	if( strncmp( request, DEF_OAUTH_REQ_TOKEN,
				 sizeof( DEF_OAUTH_REQ_TOKEN ) - 1 ) == 0 )
	{
		length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
						   "%s", DEF_HTTPH_AUTH_CALLBACK );
		/* twice percent encode							*/
		encodePercent( oauth_info.call_back_url,
					   &buffer[ length ],
					   strnlen( oauth_info.call_back_url,
								DEF_OAUTH_MAX_CALL_BACK_LEN ),
					   false );
		param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
							  "=%s&", &buffer[ length ] );
		length += encodePercent( param, &buffer[ length ], param_len, false );
	}

	/* insert befor oauth_consumer_key					*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_CONSUMER_KEY, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_CONSUMER_KEY ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ],
									 param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* oauth_consumer_key								*/
	length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
						"%s", DEF_HTTPH_AUTH_CONSUMER_KEY );
	param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						  "=%s&", oauth_info.consumer_key );

	length += encodePercent( param, &buffer[ length ], param_len, false );

	/* insert befor oauth_nonce						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_NONCE, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_NONCE ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* oauth_nonce										*/
	length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
						"%s", DEF_HTTPH_AUTH_NONCE );
	setOauthNonce( oauth_nonce, sizeof( oauth_nonce ) );
	param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE, "=%s&", oauth_nonce );

	length += encodePercent( param, &buffer[ length ], param_len, false );

	/* insert befor oauth_signature_method				*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_SIG_METHOD, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_SIG_METHOD ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* oauth_signature_method							*/
	length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
						"%s", DEF_HTTPH_AUTH_SIG_METHOD );
	param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						  "=%s&", DEF_HTTPH_AUTH_SIG_HMAC_SHA1 );

	length += encodePercent( param, &buffer[ length ], param_len, false );

	/* insert befor oauth_timestamp						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_TIMESTAMP, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_TIMESTAMP ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* oauth_timestamp									*/
	length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
						"%s", DEF_HTTPH_AUTH_TIMESTAMP );
	setOauthTimeStamp( oauth_timestamp, sizeof( oauth_timestamp ) );
	param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						  "=%s&", oauth_timestamp );

	length += encodePercent( param, &buffer[ length ], param_len, false );

	/* insert befor oauth_token							*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_TOKEN, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_TOKEN ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}
	/* oauth_token										*/
	if( oauth_info.access_token )
	{
		if( strncmp( request, DEF_OAUTH_REQ_TOKEN,
					 sizeof( DEF_OAUTH_REQ_TOKEN ) - 1 ) != 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", DEF_HTTPH_AUTH_TOKEN );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", oauth_info.access_token );	// token is raw data

			length += encodePercent( param, &buffer[ length ], param_len, false );
		}
	}

	/* insert befor oauth_version						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_VERSION, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_VERSION ) - 1 ) > 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s&", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}
	/* oauth_version									*/
	length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
						"%s", DEF_HTTPH_AUTH_VERSION );
	param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						  "=%s", DEF_HTTPH_AUTH_VERSION_NUM );

	length += encodePercent( param, &buffer[ length ], param_len, false );

	/* insert after oauth_version						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( DEF_HTTPH_AUTH_VERSION, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_VERSION ) - 1 ) < 0 )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%%26" );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s", req_param[ i ].name );
			/* twice percent encode						*/
			param_len = strnlen( req_param[ i ].param,
								 DEF_API_MAX_REQ_PARAM_LEN );
			encodePercent( req_param[ i ].param, &buffer[ length ],
						   param_len, false );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "=%s", &buffer[ length ] );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;
		}
	}

	param_index = 0;

	logMessage( "%s\n", buffer );
	/* ------------------------------------------------------------------------ */
	/* make signing key															*/
	/* ------------------------------------------------------------------------ */
	if( !oauth_info.consumer_secret )
	{
		logMessage( "consumer secret is not set\n" );
		return( -1 );
	}

	if( !oauth_info.access_secret )
	{
		skey_len = strnlen( ( const char* )oauth_info.consumer_secret,
							DEF_OAUTH_MAX_CONSUMER_SECRET )
					+ sizeof( char );	// null terminator
		logMessage( "skey_lne1: %d\n", skey_len );
	}
	else
	{
		skey_len = strnlen( ( const char* )oauth_info.consumer_secret,
							DEF_OAUTH_MAX_CONSUMER_SECRET )
					+ strnlen( ( const char* )oauth_info.access_secret,
							   DEF_OAUTH_MAX_ACCESS_SECRET )
					+ sizeof( char );	// null terminator
		logMessage( "skey_lne2: %d\n", skey_len );
	}
	signing_key = ( char* )malloc( skey_len );

	if( !signing_key )
	{
		free( param );
		logMessage( "cannot allocate signing key\n" );
		return( -1 );
	}

	if( !oauth_info.consumer_secret )
	{
		free( param );
		logMessage( "conuser secret is not set\n" );
		return( -1 );
	}
	
	if( !oauth_info.access_secret )
	{
		snprintf( signing_key, skey_len + 1, "%s&", oauth_info.consumer_secret );
	}
	else
	{
		snprintf( signing_key, skey_len + 1, "%s&%s",
				 oauth_info.consumer_secret, oauth_info.access_secret );
	}

	/* ------------------------------------------------------------------------ */
	/* encode hmac-sha1															*/
	/* ------------------------------------------------------------------------ */
	logMessage( "key : %s\n", signing_key );
	logMessage( "buffer length : %d\n", length - 1 );
	logMessage( "buffer strlen : %d\n", strlen( buffer ) );
	logMessage( "signing_key length : %d\n", skey_len );
	logMessage( "signing_key strlen : %d\n", strlen( signing_key ) );
	result = encodeHmacSha1( ( unsigned char* )buffer, length,
							 ( unsigned char* )signing_key, skey_len, hash );

	free( signing_key );

	if( result < 0 )
	{
		free( param );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get signature															*/
	/* ------------------------------------------------------------------------ */
	hash_base64		= ( char* )malloc( DEF_SHA1_DIGEST_LENGTH * 3 );

	if( !hash_base64 )
	{
		logMessage( "cannot allocate hash_base64f\n" );

		free( param );
		return( -1 );
	}

	logMessage( "size of hmac hash:%d\n", result );

	result = encodeBase64( (const char*) hash, result, hash_base64 );

	oauth_signature	= ( char* )malloc( DEF_SHA1_DIGEST_LENGTH * 3 );

	if( !oauth_signature )
	{
		free( hash_base64 );
		free( param );
		logMessage( "cannot allocate hash_hmac\n" );
		return( -1 );
	}

	result = encodePercent( hash_base64, oauth_signature, result, false );
	logMessage( "hash_hmac:%s %d\n", oauth_signature, result );

	free( hash_base64 );

	/* ------------------------------------------------------------------------ */
	/* send a command															*/
	/* ------------------------------------------------------------------------ */
	/* POST												*/
	length = snprintf( buffer, sizeof( buffer ),
					   "%s %s%s", http_com, api_grp, request );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	length = 0;
	
	/* insert get parameters													*/
	if( strncmp( http_com, DEF_HTTPH_GET, strlen( DEF_HTTPH_GET ) ) == 0 )
	{
		bool	first_amp = true;

		for( i = param_index ; i < n_param ; i++ )
		{
			if( strncmp( "oauth_", req_param[ i ].name,
						 sizeof( "oauth_" ) - 1 ) == 0 )
			{
				continue;
			}

			if( strncmp( "xoauth_", req_param[ i ].name,
						 sizeof( "xoauth_" ) - 1 ) == 0 )
			{
				continue;
			}

			if( first_amp )
			{
				length = snprintf( &buffer[ length ], sizeof( buffer ) - length,
								   "?" );
				first_amp = false;
			}
			else
			{
				length = snprintf( &buffer[ length ], sizeof( buffer ) - length,
								   "&" );
			}

			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"%s=", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			param_index = i + 1;

			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( param );
				free( oauth_signature );
				return( -1 );
			}
			
			length = 0;
		}
	}

	length += snprintf( buffer, sizeof( buffer ), " %s\r\n", DEF_HTTPH_HTTP_VER );
	
	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	/* send User-Agent									*/
	length = snprintf( buffer, sizeof( buffer ), "%s\r\n", DEF_HTTPH_USER_AGENT_FS );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	/* send Host										*/
	length = snprintf( buffer, sizeof( buffer ),
					   "%s %s\r\n", DEF_HTTPH_HOST, oauth_info.host_name );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	/* send Accept										*/
	length = snprintf( buffer, sizeof( buffer ), "%s\r\n", DEF_HTTPH_ACCEPT_ASTA );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	/* send Connection									*/
	if( connection )
	{
		length = snprintf( buffer, sizeof( buffer ),
						   "%s\r\n", DEF_HTTPH_CONNECTION_ALIVE );
	}
	else
	{
		length = snprintf( buffer, sizeof( buffer ),
						   "%s\r\n", DEF_HTTPH_CONNECTION_CLOSE );
	}

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	/* send Content-Type								*/
	length = snprintf( buffer, sizeof( buffer ),
					   "%s\r\n", DEF_HTTPH_CONTENT_TYPE_URLENC );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	/* Authorization									*/
	length = snprintf( buffer, sizeof( buffer ),
					   "%s ", DEF_HTTPH_AUTHORIZATION_OAUTH );


	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		free( oauth_signature );
		return( -1 );
	}

	param_index = 0;

	/* insert befor oauth_callback						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_CALLBACK, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_CALLBACK ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   "%s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
							   "\", " );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}
	
	/* Authorization:oauth_callback						*/
	if( strncmp( request, DEF_OAUTH_REQ_TOKEN,
				 sizeof( DEF_OAUTH_REQ_TOKEN ) - 1 ) == 0 )
	{
		length = snprintf( buffer, sizeof( buffer ),
						   "%s=\"", DEF_HTTPH_AUTH_CALLBACK );
		param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
							  "%s", oauth_info.call_back_url );
		length += encodePercent( param, &buffer[ length ], param_len, false );
		length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
							"\"," );

		if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
		{
			free( oauth_signature );
			free( param );
			return( -1 );
		}
	}

	/* insert befor oauth_consumer_key					*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_CONSUMER_KEY, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_CONSUMER_KEY ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   "%s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								 "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\", " );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization:oauth_consumer_key					*/
	length = snprintf( buffer, sizeof( buffer ),
					   "%s=\"%s\"", DEF_HTTPH_AUTH_CONSUMER_KEY,
											 oauth_info.consumer_key );
	
	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( oauth_signature );
		free( param );
		return( -1 );
	}

	/* insert befor oauth_consumer_key					*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_NONCE, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_NONCE ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization:oauth_nonce						*/
	length = snprintf( buffer, sizeof( buffer ),
					   ", %s=\"%s\"", DEF_HTTPH_AUTH_NONCE, oauth_nonce );
	
	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( oauth_signature );
		free( param );
		return( -1 );
	}

	/* insert befor oauth_signature						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_SIGNATURE, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_SIGNATURE ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}
	
	/* Authorization:oauth_signature					*/
	length = snprintf( buffer, sizeof( buffer ),
					   ", %s=\"%s\"", DEF_HTTPH_AUTH_SIGNATURE,
											 oauth_signature );
	free( oauth_signature );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	/* insert befor oauth_signature_method				*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_SIG_METHOD, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_SIG_METHOD ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization:oauth_signature_method				*/
	length = snprintf( buffer, sizeof( buffer ),
					   ", %s=\"%s\"", DEF_HTTPH_AUTH_SIG_METHOD,
											 DEF_HTTPH_AUTH_SIG_HMAC_SHA1 );
	
	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	/* insert befor oauth_timestamp						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_TIMESTAMP, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_TIMESTAMP ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization:oauth_timestamp					*/
	length = snprintf( buffer, sizeof( buffer ),
					   ", %s=\"%s\"", DEF_HTTPH_AUTH_TIMESTAMP,
											 oauth_timestamp );
	
	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	/* insert befor oauth_token							*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_TOKEN, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_TOKEN ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization:oauth_token						*/
	if( oauth_info.access_token )
	{
		if( strncmp( request, DEF_OAUTH_REQ_TOKEN,
					 sizeof( DEF_OAUTH_REQ_TOKEN ) - 1 ) != 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"%s\"", DEF_HTTPH_AUTH_TOKEN,
													 oauth_info.access_token );

			if( sendSSLMessage( session, ( const unsigned char* )buffer, length ) < 0 )
			{
				free( param );
				return( -1 );
			}
		}
	}

	/* insert befor oauth_version						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					  sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( strncmp( "oauth_", req_param[ i ].name,
					 sizeof( "oauth_" ) - 1 ) != 0 )
		{
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_VERSION, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_VERSION ) - 1 ) > 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ],
									 param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization:oauth_version						*/
	length = snprintf( buffer, sizeof( buffer ),
					   ", %s=\"%s\"", DEF_HTTPH_AUTH_VERSION,
											DEF_HTTPH_AUTH_VERSION_NUM );
	
	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	/* insert after oauth_version						*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			param_index = i;
			break;
		}

		if( 0 != strncmp( "oauth_", req_param[ i ].name, sizeof( "oauth_" ) - 1 ) )
		{
			param_index = i;
			continue;
		}

		if( strncmp( DEF_HTTPH_AUTH_VERSION, req_param[ i ].name,
					 sizeof( DEF_HTTPH_AUTH_VERSION ) - 1 ) < 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* insert xoauth									*/
	for( i = param_index ; i < n_param ; i++ )
	{
		if( strncmp( "xoauth_", req_param[ i ].name,
					 sizeof( "xoauth_" ) - 1 ) == 0 )
		{
			length = snprintf( buffer, sizeof( buffer ),
							   ", %s=\"", req_param[ i ].name );
			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
			length += snprintf( &buffer[ length ], sizeof( buffer ), "\"" );
			/* send message														*/
			if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
			{
				free( oauth_signature );
				free( param );
				return( -1 );
			}
			param_index = i + 1;
		}
		else
		{
			param_index = i;
			break;
		}
	}

	/* Authorization: Oatuh's <CR><LF>					*/
	length = sprintf( buffer, "\r\n" );

	if( sendSSLMessage( session, ( void* )buffer, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	length		= 0;
	com_len		= 0;
	param_index	= 0;

	/* make post body									*/
	if( strncmp( http_com, DEF_HTTPH_POST, strlen( DEF_HTTPH_POST ) ) == 0 )
	{
		bool	insert_amp = false;
		for( i = param_index ; i < n_param ; i++ )
		{
			if( strncmp( "oauth_", req_param[ i ].name,
						 sizeof( "oauth_" ) - 1 ) == 0 )
			{
				continue;
			}

			if( strncmp( "xoauth_", req_param[ i ].name,
						 sizeof( "xoauth_" ) - 1 ) == 0 )
			{
				continue;
			}

			if( insert_amp )
			{
				length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
									"&%s=", req_param[ i ].name );
			}
			else
			{
				length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
									"%s=", req_param[ i ].name );
				insert_amp = true;
			}

			param_len = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
								  "%s", req_param[ i ].param );
			length += encodePercent( param, &buffer[ length ], param_len, false );
		}

		if( 0 < length )
		{
			length += snprintf( &buffer[ length ], sizeof( buffer ) - length,
								"\r\n" );
			com_len = length;
		}
	}
	
	/* send Content-Length								*/
	if( 2 <= com_len )
	{
		length = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						   //"%s %d\r\n", DEF_HTTPH_CONTENT_LENGTH, com_len );
						   "%s %d\r\n", DEF_HTTPH_CONTENT_LENGTH, com_len - 2 );
						   // -2 means except for \r\n 
	}
	else
	{
		length = snprintf( param, DEF_OAUTH_PARAM_BUF_SIZE,
						   "%s 0\r\n", DEF_HTTPH_CONTENT_LENGTH );
	}

	if( sendSSLMessage( session, ( void* )param, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	/* <CR><LF>											*/
	length = sprintf( param, "\r\n" );

	if( sendSSLMessage( session, ( void* )param, length ) < 0 )
	{
		free( param );
		return( -1 );
	}

	free( param );

	/* send  post body									*/
	if( strncmp( http_com, DEF_HTTPH_POST, strlen( DEF_HTTPH_POST ) ) == 0 )
	{
		if( 0 < com_len )
		{
			buffer[ com_len + 1 ] = '\0';
			if( sendSSLMessage( session, ( void* )buffer, com_len ) < 0 )
			{
				return( -1 );
			}
		}
	}

	return( 0 );
}


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
	Function	:setOauthTimeStamp
	Input		:char *timestamp
				 < timestamp buffer >
				 int size
				 < size of timestamp buffer >
	Output		:unsigned char *timestamp
	Return		:void
	Description	:set unix epoc
================================================================================
*/
static void setOauthTimeStamp( char *timestamp, int size )
{
	int		i;
	time_t	unix_time;

	for( i = 0 ; i < size ; i++ )
	{
		timestamp[ i ] = 0x00;
	}

	time( &unix_time );

	sprintf( timestamp, "%ld", unix_time );
}


/*
================================================================================
	Function	:setOauthNonce
	Input		:char *nonce
				 < once number buffer >
				 int size
				 < size of buffer >
	Output		:unsigned char *nonce
	Return		:void
	Description	:set random number to nonce
================================================================================
*/
static void setOauthNonce( char *nonce, int size )
{
	static unsigned int count = 0;
	int		i;
	
	for( i = 0 ; i < size ; i++ )
	{
		nonce[ i ] = 0x00;
	}

	srand( ( int )time( NULL ) );

	//sprintf( nonce, "%ld%d", ( long int )rand( ), count++ );
	sprintf( nonce, "%016d%016d", ( int )rand( ), count++ );

	count++;
}
/*
================================================================================
	Function	:encodePercent
	Input		:const unsigned char *org
				 < original message >
				 unsigned char *dst
				 < buffer to store encoded message >
				 int size
				 < size of original message >
				 BOOL to_big
				 < True:alphabets are translated to big capital >
	Output		:unsignec char *dst
				 < encedd message >
	Return		:int
				 < the length of encoded message >
	Description	:percentage encodign
================================================================================
*/
static int encodePercent( const char *org, char *dst, int size, bool to_big )
{
	int		org_i;
	int		dst_i;

	for( org_i = 0, dst_i = 0 ; org_i < size ; org_i++ )
	{
		if( ( '0' <= org[ org_i ] ) && ( org[ org_i ] <= '9' ) )
		{
			dst[ dst_i++ ] = org[ org_i ];
			continue;
		}

		if( ( 'A' <= org[ org_i ] ) && ( org[ org_i ] <= 'Z' ) )
		{
			dst[ dst_i++ ] = org[ org_i ];
			continue;
		}

		if( ( 'a' <= org[ org_i ] ) && ( org[ org_i ] <= 'z' ) )
		{
			if( to_big )
			{
				dst[ dst_i++ ] = 'A' + ( org[ org_i ] - 'a' );
			}
			else
			{
				dst[ dst_i++ ] = org[ org_i ];
			}
			continue;
		}

		if( ( '-' == org[ org_i ] ) ||
			( '.' == org[ org_i ] ) ||
			( '_' == org[ org_i ] ) ||
			( '~' == org[ org_i ] ) )
		{
			dst[ dst_i++ ] = org[ org_i ];
			continue;
		}

		dst[ dst_i++ ] = '%';
		dst[ dst_i++ ] = hex2Ascii( ( ( org[ org_i ] & 0xF0 ) >> 4 ) & 0x0F );
		dst[ dst_i++ ] = hex2Ascii( org[ org_i ] & 0x0F );
	}

	dst[ dst_i ] = 0x00;

	return( dst_i );
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

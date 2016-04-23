/*******************************************************************************
 File:oauth.h
 Description:Definitions of oath

*******************************************************************************/
#ifndef	__OAUTH_H__
#define	__OAUTH_H__

#include <stdbool.h>

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
#define	CONCAT2( STR1, STR2 )				STR1 STR2
#define	CONCAT3( STR1, STR2, STR3 )			STR1 STR2 STR3
#define	CONCAT4( STR1, STR2, STR3, STR4 )	STR1 STR2 STR3 STR4

#define	DEF_OAUTH_CONNECTION_ALIVE			(true)
#define	DEF_OAUTH_CONNECTION_CLOSE			(false)


#define	DEF_OAUTH_MAX_CONSUMER_KEY			64
#define	DEF_OAUTH_MAX_CONSUMER_SECRET		64
#define	DEF_OAUTH_MAX_ACCESS_TOKEN			64
#define	DEF_OAUTH_MAX_ACCESS_SECRET			64

#define	DEF_OAUTH_MAX_CALL_BACK_LEN			1024
/*
--------------------------------------------------------------------------------
	OAuth Http Header
--------------------------------------------------------------------------------
*/
#define	DEF_HTTPH_AUTH_CONSUMER_KEY			"oauth_consumer_key"
#define	DEF_HTTPH_AUTH_NONCE				"oauth_nonce"
#define	DEF_HTTPH_AUTH_CALLBACK				"oauth_callback"
#define	DEF_HTTPH_AUTH_SIGNATURE			"oauth_signature"
#define	DEF_HTTPH_AUTH_SIG_METHOD			"oauth_signature_method"
#define	DEF_HTTPH_AUTH_SIG_HMAC_SHA1		"HMAC-SHA1"
#define	DEF_HTTPH_AUTH_TIMESTAMP			"oauth_timestamp"
#define	DEF_HTTPH_AUTH_TOKEN				"oauth_token"
#define	DEF_HTTPH_AUTH_TOKEN_SECRET			"oauth_token_secret"
#define	DEF_HTTPH_AUTH_VERSION				"oauth_version"
#define	DEF_HTTPH_AUTH_VERSION_NUM			"1.0"

/*
--------------------------------------------------------------------------------
	OAuth
--------------------------------------------------------------------------------
*/
// API Group
#define	DEF_API_GRP_OAUTH					"oauth"
#define	DEF_API_GRP_OAUTH2					"oauth2"

// GET oauth/authenticate
#define	DEF_OAUTH_AUTHENTICATE				"authenticate"
// GET oauth/authorize
#define	DEF_OAUTH_AUTHORIZE					"authorize"
// POST oauth/access_token
#define	DEF_OAUTH_ACCESS_TOKEN				"access_token"
#define	DEF_OAUTH_ACCESS_SECRET				"access_secret"
// POST oauth/request_token
#define	DEF_OAUTH_REQ_TOKEN					"request_token"
// POST oauth2/token
#define	DEF_OAUTH2_TOKEN					"token"
// POST oauth2/invalidate_token
/*
================================================================================

	Management

================================================================================
*/
#define	DEF_API_NO_REQ_PARAM		NULL
#define	DEF_API_NO_REQ_PARAM_NUM	0
#define	DEF_API_MAX_REQ_PARAM_LEN	1024
struct req_param
{
	char	*name;			// name of parameter
	char	*param;			// value of parameter
};

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
void initOauth( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:registerOauthInfo
	Input		:const char *host_name
				 < host name to which we request >
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
					   const char *access_secret );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:unregisterOauthInfo
	Input		:void
	Output		:void
	Return		:void
	Description	:unregister oauth information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void unregisterOauthInfo( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyAccessKey
	Input		:void
	Output		:void
	Return		:void
	Description	:destroy access key
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroyAccessKey( void );

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
int setOauthAccessToken( const char *token, int length );

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
int setOauthAccessSecret( const char *secret, int length );

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
int setOauthVerifier( const char *verifier, int length );

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
const char* getOauthAccessToken( void );

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
const char* getOauthAccessSecret( void );

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
const char* getOauthHostName( void );

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
const char* getOauthCallBackUrl( void );

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
const char* getOauthVerifier( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:needAuthorization
	Input		:void
	Output		:void
	Return		:bool
				 < true:need to authorize >
	Description	:test whether gotten access token and secret
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
bool needAuthorization( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:sendOauthMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *http_com
				 < http command >
				 const char *api_grp
				 < api gropu for requesting >
				 < http requet command >
				 const char *request
				 < api of the service >
				 struct req_param **req_param
				 < array of parameters of api >
				 int n_param
				 < number of array of parameters of api >
				 bool connection
				 < true:keep alive flase:close >
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
					  bool connection );


#endif	// __OAUTH_H__

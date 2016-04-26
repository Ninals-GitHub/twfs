/*******************************************************************************
 File:twitter_api.c
 Description:Procedures of Twitter API

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "twitter_operation.h"
#include "lib/ascii.h"
#include "lib/log.h"
#include "lib/utf.h"
#include "net/twitter_api.h"
#include "net/oauth.h"
#include "net/http.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int
recvTokenAndSecret( struct ssl_session *session,
					struct http_ctx *hctx,
					bool screen_name );
static int recvDiscard( struct ssl_session *session, struct http_ctx *hctx );
static int commonStatusesTimeLine( const char *request,
								   struct ssl_session *session,
								   const int count,
								   const char *since_id,
								   const char *max_id,
								   const bool trim_user,
								   const bool exclude_replies,
								   const bool contributor_details,
								   const bool include_entities,
								   const char *user_id,
								   const char *screen_name,
								   const bool include_rts,
								   const bool include_user_entities );
static int commonRecvTimeLine( struct ssl_session *session );
static int commonGetLists( const char *request,
						   struct ssl_session *session,
						   const char *user_id,
						   const char *screen_name,
						   const int count,
						   const char *cursor,
						   const bool filter_to_owned_lists );
static int commonGetMembers( const char *request,
							 struct ssl_session *session,
							 const char *list_id,
							 const char *slug,
							 const char *owner_screen_name,
							 const char *owner_id,
							 const char *cursor,
							 bool include_entities,
							 bool skip_status );
static int commonMembersCreateDestroy( const char *request,
									   struct ssl_session *session,
									   const char *list_id,
									   const char *slug,
									   const char *user_id,
									   const char *screen_name,
									   const char *owner_screen_name,
									   const char *owner_id );
static int commonSubscribersCreateDestroy( const char *request,
										   struct ssl_session *session,
										   const char *list_id,
										   const char *slug,
										   const char *owner_screen_name,
										   const char *owner_id );
static int
commonGetFriendsFollowers( struct ssl_session *session,
						   const char *request_grp,
						   const char *user_id,
						   const char *screen_name,
						   const char *cursor,
						   const bool stringify_ids,
						   const int count );
static int
commonPostFriendshipCreateDestroy( struct ssl_session *session,
								   const char *request,
								   const char *user_id,
								   const char *screen_name,
								   const bool follow );
static int
commonGetFriendsFollowersList( struct ssl_session *session,
							   const char *request_grp,
							   const char *user_id,
							   const char *screen_name,
							   const char *cursor,
							   const int count,
							   const bool skip_status,
							   const bool include_user_entities );
static int
commonPostBlocksCreateDestroy( struct ssl_session *session,
							   const char *request,
							   const char *user_id,
							   const char *screen_name,
							   const bool include_entities,
							   const bool skip_status );
static int
commonPostFavoritesCreateDestroy( struct ssl_session *session,
								  const char *request,
								  const char *id,
								  const bool include_entities );

/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_TWAPI_MAX_LINE_1			1024

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
================================================================================

	Timelines

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getStatusesHomeTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of tweets to retreive >
				 const char *since_id
				 < retreive tweets greater than this id >
				 const char *max_id
				 < retreive tweets less than this id >
				 const bool trim_user
				 < true:only the status authos numerical id >
				 const bool exclude_replies
				 < true:only numerical id >
				 const bool contributor_details
				 < true:include enhanced contributors element >
				 const bool include_entities
				 < true:include entities node >
	Output		:void
	Return		:int
				 < result >
	Description	:get statuses/home_timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getStatusesHomeTimeLine( struct ssl_session *session,
							 const int count,
							 const char *since_id,
							 const char *max_id,
							 const bool trim_user,
							 const bool exclude_replies,
							 const bool contributor_details,
							 const bool include_entities )
{
	int					result;

	result = commonStatusesTimeLine( DEF_REST_TL_HOME,
									 session,
									 count,
									 since_id,
									 max_id,
									 trim_user,
									 exclude_replies,
									 contributor_details,
									 include_entities,
									 NULL,
									 NULL,
									 false,
									 false );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getStatusesMentionsTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of tweets to retreive >
				 const char *since_id
				 < retreive tweets greater than this id >
				 const char *max_id
				 < retreive tweets less than this id >
				 const bool trim_user
				 < true:only numerical id >
				 const bool contributor_details
				 < true:include enhanced contributors element >
				 const bool include_entities
				 < true:include entities node >
	Output		:void
	Return		:int
				 < result >
	Description	:get statuses/mentions_timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getStatusesMentionsTimeLine( struct ssl_session *session,
								 const int count,
								 const char *since_id,
								 const char *max_id,
								 const bool trim_user,
								 const bool contributor_details,
								 const bool include_entities )
{
	int					result;

	result = commonStatusesTimeLine( DEF_REST_TL_MENTIONS,
									 session,
									 count,
									 since_id,
									 max_id,
									 trim_user,
									 false,
									 contributor_details,
									 include_entities,
									 NULL,
									 NULL,
									 false,
									 false );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getStatusesUserTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < id of user >
				 const char *screen_name >
				 < screen name >
				 const int count
				 < number of tweets to retreive >
				 const char *since_id
				 < retreive tweets greater than this id >
				 const char *max_id
				 < retreive tweets less than this id >
				 const bool trim_user
				 < true:only numerical id >
				 const bool exclude_replies
				 < true:exclude reply tweets >
				 const bool contributor_details
				 < true:include enhanced contributors element >
				 const bool include_rts
				 < true:include retweets >
	Output		:void
	Return		:int
				 < result >
	Description	:get statuses/user_timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getStatusesUserTimeLine( struct ssl_session *session,
							 const char *user_id,
							 const char *screen_name,
							 const int count,
							 const char *since_id,
							 const char *max_id,
							 const bool trim_user,
							 const bool exclude_replies,
							 const bool contributor_details,
							 const bool include_rts )
{
	int					result;

	result = commonStatusesTimeLine( DEF_REST_TL_USER,
									 session,
									 count,
									 since_id,
									 max_id,
									 trim_user,
									 exclude_replies,
									 contributor_details,
									 false,
									 user_id,
									 screen_name,
									 include_rts,
									 false );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getStatusesRetweetOfMe
	Input		:struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of tweets to retreive >
				 const char *since_id
				 < retreive tweets greater than this id >
				 const char *max_id
				 < retreive tweets less than this id >
				 const bool trim_user
				 < true:only numerical id >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < result >
	Description	:get statuses/user_timeline
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getStatusesRetweetOfMe( struct ssl_session *session,
							const int count,
							const char *since_id,
							const char *max_id,
							const bool trim_user,
							const bool include_entities,
							const bool include_user_entities )
{
	int					result;

	result = commonStatusesTimeLine( DEF_REST_RETWEETS_OF_ME,
									 session,
									 count,
									 since_id,
									 max_id,
									 trim_user,
									 false,
									 false,
									 include_entities,
									 NULL,
									 NULL,
									 false,
									 include_user_entities );
	
	return( result );
}

/*
================================================================================

	Tweets

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postStatusesUpdate
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *status
				 < mention to tweet >
				 const char *in_reply_to_status_id
				 < reply to this id >
				 const char *latitude
				 < latitude of the location this tweet refers to >
				 const char *longitude
				 < longitude of the location this tweet refers to >
				 const char *place_id
				 < a place in the world >
				 const bool disply_coodinates
				 < whether or not to put a pin on the exact coordinates >
				 const bool trim_user
				 < true:tweet returned in a timeline includes only authors id >
	Output		:void
	Return		:int
				 < result >
	Description	:request statuses/update
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postStatusesUpdate( struct ssl_session *session,
						const char *status,
						const char *in_reply_to_status_id,
						const char *latitude,
						const char *longitude,
						const char *place_id,
						const bool display_coodinates,
						const bool trim_user )
{
	struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_TWEETS_UPDATE_NUM_PARAM ];
	int					result;
	int					p_num;

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;
#if 1
	param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_DISP_COORD;
	if( display_coodinates )
	{
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}
	else
	{
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}
#endif

	if( in_reply_to_status_id )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_IN_REPLY;
		param[ p_num++ ].param	= ( char* )in_reply_to_status_id;
	}

	if( latitude )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_LAT;
		param[ p_num++ ].param	= ( char* )latitude;
	}

	if( longitude )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_LONG;
		param[ p_num++ ].param	= ( char* )longitude;
	}

	if( place_id )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_PLACE_ID;
		param[ p_num++ ].param	= ( char* )place_id;
	}

	if( status )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_STATUS;
		param[ p_num++ ].param	= ( char* )status;
	}

#if 1
	param[ p_num   ].name	= DEF_REST_TWEETS_UPDATE_TRIM_USER;
	if( trim_user )
	{
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}
	else
	{
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}
#endif

	/* ------------------------------------------------------------------------ */
	/* post statuses/update														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_STATUSES "/",
							   DEF_REST_TWEETS_UPDATE,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to tweet\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	recvDiscard( session, &hctx );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postStatusesDestroyId
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < tweet id >
				 const bool trim_user
				 < true:tweet returned in a timeline includes only authors id >
	Output		:void
	Return		:int
				 < result >
	Description	:request statuses/destroy/:id
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postStatusesDestroyId( struct ssl_session *session,
						   const char *id,
						   const bool trim_user )
{
	struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_TWEETS_DESTROY_NUM_PARAM ];
	int					result;
	int					p_num;
	char				command[ sizeof( DEF_REST_TWEETS_DESTROY ) - 1 + 
								 sizeof( "/"".json" ) - 1 +
								 DEF_TWAPI_MAX_USER_ID_LEN ];
	int					com_len;

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( trim_user )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_DESTROY_TRIM_USER;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	/* ------------------------------------------------------------------------ */
	/* post statuses/destroy/:id												*/
	/* ------------------------------------------------------------------------ */
	com_len = strlen( id );
	com_len += sizeof( DEF_REST_TWEETS_DESTROY ) - 1;
	com_len += sizeof( ".json" ) - 1;
	com_len += 1;	// for null terminator

	if( !command || ( sizeof( command ) < com_len ) )
	{
		return( -1 );
	}

	snprintf( command, sizeof( command ), "%s/%s.json",
										  DEF_REST_TWEETS_DESTROY, id );

	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_STATUSES "/",
							   command,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to tweet\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

#if 1
	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}
#endif

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	recvDiscard( session, &hctx );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postStatusesRetweetId
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < tweet id >
				 const bool trim_user
				 < true:tweet returned in a timeline includes only authors id >
	Output		:void
	Return		:int
				 < result >
	Description	:request statuses/retweet/:id
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postStatusesRetweetId( struct ssl_session *session,
						   const char *id,
						   const bool trim_user )
{
	//struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_TWEETS_RETWEET_NUM_PARAM ];
	int					result;
	int					p_num;
	char				command[ sizeof( DEF_REST_TWEETS_RETWEET ) - 1 + 
								 sizeof( "/"".json" ) - 1 +
								 DEF_TWAPI_MAX_USER_ID_LEN ];
	int					com_len;

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( trim_user )
	{
		param[ p_num   ].name	= DEF_REST_TWEETS_RETWEET_TRIM_USER;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	/* ------------------------------------------------------------------------ */
	/* post statuses/retweet/:id												*/
	/* ------------------------------------------------------------------------ */
	com_len = strlen( id );
	com_len += sizeof( DEF_REST_TWEETS_RETWEET ) - 1;
	com_len += sizeof( ".json" ) - 1;
	com_len += 1;	// for null terminator

	if( !command || ( sizeof( command ) < com_len ) )
	{
		return( -1 );
	}

	snprintf( command, sizeof( command ), "%s/%s.json",
										  DEF_REST_TWEETS_RETWEET, id );

	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_STATUSES "/",
							   command,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to tweet\n" );
		return( -1 );
	}
#if 0
	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

#if 1
	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}
#endif

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	recvDiscard( session, &hctx );
#endif

	return( result );
}

/*
================================================================================

	Direct Message

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiDirectMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of direct messages to retreive >
				 const char *since_id
				 < retreive direct messages greater than this id >
				 const char *max_id
				 < retreive direct messages less than this id >
				 const bool include_entities
				 < true:include entities >
				 const bool skip_status
				 < true:not includeing user object >
	Output		:void
	Return		:int
				 < result >
	Description	:get direct_message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiDirectMessages( struct ssl_session *session,
							const int count,
							const char *since_id,
							const char *max_id,
							const bool include_entities,
							const bool skip_status )
{
	struct req_param	param[ DEF_REST_DM_NUM_PARAM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_DM_COUNT;
		if( DEF_REST_DM_MAX_COUNT < count )
		{
			sprintf( c_count, "%d", DEF_REST_DM_MAX_COUNT );
		}
		else
		{
			sprintf( c_count, "%d", count );
		}
		param[ p_num++ ].param = c_count;
	}

	
	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_DM_INCLUDE_ENTITIES;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( max_id )
	{
		param[ p_num   ].name	= DEF_REST_DM_MAX_ID;
		param[ p_num++ ].param	= ( char* )max_id;
	}

	if( since_id )
	{
		param[ p_num   ].name	= DEF_REST_DM_SINCE_ID;
		param[ p_num++ ].param	= ( char* )since_id;
	}

	if( skip_status )
	{
		param[ p_num   ].name	= DEF_REST_DM_SKIP_STATUS;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	/* ------------------------------------------------------------------------ */
	/* get direct_messages														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/",
							   DEF_REST_DIRECT_MESSAGES,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiDirectMessageSent
	Input		:struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of direct messages to retreive >
				 const char *since_id
				 < retreive direct messages greater than this id >
				 const char *max_id
				 < retreive direct messages less than this id >
				 const int page
				 < page of results to retrieve >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < result >
	Description	:get direct_message/sent
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiDirectMessagesSent( struct ssl_session *session,
								const int count,
								const char *since_id,
								const char *max_id,
								const int page,
								const bool include_entities )
{
	struct req_param	param[ DEF_REST_DM_SENT_NUM_PARAM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];
	char				p_page[ DEF_REST_INT_MAX_LENGTH ];

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_DM_SENT_COUNT;
		if( DEF_REST_DM_MAX_COUNT < count )
		{
			sprintf( c_count, "%d", DEF_REST_DM_SENT_MAX_COUNT );
		}
		else
		{
			sprintf( c_count, "%d", count );
		}
		param[ p_num++ ].param = c_count;
	}

	
	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_DM_SENT_INCLUDE_ENTITIES;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( max_id )
	{
		param[ p_num   ].name	= DEF_REST_DM_SENT_MAX_ID;
		param[ p_num++ ].param	= ( char* )max_id;
	}

	if( 0 < page )
	{
		param[ p_num   ].name	= DEF_REST_DM_SENT_PAGE;
		sprintf( p_page, "%d", page );
		param[ p_num++ ].param	= p_page;
	}

	if( since_id )
	{
		param[ p_num   ].name	= DEF_REST_DM_SENT_SINCE_ID;
		param[ p_num++ ].param	= ( char* )since_id;
	}

	/* ------------------------------------------------------------------------ */
	/* get direct_messages														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_DMESSAGE "/",
							   DEF_REST_DM_SENT ,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getDirectMessageShow
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < id of the direct message >
	Output		:void
	Return		:int
				 < result >
	Description	:get direct_message/show
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getDirectMessagesShow( struct ssl_session *session, const char *id )
{
	struct req_param	param[ DEF_REST_DM_SHOW_NUM_PARAM ];
	int					result;
	int					p_num;

	if( !id )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	param[ p_num   ].name	= DEF_REST_DM_SHOW_ID;
	param[ p_num++ ].param	= ( char* )id;

	/* ------------------------------------------------------------------------ */
	/* get direct_messages														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_DMESSAGE "/",
							   DEF_REST_DM_SHOW ,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postDirectMessageNew
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < user id to which send a direct message >
				 const char *screen_name
				 < screen name to which send a direct message >
				 const char *text
				 < message to send >
	Output		:void
	Return		:int
				 < result >
	Description	:post direct_message/new
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postDirectMessagesNew( struct ssl_session *session,
						   const char *user_id,
						   const char *screen_name,
						   const char *text )
{
	struct req_param	param[ DEF_REST_DM_NEW_NUM_PARAM ];
	int					result;
	int					p_num;

	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( screen_name )
	{
		if( !user_id )
		{
			param[ p_num   ].name	= DEF_REST_DM_NEW_SCREEN_NAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}

	param[ p_num   ].name	= DEF_REST_DM_NEW_TEXT;
	param[ p_num++ ].param	= ( char* )text;

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_DM_NEW_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}

	/* ------------------------------------------------------------------------ */
	/* get direct_messages														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_DMESSAGE "/",
							   DEF_REST_DM_NEW ,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postDirectMessageDestroy
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < id to destroy >
				 const bool include_entities
				 < false:entities node is not set >
	Output		:void
	Return		:int
				 < result >
	Description	:post direct_message/destroy
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postDirectMessagesDestroy( struct ssl_session *session,
							   const char *id,
							   const bool include_entities )
{
	struct req_param	param[ DEF_REST_DM_DESTROY_NUM_PARAM ];
	int					result;
	int					p_num;

	if( !id )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	param[ p_num   ].name	= DEF_REST_DM_DESTROY_ID;
	param[ p_num++ ].param	= ( char* )id;

	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_DM_DESTROY_INCLUDE_ENT;
		param[ p_num++ ].param	= ( char* )DEF_REST_BOOL_FALSE;
	}

	/* ------------------------------------------------------------------------ */
	/* get direct_messages														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_DMESSAGE "/",
							   DEF_REST_DM_DESTROY ,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to destroy direct message\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	//recvDiscard( session, &hctx );
	commonRecvTimeLine( session );

	return( 0 );
}

/*
================================================================================

	Friends & Followers

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getFriendsIds
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < page cursor >
				 const bool stringify_ids
				 < true:id is stringified >
				 const int count
				 < count of ids to get >
	Output		:void
	Return		:int
				 < result >
	Description	:get friends ids
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getFriendsIds( struct ssl_session *session,
				   const char *user_id,
				   const char *screen_name,
				   const char *cursor,
				   const bool stringify_ids,
				   const int count )
{
	int		result;

	result = commonGetFriendsFollowers( session,
										DEF_TWTR_API_GRP_FRIENDS,
										user_id,
										screen_name,
										cursor,
										stringify_ids,
										count );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getFollowersIds
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < page cursor >
				 const bool stringify_ids
				 < true:id is stringified >
				 const int count
				 < count of ids to get >
	Output		:void
	Return		:int
				 < result >
	Description	:get followers ids
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getFollowersIds( struct ssl_session *session,
					 const char *user_id,
					 const char *screen_name,
					 const char *cursor,
					 const bool stringify_ids,
					 const int count )
{
	int		result;

	result = commonGetFriendsFollowers( session,
										DEF_TWTR_API_GRP_FOLLOWERS,
										user_id,
										screen_name,
										cursor,
										stringify_ids,
										count );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postFriendshipCreate
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const bool follow
				 < true:notify to user >
	Output		:void
	Return		:int
				 < status >
	Description	:post friendship/create
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postFriendshipCreate( struct ssl_session *session,
						  const char *user_id,
						  const char *screen_name,
						  const bool follow )
{
	int		result;

	result = commonPostFriendshipCreateDestroy( session,
												DEF_REST_FSHIP_CREATE,
												user_id,
												screen_name,
												follow );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postFriendshipDestroy
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
	Output		:void
	Return		:int
				 < status >
	Description	:post friendship/destroy
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postFriendshipDestroy( struct ssl_session *session,
						   const char *user_id,
						   const char *screen_name )
{
	int		result;

	result = commonPostFriendshipCreateDestroy( session,
												DEF_REST_FSHIP_DESTROY,
												user_id,
												screen_name,
												false );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiFriendsList
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < page cursor >
				 const int count
				 < count to get >
				 const bool skip_status
				 < true:skip status >
				 const bool include_user_entities
	Output		:void
	Return		:int
				 < status >
	Description	:get friends/list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiFriendsList( struct ssl_session *session,
						 const char *user_id,
						 const char *screen_name,
						 const char *cursor,
						 const int count,
						 const bool skip_status,
						 const bool include_user_entities )
{
	int		result;

	result = commonGetFriendsFollowersList( session,
											DEF_TWTR_API_GRP_FRIENDS,
											user_id,
											screen_name,
											cursor,
											count,
											skip_status,
											include_user_entities );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiFollowersList
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < page cursor >
				 const int count
				 < count to get >
				 const bool skip_status
				 < true:skip status >
				 const bool include_user_entities
	Output		:void
	Return		:int
				 < status >
	Description	:get follwers/list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiFollowersList( struct ssl_session *session,
						   const char *user_id,
						   const char *screen_name,
						   const char *cursor,
						   const int count,
						   const bool skip_status,
						   const bool include_user_entities )
{
	int		result;

	result = commonGetFriendsFollowersList( session,
											DEF_TWTR_API_GRP_FOLLOWERS,
											user_id,
											screen_name,
											cursor,
											count,
											skip_status,
											include_user_entities );

	return( result );
}
/*
================================================================================

	Users

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiBlocksList
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *cursor
				 < page cursor >
				 const bool skip_status
				 < true:skip status >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < status >
	Description	:get follwers/list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiBlocksList( struct ssl_session *session,
						const char *cursor,
						const bool skip_status,
						const bool include_entities )
{
	struct req_param	param[ DEF_REST_BLOCKS_LIST_NUM_PARAM ];
	int					result;
	int					p_num;
	char				zero_cur[ 3 ];


	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	param[ p_num ].name	= DEF_REST_BLOCKS_LIST_CURSOR;

	if( !cursor )
	{
		zero_cur[ 0 ] = '-';
		zero_cur[ 1 ] = '1';
		zero_cur[ 2 ] = '\0';

		param[ p_num++ ].param = zero_cur;
	}
	else
	{
		param[ p_num++ ].param = ( char* )cursor;
	}

	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_BLOCKS_LIST_INC_ENTITIES;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( skip_status )
	{
		param[ p_num   ].name	= DEF_REST_BLOCKS_LIST_SKIP_STATUS;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}
	
	/* ------------------------------------------------------------------------ */
	/* get blocks/list															*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_BLOCKS "/",
							   DEF_REST_BLOCKS_LIST,
							   param,
							   p_num,
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postTwapiBlocksCreate
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *user_id
				 < user id of twitter >
				 const char *screen_name
				 < screen name of twitter >
				 const bool skip_status
				 < true:skip status >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < status >
	Description	:create a block for a specified user
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postTwapiBlocksCreate( struct ssl_session *session,
						   const char *user_id,
						   const char *screen_name,
						   const bool skip_status,
						   const bool include_entities )
{
	int		result;

	result = commonPostBlocksCreateDestroy( session,
											DEF_REST_BLOCKS_CREATE,
											user_id,
											screen_name,
											include_entities,
											skip_status );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postTwapiBlocksDestroy
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *user_id
				 < user id of twitter >
				 const char *screen_name
				 < screen name of twitter >
				 const bool skip_status
				 < true:skip status >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < status >
	Description	:destroy a block for a specified user
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postTwapiBlocksDestroy( struct ssl_session *session,
							const char *user_id,
							const char *screen_name,
							const bool skip_status,
							const bool include_entities )
{
	int		result;

	result = commonPostBlocksCreateDestroy( session,
											DEF_REST_BLOCKS_DESTROY,
											user_id,
											screen_name,
											include_entities,
											skip_status );
	
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiUsersShow
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < id of user >
				 const char *screen_name
				 < screen name of user >
				 bool include_entities
				 < true:include eintities >
	Output		:void
	Return		:int
				 < status >
	Description	:pot users/show
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiUsersShow( struct ssl_session *session,
					   const char *user_id,
					   const char *screen_name,
					   bool include_entities )
{
	struct req_param	param[ DEF_REST_USERS_SHOW_NUM_PARAM ];
	int					result;
	int					p_num;

	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_USERS_SHOW_INC_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_USERS_SHOW_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}
	else
	{
		param[ p_num   ].name	= DEF_REST_USERS_SHOW_SNAME;
		param[ p_num++ ].param	= ( char* )screen_name;
	}

	/* ------------------------------------------------------------------------ */
	/* post users/show															*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_USERS "/",
							   DEF_REST_USERS_SHOW,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to post lists/create\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}
/*
================================================================================

	Favorites

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiFavoritesList
	Input		:struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of favorites list to retreive >
				 const char *since_id
				 < retreive favorites list greater than this id >
				 const char *max_id
				 < retreive favorites list less than this id >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < result >
	Description	:get favorites/list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiFavoritesList( struct ssl_session *session,
						   const int count,
						   const char *since_id,
						   const char *max_id,
						   const bool include_entities )
{
	struct req_param	param[ DEF_REST_FAV_LIST_NUM_PARAM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_FAV_LIST_CNT;
		if( DEF_REST_DM_MAX_COUNT < count )
		{
			sprintf( c_count, "%d", DEF_REST_FAV_LIST_CNT_MAX_COUNT );
		}
		else
		{
			sprintf( c_count, "%d", count );
		}
		param[ p_num++ ].param = c_count;
	}

	
	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_FAV_LIST_INC_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( max_id )
	{
		param[ p_num   ].name	= DEF_REST_FAV_LIST_MAX_ID;
		param[ p_num++ ].param	= ( char* )max_id;
	}

	if( since_id )
	{
		param[ p_num   ].name	= DEF_REST_FAV_LIST_SINCE_ID;
		param[ p_num++ ].param	= ( char* )since_id;
	}

	/* ------------------------------------------------------------------------ */
	/* get favorites/list														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION
							   "/" DEF_TWTR_API_GRP_FAVORITES "/",
							   DEF_REST_FAVORITES_LIST,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( 0 );
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postFavoritesDestroy
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *id
				 < tweet id to be destroyed >
				 const bool include_entities
				 < include entities or not >
	Output		:void
	Return		:int
				 < status >
	Description	:post favorites/destroy
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postFavoritesDestroy( struct ssl_session *session,
						  const char *id,
						  const bool include_entities )
{
	int		result;

	result = commonPostFavoritesCreateDestroy( session,
											   DEF_REST_FAVORITES_DESTROY,
											   id,
											   include_entities );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postFavoritesCreate
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *id
				 < tweet id to be destroyed >
				 const bool include_entities
				 < include entities or not >
	Output		:void
	Return		:int
				 < status >
	Description	:post favorites/create
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postFavoritesCreate( struct ssl_session *session,
						 const char *id,
						 const bool include_entities )
{
	int		result;

	result = commonPostFavoritesCreateDestroy( session,
											   DEF_REST_FAVORITES_CREATE,
											   id,
											   include_entities );

	return( result );
}

/*
================================================================================

	Lists

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsStatuses
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of lists >
				 const char *owner_screen_name
				 < screen name of owner of lists >
				 const char *owner_id
				 < owner id of lists >
				 const char *since_id
				 < retreive favorites list greater than this id >
				 const char *max_id
				 < retreive favorites list less than this id >
				 const int count
				 < number of list statuses to retreive >
				 const bool include_entities
				 < true:include entities >
				 const bool include_rts
				 < true:include retweets >
	Output		:void
	Return		:int
				 < result >
	Description	:get lists/statuses
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsStatuses( struct ssl_session *session,
						   const char *list_id,
						   const char *slug,
						   const char *owner_screen_name,
						   const char *owner_id,
						   const char *since_id,
						   const char *max_id,
						   const int count,
						   const bool include_entities,
						   const bool include_rts )
{
	struct req_param	param[ DEF_REST_LISTS_STS_PARAM_NUM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];

	if( !list_id && !slug )
	{
		return( -1 );
	}

	if( slug && ( !owner_screen_name && !owner_id ) )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_COUNT;
		if( DEF_REST_LISTS_STS_MAX_COUNT < count )
		{
			snprintf( c_count, sizeof( c_count ),
					  "%d", DEF_REST_LISTS_STS_MAX_COUNT );
		}
		else
		{
			snprintf( c_count, sizeof( c_count ), "%d", count );
		}

		param[ p_num++ ].param = c_count;
	}
	
	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_INC_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( include_rts == false )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_INC_RTW;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( max_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_MAX_ID;
		param[ p_num++ ].param	= ( char* )max_id;
	}

	if( !list_id && !owner_screen_name && owner_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_OWN_ID;
		param[ p_num++ ].param	= ( char* )owner_id;
	}

	if( !list_id && owner_screen_name && !owner_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_OWN_SNAME;
		param[ p_num++ ].param	= ( char* )owner_screen_name;
	}
	
	if( since_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_SINCE_ID;
		param[ p_num++ ].param	= ( char* )since_id;
	}

	if( !list_id && slug )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_STS_SLUG;
		param[ p_num++ ].param	= ( char* )slug;
	}

	/* ------------------------------------------------------------------------ */
	/* get lists/statuses														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS "/",
							   DEF_REST_LISTS_STATUSES,
							   param,
							   p_num,
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get lists timeline\n" );
		return( -1 );
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsOwnerships
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < id of user >
				 const char *screen_name
				 < screen name of user >
				 const int count
				 < number of lists to retreive >
				 const char *cursor
				 < cursor of pages >
	Output		:void
	Return		:int
				 < result >
	Description	:get lists/ownerships
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsOwnerships( struct ssl_session *session,
							 const char *user_id,
							 const char *screen_name,
							 const int count,
							 const char *cursor )
{
	int		result;

	result = commonGetLists( DEF_REST_LISTS_OWNERSHIPS,
							 session,
							 user_id,
							 screen_name,
							 count,
							 cursor,
							 false );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsSubscriptions
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < id of user >
				 const char *screen_name
				 < screen name of user >
				 const int count
				 < number of lists to retreive >
				 const char *cursor
				 < cursor of pages >
	Output		:void
	Return		:int
				 < result >
	Description	:get lists/subscriptions
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsSubscriptions( struct ssl_session *session,
								const char *user_id,
								const char *screen_name,
								const int count,
								const char *cursor )
{
	int		result;

	result = commonGetLists( DEF_REST_LISTS_SUBSCRIPTIONS,
							 session,
							 user_id,
							 screen_name,
							 count,
							 cursor,
							 false );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsMemberships
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < id of user >
				 const char *screen_name
				 < screen name of user >
				 const int count
				 < number of lists to retreive >
				 const char *cursor
				 < cursor of pages >
				 cont bool filter_to_owned_lists
				 < true : get only owner's lists >
	Output		:void
	Return		:int
				 < result >
	Description	:get lists/memberships
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsMemberships( struct ssl_session *session,
							  const char *user_id,
							  const char *screen_name,
							  const int count,
							  const char *cursor,
							  const bool filter_to_owned_lists )
{
	int		result;

	result = commonGetLists( DEF_REST_LISTS_MEMBERSHIPS,
							 session,
							 user_id,
							 screen_name,
							 count,
							 cursor,
							 filter_to_owned_lists );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsMembers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
				 const char *cursor
				 < cursor of list >
				 bool include_entities
				 < true: include entities >
				 bool skip_status
				 < true : skip status >
	Output		:void
	Return		:int
				 < result >
	Description	:get lists/members
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsMembers( struct ssl_session *session,
						  const char *list_id,
						  const char *slug,
						  const char *owner_screen_name,
						  const char *owner_id,
						  const char *cursor,
						  bool include_entities,
						  bool skip_status )
{
	int		result;

	result = commonGetMembers( DEF_REST_LISTS_MEMBERS,
							   session,
							   list_id,
							   slug,
							   owner_screen_name,
							   owner_id,
							   cursor,
							   include_entities,
							   skip_status );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsSubscribers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
				 const char *cursor
				 < cursor of list >
				 bool include_entities
				 < true: include entities >
				 bool skip_status
				 < true : skip status >
	Output		:void
	Return		:int
				 < result >
	Description	:get lists/subscribers
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsSubscribers( struct ssl_session *session,
							  const char *list_id,
							  const char *slug,
							  const char *owner_screen_name,
							  const char *owner_id,
							  const char *cursor,
							  bool include_entities,
							  bool skip_status )
{
	int		result;

	result = commonGetMembers( DEF_REST_LISTS_SUBSCRIBERS,
							   session,
							   list_id,
							   slug,
							   owner_screen_name,
							   owner_id,
							   cursor,
							   include_entities,
							   skip_status );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postTwapiListsCreateMembers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *user_id
				 < id of user to create >
				 const char *screen_name
				 < screen name of user to create >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
	Output		:void
	Return		:int
				 < result >
	Description	:post lists/members/create
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postTwapiListsCreateMembers( struct ssl_session *session,
								 const char *list_id,
								 const char *slug,
								 const char *user_id,
								 const char *screen_name,
								 const char *owner_screen_name,
								 const char *owner_id )
{
	int		result;

	result = commonMembersCreateDestroy( DEF_REST_LISTS_MEMS_CREATE,
										 session,
										 list_id,
										 slug,
										 user_id,
										 screen_name,
										 owner_screen_name,
										 owner_id );

	if( result < 0 )
	{
		logMessage( "fail to create list members\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	commonRecvTimeLine( session );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postTwapiListsDestroyMembers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *user_id
				 < id of user to destroy >
				 const char *screen_name
				 < screen name of user to destroy >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
	Output		:void
	Return		:int
				 < result >
	Description	:post lists/members/destroy
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postTwapiListsDestroyMembers( struct ssl_session *session,
								  const char *list_id,
								  const char *slug,
								  const char *user_id,
								  const char *screen_name,
								  const char *owner_screen_name,
								  const char *owner_id )
{
	int		result;

	result = commonMembersCreateDestroy( DEF_REST_LISTS_MEMS_DESTROY,
										 session,
										 list_id,
										 slug,
										 user_id,
										 screen_name,
										 owner_screen_name,
										 owner_id );

	if( result < 0 )
	{
		logMessage( "fail to destroy list members\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	commonRecvTimeLine( session );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postTwapiListsCreateSubscribers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
	Output		:void
	Return		:int
				 < result >
	Description	:post lists/subscribers/create
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postTwapiListsCreateSubscribers( struct ssl_session *session,
									 const char *list_id,
									 const char *slug,
									 const char *owner_screen_name,
									 const char *owner_id )
{
	int		result;

	result = commonSubscribersCreateDestroy( DEF_REST_LISTS_SUBS_CREATE,
											 session,
											 list_id,
											 slug,
											 owner_screen_name,
											 owner_id );

	if( result < 0 )
	{
		logMessage( "fail to subscribe list\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	commonRecvTimeLine( session );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postTwapiListsDestroySubscribers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
	Output		:void
	Return		:int
				 < result >
	Description	:post lists/subscribers/destroy
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postTwapiListsDestroySubscribers( struct ssl_session *session,
									  const char *list_id,
									  const char *slug,
									  const char *owner_screen_name,
									  const char *owner_id )
{
	int		result;

	result = commonSubscribersCreateDestroy( DEF_REST_LISTS_SUBS_DESTROY,
											 session,
											 list_id,
											 slug,
											 owner_screen_name,
											 owner_id );

	if( result < 0 )
	{
		logMessage( "fail to subscribe list\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	commonRecvTimeLine( session );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postListsCreate
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *name
				 < name of list >
				 bool mode_private
				 < mode of list : true : private >
				 const char *description
				 < description for the list >
	Output		:void
	Return		:int
				 < status >
	Description	:pot lists/create
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postListsCreate( struct ssl_session *session,
					 const char *name,
					 bool mode_private,
					 const char *description )
{
	struct req_param	param[ DEF_REST_LISTS_CREATE_PARAM_NUM ];
	int					result;
	int					p_num;

	if( !name )
	{
		return( -1 );
	}

	if( description )
	{
		if( ( DEF_TWAPI_ACTUAL_MAX_LDESC + 1 ) <=
			strnlen( description, DEF_TWAPI_ACTUAL_MAX_LDESC + 1 ) )
		{
			return( -1 );
		}

		if( DEF_TWAPI_MAX_LDESC <
			utf8StrnLen( ( const uint8_t* )description,
						  DEF_TWAPI_ACTUAL_MAX_LDESC ) )
		{
			return( -1 );
		}
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( description )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_CREATE_DESC;
		param[ p_num++ ].param	= ( char* )description;
	}

	if( mode_private )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_CREATE_MODE;
		param[ p_num++ ].param	= DEF_REST_LISTS_CREATE_MODE_PRIVATE;
	}
	else
	{
		param[ p_num   ].name	= DEF_REST_LISTS_CREATE_MODE;
		param[ p_num++ ].param	= DEF_REST_LISTS_CREATE_MODE_PUBLIC;
	}

	param[ p_num   ].name		= DEF_REST_LISTS_CREATE_NAME;
	param[ p_num++ ].param		= ( char* )name;

	/* ------------------------------------------------------------------------ */
	/* post lists/create														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS "/",
							   DEF_REST_LISTS_CREATE,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to post lists/create\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postListsDestroy
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *owner_screen_name
				 < owner screen name whose list is to be destroyed >
				 const char *owner_id
				 < owner id of which list is to be destroyed >
				 const char *list_id
				 < list id to destroy >
				 const char *slug
				 < slug of list to destroy >
	Output		:void
	Return		:int
				 < status >
	Description	:pot lists/destroy
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postListsDestroy( struct ssl_session *session,
					  const char *owner_screen_name,
					  const char *owner_id,
					  const char *list_id,
					  const char *slug )
{
	struct req_param	param[ DEF_REST_LISTS_DESTROY_PARAM_NUM ];
	int					result;
	int					p_num;

	if( !list_id && !slug )
	{
		return( -1 );
	}

	if( slug )
	{
		if( !owner_screen_name && !owner_id )
		{
			return( -1 );
		}
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( list_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_DESTROY_LIST_ID;
		param[ p_num++ ].param	= ( char* )list_id;
	}
	else
	{
		if( owner_id )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_DESTROY_OWN_ID;
			param[ p_num++ ].param	= ( char* )owner_id;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_DESTROY_OWN_SNAME;
			param[ p_num++ ].param	= ( char* )owner_screen_name;
		}

		param[ p_num   ].name	= DEF_REST_LISTS_DESTROY_SLUG;
		param[ p_num++ ].param	= ( char* )slug;
	}

	/* ------------------------------------------------------------------------ */
	/* post lists/create														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS "/",
							   DEF_REST_LISTS_DESTROY,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to post lists/create\n" );
		return( -1 );
	}

	return( commonRecvTimeLine( session ) );
	//return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwapiListsShow
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *owner_screen_name
				 < owner screen name whose list is to be shown >
				 const char *owner_id
				 < owner id of which list is to be shown >
				 const char *list_id
				 < list id to show >
				 const char *slug
				 < slug of list to show >
	Output		:void
	Return		:int
				 < status >
	Description	:get lists/show
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwapiListsShow( struct ssl_session *session,
					   const char *owner_screen_name,
					   const char *owner_id,
					   const char *list_id,
					   const char *slug )
{
	struct req_param	param[ DEF_REST_LISTS_SHOW_PARAM_NUM ];
	int					result;
	int					p_num;

	if( !list_id && !slug )
	{
		return( -1 );
	}

	if( slug )
	{
		if( !owner_screen_name && !owner_id )
		{
			return( -1 );
		}
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( list_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_SHOW_LIST_ID;
		param[ p_num++ ].param	= ( char* )list_id;
	}
	else
	{
		if( owner_id )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SHOW_OWN_ID;
			param[ p_num++ ].param	= ( char* )owner_id;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SHOW_OWN_SNAME;
			param[ p_num++ ].param	= ( char* )owner_screen_name;
		}

		param[ p_num   ].name	= DEF_REST_LISTS_SHOW_SLUG;
		param[ p_num++ ].param	= ( char* )slug;
	}

	/* ------------------------------------------------------------------------ */
	/* post lists/create														*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS "/",
							   DEF_REST_LISTS_SHOW,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to post lists/create\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}


/*
================================================================================

	OAuth

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postOauthRequestToken
	Input		:struct ssl_session *session
				 < ssl session >
	Output		:void
	Return		:int
				 < result >
	Description	:request oauth/requst_token
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postOauthRequestToken( struct ssl_session *session )
{
	struct http_ctx	hctx;
	int				result;

	/* ------------------------------------------------------------------------ */
	/* destroy access token and access secret									*/
	/* ------------------------------------------------------------------------ */
	destroyAccessKey( );

	/* ------------------------------------------------------------------------ */
	/* get oauth token and oauth secret											*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_API_GRP_OAUTH "/",
							   DEF_REST_OAUTH_REQ_TOKEN,
							   DEF_API_NO_REQ_PARAM,
							   DEF_API_NO_REQ_PARAM_NUM,
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		/* failure to get keys													*/
		logMessage( "fail to get keys\n" );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* get http headers															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );
	
	/* ------------------------------------------------------------------------ */
	/* receive response body and analyze it										*/
	/* ------------------------------------------------------------------------ */
	result = recvTokenAndSecret( session, &hctx, false );

	if( result < 0 )
	{
		return( -1 );
	}

	logMessage( "oauth token:%s\n", getOauthAccessToken( ) );
	logMessage( "oauth secret:%s\n", getOauthAccessSecret( ) );
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:postOauthAccessToken
	Input		:struct ssl_session *session
				 < ssl session >
	Output		:void
	Return		:int
				 < result >
	Description	:request oauth/access_token
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int postOauthAccessToken( struct ssl_session *session )
{
	struct http_ctx		hctx;
	struct req_param	param[ 1 ];
	int					result;

	if( ( param[ 0 ].param = ( char* )getOauthVerifier( ) ) == NULL )
	{
		return( -1 );
	}

	param[ 0 ].name = DEF_TWTR_HTTPH_AUTH_VERIFIER;

	/* ------------------------------------------------------------------------ */
	/* get access token and access secret										*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_API_GRP_OAUTH "/",
							   DEF_REST_OAUTH_ACCESS_TOKEN,
							   param,
							   1,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get access token\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive response body and analyze it										*/
	/* ------------------------------------------------------------------------ */
	result = recvTokenAndSecret( session, &hctx, true );

	if( result < 0 )
	{
		return( -1 );
	}

	logMessage( "oauth token:%s\n", getOauthAccessToken( ) );
	logMessage( "oauth secret:%s\n", getOauthAccessSecret( ) );
	logMessage( "user_id:%s\n", getTwapiUserId( ) );
	logMessage( "screen_name:%s\n", getTwapiScreenName( ) );



	return( result );
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
	Function	:recvTokenAndSecret
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 bool screen_name
				 < true:get screnn name false:nothing to do >
	Output		:void
	Return		:int
				 < status >
	Description	:receive access token and access secret
================================================================================
*/
static int
recvTokenAndSecret( struct ssl_session *session,
					struct http_ctx *hctx,
					bool screen_name )
{
	unsigned char	buffer[ DEF_TWAPI_MAX_LINE_1 ];
	int				length;
	int				index;
	int				key_len;

	if( DEF_TWAPI_MAX_LINE_1 < hctx->content_length )
	{
		return( -1 );
	}

	length = recvSSLMessage( session, buffer, hctx->content_length );


	if( length < 0 )
	{
		return( length );
	}

	if( length != hctx->content_length )
	{
		return( -1 );
	}

	/* for strstr																*/
	buffer[ length ] = '\0';
	length++;
	logMessage( "%s", buffer );

	if( strnCaseCmp( DEF_HTTPH_AUTH_TOKEN, ( const char* )buffer,
					 sizeof( DEF_HTTPH_AUTH_TOKEN ) - 1 ) == 0 )
	{
		index = sizeof( DEF_HTTPH_AUTH_TOKEN ) - 1;
		if( buffer[ index++ ] != ( unsigned char )'=' )
		{
			return( -1 );
		}

		/* -------------------------------------------------------------------- */
		/* destroy temporary token and secret									*/
		/* -------------------------------------------------------------------- */
		destroyAccessKey( );

		/* -------------------------------------------------------------------- */
		/* search first '=' and save access token								*/
		/* -------------------------------------------------------------------- */
		key_len = 0;
		while( ( index + key_len ) < length )
		{
			if( buffer[ index + key_len ] == '&' )
			{
				if( setOauthAccessToken( ( const char* )&buffer[ index ],
										 key_len ) < 0 )
				{
					return( -1 );
				}
				index += key_len + 1;
				break;
			}
			key_len++;
		}

		if( length < index )
		{
			return( -1 );
		}

		if( strnCaseCmp( DEF_HTTPH_AUTH_TOKEN_SECRET,
						 ( const char* )&buffer[ index ],
						 sizeof( DEF_HTTPH_AUTH_TOKEN_SECRET ) - 1 ) != 0 )
		{
			return( -1 );
		}

		index += sizeof( DEF_HTTPH_AUTH_TOKEN_SECRET ) - 1;

		if( buffer[ index++ ] != ( unsigned char )'=' )
		{
			return( -1 );
		}

		/* -------------------------------------------------------------------- */
		/* search access secret													*/
		/* -------------------------------------------------------------------- */
		key_len = 0;
		while( ( index + key_len ) < length  )
		{
			if( buffer[ index + key_len ] == '&' )
			{
				if( setOauthAccessSecret( ( const char* )&buffer[ index ],
										  key_len ) < 0 )
				{
					return( -1 );
				}

				/* ------------------------------------------------------------ */
				/* found token and secret!										*/
				/* ------------------------------------------------------------ */
				if( screen_name )
				{
					/* go to next procedure										*/
					index += key_len + 1;
					break;
				}
				else
				{
					return( 0 );
				}
			}
			key_len++;
		}

		if( length < index )
		{
			return( -1 );
		}

		if( strnCaseCmp( DEF_TWAPI_USERS_USER_ID,
						 ( const char* )&buffer[ index ],
						 sizeof( DEF_TWAPI_USERS_USER_ID ) - 1 ) != 0 )
		{
			return( -1 );
		}

		index += sizeof( DEF_TWAPI_USERS_USER_ID ) - 1;

		if( buffer[ index++ ] != ( unsigned char )'=' )
		{
			return( -1 );
		}

		/* -------------------------------------------------------------------- */
		/* search user id														*/
		/* -------------------------------------------------------------------- */
		key_len = 0;
		while( ( index + key_len ) < length )
		{
			if( buffer[ index + key_len ] == '&' )
			{
				setTwapiUserId( ( const char* )&buffer[ index ], key_len );

				index += key_len + 1;
				break;
			}
			key_len++;
		}

		if( length < index )
		{
			return( -1 );
		}

		if( strnCaseCmp( DEF_TWAPI_USERS_SCREEN_NAME,
						 ( const char* )&buffer[ index ],
						 sizeof( DEF_TWAPI_USERS_SCREEN_NAME ) - 1 ) != 0 )
		{
			return( -1 );
		}

		index += sizeof( DEF_TWAPI_USERS_SCREEN_NAME ) - 1;

		if( buffer[ index++ ] != ( unsigned char )'=' )
		{
			return( -1 );
		}

		/* -------------------------------------------------------------------- */
		/* search screen name													*/
		/* -------------------------------------------------------------------- */
		key_len = 0;
		logMessage( "\nindex:%d\n", index );
		while( ( index + key_len ) < length )
		{
			logMessage( "%c", buffer[ index + key_len ] );
			if( buffer[ index + key_len ] == '&'  ||
				buffer[ index + key_len ] == '\r' ||
				buffer[ index + key_len ] == '\n' ||
				buffer[ index + key_len ] == '\0' )
			{
				setTwapiScreenName( ( const char* )&buffer[ index ], key_len );

				/* found all!													*/
				return( 0 );
			}
			key_len++;
		}
	}

	return( -1 );
}
/*
================================================================================
	Function	:recvDiscard
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
	Output		:void
	Return		:int
				 < status >
	Description	:receive data and discard them
================================================================================
*/
static int recvDiscard( struct ssl_session *session, struct http_ctx *hctx )
{
	int				recv_len;
	int				i;
	unsigned char	buffer[ 1 ];

	for( i = 0 ; i < hctx->content_length ; i++ )
	{
		recv_len = recvSSLMessage( session, buffer, sizeof( buffer ) );
		logMessage( "%c", buffer[ 0 ] );
	}

	logMessage( "received complete\n" );

	return( 0 );
}
/*
================================================================================
	Function	:commonStatusesTimeLine
	Input		:const char *request
				 < request : home_timeline.json, mentions_timelin.json >
				 struct ssl_session *session
				 < ssl session >
				 const int count
				 < number of tweets to retreive >
				 const char *since_id
				 < retreive tweets greater than this id >
				 const char *max_id
				 < retreive tweets less than this id >
				 const bool trim_user
				 < true:only the status authos numerical id >
				 const bool exclude_replies
				 < true:exclude repliy tweets
				   NOTE* mentions_timeline does not have this field.
				         Must set false if mentions_timeline >
				 const bool contributor_details
				 < true:include enhanced contributors element >
				 const bool include_entities
				 < true:include entities node >
				 const char *user_id
				 < id of user NOTE* user_timeline only >
				 const char *screen_name
				 < screen name NOTE* user_timeline only >
				 const bool include_rts
				 < true:include retweets NOTE *user_timeline only >
				 const bool include_user_entities
				 < true:include user entities NOTE *retweet_of_me only >
	Output		:void
	Return		:int
				 < result >
	Description	:get statuses/{home,mensions,user}_timeline, retweet_of_me
================================================================================
*/
static int commonStatusesTimeLine( const char *request,
								   struct ssl_session *session,
								   const int count,
								   const char *since_id,
								   const char *max_id,
								   const bool trim_user,
								   const bool exclude_replies,
								   const bool contributor_details,
								   const bool include_entities,
								   const char *user_id,
								   const char *screen_name,
								   const bool include_rts,
								   const bool include_user_entities )
{
	struct req_param	param[ DEF_REST_TL_USER_NUM_PARAM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];
	bool				isUserTimeline;

	isUserTimeline = ( strncmp( request, DEF_REST_TL_USER,
								sizeof( DEF_REST_TL_USER ) - 1 ) == 0 );
	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( exclude_replies )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_XREPLIES;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	if( contributor_details )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_CONTRI_DETAILS;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_COUNT;
		if( DEF_REST_TL_HOME_MAX_COUNT < count )
		{
			snprintf( c_count, sizeof( c_count ),
					  "%d", DEF_REST_TL_HOME_MAX_COUNT );
		}
		else
		{
			snprintf( c_count, sizeof( c_count ), "%d", count );
		}

		param[ p_num++ ].param = c_count;
	}

	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_INCLUDE_ENTITIES;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( isUserTimeline )
	{
		if( include_rts == false )
		{
			param[ p_num   ].name	= DEF_REST_TL_USER_INCLUDE_RTS;
			param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
		}
	}

	if( include_user_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_RT_OF_ME_INC_USR_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( max_id )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_MAX_ID;
		param[ p_num++ ].param	= ( char* )max_id;
	}

	if( isUserTimeline )
	{
		if( !user_id )
		{
			param[ p_num   ].name	= DEF_REST_TL_USER_SCREEN_NAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}

	if( since_id )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_SINCE_ID;
		param[ p_num++ ].param	= ( char* )since_id;
	}

	if( trim_user )
	{
		param[ p_num   ].name	= DEF_REST_TL_HOME_TRIM_USER;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	if( isUserTimeline )
	{
		if( !screen_name )
		{
			param[ p_num   ].name	= DEF_REST_TL_USER_USER_ID;
			param[ p_num++ ].param	= ( char* )user_id;
		}
	}

	/* ------------------------------------------------------------------------ */
	/* get statuses/home_timeline												*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_STATUSES "/",
							   request,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}

/*
================================================================================
	Function	:commonRecvTimeLine
	Input		:struct ssl_session *session
				 < ssl session >
	Output		:void
	Return		:int
				 < status >
	Description	:common recieve procudures of timeline
================================================================================
*/
static int commonRecvTimeLine( struct ssl_session *session )
{
	struct http_ctx		hctx;
	int					result;

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	return( recvDiscard( session, &hctx ) );
}

/*
================================================================================
	Function	:commonGetLists
	Input		:const char *request
				 < request : lists/memberships, subscriptions, ownerships >
				 struct ssl_session *session
				 < ssl session >
				 const char *user_id
				 < id of user >
				 const char *screen_name
				 < screen name of user >
				 const int count
				 < number of lists to retreive >
				 const char *cursor
				 < cursor of pages >
				 const bool filter_to_owned_lists
				 < true:get only ownerships list >
	Output		:void
	Return		:int
				 < status >
	Description	:common recieve procudures of lists
================================================================================
*/
static int commonGetLists( const char *request,
						   struct ssl_session *session,
						   const char *user_id,
						   const char *screen_name,
						   const int count,
						   const char *cursor,
						   const bool filter_to_owned_lists )
{
	struct req_param	param[ DEF_REST_LISTS_MEM_PARAM_NUM ];
	int					result;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];
	bool				isMemberships;
	int					p_num;

	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	isMemberships = ( strncmp( request, DEF_REST_LISTS_MEMBERSHIPS,
							   sizeof( DEF_REST_LISTS_MEMBERSHIPS ) - 1 ) == 0 );
	
	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( !isMemberships )
	{
		if( 0 < count )
		{
			param[ p_num  ].name	= DEF_REST_LISTS_SUB_COUNT;
			if( DEF_REST_LISTS_SUB_MAX_COUNT < count )
			{
				snprintf( c_count, sizeof( c_count ),
						  "%d", DEF_REST_TL_HOME_MAX_COUNT );
			}
			else
			{
				snprintf( c_count, sizeof( c_count ), "%d", count );
			}

			param[ p_num++ ].param	= c_count;
		}
	}

	if( cursor )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_MEM_CUR;
		param[ p_num++ ].param	= ( char* )cursor;
	}

	if( isMemberships )
	{
		if( filter_to_owned_lists )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEM_FILTER;
			param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
		}
	}

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_MEM_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}
	else
	{
		param[ p_num   ].name	= DEF_REST_LISTS_MEM_SNAME;
		param[ p_num++ ].param	= ( char* )screen_name;
	}

	/* ------------------------------------------------------------------------ */
	/* get lists/memberships													*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS "/",
							   request,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to get list of lists\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}

/*
================================================================================
	Function	:commonGetMembers
	Input		:const char *request
				 < request : lists/members, subscribers >
				 struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of lists >
				 const char *slug
				 < slug of list >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of owner >
				 const char *cursor
				 < cursor of list >
				 bool include_entities
				 < true: include entities >
				 bool skip_status
				 < true : skip status >
	Output		:void
	Return		:int
				 < status >
	Description	:common recieve procudures of members/subscribers of lists
================================================================================
*/
static int commonGetMembers( const char *request,
							 struct ssl_session *session,
							 const char *list_id,
							 const char *slug,
							 const char *owner_screen_name,
							 const char *owner_id,
							 const char *cursor,
							 bool include_entities,
							 bool skip_status )

{
	struct req_param	param[ DEF_REST_LISTS_SCRIB_PARAM_NUM ];
	int					result;
	int					p_num;

	if( !slug && !list_id )
	{
		return( -1 );
	}

	if( slug && ( !owner_screen_name && !owner_id ) )
	{
		return( -1 );
	}
	
	if( !request )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( cursor )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_CUR;
		param[ p_num++ ].param	= ( char* )cursor;
	}

	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_INC_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	if( list_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_LIST_ID;
		param[ p_num++ ].param	= ( char* )list_id;

		if( skip_status )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_SKIP_STS;
			param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
		}
	}
	else
	{
		if( owner_id )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_OWN_ID;
			param[ p_num++ ].param	= ( char* )owner_id;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_OWN_SNAME;
			param[ p_num++ ].param	= ( char* )owner_screen_name;
		}

		if( skip_status )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_SKIP_STS;
			param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
		}

		param[ p_num   ].name	= DEF_REST_LISTS_SCRIB_SLUG;
		param[ p_num++ ].param	= ( char* )slug;
	}


	/* ------------------------------------------------------------------------ */
	/* get lists/memberships													*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_GET,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS "/",
							   request,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to get list of lists\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}

/*
================================================================================
	Function	:commonMembersCreateDestroy
	Input		:const char *request
				 < request : lists/memberships, subscriptions, ownerships >
				 struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of list >
				 const char *slug
				 < slug of list >
				 const char *user_id
				 < id of user >
				 const char *screen_name
				 < screen name of user to destroy/create >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of onwer >
	Output		:void
	Return		:int
				 < status >
	Description	:common procudures of lists/members/{create, destroy}
================================================================================
*/
static int commonMembersCreateDestroy( const char *request,
									   struct ssl_session *session,
									   const char *list_id,
									   const char *slug,
									   const char *user_id,
									   const char *screen_name,
									   const char *owner_screen_name,
									   const char *owner_id )
{
	struct req_param	param[ DEF_REST_LISTS_MEMS_DSTRY_PARAM_NUM ];
	int					result;
	int					p_num;

	if( !list_id && !slug )
	{
		return( -1 );
	}

	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	if( slug && ( !owner_screen_name && !owner_id ) )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( list_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_LIST_ID;
		param[ p_num++ ].param	= ( char* )list_id;

		if( user_id )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_USER_ID;
			param[ p_num++ ].param	= ( char* )user_id;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_SNAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}
	else
	{
		if( owner_id )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_OWN_ID;
			param[ p_num++ ].param	= ( char* )owner_id;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_OWN_SNAME;
			param[ p_num++ ].param	= ( char* )owner_screen_name;
		}

		if( screen_name )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_SNAME;
			param[ p_num++ ].param	= ( char* )screen_name;

			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_SLUG;
			param[ p_num++ ].param	= ( char* )slug;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_SLUG;
			param[ p_num++ ].param	= ( char* )slug;

			param[ p_num   ].name	= DEF_REST_LISTS_MEMS_DSTRY_USER_ID;
			param[ p_num++ ].param	= ( char* )user_id;
		}
	}

	/* ------------------------------------------------------------------------ */
	/* post lists/memberships													*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS_MEMS "/",
							   request,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to post list of lists\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}

/*
================================================================================
	Function	:commonSubscribersCreateDestroy
	Input		:const char *request
				 < request : lists/memberships, subscriptions, ownerships >
				 struct ssl_session *session
				 < ssl session >
				 const char *list_id
				 < id of list >
				 const char *slug
				 < slug of list >
				 const char *owner_screen_name
				 < screen name of owner >
				 const char *owner_id
				 < id of onwer >
	Output		:void
	Return		:int
				 < status >
	Description	:common procudures of lists/subscribers/{create, destroy}
================================================================================
*/
static int commonSubscribersCreateDestroy( const char *request,
										   struct ssl_session *session,
										   const char *list_id,
										   const char *slug,
										   const char *owner_screen_name,
										   const char *owner_id )
{
	struct req_param	param[ DEF_REST_LISTS_SUBS_CRT_PARAM_NUM ];
	int					result;
	int					p_num;

	if( !list_id && !slug )
	{
		return( -1 );
	}

	if( slug && ( !owner_screen_name && !owner_id ) )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( list_id )
	{
		param[ p_num   ].name	= DEF_REST_LISTS_SUBS_CRT_LIST_ID;
		param[ p_num++ ].param	= ( char* )list_id;
	}
	else
	{
		if( owner_id )
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SUBS_CRT_OWN_ID;
			param[ p_num++ ].param	= ( char* )owner_id;
		}
		else
		{
			param[ p_num   ].name	= DEF_REST_LISTS_SUBS_CRT_OWN_SNAME;
			param[ p_num++ ].param	= ( char* )owner_screen_name;
		}

		param[ p_num   ].name	= DEF_REST_LISTS_SUBS_CRT_SLUG;
		param[ p_num++ ].param	= ( char* )slug;
	}

	/* ------------------------------------------------------------------------ */
	/* post lists/subscribers/{create, destroy}									*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_LISTS_SUBS "/",
							   request,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	if( result < 0 )
	{
		logMessage( "fail to post list of lists\n" );
		return( -1 );
	}

	//return( commonRecvTimeLine( session ) );
	return( result );
}

/*
================================================================================
	Function	:commonPostFriendshipCreateDestroy
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *request
				 < request of api >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const bool follow
				 < true:notify to user >
	Output		:void
	Return		:int
				 < status >
	Description	:common procedure of post friendship/{create, destroy}
================================================================================
*/
static int
commonPostFriendshipCreateDestroy( struct ssl_session *session,
								   const char *request,
								   const char *user_id,
								   const char *screen_name,
								   const bool follow )
{
	struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_FSHIP_CREATE_NUM_PARAM ];
	int					result;
	int					p_num;
	
	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( strcmp( DEF_REST_FSHIP_CREATE, request ) == 0 )
	{
		if( follow )
		{
			param[ p_num   ].name	= DEF_REST_FSHIP_CREATE_FOLLOW;
			param[ p_num++ ].param	= ( char* )DEF_REST_BOOL_TRUE;
		}
	}

	if( !user_id )
	{
		if( screen_name )
		{
			param[ p_num   ].name	= DEF_REST_FSHIP_CREATE_SCREEN_NAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_FSHIP_CREATE_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}

	/* ------------------------------------------------------------------------ */
	/* create/destroy friendship												*/
	/* ------------------------------------------------------------------------ */
	if( strcmp( DEF_REST_FSHIP_CREATE, request ) == 0 )
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_POST,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_FSHIP "/",
								   DEF_REST_FSHIP_CREATE,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	else
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_POST,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_FSHIP "/",
								   DEF_REST_FSHIP_DESTROY,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	return( recvDiscard( session, &hctx ) );
}

/*
================================================================================
	Function	:commonGetFriendsFollowers
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *request
				 < request group of api >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < page cursor >
				 const bool stringify_ids
				 < true:id is stringified >
				 const int count
				 < count of ids to get >
	Output		:void
	Return		:int
				 < status >
	Description	:common procudure of get {followers, friends}/ids
================================================================================
*/
static int
commonGetFriendsFollowers( struct ssl_session *session,
						   const char *request_grp,
						   const char *user_id,
						   const char *screen_name,
						   const char *cursor,
						   const bool stringify_ids,
						   const int count )
{
	struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_FRIENDS_IDS_NUM_PARAM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH ];

	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_IDS_COUNT;
		if( DEF_REST_FRIENDS_IDS_MAX_COUNT < count )
		{
			sprintf( c_count, "%d", DEF_REST_FRIENDS_IDS_MAX_COUNT );
		}
		else
		{
			sprintf( c_count, "%d", count );
		}
		param[ p_num++ ].param = c_count;
	}

	if( cursor )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_IDS_CURSOR;
		param[ p_num++ ].param	= ( char* )cursor;
	}

	if( !user_id )
	{
		if( screen_name )
		{
			param[ p_num   ].name	= DEF_REST_FRIENDS_IDS_SCREEN_NAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}
	
	if( stringify_ids )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_IDS_STR_IDS;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_IDS_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}

	/* ------------------------------------------------------------------------ */
	/* get friends/followers ids												*/
	/* ------------------------------------------------------------------------ */
	if( strcmp( DEF_TWTR_API_GRP_FRIENDS, request_grp ) == 0 )
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_GET,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_FRIENDS "/",
								   DEF_REST_FRIENDS_IDS,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	else
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_GET,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_FOLLOWERS "/",
								   DEF_REST_FOLLOWERS_IDS,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	return( recvDiscard( session, &hctx ) );
}
/*
================================================================================
	Function	:commonGetFriendsFollowersList
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *request_grp
				 < request group of api >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const char *cursor
				 < page cursor >
				 const int count
				 < count to get >
				 const bool skip_status
				 < true:skip status >
				 const bool include_user_entities
				 < false:not include user entities >
	Output		:void
	Return		:int
				 < status >
	Description	:common procudure of get {flollwers, friends}/list
================================================================================
*/
static int
commonGetFriendsFollowersList( struct ssl_session *session,
							   const char *request_grp,
							   const char *user_id,
							   const char *screen_name,
							   const char *cursor,
							   const int count,
							   const bool skip_status,
							   const bool include_user_entities )
{
	struct req_param	param[ DEF_REST_FRIENDS_LIST_NUM_PARAM ];
	int					result;
	int					p_num;
	char				c_count[ DEF_REST_INT_MAX_LENGTH + 1 ];

	if( !user_id && !screen_name )
	{
		return( -1 );
	}
	
	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( 0 < count )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_LIST_COUNT;
		if( DEF_REST_FRIENDS_LIST_MAX_COUNT < count )
		{
			snprintf( c_count, DEF_REST_INT_MAX_LENGTH + 1,
					  "%d", DEF_REST_FRIENDS_LIST_MAX_COUNT );
		}
		else
		{
			snprintf( c_count, DEF_REST_INT_MAX_LENGTH + 1, "%d", count );
		}
		param[ p_num++ ].param = c_count;
	}

	if( cursor )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_LIST_CURSOR;
		param[ p_num++ ].param	= ( char* )cursor;
	}

	if( include_user_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_LIST_INC_USR_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}


	if( !user_id )
	{
		if( screen_name )
		{
			param[ p_num   ].name	= DEF_REST_FRIENDS_LIST_SCREEN_NAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}

	if( skip_status )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_LIST_SKIP_STATUS;
		param[ p_num++ ].param	= DEF_REST_BOOL_TRUE;
	}
	

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_FRIENDS_LIST_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}

	/* ------------------------------------------------------------------------ */
	/* get friends/followers ids												*/
	/* ------------------------------------------------------------------------ */
	if( strcmp( DEF_TWTR_API_GRP_FRIENDS, request_grp ) == 0 )
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_GET,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_FRIENDS "/",
								   DEF_REST_FRIENDS_LIST,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	else
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_GET,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_FOLLOWERS "/",
								   DEF_REST_FOLLOWERS_LIST,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}

	return( result );
	
#if 0
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	return( recvDiscard( session, &hctx ) );
#endif
}

/*
================================================================================
	Function	:commonPostBlocksCreateDestroy
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *request
				 < request of api >
				 const char *user_id
				 < user id >
				 const char *screen_name
				 < screen name >
				 const bool include_entities
				 < true:include entities >
				 const bool skip_status
				 < true:skip status >
	Output		:void
	Return		:int
				 < status >
	Description	:common procedure of post blocks/{create, destroy}
================================================================================
*/
static int
commonPostBlocksCreateDestroy( struct ssl_session *session,
							   const char *request,
							   const char *user_id,
							   const char *screen_name,
							   const bool include_entities,
							   const bool skip_status )
{
	struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_BLOCKS_CREATE_NUM_PARAM ];
	int					result;
	int					p_num;
	
	if( !user_id && !screen_name )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( !user_id )
	{
		if( screen_name )
		{
			param[ p_num   ].name	= DEF_REST_BLOCKS_CREATE_SNAME;
			param[ p_num++ ].param	= ( char* )screen_name;
		}
	}

	if( user_id )
	{
		param[ p_num   ].name	= DEF_REST_BLOCKS_CREATE_USER_ID;
		param[ p_num++ ].param	= ( char* )user_id;
	}

	/* ------------------------------------------------------------------------ */
	/* create/destroy blocks													*/
	/* ------------------------------------------------------------------------ */
	if( strcmp( DEF_REST_BLOCKS_CREATE, request ) == 0 )
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_POST,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_BLOCKS "/",
								   DEF_REST_BLOCKS_CREATE,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	else
	{
		result = sendOauthMessage( session,
								   DEF_HTTPH_POST,
								   "/" DEF_TWTR_API_VERSION "/"
								   DEF_TWTR_API_GRP_BLOCKS "/",
								   DEF_REST_BLOCKS_DESTROY,
								   param,
								   p_num,
								   //DEF_OAUTH_CONNECTION_CLOSE );
								   DEF_OAUTH_CONNECTION_ALIVE );
	}
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	return( recvDiscard( session, &hctx ) );
}

/*
================================================================================
	Function	:commonPostFavoritesCreateDestroy
	Input		:struct ssl_session *session
				 < ssl_session >
				 const char *request
				 < request of api >
				 const char *id
				 < tweet id >
				 const bool include_entities
				 < true:include entities >
	Output		:void
	Return		:int
				 < status >
	Description	:common procedure of post favorites/{create, destroy}
================================================================================
*/
static int
commonPostFavoritesCreateDestroy( struct ssl_session *session,
								  const char *request,
								  const char *id,
								  const bool include_entities )
{
	struct http_ctx		hctx;
	struct req_param	param[ DEF_REST_FAV_DEST_PARAM_NUM ];
	int					result;
	int					p_num;
	
	if( !id )
	{
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make parameters															*/
	/* ------------------------------------------------------------------------ */
	p_num = 0;

	if( id )
	{
		param[ p_num   ].name	= DEF_REST_FAV_DEST_ID;
		param[ p_num++ ].param	= ( char* )id;
	}

	if( include_entities == false )
	{
		param[ p_num   ].name	= DEF_REST_FAV_DEST_INC_ENT;
		param[ p_num++ ].param	= DEF_REST_BOOL_FALSE;
	}

	/* ------------------------------------------------------------------------ */
	/* create/destroy friendship												*/
	/* ------------------------------------------------------------------------ */
	result = sendOauthMessage( session,
							   DEF_HTTPH_POST,
							   "/" DEF_TWTR_API_VERSION "/"
							   DEF_TWTR_API_GRP_FAVORITES "/",
							   request,
							   param,
							   p_num,
							   //DEF_OAUTH_CONNECTION_CLOSE );
							   DEF_OAUTH_CONNECTION_ALIVE );
	
	if( result < 0 )
	{
		logMessage( "fail to get home timeline\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* get http heades															*/
	/* ------------------------------------------------------------------------ */
	initHttpContext( &hctx );

	result = recvHttpHeader( session, &hctx );

	if( result < 0 || hctx.status_code != DEF_HTTPH_STATUS_OK )
	{
		/* failure to get headers												*/
		logMessage( "get ilegal http headers\n" );
		if( hctx.content_length )
		{
			recvDiscard( session, &hctx );
		}
		return( -1 );
	}

	logMessage( "------------------------------------\n" );
	logMessage( "status code : %d\n", hctx.status_code );
	logMessage( "content length : %d\n", hctx.content_length );
	logMessage( "------------------------------------\n" );

	/* ------------------------------------------------------------------------ */
	/* receive body																*/
	/* ------------------------------------------------------------------------ */
	return( recvDiscard( session, &hctx ) );
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

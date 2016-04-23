/*******************************************************************************
 File:twitter_operation.h
 Description:Definitions of Operations for Twitter

*******************************************************************************/
#ifndef	__TWITTER_OPERATION_H__
#define	__TWITTER_OPERATION_H__

#include "net/ssl.h"
#include "net/http.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
struct ssl_session;


/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_TWOPE_MAX_TWEET_COUNT		20
#define	DEF_TWOPE_MAX_DM_COUNT			20
#define	DEF_TWOPE_MAX_FF_COUNT			20		// follower/friends list
#define	DEF_TWOPE_MAX_FAV_COUNT			20
#define	DEF_TWOPE_MAX_LISTS_TWEET_COUNT	DEF_TWOPE_MAX_TWEET_COUNT
#define	DEF_TWOPE_MAX_LISTS_COUNT		20

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
	Function	:initTwitterOperation
	Input		:void
	Output		:void
	Return		:void
	Description	:initialize twitter operation
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initTwitterOperation( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setTwapiUserId
	Input		:const char *set_user_id
				 < user id >
				 int size
				 < size of user id>
	Output		:void
	Return		:void
	Description	:set user id
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void setTwapiUserId( const char *set_user_id, int size );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setTwapiScreenName
	Input		: const char *set_screen_name
				 < screen name >
				 int size
				 < size of screen name >
	Output		:void
	Return		:void
	Description	:set screen name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void setTwapiScreenName( const char *set_screen_name, int size );

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
const char* getTwapiUserId( void );

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
const char* getTwapiScreenName( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:witeTwapiConfigurations
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
							  const char *access_secret );

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
							 char *access_secret, int access_secret_len );

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
int clipTwopeScreenNameFromMsg( const char *msg, char *screen_name );

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
					 const char *last );

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
						 const char *last );

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
					 const char *last );

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
							const char *last );

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
int tweetTwapi( struct ssl_session *session, const char *mention,  int size );

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
int removeTweet( struct ssl_session *session, const char *id );

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
			 const char *id );

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
int requestFollow( struct ssl_session *session, const char *follow );

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
int requestUnfollow( struct ssl_session *session, const char *unfollow );

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
					  const char *cursor );

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
					 const char *cursor );

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
				  const char *cursor );

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
int requestBlock( struct ssl_session *session, const char *blockee );

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
int requestUnblock( struct ssl_session *session, const char *unblockee );

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
					const char *screen_name );

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
					   int size );

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
					   const char *last );

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
						   const char *last );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:removeDirectMessage
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < direct message id to remove >
	Output		:void
	Return		:int
				 < status >
	Description	:remove a direct message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int removeDirectMessage( struct ssl_session *session, const char *id );

/*
================================================================================

	Favorites

================================================================================
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:removeFavorite
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *id
				 < tweet id to which remove favorite >
	Output		:void
	Return		:int
				 < status >
	Description	:remove favorite
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int removeFavorite( struct ssl_session *session, const char *id );

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
					  const char *last );

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
int requestFavorite( struct ssl_session *sessioin, const char *id );

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
int requestUnFavorite( struct ssl_session *sessioin, const char *id );

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
					  const char *last );

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
						const char *cursor );

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
						   const char *cursor );

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
						 const char *cursor );
						
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
					 const char *cursor );

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
						 const char *cursor );

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
						const char *screen_name );

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
						 const char *screen_name );

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
					const char *owner_screen_name );

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
						const char *owner_screen_name );

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
				 const char *description );

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
				 const char *slug );

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
			   const char *slug );

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
int requestTwapiOauth( struct ssl_session *session );


#endif	// __TWITTER_OPERATION_H__

/*******************************************************************************
 File:twitter_api.h
 Description:Definitions of API for Twitter

*******************************************************************************/
#ifndef	__TWITTER_API_H__
#define	__TWITTER_API_H__

#include <stdbool.h>

#include "net/ssl.h"


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

#define	DEF_TWTR_HTTPH_AUTH_CALLBACK_URL	"oob"
#define	DEF_TWTR_HTTPH_AUTH_CALLBACK_URL_B	"http%3A%2F%2F127.0.0.1"
#define	DEF_TWTR_HTTPH_HOST_NAME			"api.twitter.com"
#define	DEF_TWTR_HTTPH_AUTH_VERIFIER		"oauth_verifier"
#define	DEF_TWTR_API_VERSION				"1.1"


#define	DEF_TWTR_PIN_CODE_MAX_SIZE			16

/*
--------------------------------------------------------------------------------
	Group of Commands
--------------------------------------------------------------------------------
*/
#define	DEF_TWTR_API_GRP_STATUSES			"statuses"
#define	DEF_TWTR_API_GRP_STATUSES_RT		"statuses/retweets"
#define	DEF_TWTR_API_GRP_STATUSES_SHOW		"statuses/show"
#define	DEF_TWTR_API_GRP_STATUSES_DESTROY	"statuses/destroy"
#define	DEF_TWTR_API_GRP_STATUSES_RTERS		"statuses/retweeters"
#define	DEF_TWTR_API_GRP_SEARCH				"search"
#define	DEF_TWTR_API_GRP_DMESSAGE			"direct_messages"
#define	DEF_TWTR_API_GRP_FSHIP				"friendships"
#define	DEF_TWTR_API_GRP_FSHIP_NO_RT		"friendships/no_retweets"
#define	DEF_TWTR_API_GRP_FOLLOWERS			"followers"
#define	DEF_TWTR_API_GRP_FRIENDS			"friends"
#define	DEF_TWTR_API_GRP_ACCOUNT			"account"
#define	DEF_TWTR_API_GRP_BLOCKS				"blocks"
#define	DEF_TWTR_API_GRP_USERS				"users"
#define	DEF_TWTR_API_GRP_FAVORITES			"favorites"
#define	DEF_TWTR_API_GRP_LISTS				"lists"
#define	DEF_TWTR_API_GRP_LISTS_MEMS			"lists/members"
#define	DEF_TWTR_API_GRP_LISTS_SUBS			"lists/subscribers"
#define	DEF_TWTR_API_GRP_SSEARCH			"saved_searches"
#define	DEF_TWTR_API_GRP_SSEARCH_SHOW		"saved_searches/show"
#define	DEF_TWTR_API_GRP_SSEARCH_CREATE		"saved_searches/create"
#define	DEF_TWTR_API_GRP_SSEARCH_DESTROY	"saved_searches/destroy"
#define	DEF_TWTR_API_GRP_GEO				"geo"
#define	DEF_TWTR_API_GRP_GEO_ID				"geo/id"
#define	DEF_TWTR_API_GRP_TRENDS				"trends"
#define	DEF_TWTR_API_GRP_OAUTH				"oauth"
#define	DEF_TWTR_API_GRP_OAUTH2				"oauth2"
#define	DEF_TWTR_API_GRP_HELP				"help"
#define	DEF_TWTR_API_GRP_APP				"application"

/*
================================================================================

	REST API v1.1 DEFINES

================================================================================
*/
#define	DEF_REST_BOOL_TRUE					"true"
#define	DEF_REST_BOOL_FALSE					"false"

#define	DEF_REST_INT_MAX_LENGTH				( 10 + 1 )
#define	DEF_REST_TWEETS_MAX_LENGTH			( 140 )		// actual byte length is
														// up to utf-8 coging
#define	DEF_REST_ACTUAL_TWEETS_MAX_LEN		( DEF_REST_TWEETS_MAX_LENGTH		\
											  * DEF_UTF8_MAX_SIZE )

#define	DEF_REST_SLUG_MAX_LENGTH			( 25 )
#define	DEF_REST_ACTUAL_SLUG_MAX_LENGTH		( DEF_REST_SLUG_MAX_LENGTH			\
											  * DEF_UTF8_MAX_SIZE )

/*
--------------------------------------------------------------------------------
	Timelines
--------------------------------------------------------------------------------
*/
#define	DEF_REST_TL_DEF_COUNT				20
#define	DEF_REST_TL_MAX_COUNT				200
// GET statuses/mentions_timeline
#define	DEF_REST_TL_MENTIONS				"mentions_timeline.json"
// parameters are same as home_timeline
// GET statuses/user_timeline
#define	DEF_REST_TL_USER					"user_timeline.json"
#define	DEF_REST_TL_USER_NUM_PARAM			9
#define	DEF_REST_TL_USER_USER_ID			"user_id"
#define	DEF_REST_TL_USER_SCREEN_NAME		"screen_name"
#define	DEF_REST_TL_USER_COUNT				"count"
#define	DEF_REST_TL_USER_SINCE_ID			"since_id"
#define	DEF_REST_TL_USER_MAX_ID				"max_id"
#define	DEF_REST_TL_USER_TRIM_USER			"trim_user"
#define	DEF_REST_TL_USER_XREPLIES			"exclude_replies"
#define	DEF_REST_TL_USER_CONTRI_DETAILS		"contributor_details"
#define	DEF_REST_TL_USER_INCLUDE_RTS		"include_rts"
// GET statuses/home_timeline
#define	DEF_REST_TL_HOME					"home_timeline.json"
#define	DEF_REST_TL_HOME_NUM_PARAM			7
#define	DEF_REST_TL_HOME_COUNT				DEF_REST_TL_USER_COUNT
#define	DEF_REST_TL_HOME_DEF_COUNT			DEF_REST_TL_DEF_COUNT
#define	DEF_REST_TL_HOME_MAX_COUNT			DEF_REST_TL_MAX_COUNT
#define	DEF_REST_TL_HOME_SINCE_ID			DEF_REST_TL_USER_SINCE_ID
#define	DEF_REST_TL_HOME_MAX_ID				DEF_REST_TL_USER_MAX_ID
#define	DEF_REST_TL_HOME_TRIM_USER			DEF_REST_TL_USER_TRIM_USER
#define	DEF_REST_TL_HOME_XREPLIES			DEF_REST_TL_USER_XREPLIES
#define	DEF_REST_TL_HOME_CONTRI_DETAILS		DEF_REST_TL_USER_CONTRI_DETAILS
#define	DEF_REST_TL_HOME_INCLUDE_ENTITIES	"include_entities"
// GET statuses/retweets_of_me
#define	DEF_REST_RETWEETS_OF_ME				"retweets_of_me.json"
#define	DEF_REST_RT_OF_ME_NUM_PARAM			6
#define	DEF_REST_RT_OF_ME_COUNT				DEF_REST_TL_USER_COUNT
#define	DEF_REST_RT_OF_ME_DEF_COUNT			DEF_REST_TL_DEF_COUNT
#define	DEF_REST_RT_OF_ME_MAX_COUNT			DEF_REST_TL_MAX_COUNT
#define	DEF_REST_RT_OF_ME_SINCE_ID			DEF_REST_TL_USER_SINCE_ID
#define	DEF_REST_RT_OF_ME_MAX_ID			DEF_REST_TL_USER_MAX_ID
#define	DEF_REST_RT_OF_ME_TRIM_USER			DEF_REST_TL_USER_TRIM_USER
#define	DEF_REST_RT_OF_ME_INCLUDE_ENTITIES	DEF_REST_TL_HOME_INCLUDE_ENTITIES
#define	DEF_REST_RT_OF_ME_INC_USR_ENT		"include_user_entities"

/*
--------------------------------------------------------------------------------
	Tweets
--------------------------------------------------------------------------------
*/
// GET statuses/retweets/:id
#define	DEF_REST_TWEETS_RETWEETS			"retweets"
#define	DEF_REST_TWEETS_RETWEETS_NUM_PARAM	3
#define	DEF_REST_TWEETS_RETWEETS_ID			"id"
#define	DEF_REST_TWEETS_RETWEETS_COUNT		"count"
#define	DEF_REST_TWEETS_RETWEETS_TRIM_USER	"trim_user"
// GET statuses/show/:id
#define	DEF_REST_TWEETS_SHOW				"show"
#define	DEF_REST_TWEETS_SHOW_NUM_PARAM		4
#define	DEF_REST_TWEETS_SHOW_ID				DEF_REST_TWEETS_RETWEETS_ID
#define	DEF_REST_TWEETS_SHOW_TRIM_USER		DEF_REST_TWEETS_RETWEETS_TRIM_USER
#define	DEF_REST_TWEETS_SHOW_INC_MY_RETW	"include_my_retweet"
#define	DEF_REST_TWEETS_SHOW_INC_ENT		"include_entities"
// POST statuses/destroy/:id
#define	DEF_REST_TWEETS_DESTROY				"destroy"
#define	DEF_REST_TWEETS_DESTROY_NUM_PARAM	1
#define	DEF_REST_TWEETS_DESTROY_TRIM_USER	DEF_REST_TWEETS_RETWEETS_TRIM_USER
// POST statuses/update
#define	DEF_REST_TWEETS_UPDATE				"update.json"
#define	DEF_REST_TWEETS_UPDATE_NUM_PARAM	7
#define	DEF_REST_TWEETS_UPDATE_STATUS		"status"
#define	DEF_REST_TWEETS_UPDATE_IN_REPLY		"in_reply_to_status_id"
#define	DEF_REST_TWEETS_UPDATE_LAT			"lat"
#define	DEF_REST_TWEETS_UPDATE_LONG			"long"
#define	DEF_REST_TWEETS_UPDATE_PLACE_ID		"place_id"
#define	DEF_REST_TWEETS_UPDATE_DISP_COORD	"display_coordinates"
#define	DEF_REST_TWEETS_UPDATE_TRIM_USER	DEF_REST_TWEETS_RETWEETS_TRIM_USER


// POST statuses/retweet/:id
#define	DEF_REST_TWEETS_RETWEET				"retweet"
#define	DEF_REST_TWEETS_RETWEET_NUM_PARAM	2
#define	DEF_REST_TWEETS_RETWEET_ID			DEF_REST_TWEETS_RETWEETS_ID
#define	DEF_REST_TWEETS_RETWEET_TRIM_USER	DEF_REST_TWEETS_RETWEETS_TRIM_USER
// POST statuses/update_with_media
#define	DEF_REST_TWEETS_UPDATE_WITH_MEDIA	"update_with_media.json"
// GET statuses/oembed
#define	DEF_REST_TWEETS_OEMBED				"oembed.json"
// GET statuses/retweeters/ids
#define	DEF_REST_TWEETS_RTERS_IDS			"ids.json"


/*
--------------------------------------------------------------------------------
	Search
--------------------------------------------------------------------------------
*/
// GET search/tweets
#define	DEF_REST_SEARCH_TWEETS				"tweets.json"


/*
--------------------------------------------------------------------------------
	Streaming
--------------------------------------------------------------------------------
*/
// POST statuses/filter
#define	DEF_REST_STREAMING_FILTER			"filter.json"
// GET statuses/sample
#define	DEF_REST_STREAMING_SAMPLE			"sample.json"
// GET statuses/firehose
#define	DEF_REST_STREAMING_FIREHOSE			"firehose.json"
// GET user
#define	DEF_REST_STREAMING_USER				"user.json"
// GET site
#define	DEF_REST_STREAMING_SITE				"site.json"


/*
--------------------------------------------------------------------------------
	Direct Messages
--------------------------------------------------------------------------------
*/
// GET direct_messages
#define	DEF_REST_DIRECT_MESSAGES			"direct_messages.json"
#define	DEF_REST_DM_NUM_PARAM				5
#define	DEF_REST_DM_SINCE_ID				"since_id"
#define	DEF_REST_DM_MAX_ID					"max_id"
#define	DEF_REST_DM_COUNT					"count"
#define	DEF_REST_DM_DEF_COUNT				DEF_REST_TL_DEF_COUNT
#define	DEF_REST_DM_MAX_COUNT				DEF_REST_TL_MAX_COUNT
#define	DEF_REST_DM_INCLUDE_ENTITIES		"include_entities"
#define	DEF_REST_DM_SKIP_STATUS				"skip_status"
// GET direct_messages/sent
#define	DEF_REST_DM_SENT					"sent.json"
#define	DEF_REST_DM_SENT_NUM_PARAM			5
#define	DEF_REST_DM_SENT_SINCE_ID			DEF_REST_DM_SINCE_ID
#define	DEF_REST_DM_SENT_MAX_ID				DEF_REST_DM_MAX_ID
#define	DEF_REST_DM_SENT_COUNT				DEF_REST_DM_COUNT
#define	DEF_REST_DM_SENT_DEF_COUNT			DEF_REST_TL_DEF_COUNT
#define	DEF_REST_DM_SENT_MAX_COUNT			DEF_REST_TL_MAX_COUNT
#define	DEF_REST_DM_SENT_PAGE				"page"
#define	DEF_REST_DM_SENT_INCLUDE_ENTITIES	DEF_REST_DM_INCLUDE_ENTITIES
// GET direct_messages/show
#define	DEF_REST_DM_SHOW					"show.json"
#define	DEF_REST_DM_SHOW_NUM_PARAM			1
#define	DEF_REST_DM_SHOW_ID					"id"
// POST direct_messages/destroy
#define	DEF_REST_DM_DESTROY					"destroy.json"
#define	DEF_REST_DM_DESTROY_NUM_PARAM		2
#define	DEF_REST_DM_DESTROY_ID				DEF_REST_DM_SHOW_ID
#define	DEF_REST_DM_DESTROY_INCLUDE_ENT		DEF_REST_DM_INCLUDE_ENTITIES
// POST direct_messages/new
#define	DEF_REST_DM_NEW						"new.json"
#define	DEF_REST_DM_NEW_NUM_PARAM			3
#define	DEF_REST_DM_NEW_USER_ID				"user_id"
#define	DEF_REST_DM_NEW_SCREEN_NAME			"screen_name"
#define	DEF_REST_DM_NEW_TEXT				"text"

/*
--------------------------------------------------------------------------------
	Friends & Followers
--------------------------------------------------------------------------------
*/
// GET friendships/no_retweets/ids
#define	DEF_REST_FSHIP_NO_RT_IDS			"ids.json"
// GET friends/ids
#define	DEF_REST_FRIENDS_IDS				"ids.json"
#define	DEF_REST_FRIENDS_IDS_NUM_PARAM		5
#define	DEF_REST_FRIENDS_IDS_USER_ID		"user_id"
#define	DEF_REST_FRIENDS_IDS_SCREEN_NAME	"screen_name"
#define	DEF_REST_FRIENDS_IDS_CURSOR			"cursor"
#define	DEF_REST_FRIENDS_IDS_STR_IDS		"stringify_ids"
#define	DEF_REST_FRIENDS_IDS_COUNT			"count"
#define	DEF_REST_FRIENDS_IDS_MAX_COUNT		5000
// GET followers/ids
#define	DEF_REST_FOLLOWERS_IDS				"ids.json"
#define	DEF_REST_FOLLOWERS_IDS_NUM_PARAM	DEF_REST_FRIENDS_IDS_NUM_PARAM
#define	DEF_REST_FOLLOWERS_IDS_USER_ID		DEF_REST_FRIENDS_IDS_USER_ID
#define	DEF_REST_FOLLOWERS_IDS_SCREEN_NAME	DEF_REST_FRIENDS_IDS_SCREEN_NAME
#define	DEF_REST_FOLLOWERS_IDS_CURSOR		DEF_REST_FRIENDS_IDS_CURSOR
#define	DEF_REST_FOLLOWERS_IDS_STR_IDS		DEF_REST_FRIENDS_IDS_STR_IDS
#define	DEF_REST_FOLLOWERS_IDS_COUNT		DEF_REST_FRIENDS_IDS_COUNT
#define	DEF_REST_FOLLOWERS_IDS_MAX_COUNT	DEF_REST_FRIENDS_IDS_MAX_COUNT
// GET friendships/lookup
#define	DEF_REST_FSHIP_LOOKUP				"lookup.json"
#define	DEF_REST_FSHIP_LOOKUP_NUM_PARAM		2
#define	DEF_REST_FSHIP_LOOKUP_SCREEN_NAME	"screen_name"
#define	DEF_REST_FSHIP_LOOKUP_USER_ID		"user_id"
// GET friendships/incoming
#define	DEF_REST_FSHIP_INCOMING				"incoming.json"
// GET friendships/outgoing
#define	DEF_REST_FSHIP_OUTGOING				"outgoing.json"
// POST friendships/create
#define	DEF_REST_FSHIP_CREATE				"create.json"
#define	DEF_REST_FSHIP_CREATE_NUM_PARAM		3
#define	DEF_REST_FSHIP_CREATE_SCREEN_NAME	DEF_REST_FSHIP_LOOKUP_SCREEN_NAME
#define	DEF_REST_FSHIP_CREATE_USER_ID		DEF_REST_FSHIP_LOOKUP_USER_ID
#define	DEF_REST_FSHIP_CREATE_FOLLOW		"follow"
// POST friendships/destroy
#define	DEF_REST_FSHIP_DESTROY				"destroy.json"
#define	DEF_REST_FSHIP_DESTROY_NUM_PARAM	2
#define	DEF_REST_FSHIP_DESTROY_SCREEN_NAME	DEF_REST_FSHIP_LOOKUP_SCREEN_NAME
#define	DEF_REST_FSHIP_DESTROY_USER_ID		DEF_REST_FSHIP_LOOKUP_USER_ID
// POST friendships/update
#define	DEF_REST_FSHIP_UPDATE				"update.json"
// GET friendship/show
#define	DEF_REST_FSHIP_SHOW					"show.json"
// GET frineds/list
#define	DEF_REST_FRIENDS_LIST				"list.json"
#define	DEF_REST_FRIENDS_LIST_NUM_PARAM		6
#define	DEF_REST_FRIENDS_LIST_USER_ID		"user_id"
#define	DEF_REST_FRIENDS_LIST_SCREEN_NAME	"screen_name"
#define	DEF_REST_FRIENDS_LIST_CURSOR		"cursor"
#define	DEF_REST_FRIENDS_LIST_COUNT			"count"
#define	DEF_REST_FRIENDS_LIST_DEF_COUNT		DEF_REST_TL_DEF_COUNT
#define	DEF_REST_FRIENDS_LIST_MAX_COUNT		DEF_REST_TL_MAX_COUNT
#define	DEF_REST_FRIENDS_LIST_SKIP_STATUS	"skip_status"
#define	DEF_REST_FRIENDS_LIST_INC_USR_ENT	"include_user_entities"
// GET followers/list
#define	DEF_REST_FOLLOWERS_LIST				"list.json"
#define	DEF_REST_FOLLOWERS_LIST_NUM_PARAM	6
#define	DEF_REST_FOLLOWERS_LIST_USER_ID		"user_id"
#define	DEF_REST_FOLLOWERS_LIST_SCREEN_NAME	"screen_name"
#define	DEF_REST_FOLLOWERS_LIST_CURSOR		"cursor"
#define	DEF_REST_FOLLOWERS_LIST_COUNT		"count"
#define	DEF_REST_FOLLOWERS_LIST_DEF_COUNT	DEF_REST_TL_DEF_COUNT
#define	DEF_REST_FOLLOWERS_LIST_MAX_COUNT	DEF_REST_TL_MAX_COUNT
#define	DEF_REST_FOLLOWERS_LIST_SKIP_STATUS	"skip_status"
#define	DEF_REST_FOLLOWERS_LIST_INC_USR_ENT	"include_user_entities"



/*
--------------------------------------------------------------------------------
	Users
--------------------------------------------------------------------------------
*/
// GET/POST account/settings
#define	DEF_REST_ACCOUNT_SETTINGS			"settings.json"
// GET account/verify_credentials
#define	DEF_REST_ACCOUNT_VERI_CRED			"verify_credentials.json"
// POST account/update_delivery_device
#define	DEF_REST_ACCOUNT_UPDATE_DELI_DEV	"update_delivery_device.json"
// POST account/update_profile
#define	DEF_REST_ACCOUNT_UPDATE_PROFILE		"update_profile.json"
// POST account/update_profile_background_image
#define	DEF_REST_ACCOUNT_UPDATE_PR_BG_IMG	"update_profile_background_image.json"
// POST account/update_profile_colors
#define	DEF_REST_ACCOUNT_UPDATE_PR_COLORS	"update_profile_colors.json"
// POST account/update_profile_image
#define	DEF_REST_ACCOUNT_UPDATE_PR_IMG		"update_profile_image.json"
// GET blocks/list
#define	DEF_REST_BLOCKS_LIST				"list.json"
#define	DEF_REST_BLOCKS_LIST_NUM_PARAM		3
#define	DEF_REST_BLOCKS_LIST_INC_ENTITIES	"inlude_entities"
#define	DEF_REST_BLOCKS_LIST_SKIP_STATUS	"skip_status"
#define	DEF_REST_BLOCKS_LIST_CURSOR			"cursor"
// GET blocks/ids
#define	DEF_REST_BLOCKS_IDS					"ids.json"
// POST blocks/create
#define	DEF_REST_BLOCKS_CREATE				"create.json"
#define	DEF_REST_BLOCKS_CREATE_NUM_PARAM	4
#define	DEF_REST_BLOCKS_CREATE_SNAME		"screen_name"
#define	DEF_REST_BLOCKS_CREATE_USER_ID		"user_id"
#define	DEF_REST_BLOCKS_CREATE_INC_ENTITIES	DEF_REST_BLOCKS_LIST_INC_ENTITIES
#define	DEF_REST_BLOCKS_CREATE_SKIP_STATUS	DEF_REST_BLOCKS_LIST_SKIP_STATUS
// POST blocks/destroy
#define	DEF_REST_BLOCKS_DESTROY				"destroy.json"
#define	DEF_REST_BLOCKS_DESTROY_NUM_PARAM	4
#define	DEF_REST_BLOCKS_DESTROY_SNAME		DEF_REST_BLOCKS_CREATE_SNAME
#define	DEF_REST_BLOCKS_DESTROY_USER_ID		DEF_REST_BLOCKS_CREATE_USER_ID
#define	DEF_REST_BLOCKS_DESTROY_INC_ENT		DEF_REST_BLOCKS_LIST_INC_ENTITIES
#define	DEF_REST_BLOCKS_DESTROY_SKIP_STATUS	DEF_REST_BLOCKS_LIST_SKIP_STATUS
// GET users/lookup
#define	DEF_REST_USERS_LOOKUP				"lookup.json"
// GET users/show
#define	DEF_REST_USERS_SHOW					"show.json"
#define	DEF_REST_USERS_SHOW_NUM_PARAM		3
#define	DEF_REST_USERS_SHOW_USER_ID			DEF_REST_BLOCKS_CREATE_USER_ID
#define	DEF_REST_USERS_SHOW_SNAME			DEF_REST_BLOCKS_CREATE_SNAME
#define	DEF_REST_USERS_SHOW_INC_ENT			DEF_REST_BLOCKS_LIST_INC_ENTITIES

// GET users/search
#define	DEF_REST_USERS_SEARCH				"search.json"
// GET users/contributees
#define	DEF_REST_USERS_CONTRIBUTEES			"contributees.json"
// GET users/contributors
#define	DEF_REST_USERS_CONTRIBUTORS			"contributors.json"
// POST account/remove_profile_banner
#define	DEF_REST_ACCOUNT_RM_PR_BANNER		"remove_profile_banner.json"
// POST account/update_profile_banner
#define	DEF_REST_ACCOUNT_UPDATE_PR_BANNER	"update_profile_banner.json"
// GET users/profile_banner
#define	DEF_REST_USERS_PR_BANNER			"profile_banner.json"


/*
--------------------------------------------------------------------------------
	Suggested Users
--------------------------------------------------------------------------------
*/
// GET users/suggestions:slug
// GET users/suggestions
#define	DEF_REST_USERS_SUGGESTIONS			"suggestions.json"
// GET users/suggestions/:slug/members

/*
--------------------------------------------------------------------------------
	Favorites
--------------------------------------------------------------------------------
*/
// GET favorites/list
#define	DEF_REST_FAVORITES_LIST				"list.json"
#define	DEF_REST_FAV_LIST_NUM_PARAM			6
#define	DEF_REST_FAV_LIST_USR_ID			"user_id"
#define	DEF_REST_FAV_LIST_SNAME				"screen_name"
#define	DEF_REST_FAV_LIST_CNT				"count"
#define	DEF_REST_FAV_LIST_CNT_MAX_COUNT		200
#define	DEF_REST_FAV_LIST_SINCE_ID			"since_id"
#define	DEF_REST_FAV_LIST_MAX_ID			"max_id"
#define	DEF_REST_FAV_LIST_INC_ENT			"include_entities"
// POST favorites/destroy
#define	DEF_REST_FAVORITES_DESTROY			"destroy.json"
#define	DEF_REST_FAV_DEST_PARAM_NUM			2
#define	DEF_REST_FAV_DEST_ID				"id"
#define	DEF_REST_FAV_DEST_INC_ENT			DEF_REST_FAV_LIST_INC_ENT
// POST favorites/create
#define	DEF_REST_FAVORITES_CREATE			"create.json"
#define	DEF_REST_FAV_CREAT_PARAM_NUM		2
#define	DEF_REST_FAV_CREAT_ID				DEF_REST_FAV_DEST_ID
#define	DEF_REST_FAV_CREAT_INC_ENT			DEF_REST_FAV_LIST_INC_ENT


/*
--------------------------------------------------------------------------------
	Lists
--------------------------------------------------------------------------------
*/
// GET lists/list
#define	DEF_REST_LISTS_LIST					"list.json"
// GET lists/statuses
#define	DEF_REST_LISTS_STATUSES				"statuses.json"
#define	DEF_REST_LISTS_STS_PARAM_NUM		9
#define	DEF_REST_LISTS_STS_LIST_ID			"list_id"
#define	DEF_REST_LISTS_STS_SLUG				"slug"
#define	DEF_REST_LISTS_STS_OWN_SNAME		"owner_screen_name"
#define	DEF_REST_LISTS_STS_OWN_ID			"owner_id"
#define	DEF_REST_LISTS_STS_SINCE_ID			"since_id"
#define	DEF_REST_LISTS_STS_MAX_ID			"max_id"
#define	DEF_REST_LISTS_STS_COUNT			"count"
#define	DEF_REST_LISTS_STS_MAX_COUNT		DEF_REST_TL_MAX_COUNT
#define	DEF_REST_LISTS_STS_INC_ENT			"include_entities"
#define	DEF_REST_LISTS_STS_INC_RTW			"include_rts"
// POST lists/members/destroy
#define	DEF_REST_LISTS_MEMS_DESTROY			"destroy.json"
#define	DEF_REST_LISTS_MEMS_DSTRY_PARAM_NUM	6
#define	DEF_REST_LISTS_MEMS_DSTRY_LIST_ID	DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_MEMS_DSTRY_SLUG		DEF_REST_LISTS_STS_SLUG
#define	DEF_REST_LISTS_MEMS_DSTRY_USER_ID	"user_id"
#define	DEF_REST_LISTS_MEMS_DSTRY_SNAME		"screen_name"
#define	DEF_REST_LISTS_MEMS_DSTRY_OWN_SNAME	DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_MEMS_DSTRY_OWN_ID	DEF_REST_LISTS_STS_OWN_ID

// GET lists/memberships
#define	DEF_REST_LISTS_MEMBERSHIPS			"memberships.json"
#define	DEF_REST_LISTS_MEM_PARAM_NUM		4
#define	DEF_REST_LISTS_MEM_USER_ID			DEF_REST_LISTS_MEMS_DSTRY_USER_ID
#define	DEF_REST_LISTS_MEM_SNAME			DEF_REST_LISTS_MEMS_DSTRY_SNAME
#define	DEF_REST_LISTS_MEM_CUR				"cursor"
#define	DEF_REST_LISTS_MEM_FILTER			"filter_to_owned_lists"

// GET lists/subscribers
#define	DEF_REST_LISTS_SUBSCRIBERS			"subscribers.json"
#define	DEF_REST_LISTS_SCRIB_PARAM_NUM		7
#define	DEF_REST_LISTS_SCRIB_LIST_ID		DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_SCRIB_SLUG			DEF_REST_LISTS_STS_SLUG
#define	DEF_REST_LISTS_SCRIB_OWN_SNAME		DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_SCRIB_OWN_ID			DEF_REST_LISTS_STS_OWN_ID
#define	DEF_REST_LISTS_SCRIB_CUR			DEF_REST_LISTS_MEM_CUR
#define	DEF_REST_LISTS_SCRIB_INC_ENT		DEF_REST_LISTS_STS_INC_ENT
#define	DEF_REST_LISTS_SCRIB_SKIP_STS		"skip_status"

// POST lists/subscribers/create
#define	DEF_REST_LISTS_SUBS_CREATE			"create.json"
#define	DEF_REST_LISTS_SUBS_CRT_PARAM_NUM	4
#define	DEF_REST_LISTS_SUBS_CRT_OWN_SNAME	DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_SUBS_CRT_OWN_ID		DEF_REST_LISTS_STS_OWN_ID
#define	DEF_REST_LISTS_SUBS_CRT_LIST_ID		DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_SUBS_CRT_SLUG		DEF_REST_LISTS_STS_SLUG
// GET lists/subscribers/show
#define	DEF_REST_LISTS_SUBS_SHOW			"show.json"
// POST lists/subscribers/destroy
#define	DEF_REST_LISTS_SUBS_DESTROY			"destroy.json"
#define	DEF_REST_LISTS_SUBS_DSTRY_PARAM_NUM	4
#define	DEF_REST_LISTS_SUBS_DSTRY_OWN_SNAME	DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_SUBS_DSTRY_OWN_ID	DEF_REST_LISTS_STS_OWN_ID
#define	DEF_REST_LISTS_SUBS_DSTRY_LIST_ID	DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_SUBS_DSTRY_SLUG		DEF_REST_LISTS_STS_SLUG
// POST lists/members/create_all
#define	DEF_REST_LISTS_MEMS_CREATE_ALL		"create_all.json"
// GET lists/members/show
#define	DEF_REST_LISTS_MEMS_SHOW			"show.json"
// GET lists/members
#define	DEF_REST_LISTS_MEMBERS				"members.json"
#define	DEF_REST_LISTS_MEMB_PARAM_NUM		7
#define	DEF_REST_LISTS_MEMB_LIST_ID			DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_MEMB_SLUG			DEF_REST_LISTS_STS_SLUG
#define	DEF_REST_LISTS_MEMB_OWN_SNAME		DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_MEMB_OWN_ID			DEF_REST_LISTS_STS_OWN_ID
#define	DEF_REST_LISTS_MEMB_CUR				DEF_REST_LISTS_MEM_CUR
#define	DEF_REST_LISTS_MEMB_INC_ENT			DEF_REST_LISTS_STS_INC_ENT
#define	DEF_REST_LISTS_MEMB_SKIP_STS		DEF_REST_LISTS_SCRIB_SKIP_STS
// POST lists/members/create
#define	DEF_REST_LISTS_MEMS_CREATE			"create.json"
#define	DEF_REST_LISTS_MEMS_CRT_PARAM_NUM	6
#define	DEF_REST_LISTS_MEMS_CRT_LIST_ID	DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_MEMS_CRT_SLUG		DEF_REST_LISTS_STS_SLUG
#define	DEF_REST_LISTS_MEMS_CRT_USER_ID		DEF_REST_LISTS_MEMS_DSTRY_USER_ID
#define	DEF_REST_LISTS_MEMS_CRT_SNAME		DEF_REST_LISTS_MEMS_DSTRY_SNAME
#define	DEF_REST_LISTS_MEMS_CRT_OWN_SNAME	DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_MEMS_CRT_OWN_ID	DEF_REST_LISTS_STS_OWN_ID
// POST lists/destroy
#define	DEF_REST_LISTS_DESTROY				"destroy.json"
#define	DEF_REST_LISTS_DESTROY_PARAM_NUM	4
#define	DEF_REST_LISTS_DESTROY_OWN_SNAME	DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_DESTROY_OWN_ID		DEF_REST_LISTS_STS_OWN_ID
#define	DEF_REST_LISTS_DESTROY_LIST_ID		DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_DESTROY_SLUG			DEF_REST_LISTS_STS_SLUG
// POST lists/update
#define	DEF_REST_LISTS_UPDATE				"update.json"
// POST lists/create
#define	DEF_REST_LISTS_CREATE				"create.json"
#define	DEF_REST_LISTS_CREATE_PARAM_NUM		3
#define	DEF_REST_LISTS_CREATE_NAME			"name"
#define	DEF_REST_LISTS_CREATE_MODE			"mode"
#define	DEF_REST_LISTS_CREATE_MODE_PUBLIC	"public"
#define	DEF_REST_LISTS_CREATE_MODE_PRIVATE	"private"
#define	DEF_REST_LISTS_CREATE_DESC			"description"
// POST lists/show
#define	DEF_REST_LISTS_SHOW					"show.json"
#define	DEF_REST_LISTS_SHOW_PARAM_NUM		4
#define	DEF_REST_LISTS_SHOW_OWN_SNAME		DEF_REST_LISTS_STS_OWN_SNAME
#define	DEF_REST_LISTS_SHOW_OWN_ID			DEF_REST_LISTS_STS_OWN_ID
#define	DEF_REST_LISTS_SHOW_LIST_ID			DEF_REST_LISTS_STS_LIST_ID
#define	DEF_REST_LISTS_SHOW_SLUG			DEF_REST_LISTS_STS_SLUG
// GET lists/subscriptions
#define	DEF_REST_LISTS_SUBSCRIPTIONS		"subscriptions.json"
#define	DEF_REST_LISTS_SUB_PARAM_NUM		4
#define	DEF_REST_LISTS_SUB_USER_ID			DEF_REST_LISTS_MEM_USER_ID
#define	DEF_REST_LISTS_SUB_SNAME			DEF_REST_LISTS_MEM_SNAME
#define	DEF_REST_LISTS_SUB_COUNT			DEF_REST_LISTS_STS_COUNT
#define	DEF_REST_LISTS_SUB_MAX_COUNT		1000
#define	DEF_REST_LISTS_SUB_CURSOR			DEF_REST_LISTS_MEM_CUR

// POST lists/members/destroy_all
#define	DEF_REST_LISTS_MEMS_DESTROY_ALL		"destroy_all.json"
// GET lists/ownerships
#define	DEF_REST_LISTS_OWNERSHIPS			"ownerships.json"
#define	DEF_REST_LISTS_OWN_PARAM_NUM		4
#define	DEF_REST_LISTS_OWN_USER_ID			DEF_REST_LISTS_MEM_USER_ID
#define	DEF_REST_LISTS_OWN_SNAME			DEF_REST_LISTS_MEM_SNAME
#define	DEF_REST_LISTS_OWN_COUNT			DEF_REST_LISTS_STS_COUNT
#define	DEF_REST_LISTS_OWN_MAX_COUNT		DEF_REST_LISTS_SUB_MAX_COUNT
#define	DEF_REST_LIsTS_OWN_CUR				DEF_REST_LISTS_MEM_CUR

/*
--------------------------------------------------------------------------------
	Saved Serches
--------------------------------------------------------------------------------
*/
// GET saved_searches/list
#define	DEF_REST_SSEARCH_LIST				"list.json"
// GET saved_searches/show/:id
// POST saved_searches/create
#define	DEF_REST_SSEARCH_CREATE				"create.json"
// POST saved_serches/destroy/:id


/*
--------------------------------------------------------------------------------
	Places & Geo
--------------------------------------------------------------------------------
*/
// GET geo/id/:place_id
// GET geo/reverse_geocode
#define	DEF_REST_GEO_R_GEOCODE				"reverse_geocode.json"
// GET geo/search
#define	DEF_REST_GEO_SEARCH					"search.json"
// GET geo/similar_places
#define	DEF_REST_GEO_SIMILAR_PLACES			"similar_places.json"
// POST geo/place
#define	DEF_REST_GEO_PLACE					"place.json"


/*
--------------------------------------------------------------------------------
	Trends
--------------------------------------------------------------------------------
*/
// GET trends/place
#define	DEF_REST_TRENDS_PLACE				"place.json"
// GET trends/available
#define	DEF_REST_TRENDS_AVAIL				"available.json"
// GET treands/closest
#define	DEF_REST_TRENDS_CLOSEST				"closest.json"


/*
--------------------------------------------------------------------------------
	Spam Reporting
--------------------------------------------------------------------------------
*/
// POST users/report_spam
#define	DEF_REST_USERS_REPORT_SPAM			"report_spam.json"


/*
--------------------------------------------------------------------------------
	OAuth
--------------------------------------------------------------------------------
*/
// GET oauth/authenticate
#define	DEF_REST_OAUTH_AUTHENTICATE			"authenticate"
// GET oauth/authorize
#define	DEF_REST_OAUTH_AUTHORIZE			"authorize"
// POST oauth/access_token
#define	DEF_REST_OAUTH_ACCESS_TOKEN			"access_token"
// POST oauth/request_token
#define	DEF_REST_OAUTH_REQ_TOKEN			"request_token"
// POST oauth2/token
#define	DEF_REST_OAUTH2_TOKEN				"token"
// POST oauth2/invalidate_token
#define	DEF_REST_OAUTH2_INV_TOKEN			"invalidate_token"


/*
--------------------------------------------------------------------------------
	Help
--------------------------------------------------------------------------------
*/
// GET help/configuration
#define	DEF_REST_HELP_CONFIG				"configuration.json"
// GET help/languages
#define	DEF_REST_HELP_LANG					"languages.json"
// GET help/privacy
#define	DEF_REST_HELP_PRIVACY				"privacy.json"
// GET help/tos
#define	DEF_REST_HELP_TOS					"tos.json"
// GET application/rate_limit_status
#define	DEF_REST_APP_RATE_LIMIT_STS			"rate_limit_status.json"


/*
================================================================================

	Twitter Objects DEFINES

================================================================================
*/
/*
--------------------------------------------------------------------------------
	Users Entity
--------------------------------------------------------------------------------
*/
/* the followings exclude null terminator	*/
#define	DEF_TWAPI_MAX_USER_ID_LEN		20		// 9223372036854775807
#define	DEF_TWAPI_MAX_SCREEN_NAME_LEN	20
#define	DEF_TWAPI_MAX_NAME_LEN			20		// actual byte length is up to
												// utf-8 coding
#define	DEF_TWAPI_MAX_URL_LEN			100
#define	DEF_TWAPI_MAX_LOCATION_LEN		30		// atucal byte length is up to
												// utf-8 coding
#define	DEF_TWAPI_MAX_DESC_LEN			160		// actual byte length is up to
												// utf-8 coding


#define	DEF_TWAPI_USERS_USER_ID			"user_id"
#define	DEF_TWAPI_USERS_SCREEN_NAME		"screen_name"

/*
--------------------------------------------------------------------------------
	Lists Object
--------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_MAX_LDESC				100
#define	DEF_TWAPI_ACTUAL_MAX_LDESC		( DEF_TWAPI_MAX_LDESC					\
										  * DEF_UTF8_MAX_SIZE					)

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
							 const bool include_entities );

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
								 const bool include_entities );

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
							 const bool include_rts );

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
				 const bool include_user_entities >
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
							const bool include_user_entities );


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
						const bool trim_user );

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
						   const bool trim_user );

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
						   const bool trim_user );

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
							const bool skip_status );

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
								const bool include_entities );

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
int getDirectMessagesShow( struct ssl_session *session, const char *id );

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
						   const char *text );

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
							   const bool include_entities );

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
	Description	:get direct_message
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getFriendsIds( struct ssl_session *session,
				   const char *user_id,
				   const char *screen_name,
				   const char *cursor,
				   const bool stringify_ids,
				   const int count );


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
					 const int count );

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
						  const bool follow );

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
						   const char *screen_name );

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
						 const bool include_user_entities );

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
						   const bool include_user_entities );

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
						const bool include_entities );

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
						   const bool include_entities );

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
							const bool include_entities );

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
					   bool include_entities );

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
						   const bool include_entities );

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
						  const bool include_entities );

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
						 const bool include_entities );

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
				 < number of favorites list to retreive >
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
						   const bool include_rts );

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
							 const char *cursor );

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
								const char *cursor );

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
							  const bool filter_to_owned_lists );

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
						  bool skip_status );

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
							  bool skip_status );

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
								 const char *owner_id );

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
								  const char *owner_id );

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
									 const char *owner_id );

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
									  const char *owner_id );

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
					 const char *description );

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
					  const char *slug );

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
					   const char *slug );

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
int postOauthRequestToken( struct ssl_session *session );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getOauthAuthorize
	Input		:struct ssl_session *session
				 < ssl session >
	Output		:void
	Return		:int
				 < result >
	Description	:request oauth/authorize
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getOauthAuthorize( struct ssl_session *session );

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
int postOauthAccessToken( struct ssl_session *session );

#endif	//__TWITTER_API_H__

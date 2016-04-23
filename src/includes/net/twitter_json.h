/*******************************************************************************
 File:twitter_json.h
 Description:Definitions of JSON of Twitter

*******************************************************************************/
#ifndef	__TWITTER_JSON_H__
#define	__TWITTER_JSON_H__

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
----------------------------------------------------------------------------------

	Tweets Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_TW_ANNOTATIONS		"annotaions"				// object
#define	DEF_TWAPI_OBJ_TW_CONTRIBUTORS		"contributors"				// [contributors] objects
#define	DEF_TWAPI_OBJ_TW_COORDINATES		"coordinates"				// [coordinates] object
#define	DEF_TWAPI_OBJ_TW_CREATED_AT			"created_at"				// string
#define	DEF_TWAPI_OBJ_TW_CUR_USR_RTW		"current_user_retweet"		// object
#define	DEF_TWAPI_OBJ_TW_ENTITIES			"entities"					// [entities] objects
#define	DEF_TWAPI_OBJ_TW_FAV_CNT			"favorite_count"			// integer favourite?
#define	DEF_TWAPI_OBJ_TW_FAVORITED			"favorited"					// boolean
#define	DEF_TWAPI_OBJ_TW_FLT_LEVEL			"filter_level"				// string
#define	DEF_TWAPI_OBJ_TW_GEO				"geo"						// object
#define	DEF_TWAPI_OBJ_TW_ID					"id"						// int64
#define	DEF_TWAPI_OBJ_TW_ID_STR				"id_str"					// string
#define	DEF_TWAPI_OBJ_TW_IN_RE_TO_SNAME		"in_reply_to_screen_name"	// string
#define	DEF_TWAPI_OBJ_TW_IN_RE_TO_SID		"in_reply_to_status_id"		// int64
#define	DEF_TWAPI_OBJ_TW_IN_RE_TO_SIDS		"in_reply_to_status_id_str"	// string
#define	DEF_TWAPI_OBJ_TW_IN_RE_TO_UID		"in_reply_to_user_id"		// int64
#define	DEF_TWAPI_OBJ_TW_IN_RE_TO_UIDS		"in_reply_to_user_id_str"	// string
#define	DEF_TWAPI_OBJ_TW_LANG				"lang"						// sting
#define	DEF_TWAPI_OBJ_TW_PLACE				"place"						// [place] object
#define	DEF_TWAPI_OBJ_TW_POS_SENS			"possibly_sensitive"		// boolean
#define	DEF_TWAPI_OBJ_TW_SCOPES				"scopes"					// object
#define	DEF_TWAPI_OBJ_TW_RTW_CNT			"retweet_count"				// integer
#define	DEF_TWAPI_OBJ_TW_RETWEETED			"retweeted"					// boolean
#define	DEF_TWAPI_OBJ_TW_RTW_STATUS			"retweeted_status"			// [tweet] object
#define	DEF_TWAPI_OBJ_TW_SOURCE				"source"					// string
#define	DEF_TWAPI_OBJ_TW_TEXT				"text"						// string
#define	DEF_TWAPI_OBJ_TW_TRUNC				"truncated"					// boolean
#define	DEF_TWAPI_OBJ_TW_USER				"user"						// [users] object
#define	DEF_TWAPI_OBJ_TW_WCOPY_RIGHT		"withheld_copyright"		// boolean
#define	DEF_TWAPI_OBJ_TW_W_IN_COUNTRY		"withheld_in_countries"		// array of string
#define	DEF_TWAPI_OBJ_TW_WSCOPE				"withheld_scope"			// string

/* direct message																*/
#define	DEF_TWAPI_OBJ_DM_CREATED_AT			DEF_TWAPI_OBJ_TW_CREATED_AT
#define	DEF_TWAPI_OBJ_DM_ID_STR				DEF_TWAPI_OBJ_TW_ID_STR
#define	DEF_TWAPI_OBJ_DM_TEXT				DEF_TWAPI_OBJ_TW_TEXT
#define	DEF_TWAPI_OBJ_DM_RECP_ID			"recipient_id"
#define	DEF_TWAPI_OBJ_DM_RECP_ID_STR		"recipient_id_str"
#define	DEF_TWAPI_OBJ_DM_RECP_SNAME			"recipient_screen_name"
#define	DEF_TWAPI_OBJ_DM_SEND_ID			"sender_id"
#define	DEF_TWAPI_OBJ_DM_SEND_ID_STR		"sender_id_str"
#define	DEF_TWAPI_OBJ_DM_SEND_SNAME			"sender_screen_name"

/*
----------------------------------------------------------------------------------

	Contributors Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_CONTRI_ID				"id"						// int64
#define	DEF_TWAPI_OBJ_CONTRI_ID_STR			"id_str"					// string
#define	DEF_TWAPI_OBJ_CONTRI_SNAME			"screen_name"				// string

/*
----------------------------------------------------------------------------------

	Coordinates Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_COORD_COORDINATES		"coordinates"				// array of float
#define	DEF_TWAPI_OBJ_COORD_TYPE			"type"						// string


/*
----------------------------------------------------------------------------------

	Users Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_USR					"user"
#define	DEF_TWAPI_OBJ_USRS					"users"
#define	DEF_TWAPI_OBJ_USR_CONTRI_ENABLED	"contributors_enabled"		// boolean
#define	DEF_TWAPI_OBJ_USR_CREATED_AT		"created_at"				// string
#define	DEF_TWAPI_OBJ_USR_DEF_PROFILE		"default_profile"			// boolean
#define	DEF_TWAPI_OBJ_USR_DEF_PRO_IMG		"default_proifle_image"		// boolean
#define	DEF_TWAPI_OBJ_USR_DESCRIPTION		"description"				// string
#define	DEF_TWAPI_OBJ_USR_ENTITIES			"entities"					// [entities] object
#define	DEF_TWAPI_OBJ_USR_FAV_CNT			"favourites_count"			// int favorite?
#define	DEF_TWAPI_OBJ_USR_FOLLOW_REQ_SENT	"follow_request_sent"		// boolean
#define	DEF_TWAPI_OBJ_USR_FOLLOWING			"following"					// boolean
#define	DEF_TWAPI_OBJ_USR_FOLLOWERS_CNT		"followers_count"			// int
#define	DEF_TWAPI_OBJ_USR_FRIENDS_CNT		"friends_count"				// int
#define	DEF_TWAPI_OBJ_USR_GEO_ENABLED		"geo_enabled"				// boolean
#define	DEF_TWAPI_OBJ_USR_ID				"id"						// int64
#define	DEF_TWAPI_OBJ_USR_ID_STR			"id_str"					// string
#define	DEF_TWAPI_OBJ_USR_IS_TRANS			"is_translator"				// boolean
#define	DEF_TWAPI_OBJ_USR_LANG				"lang"						// string
#define	DEF_TWAPI_OBJ_USR_LISTED_CNT		"listed_count"				// int
#define	DEF_TWAPI_OBJ_USR_LOCATION			"location"					// string
#define	DEF_TWAPI_OBJ_USR_NAME				"name"						// string
#define	DEF_TWAPI_OBJ_USR_NOTIFICATIONS		"notifications"				// boolean
#define	DEF_TWAPI_OBJ_USR_PRO_BG_COLOR		"profile_background_color"	// string
#define	DEF_TWAPI_OBJ_USR_PRO_BG_IMG_URL	"profile_background_image_url"	// strubg
#define	DEF_TWAPI_OBJ_USR_PRO_BG_IMG_URL_H	"profile_background_image_url_https"	// string
#define	DEF_TWAPI_OBJ_USR_PRO_BG_TILE		"profile_backgournd_tile"	// boolean
#define	DEF_TWAPI_OBJ_USR_PRO_BANNER_URL	"profile_banner_url"		// string
#define	DEF_TWAPI_OBJ_USR_PRO_IMG_URL		"profile_image_url"			// string
#define	DEF_TWAPI_OBJ_USR_PRO_IMG_URL_H		"profile_image_url_https"	// string
#define	DEF_TWAPI_OBJ_USR_PRO_LINK_COLOR	"profile_link_color"		// string
#define	DEF_TWAPI_OBJ_USR_PRO_SB_COLOR		"profile_sidebar_border_ccolor"	// string
#define	DEF_TWAPI_OBJ_USR_PRO_SF_COLOR		"profile_sidebar_fill_color"	// string
#define	DEF_TWAPI_OBJ_USR_PRO_TEXT_COLOR	"profile_text_color"			// string
#define	DEF_TWAPI_OBJ_USR_PRO_UBG_IMG		"profile_use_background_image"	// boolean
#define	DEF_TWAPI_OBJ_USR_PROTECTED			"protected"					// boolean
#define	DEF_TWAPI_OBJ_USR_SNAME				"screen_name"				// string
#define	DEF_TWAPI_OBJ_USR_SHOW_ALLIN_MEDIA	"show_all_inline_media"		// boolean
#define	DEF_TWAPI_OBJ_USR_STATUS			"status"					// [tweet] object
#define	DEF_TWAPI_OBJ_USR_STATUSES_CNT		"statuses_count"			// int
#define	DEF_TWAPI_OBJ_USR_TIME_ZONE			"time_zone"					// string
#define	DEF_TWAPI_OBJ_USR_URL				"url"						// string
#define	DEF_TWAPI_OBJ_USR_UTC_OFFSET		"utc_offset"				// int
#define	DEF_TWAPI_OBJ_USR_VERIFIED			"verified"					// boolean
#define	DEF_TWAPI_OBJ_USR_WIN_COUNTRY		"withheld_in_countries"		// string
#define	DEF_TWAPI_OBJ_USR_WSCOPE			"withheld_scope"			// string

/* direct message																 */
#define	DEF_TWAPI_OBJ_DM_USR_RECP			"recipient"
#define	DEF_TWAPI_OBJ_DM_USR_SEND			"sender"
#define	DEF_TWAPI_OBJ_DM_USR_ID_STR			DEF_TWAPI_OBJ_USR_ID_STR
#define	DEF_TWAPI_OBJ_DM_USR_NAME			DEF_TWAPI_OBJ_USR_NAME
#define	DEF_TWAPI_OBJ_DM_USR_PROTECTED		DEF_TWAPI_OBJ_USR_PROTECTED
#define	DEF_TWAPI_OBJ_DM_USR_SNAME			DEF_TWAPI_OBJ_USR_SNAME

/*
----------------------------------------------------------------------------------

	Entities Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_ENT_HASHTAG			"hashtags"					// [hashtag] object
#define	DEF_TWAPI_OBJ_ENT_MEDIA				"media"						// [media] object
#define	DEF_TWAPI_OBJ_ENT_URL				"urls"						// [url] object
#define	DEF_TWAPI_OBJ_ENT_USR_MENTION		"user_mentions"				// [user_mention] object

/*
----------------------------------------------------------------------------------

	Hashtags Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_HTAGS_INDICES			"indices"					// array of int
#define	DEF_TWAPI_OBJ_HTAGS_TEXT			"text"						// string

/*
----------------------------------------------------------------------------------

	Media Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_MEDIA_DISP_URL		"display_url"				// string
#define	DEF_TWAPI_OBJ_MEDIA_EXPND_URL		"expanded_url"				// string
#define	DEF_TWAPI_OBJ_MEDIA_ID				"id"						// int64
#define	DEF_TWAPI_OBJ_MEDIA_ID_STR			"id_str"					// string
#define	DEF_TWAPI_OBJ_MEDIA_INDICES			"indices"					// array of int
#define	DEF_TWAPI_OBJ_MEDIA_M_URL			"media_url"					// string
#define	DEF_TWAPI_OBJ_MEDIA_M_URL_H			"media_url_https"			// string
#define	DEF_TWAPI_OBJ_MEDIA_SIZE			"sizes"						// [size] object
#define	DEF_TWAPI_OBJ_MEDIA_SRC_ST_ID		"source_status_id"			// int64
#define	DEF_TWAPI_OBJ_MEDIA_SRC_ST_ID_STR	"source_status_id_str"		// string
#define	DEF_TWAPI_OBJ_MEDIA_TYPE			"type"						// string
#define	DEF_TWAPI_OBJ_MEDIA_URL				"url"						// string

/*
----------------------------------------------------------------------------------

	Size Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_SIZE_H				"h"							// int
#define	DEF_TWAPI_OBJ_SIZE_RESIZE			"resize"					// string
#define	DEF_TWAPI_OBJ_SIZE_W				"w"							// int

/*
----------------------------------------------------------------------------------

	Sizes Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_SIZES_THUMB			"thumb"						// [size] object
#define	DEF_TWAPI_OBJ_SIZES_LARGE			"large"						// [size] object
#define	DEF_TWAPI_OBJ_SIZES_MEDIUM			"medium"					// [size] object
#define	DEF_TWAPI_OBJ_SIZES_SMALL			"small"						// [size] object

/*
----------------------------------------------------------------------------------

	URL Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_URL_DISP_URL			"display_url"				// string
#define	DEF_TWAPI_OBJ_URL_EXPND_URL			"expanded_url"				// string
#define	DEF_TWAPI_OBJ_URL_INDICES			"indices"					// array of int
#define	DEF_TWAPI_OBJ_URL_URL				"url"						// string

/*
----------------------------------------------------------------------------------

	User Mention Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_USRM_ID				"id"						// int64
#define	DEF_TWAPI_OBJ_USRM_ID_STR			"id_str"					// string	
#define	DEF_TWAPI_OBJ_USRM_INDICES			"indices"					// array of int
#define	DEF_TWAPI_OBJ_USRM_NAME				"name"						// string
#define	DEF_TWAPI_OBJ_USRM_SNAME			"screen_name"				// string


/*
----------------------------------------------------------------------------------

	Places Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_PL_ATTR				"attributes"				// [attribute] object
#define	DEF_TWAPI_OBJ_PL_BNDBOX				"bounding_box"				// [bounding_box] object
#define	DEF_TWAPI_OBJ_PL_COUNTRY			"country"					// string
#define	DEF_TWAPI_OBJ_PL_COUNTRY_CODE		"country_code"				// string
#define	DEF_TWAPI_OBJ_PL_FULL_NAME			"full_name"					// string
#define	DEF_TWAPI_OBJ_PL_ID					"id"						// string
#define	DEF_TWAPI_OBJ_PL_NAME				"name"						// string
#define	DEF_TWAPI_OBJ_PL_PL_TYPE			"place_type"				// string
#define	DEF_TWAPI_OBJ_PL_URL				"url"						// string

/*
----------------------------------------------------------------------------------

	Bounding Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_BNDBOX_COORD			"coordinates"				// aray of float
#define	DEF_TWAPI_OBJ_BNDBOX_TYPE			"type"						// string

/*
----------------------------------------------------------------------------------

	List Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_LIST_PREV_CUR			"previous_cursor"
#define	DEF_TWAPI_OBJ_LIST_PREV_CUR_STR		"previous_cursor_str"
#define	DEF_TWAPI_OBJ_LIST_NEXT_CUR			"next_curosor"
#define	DEF_TWAPI_OBJ_LIST_NEXT_CUR_STR		"next_cursor_str"

/*
----------------------------------------------------------------------------------

	Lists Object

----------------------------------------------------------------------------------
*/
#define	DEF_TWAPI_OBJ_LISTS					"lists"
#define	DEF_TWAPI_OBJ_LISTS_CREATED_AT		"created_at"
#define	DEF_TWAPI_OBJ_LISTS_NAME			"name"
#define	DEF_TWAPI_OBJ_LISTS_SLUG			"slug"
#define	DEF_TWAPI_OBJ_LISTS_URI				"uri"
#define	DEF_TWAPI_OBJ_LISTS_ID_STR			"id_str"
#define	DEF_TWAPI_OBJ_LISTS_SUB_CNT			"subscriber_count"
#define	DEF_TWAPI_OBJ_LISTS_MEM_CNT			"member_count"
#define	DEF_TWAPI_OBJ_LISTS_MODE			"mode"
#define	DEF_TWAPI_OBJ_LISTS_ID				"id"
#define	DEF_TWAPI_OBJ_LISTS_FULL_NAME		"full_name"
#define	DEF_TWAPI_OBJ_LISTS_DESC			"description"


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
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	//__TWITTER_JSON_H__

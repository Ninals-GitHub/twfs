/*******************************************************************************
 File:twfs.h
 Description:Definitions of twitter pesudo file sytem operations

*******************************************************************************/
#ifndef	__TWFS_H__
#define	__TWFS_H__

#include <limits.h>

//#include "twfs_internal.h"
//#include "net/twitter_api.h"

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
#define	FUSE_USE_VERSION				26

// for pwrite
#define	XOPEN_SOURCE					500

#define	DEF_TWFS_PATH_MAX				PATH_MAX

/*
-------------------------------------------------------------------------------
	Twitter Filesystem Home Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_LOGIN_NAME				"login_name"
#define	DEF_TWFS_PATH_TL				"home_timeline"
#define	DEF_TWFS_PATH_USER_TL			"user_timeline"
#define	DEF_TWFS_PATH_TWEET				"tweet"

/*
-------------------------------------------------------------------------------
	/[screen_name]/status/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_STATUS		"status"

/*
-------------------------------------------------------------------------------
	/[screen_name]/retweet/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_RETWEET		"retweet"

/*
-------------------------------------------------------------------------------
	/[screen_name]/status/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_FAV			"favorites"
#define	DEF_TWFS_PATH_FAV_LIST			"list"

/*
-------------------------------------------------------------------------------
	/[screen_name]/blocks/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_BLOCKS		"blocks"
#define	DEF_TWFS_PATH_BLOCK_LIST		"list"
#define	DEF_TWFS_PATH_DOT_BLOCK_LIST	".list"

/*
-------------------------------------------------------------------------------
	/[screen_name]/notifications/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_NOTI			"notifications"
#define	DEF_TWFS_PATH_AT_TW				"@tweet"
#define	DEF_TWFS_PATH_RTW				"retweets_of_me"

/*
-------------------------------------------------------------------------------
	/[screen_name]/direct_messages/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_DM			"direct_message"
#define	DEF_TWFS_PATH_DM_MSG			"message"
#define	DEF_TWFS_PATH_DM_SEND_TO		"send_to"

/*
-------------------------------------------------------------------------------
	/[screen_name]/{follower,following}/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_FOLLOWERS		"follower"
#define	DEF_TWFS_PATH_DIR_FRIENDS		"following"
#define	DEF_TWFS_PATH_FF_LIST			"list"		// {follwer,follwing}/list
#define	DEF_TWFS_PATH_FF_DOT_LIST		".list"		// {follwer,follwing}/.list

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_LISTS			"lists"
//#define	DEF_TWFS_PATH_LIST				"list"

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/subscriptions/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_SUB			"subscriptions"
#define	DEF_TWFS_PATH_SUB_LIST			"list"
#define	DEF_TWFS_PATH_SUB_DOT_LIST		".list"
//#define	DEF_TWFS_PATH_SUB_SUBSCRIBERS	"subscribers"
//#define	DEF_TWFS_PATH_SUB_MEMBERSHIPS	"members"

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/my_list Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_OWN			"my_list"
#define	DEF_TWFS_PATH_OWN_LIST			DEF_TWFS_PATH_SUB_LIST
#define	DEF_TWFS_PATH_OWN_DOT_LIST		DEF_TWFS_PATH_SUB_DOT_LIST
//#define	DEF_TWFS_PATH_OWN_SUBSCRIBERS	"subscribers"
//#define	DEF_TWFS_PATH_OWN_MEMBERSHIPS	"members"

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/added Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_ADD			"added"
#define	DEF_TWFS_PATH_ADD_LIST			DEF_TWFS_PATH_SUB_LIST
#define	DEF_TWFS_PATH_ADD_DOT_LIST		DEF_TWFS_PATH_SUB_DOT_LIST

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/{subscriptions, my_list, added}/[list name] Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_LNAME_LDESC		"description"
#define	DEF_TWFS_PATH_LNAME_TL			"timeline"

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/{subscriptions, my_list, added}/[list name]/members
	Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_LNAME_MEM			"members"
#define	DEF_TWFS_PATH_LNAME_MEM_LIST		DEF_TWFS_PATH_SUB_LIST
#define	DEF_TWFS_PATH_LNAME_MEM_DOT_LIST	DEF_TWFS_PATH_SUB_DOT_LIST

/*
-------------------------------------------------------------------------------
	/[screen_name]/lists/{subscriptions, my_list, added}/[list name]/subscriber
	Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_LNAME_SUB			"subscribers"
#define	DEF_TWFS_PATH_LNAME_SUB_LIST		DEF_TWFS_PATH_SUB_LIST
#define	DEF_TWFS_PATH_LNAME_SUB_DOT_LIST	DEF_TWFS_PATH_SUB_DOT_LIST

/*
-------------------------------------------------------------------------------
	/[screen_name]/account/ Directory
-------------------------------------------------------------------------------
*/
#define	DEF_TWFS_PATH_DIR_ACCOUNT		"account"
#define	DEF_TWFS_PATH_PROFILE			"profile"
#define	DEF_TWFS_PATH_SETTINGS			"settings"


/*
---------------------------------------------------------------------------------

	Timelines, Message or Lists Files Information

---------------------------------------------------------------------------------
*/
struct twfs_file
{
	char	*tl;			// timeline file ( includes message/list file )
	size_t	size;			// atcual file size of timeline file
	size_t	tl_size;		// timline file size over twfs
	int		fd;				// timeline file descritpor
};

/* ---------------------------------------------------------------------------- */
/* header format																*/
/* ---------------------------------------------------------------------------- */
/* header of timeline has total timeline file size
	total file saize is recorded first line
	example:
	18446744073709551615

	for follower/following list file, next cursor and previous cursor are
	recoded.
	example:
	18446744073709551615
	18446744073709551615
	-18446744073709551615
	
	first is total length, second is next cursor, third is prev cursor

*/
#define	DEF_TWFS_HEAD_TL_SIZE_FIELD			20
#define	DEF_TWFS_HEAD_TL_SIZE_FIELD_NEXT	1	// for '\n'

#define	DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN		( DEF_TWFS_HEAD_TL_SIZE_FIELD		\
											  + DEF_TWFS_HEAD_TL_SIZE_FIELD_NEXT )

#define	DEF_TWFS_OFFSET_BODY_OF_TL			( DEF_TWFS_HEAD_TL_SIZE_FIELD		\
											  + DEF_TWFS_HEAD_TL_SIZE_FIELD_NEXT )

#define	DEF_TWFS_HEAD_FF_SIZE_FIELD				20
#define	DEF_TWFS_HEAD_FF_SIZE_FIELD_NEXT		1
#define	DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD			20
#define	DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_NEXT	1
#define	DEF_TWFS_HEAD_FF_PREV_CUR_FIELD			20
#define	DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_NEXT	1

#define	DEF_TWFS_HEAD_FF_SIZE_FIELD_LEN		( DEF_TWFS_HEAD_FF_SIZE_FIELD			\
											  + DEF_TWFS_HEAD_FF_SIZE_FIELD_NEXT	)
#define	DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN	( DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD		\
											  + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_NEXT)
#define	DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN	( DEF_TWFS_HEAD_FF_PREV_CUR_FIELD		\
											  + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_NEXT)
#define	DEF_TWFS_OFFSET_FF_NEXT_CUR_FIELD	DEF_TWFS_HEAD_FF_SIZE_FIELD_LEN
#define	DEF_TWFS_OFFSET_FF_PREV_CUR_FIELD	( DEF_TWFS_OFFSET_FF_NEXT_CUR_FIELD		\
											  + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN	)
#define	DEF_TWFS_OFFSET_BODY_OF_FF			( DEF_TWFS_HEAD_FF_SIZE_FIELD_LEN		\
											  + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN	\
											  + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN	)
#define	DEF_TWFS_HEAD_FF_LEN				DEF_TWFS_OFFSET_BODY_OF_FF


/* ---------------------------------------------------------------------------- */
/* timeline, tweet, direct message file format									*/
/* ---------------------------------------------------------------------------- */
/*
	the record of timeline file etc. consist of the following format

	[ id ] [ screen name ] [ Retweet flag ] [ Retweet text length ] [ text len ]

	length of each field is :
	[ 20 ] [ 20 ] [ 1 ] [ 2 ] [ 10(int) ]

	for list file of lists directory:
	[ slug ] [ screen name ] [ flag ] [ flag text length ] [ text len ]
	note: flag of lists is meaningless. it is just for common procedure
	      both of interpretation of timeline file and lists file

	length of each field
	[ 100 ] [ 20 ] [ 1(F) ] [ 2(fixed to "0000" ] [ 10 ]

	example:
	<for retweet>
	18446744073709551615 01235678901234567890 R 64 4294967295\n
	<for normal tweet>
	18446744073709551615 01235678901234567890 N 0  4294967295\n
	<for received direct message>
	18446744073709551615 01235678901234567890 r 0  4294967295\n
	<for sent direct message>
	18446744073709551615 01235678901234567890 s 0  4294967295\n
	- Retweet text length field is fixed length.
	<for follower/following list>
	18446744073709551615 01235678901234567890 N 0  4294967295\n
	- id is user's id
	- Retweet flag field is fixed 'L'
	- Retweet text length field is fixed length.
	- text len is fixed length

	Retweet flag:
	[R] : Retweet file
	[N] : tweet file
	[r] : direct message file which received from
	[s] : direct message file which sent to
	[l] : direct message file which sender and recipient are same user
	[L] : follower/friends list file
	[F] : meaningless.

*/
#define	DEF_TWFS_ID_FIELD				DEF_TWAPI_MAX_USER_ID_LEN
#define	DEF_TWFS_ID_FIELD_NEXT			1
#define	DEF_TWFS_SNAME_FIELD			DEF_TWAPI_MAX_SCREEN_NAME_LEN
#define	DEF_TWFS_SNAME_FIELD_NEXT		1
#define	DEF_TWFS_RTW_FIELD				1
#define	DEF_TWFS_RTW_FIELD_NEXT			1	// Retweeted by [screen_name ]
#define	DEF_TWFS_RTW_LEN_FIELD			2
#define	DEF_TWFS_RTW_LEN_FIELD_NEXT		1
#define	DEF_TWFS_TEXT_LEN_FIELD			10
#define	DEF_TWFS_TEXT_LEN_FIELD_NEXT	1	// for new line

#define	DEF_TWFS_TL_RECORD_LEN			( DEF_TWFS_ID_FIELD						\
										  + DEF_TWFS_ID_FIELD_NEXT				\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT				\
										  + DEF_TWFS_RTW_LEN_FIELD				\
										  + DEF_TWFS_RTW_LEN_FIELD_NEXT			\
										  + DEF_TWFS_TEXT_LEN_FIELD				\
										  + DEF_TWFS_TEXT_LEN_FIELD_NEXT )

#define	DEF_TWFS_OFFSET_ID_FIELD		0
#define	DEF_TWFS_OFFSET_SNAME_FIELD		( DEF_TWFS_OFFSET_ID_FIELD				\
										  + DEF_TWFS_ID_FIELD					\
										  + DEF_TWFS_ID_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_RTW_FIELD		( DEF_TWFS_OFFSET_ID_FIELD				\
										  + DEF_TWFS_ID_FIELD					\
										  + DEF_TWFS_ID_FIELD_NEXT				\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_DM_FIELD		DEF_TWFS_OFFSET_RTW_FIELD
#define	DEF_TWFS_OFFSET_RTW_LEN_FIELD	( DEF_TWFS_OFFSET_ID_FIELD				\
										  + DEF_TWFS_ID_FIELD					\
										  + DEF_TWFS_ID_FIELD_NEXT				\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_TEXT_LEN_FIELD	( DEF_TWFS_OFFSET_ID_FIELD				\
										  + DEF_TWFS_ID_FIELD					\
										  + DEF_TWFS_ID_FIELD_NEXT				\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT				\
										  + DEF_TWFS_RTW_LEN_FIELD				\
										  + DEF_TWFS_RTW_LEN_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_TEXT_LEN_FIELD_NEXT	( DEF_TWFS_OFFSET_ID_FIELD			\
										  + DEF_TWFS_ID_FIELD					\
										  + DEF_TWFS_ID_FIELD_NEXT				\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT				\
										  + DEF_TWFS_RTW_LEN_FIELD				\
										  + DEF_TWFS_RTW_LEN_FIELD_NEXT			\
										  + DEF_TWFS_TEXT_LEN_FIELD )

#define	DEF_TWFS_SLUG_FIELD				DEF_REST_ACTUAL_SLUG_MAX_LENGTH
#define	DEF_TWFS_SLUG_FIELD_NEXT		1

#define	DEF_TWFS_LISTS_RECORD_LEN		( DEF_TWFS_SLUG_FIELD					\
										  + DEF_TWFS_SLUG_FIELD_NEXT			\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT				\
										  + DEF_TWFS_RTW_LEN_FIELD				\
										  + DEF_TWFS_RTW_LEN_FIELD_NEXT			\
										  + DEF_TWFS_TEXT_LEN_FIELD				\
										  + DEF_TWFS_TEXT_LEN_FIELD_NEXT )

#define	DEF_TWFS_OFFSET_SLUG_FIELD		0
#define	DEF_TWFS_OFFSET_LISTS_SNAME		( DEF_TWFS_OFFSET_SLUG_FIELD			\
										  + DEF_TWFS_SLUG_FIELD					\
										  + DEF_TWFS_SLUG_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_LISTS_FLG		( DEF_TWFS_OFFSET_SLUG_FIELD			\
										  + DEF_TWFS_SLUG_FIELD					\
										  + DEF_TWFS_SLUG_FIELD_NEXT			\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_LISTS_FLG_LEN	( DEF_TWFS_OFFSET_SLUG_FIELD			\
										  + DEF_TWFS_SLUG_FIELD					\
										  + DEF_TWFS_SLUG_FIELD_NEXT			\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_LISTS_TEXT_LEN	( DEF_TWFS_OFFSET_SLUG_FIELD			\
										  + DEF_TWFS_SLUG_FIELD					\
										  + DEF_TWFS_SLUG_FIELD_NEXT			\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT				\
										  + DEF_TWFS_RTW_LEN_FIELD				\
										  + DEF_TWFS_RTW_LEN_FIELD_NEXT )
#define	DEF_TWFS_OFFSET_LISTS_TEXT_NEXT	( DEF_TWFS_OFFSET_SLUG_FIELD			\
										  + DEF_TWFS_SLUG_FIELD					\
										  + DEF_TWFS_SLUG_FIELD_NEXT			\
										  + DEF_TWFS_SNAME_FIELD				\
										  + DEF_TWFS_SNAME_FIELD_NEXT			\
										  + DEF_TWFS_RTW_FIELD					\
										  + DEF_TWFS_RTW_FIELD_NEXT				\
										  + DEF_TWFS_RTW_LEN_FIELD				\
										  + DEF_TWFS_RTW_LEN_FIELD_NEXT			\
										  + DEF_TWFS_TEXT_LEN_FIELD )

/* ---------------------------------------------------------------------------- */
/* profile file format															*/
/* ---------------------------------------------------------------------------- */
/*
	--------------------------------------------- 44 '-'s. '\n'
	[ id           ]:id. maximum length is 20.'\n'
	[ created at   ]:time. maximum length is 31.'\n'
	[ name         ]:name of user. maximum length is 20 * 4 ( utf-8 encoded ).'\n'
	[ screen name  ]:screen name. maximum length is 20
	[ description  ]:description of user. maximum length is 160 * 4 ( utf-8 ).'\n'
	[ location     ]:location. maximum length is 30 * 4.'\n'
	[ url          ]:url. maximum length is 100.'\n'
	'\n'
	[ statuses     ]:number of statuses. maximum length is 20.'\n'
	[ favorites    ]:number of favorites. maximum length is 20.'\n'
	[ followings   ]:number of following. maximum length is 20.'\n'
	[ followers    ]:number of followers. maximum length is 20.'\n'
	[ listed count ]:number of listed count. maximum length is 20.
	'\n'
	following [*]   followed [*]    verified [*]



	if authenticated user follows the user, fill [*] in following item.
	if authenticated user is followed by the user, fill [*] in followed item.
	if the user is verified user, fill [*] in verified item.
*/
#define	DEF_PROF_SEPARATOR					"--------------------------------------------"
#define	DEF_PROF_ID							"[ id           ]:"
#define	DEF_PROF_CREATED_AT					"[ created at   ]:"
#define	DEF_PROF_NAME						"[ name         ]:"
#define	DEF_PROF_SNAME						"[ screen name  ]:"
#define	DEF_PROF_DESCRIPTION				"[ description  ]:"
#define	DEF_PROF_LOCATION					"[ location     ]:"
#define	DEF_PROF_URL						"[ url          ]:"

#define	DEF_PROF_STATUSES					"[ tweets       ]:"
#define	DEF_PROF_FAVS						"[ favorites    ]:"
#define	DEF_PROF_FOLLOWING					"[ following    ]:"
#define	DEF_PROF_FOLLOWERS					"[ followers    ]:"
#define	DEF_PROF_LISTED_CNT					"[ listed       ]:"

#define	DEF_PROF_NOW_FOLLOWING				"follwing"
#define	DEF_PROF_FOLLOWED_BY				"followed"
#define	DEF_PROF_VERIFIED					"verified"

/* filed length																	*/
#define	DEF_PROF_SEPARATOR_FIELD			44
#define	DEF_PROF_SEPARATOR_FIELD_NEXT		1		// '\n'
#define	DEF_PROF_SEPARATOR_FIELD_LEN		( DEF_PROF_SEPARATOR_FIELD			\
											  + DEF_PROF_SEPARATOR_FIELD_NEXT	)
#define	DEF_PROF_ITEM_FIELD					17
#define	DEF_PROF_ITEM_FIELD_LEN				( DEF_PROF_ITEM_FIELD				)
#define	DEF_PROF_NUM_ITEM_FIELD				12
#define	DEF_PROF_ALL_ITEM_FIELD_LEN			( DEF_PROF_ITEM_FIELD_LEN			\
											  * DEF_PROF_NUM_ITEM_FIELD			)
#define	DEF_PROF_ID_FIELD					DEF_TWFS_ID_FIELD
#define	DEF_PROF_ID_FIELD_NEXT				1	// '\n'
#define	DEF_PROF_ID_FIELD_LEN				( DEF_PROF_ID_FIELD					\
											  + DEF_PROF_ID_FIELD_NEXT			)
#define	DEF_PROF_CREATED_AT_FIELD			30
#define	DEF_PROF_CREATED_AT_FIELD_NEXT		1
#define	DEF_PROF_CREATED_AT_FIELD_LEN		( DEF_PROF_CREATED_AT_FIELD			\
											  + DEF_PROF_CREATED_AT_FIELD_NEXT	)
#define	DEF_PROF_NAME_FIELD					( DEF_TWAPI_MAX_NAME_LEN			\
											  * DEF_UTF8_MAX_SIZE				)
#define	DEF_PROF_NAME_FIELD_NEXT			1	// '\n'
#define	DEF_PROF_NAME_FIELD_LEN				( DEF_PROF_NAME_FIELD				\
											  + DEF_PROF_NAME_FIELD_NEXT		)
#define	DEF_PROF_SNAME_FIELD				DEF_TWFS_SNAME_FIELD
#define	DEF_PROF_SNAME_FIELD_NEXT			1	// '\n'
#define	DEF_PROF_SNAME_FIELD_LEN			( DEF_PROF_SNAME_FIELD				\
											  + DEF_PROF_SNAME_FIELD_NEXT		)
#define	DEF_PROF_DESC_FIELD					( DEF_TWAPI_MAX_DESC_LEN			\
											  * DEF_UTF8_MAX_SIZE				)
#define	DEF_PROF_DESC_FIELD_NEXT			1	// '\n'
#define	DEF_PROF_DESC_FIELD_LEN				( DEF_PROF_DESC_FIELD				\
											  + DEF_PROF_DESC_FIELD_NEXT		)
#define	DEF_PROF_LOCATION_FIELD				( DEF_TWAPI_MAX_LOCATION_LEN		\
											  * DEF_UTF8_MAX_SIZE				)
#define	DEF_PROF_LOCATION_FIELD_NEXT		1	// '\n'
#define	DEF_PROF_LOCATION_FIELD_LEN			( DEF_PROF_LOCATION_FIELD			\
											  + DEF_PROF_LOCATION_FIELD_NEXT	)
#define	DEF_PROF_URL_FIELD					DEF_TWAPI_MAX_URL_LEN
#define	DEF_PROF_URL_FIELD_NEXT				1	// '\n'
#define	DEF_PROF_URL_FIELD_LEN				( DEF_PROF_URL_FIELD				\
											  + DEF_PROF_URL_FIELD_NEXT			)
// for tweets, favorites, following, followers, listed
#define	DEF_PROF_COUNT_FIELD				DEF_TWFS_ID_FIELD
#define	DEF_PROF_COUNT_FIELD_NEXT			1	// 
#define	DEF_PROF_COUNT_FIELD_LEN			( DEF_PROF_COUNT_FIELD				\
											  + DEF_PROF_COUNT_FIELD_NEXT		)
#define	DEF_PROF_NUM_COUNT_FIELD			5
#define	DEF_PROF_ALL_COUNT_FIELD_LEN		( DEF_PROF_COUNT_FIELD_LEN			\
											  * DEF_PROF_NUM_COUNT_FIELD		)
#define	DEF_PROF_SPACE_FR_AND_FL			4
#define	DEF_PROF_SPACE_FL_AND_VERI			3
#define	DEF_PROF_CHECK_BOX					3	// [*]
#define	DEF_PROF_CHECK_BOX_PREV				1	// space befor [*]

// following [*]    followed [*]   verified [*]
#define	DEF_PROF_LAST_FIELD_LEN				( 8									\
											  + DEF_PROF_CHECK_BOX_PREV			\
											  + DEF_PROF_CHECK_BOX				\
											  + DEF_PROF_SPACE_FR_AND_FL		\
											  + 8								\
											  + DEF_PROF_CHECK_BOX_PREV			\
											  + DEF_PROF_CHECK_BOX				\
											  + DEF_PROF_SPACE_FL_AND_VERI		\
											  + 8								\
											  + DEF_PROF_CHECK_BOX_PREV			\
											  + DEF_PROF_CHECK_BOX				\
											  + 1 )	// 1 means '\n'

#define	DEF_PROF_TEXT_LEN					( DEF_PROF_SEPARATOR_FIELD_LEN		\
											  + DEF_PROF_ALL_ITEM_FIELD_LEN		\
											  + DEF_PROF_ID_FIELD_LEN			\
											  + DEF_PROF_CREATED_AT_FIELD_LEN	\
											  + DEF_PROF_NAME_FIELD_LEN			\
											  + DEF_PROF_SNAME_FIELD_LEN		\
											  + DEF_PROF_DESC_FIELD_LEN			\
											  + DEF_PROF_LOCATION_FIELD_LEN		\
											  + DEF_PROF_URL_FIELD_LEN			\
											  + 1								\
											  + DEF_PROF_ALL_COUNT_FIELD_LEN	\
											  + 1								\
											  + DEF_PROF_LAST_FIELD_LEN			)

/* ---------------------------------------------------------------------------- */
/* list description file format													*/
/* ---------------------------------------------------------------------------- */
/*
	--------------------------------------------- 44 '-'s. '\n'
	[ list id           ]:list id. maximum length is 20.'\n'
	[ list name         ]:slug. maximum length is 100.'\n'
	[ owner id          ]:id of user. maximum length is 20.'\n'
	[ owner name        ]:name of user. maximum length is 20 * 4.'\n'
	[ owner screen name ]:screen name of user. maximum length is 20.'\n'
	[ description       ]:description. maximum length is 100 * 4.'\n'
	'\n'
	[ subscribers       ]:subscribers. maximum length is 20.'\n'
	[ members           ]:members. maximum length is 20.'\n'
*/
#define	DEF_LDESC_SEPARATOR					"--------------------------------------------"
#define	DEF_LDESC_ID						"[ list id           ]:"
#define	DEF_LDESC_NAME						"[ list name         ]:"
#define	DEF_LDESC_OWN_ID					"[ owner id          ]:"
#define	DEF_LDESC_OWN_NAME					"[ owner name        ]:"
#define	DEF_LDESC_OWN_SNAME					"[ owner screen name ]:"
#define	DEF_LDESC_DESC						"[ description       ]:"

#define	DEF_LDESC_SUB						"[ subscribers       ]:"
#define	DEF_LDESC_MEM						"[ members           ]:"

/* filed length																	*/
#define	DEF_LDESC_SEPARATOR_FIELD			44
#define	DEF_LDESC_SEPARATOR_FIELD_NEXT		1		// '\n'
#define	DEF_LDESC_SEPARATOR_FIELD_LEN		( DEF_LDESC_SEPARATOR_FIELD			\
											  + DEF_LDESC_SEPARATOR_FIELD_NEXT	)
#define	DEF_LDESC_ITEM_FIELD				22
#define	DEF_LDESC_ITEM_FIELD_LEN			( DEF_LDESC_ITEM_FIELD				)
#define	DEF_LDESC_NUM_ITEM_FIELD			8
#define	DEF_LDESC_ALL_ITEM_FIELD_LEN		( DEF_LDESC_ITEM_FIELD_LEN			\
											  * DEF_LDESC_NUM_ITEM_FIELD		)
#define	DEF_LDESC_ID_FIELD					DEF_TWFS_ID_FIELD
#define	DEF_LDESC_ID_FIELD_NEXT				1	// '\n'
#define	DEF_LDESC_ID_FIELD_LEN				( DEF_LDESC_ID_FIELD				\
											  + DEF_LDESC_ID_FIELD_NEXT			)
#define	DEF_LDESC_NAME_FIELD				( DEF_REST_ACTUAL_SLUG_MAX_LENGTH	)
#define	DEF_LDESC_NAME_FIELD_NEXT			1	// '\n'
#define	DEF_LDESC_NAME_FIELD_LEN			( DEF_LDESC_NAME_FIELD				\
											  + DEF_LDESC_NAME_FIELD_NEXT		)
#define	DEF_LDESC_OWN_NAME_FIELD			( DEF_TWAPI_MAX_NAME_LEN			\
											  * DEF_UTF8_MAX_SIZE				)
#define	DEF_LDESC_OWN_NAME_FIELD_NEXT		1	// '\n'
#define	DEF_LDESC_OWN_NAME_FIELD_LEN		( DEF_LDESC_OWN_NAME_FIELD			\
											  + DEF_LDESC_OWN_NAME_FIELD_NEXT	)
#define	DEF_LDESC_OWN_ID_FIELD				DEF_TWFS_ID_FIELD
#define	DEF_LDESC_OWN_ID_FIELD_NEXT			1	// '\n'
#define	DEF_LDESC_OWN_ID_FIELD_LEN			( DEF_LDESC_OWN_ID_FIELD			\
											  + DEF_LDESC_OWN_ID_FIELD_NEXT		)
#define	DEF_LDESC_OWN_SNAME_FIELD			DEF_TWFS_SNAME_FIELD
#define	DEF_LDESC_OWN_SNAME_FIELD_NEXT		1	// '\n'
#define	DEF_LDESC_OWN_SNAME_FIELD_LEN		( DEF_LDESC_OWN_SNAME_FIELD			\
											  + DEF_LDESC_OWN_SNAME_FIELD_NEXT	)
#define	DEF_LDESC_OWN_SNAME_FIELD			DEF_TWFS_SNAME_FIELD
#define	DEF_LDESC_OWN_SNAME_FIELD_NEXT		1	// '\n'
#define	DEF_LDESC_OWN_SNAME_FIELD_LEN		( DEF_LDESC_OWN_SNAME_FIELD			\
											  + DEF_LDESC_OWN_SNAME_FIELD_NEXT	)
#define	DEF_LDESC_DESC_FIELD				DEF_TWAPI_ACTUAL_MAX_LDESC
#define	DEF_LDESC_DESC_FIELD_NEXT			1	// '\n'
#define	DEF_LDESC_DESC_FIELD_LEN			( DEF_LDESC_DESC_FIELD				\
											  + DEF_LDESC_DESC_FIELD_NEXT		)
#define	DEF_LDESC_SUB_FIELD					DEF_TWFS_ID_FIELD
#define	DEF_LDESC_SUB_FIELD_NEXT			1	// '\n'
#define	DEF_LDESC_SUB_FIELD_LEN				( DEF_LDESC_SUB_FIELD				\
											  + DEF_LDESC_SUB_FIELD_NEXT		)
#define	DEF_LDESC_MEM_FIELD					DEF_TWFS_ID_FIELD
#define	DEF_LDESC_MEM_FIELD_NEXT			1	// '\n'
#define	DEF_LDESC_MEM_FIELD_LEN				( DEF_LDESC_MEM_FIELD				\
											  + DEF_LDESC_MEM_FIELD_NEXT		)

#define	DEF_LDESC_TEXT_LEN					( DEF_LDESC_SEPARATOR_FIELD_LEN		\
											  + DEF_LDESC_ALL_ITEM_FIELD_LEN	\
											  + DEF_LDESC_ID_FIELD_LEN			\
											  + DEF_LDESC_NAME_FIELD_LEN		\
											  + DEF_LDESC_OWN_NAME_FIELD_LEN	\
											  + DEF_LDESC_OWN_ID_FIELD_LEN		\
											  + DEF_LDESC_OWN_NAME_FIELD_LEN	\
											  + DEF_LDESC_OWN_SNAME_FIELD_LEN	\
											  + DEF_LDESC_DESC_FIELD_LEN		\
											  + 1								\
											  + DEF_LDESC_SUB_FIELD_LEN			\
											  + DEF_LDESC_MEM_FIELD_LEN			)


typedef enum
{
	E_TWFS_READ_NONE,
	E_TWFS_READ_OFFSET_IN_RTW,
	E_TWFS_READ_OFFSET_IN_TEXT,
}E_TWFS_READ_STATE;

struct twfs_read
{
	off_t				twfs_offset;
	size_t				file_size;
	E_TWFS_READ_STATE	state;
};

#define	DEF_TWFS_RTW_MESSAGE			"Retweeted by "
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
--------------------------------------------------------------------------------

	defined at main.c

--------------------------------------------------------------------------------
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
int getConfigurationFilePath( const char *id, char *path );

/*
--------------------------------------------------------------------------------

	defined at twfs.c

--------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initTwitterFileSystem
	Input		:void ( *call_back )( void )
				 < callback destroy function >
	Output		:void
	Return		:int
				 < status >
	Description	:initialize twfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initTwitterFileSystem( void( *call_back )( void ) );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setTwfsDestroy
	Input		:void ( *call_back )( void )
				 < callback destroy function >
	Output		:void
	Return		:void
	Description	:register destroy function
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void setTwfsDestroy( void ( *call_back )( void ) );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:startTwfs
	Input		:int argc
				 < number of arguments >
				 char *argv[ ]
				 < arguments >
	Output		:void
	Return		:int
				 < status >
	Description	:start twfs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int startTwfs( int argc, char *argv[ ] );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getRootAbsPath
	Input		:char *r_path
				 < path to file over root directory >
				 const char *path
				 < path to file over twfs >
	Outpu		:char *r_path
				 < path to file over root directory >
	Return		:int
				 < status >
	Description	:get absolute file path under root directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void getRootAbsPath( char *r_path, const char *path );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getMountAbsPath
	Input		:char *r_path
				 < path to file over mount directory >
				 const char *path
				 < path to file over twfs >
	Outpu		:char *r_path
				 < path to file over mount directory >
	Return		:int
				 < status >
	Description	:get absolute file path under mount directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void getMountAbsPath( char *r_path, const char *path );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setRootDirPath
	Input		:char *root
				 < path to file over root directory >
	Outpu		:void
	Return		:int
				 < status >
	Description	:set root directory file path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int setRootDirPath( const char *root );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:setMountDirPath
	Input		:char *mount
				 < path to file over mount directory >
	Outpu		:void
	Return		:int
				 < status >
	Description	:set mount directory file path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int setMountDirPath( const char *mount );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getRootDirPath
	Input		:void
	Outpu		:void
	Return		:const char*
				 < root directory path >
	Description	:get root directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getRootDirPath( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getMountDirPath
	Input		:void
	Outpu		:void
	Return		:const char*
				 < mount directory path >
	Description	:get mount directory path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
const char* getMountDirPath( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getRootDirLength
	Input		:void
	Outpu		:void
	Return		:int
				 < length of root directory path name >
	Description	:get length of root directory path name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getRootDirLength( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getMountDirLength
	Input		:void
	Outpu		:void
	Return		:int
				 < length of mount directory path name >
	Description	:get length of mount directory path name
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getMountDirLength( void );

#endif	//__TWFS_H__

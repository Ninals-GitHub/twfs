/*******************************************************************************
 File:twfs_internal.h
 Description:Definitions of internal operations of twfs

*******************************************************************************/
#ifndef	__TWFS_INTERNAL_H__
#define	__TWFS_INTERNAL_H__

#include <stdbool.h>

/*
================================================================================

	Prototype Statements

================================================================================
*/
struct twfs_file;
struct twfs_read;
struct ssl_session;


/*
================================================================================

	DEFINES

================================================================================
*/
/*
--------------------------------------------------------------------------------

	Twfs Requests

--------------------------------------------------------------------------------
*/
typedef enum
{
	E_TWFS_REQ_READ_HOME_TL,
	E_TWFS_REQ_READ_HOME_TWEET,
	E_TWFS_REQ_READ_USER_TL,
	E_TWFS_REQ_READ_MENTIONS_TL,
	E_TWFS_REQ_READ_RTW_OF_ME_TL,
	E_TWFS_REQ_READ_DM,
	E_TWFS_REQ_READ_DM_SENT,
	E_TWFS_REQ_READ_FOLLOWING_LIST,
	E_TWFS_REQ_READ_FOLLOWER_LIST,
	E_TWFS_REQ_READ_FAV_LIST,
	E_TWFS_REQ_READ_AUTH_FAV_LIST,
	E_TWFS_REQ_READ_BLOCK_LIST,
	E_TWFS_REQ_READ_AUTH_BLOCK_LIST,
	E_TWFS_REQ_READ_LISTS_SUB_TL,
	E_TWFS_REQ_READ_LISTS_OWN_TL,
	E_TWFS_REQ_READ_LISTS_ADD_TL,
	E_TWFS_REQ_READ_LISTS_SUB_LIST,
	E_TWFS_REQ_READ_LISTS_OWN_LIST,
	E_TWFS_REQ_READ_LISTS_ADD_LIST,
	E_TWFS_REQ_READ_LISTS_MEMBERS,
	E_TWFS_REQ_READ_LISTS_SUBSCRIBERS,
	E_TWFS_REQ_CREATE_LISTS,
	E_TWFS_REQ_SHOW_LISTS,
} E_TWFS_REQ;


/*
--------------------------------------------------------------------------------

	Twfs File Types

--------------------------------------------------------------------------------
*/
typedef	enum
{
	/* ------------------------------------------------------------------------ */
	/* ordinary file( includes directory )										*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_FILE_REG,			/* regular file or directory					*/	// 0
	/* ------------------------------------------------------------------------ */
	/* first screen name path													*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_FILE_FIRST_SNAME,	/* [screen_name]/								*/	// 1
	/* ------------------------------------------------------------------------ */
	/* tweet, timeline															*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_FILE_TWEET,			/* [screen_name]/tweet file						*/	// 2
	E_TWFS_FILE_AUTH_TWEET,		/* [authorized]/tweet file						*/	// 3
	E_TWFS_FILE_TL,				/* [screen_name]/home_timeline					*/	// 4
	E_TWFS_FILE_AUTH_TL,		/* [authorized]/home_timeline					*/	// 5
	E_TWFS_FILE_USER_TL,		/* [screen_name]/user_timeline					*/	// 6
	E_TWFS_FILE_AUTH_USER_TL,	/* [authorized]/user_timeline					*/	// 7
	/* ------------------------------------------------------------------------ */
	/* status/[*any*]															*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_STATUS,			/* [screen_name]/status/						*/	// 8
	E_TWFS_FILE_STATUS,			/* [screen_name]/status/[*any*]					*/	// 9
	E_TWFS_FILE_AUTH_STATUS,	/* [authorized]/status/[*any*]					*/	// 10
	/* ------------------------------------------------------------------------ */
	/* retweet/[*any*]															*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_RTW,				/* [screen_name]/retweet/						*/	// 11
	E_TWFS_FILE_RTW,			/* [screen_name]/retweet/[*any*]				*/	// 12
	E_TWFS_FILE_AUTH_RTW,		/* [authorized]/retweet/[*any*]					*/	// 13
	/* ------------------------------------------------------------------------ */
	/* notifications/															*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_NOTI,			/* [screen_name]/notifications/					*/	// 14
	E_TWFS_FILE_NOTI_AT_TW,		/* [screen_name]/notifications/@tweet			*/	// 15
	E_TWFS_FILE_AUTH_NOTI_AT_TW,/* [authorized]notifications/@tweet				*/	// 16
	E_TWFS_FILE_NOTI_RTW,		/* [screen_name]/notifications/retweets_of_me	*/	// 17
	E_TWFS_FILE_AUTH_NOTI_RTW,	/* [authorized]/notifications/retweets_of_me	*/	// 18
	/* ------------------------------------------------------------------------ */
	/* account/																	*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_ACCOUNT,			/* [screen_name]/account/						*/	// 19
	E_TWFS_FILE_ACC_PROFILE,	/* [screen_name]/account/profile				*/	// 20
	E_TWFS_FILE_AUTH_ACC_PROFILE,/*[authorized]/account/profile					*/	// 21
	E_TWFS_FILE_ACC_SETTING,	/* [screen_name]/account/setting				*/	// 22
	E_TWFS_FILE_AUTH_ACC_SETTING,/*[authorized]/account/setting					*/	// 23
	/* ------------------------------------------------------------------------ */
	/* favorites/																*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_FAV,				/* [screen_name]/favorites/						*/	// 24
	E_TWFS_FILE_FAV_LIST,		/* [screen_name]/favorites/list					*/	// 25
	E_TWFS_FILE_AUTH_FAV_LIST,	/* [authorized]/favorites/list					*/	// 26
	E_TWFS_FILE_FAV_TWEET,		/* [screen_name]favorites/[*any*]				*/	// 27
	E_TWFS_FILE_AUTH_FAV_TWEET,	/* [authorized]/favorites/[*any*]				*/	// 28
	/* ------------------------------------------------------------------------ */
	/* blocks/																	*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_BLOCKS,			/* [screen_name]/blocks/						*/	// 29
	E_TWFS_FILE_AUTH_BLOCKS,	/* [authorized]/blocks/[*any*]					*/	// 30
	E_TWFS_FILE_BLOCKS_LIST,	/* [screen_name]/blocks/list					*/	// 31
	E_TWFS_FILE_DOT_BLOCKS_LIST,/* [screen_name]/blocks/.list					*/	// 32
	E_TWFS_FILE_AUTH_BLOCKS_LIST,/*[authorized]/blocks/list						*/	// 33
	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST,/*[authorized]/blocks/.list				*/	// 34

	/* ------------------------------------------------------------------------ */
	/* directmessage/															*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_DM,				/* [screen_name]/direct_message/				*/	// 35
	//E_TWFS_DIR_AUTH_DM,			/* [authorized]/direct_message/					*/
	E_TWFS_FILE_DM_MSG,			/* [screen_name]/direct_message/message			*/	// 36
	E_TWFS_FILE_AUTH_DM_MSG,	/* [authorized]/direct_message/message			*/	// 37
	E_TWFS_FILE_DM_SEND_TO,		/* [screen_name]/direct_message/send_to			*/	// 38
	E_TWFS_FILE_AUTH_DM_SEND_TO,/* [authorized]/direct_message/send_to			*/	// 39


	E_TWFS_DIR_DM_FR,			/* [screen_name]/direct_message/[friends]/		*/	// 40
	E_TWFS_FILE_DM_FR_MSG,		/* [screen_name]/direct_message/[friends]/messag*/	// 41
	E_TWFS_FILE_AUTH_DM_FR_MSG,	/* [authorized]/direct_message/[friends]/message*/	// 42
	E_TWFS_FILE_DM_FR_SEND_TO,	/* [screen_name]/direct_message/[frineds]/send_t*/	// 43
	E_TWFS_FILE_AUTH_DM_FR_SEND_TO,													// 44
	E_TWFS_FILE_DM_FR_STATUS,	/* [screen_name]/direct_message/[friends]/[any]	*/	// 45
	E_TWFS_FILE_AUTH_DM_FR_STATUS,/* [authorized]/direct_message/[friends]/[any]*/	// 46
	/* ------------------------------------------------------------------------ */
	/* follower/																*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_FL,				/* [screen_name]/follower/						*/	// 47
	//E_TWFS_DIR_AUTH_FL,			/* [authorized]/follower/						*/
	E_TWFS_FILE_FL_LIST,		/* [screen_name]/follower/list					*/	// 48
	E_TWFS_FILE_FL_DOT_LIST,	/* [screen_name]/follower/.list					*/	// 49

	E_TWFS_DIR_FL_FL,			/* [screen_name]/follower/[follower]/			*/	// 50
	E_TWFS_DIR_AUTH_FL_FL,		/* [authorized]/follower/[follower]/			*/	// 51
	/* ------------------------------------------------------------------------ */
	/* following/																*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_FR,				/* [screen_name]/following/						*/	// 52
	//E_TWFS_DIR_AUTH_FR,			/* [authorized]/following/						*/
	E_TWFS_FILE_FR_LIST,		/* [screen_name]/following/list					*/	// 53
	E_TWFS_FILE_FR_DOT_LIST,	/* [screen_name]/following/.list				*/	// 54

	E_TWFS_DIR_FR_FR,			/* [screen_name]/following/[friends]/			*/	// 55
	E_TWFS_DIR_AUTH_FR_FR,		/* [authorized/following/[friends]/				*/	// 56
	/* ------------------------------------------------------------------------ */
	/* lists/																	*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_LISTS,			/* [screen_name]/lists/							*/	// 57
	//E_TWFS_FILE_LISTS_LIST,		/* [screen_name]/lists/list						*/
	//E_TWFS_FILE_AUTH_LISTS_LIST,/* [authorized]/lists/list						*/
	/* ------------------------------------------------------------------------ */
	/* lists/subscriptions/														*/
	/* ------------------------------------------------------------------------ */
	E_TWFS_DIR_LISTS_SUB,		/* [screen_name]/lists/subscriptions/			*/	// 58
	E_TWFS_FILE_LISTS_SUB_LIST,	/* [screen_name]/lists/subscriptions/list		*/	// 59
	E_TWFS_FILE_LISTS_SUB_DOT_LIST,
						/* [screen_name]/lists/subscriptions/.list				*/	// 60
	/* lists/subscriptions/[list_name]/											*/
	E_TWFS_DIR_LISTS_SUB_LNAME,	/* [sname]/lists/subscriptions/[lname]/			*/	// 61


	/* lists/subscriptions/[list_name]/[owner]/										*/
	//E_TWFS_DIR_LISTS_SUB_LNAME_OWNER,
								/* [sname]/lists/subscriptions/[lname]/[owner]	*/


	E_TWFS_FILE_LISTS_SUB_LNAME_LDESC,
				/* [sname]/lists/subscriptions/[lname]/[owner]/list_description	*/	// 62
	E_TWFS_FILE_LISTS_SUB_LNAME_TL,
				/* [sname]/lists/subscriptions/[lname]/[owner]/timeline			*/	// 63
	
	/* lists/subscriptions/[list_name]/[owner]/members/							*/
	E_TWFS_DIR_LISTS_SUB_LNAME_MEM,
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/			*/	// 64
	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST,
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/list		*/	// 65
	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST,
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/.list	*/	// 66
	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_SNAME,
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/[sname]	*/	// 67
	E_TWFS_FILE_AUTH_LISTS_SUB_LNAME_MEM_SNAME,
				/* [auth]/lists/subscriptions/[lname]/[owner]/members/[sname]	*/	// 68

	/* lists/subscriptions/[list_name]/[owner]/subscribers/						*/
	E_TWFS_DIR_LISTS_SUB_LNAME_SUB,
				/* lists/subscriptions/[list_name]/[owner]/subscribers/			*/	// 69
	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST,
			/* [sname]/lists/subscriptions/[lname]/[owner]/subscribers/list		*/	// 70
	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST,
			/* [sname]/lists/subscriptions/[lname]/[owner]/subscribers/.list	*/	// 71
	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_SNAME,
			/* [sname]/lists/subscriptions/[lname]/[owner]/subscribers/[sname]	*/	// 72
	E_TWFS_FILE_AUTH_LISTS_SUB_LNAME_SUB_SNAME,
			/* [auth]/lists/subscriptions/[lname]/[owner]/subscribers/[sname]	*/	// 73

	/* ------------------------------------------------------------------------ */
	/* lists/my_list/															*/
	/* ------------------------------------------------------------------------ */
	/* lists/my_list/															*/
	E_TWFS_DIR_LISTS_OWN,		/* [screen_name]/lists/my_list/					*/	// 74
	E_TWFS_FILE_LISTS_OWN_LIST,	/* [screen_name]/lists/my_list/list				*/	// 75
	E_TWFS_FILE_LISTS_OWN_DOT_LIST,
					/* [screen_name]/lists/my_list/.list						*/	// 76

	/* lists/my_list/[list_name]/												*/
	E_TWFS_DIR_LISTS_OWN_LNAME,	/* [screen_name]/lists/my_list/[lname]/			*/	// 77
	E_TWFS_DIR_AUTH_LISTS_OWN_LNAME,												// 78

	/* lists/my_list/[list_name]/[owner]/										*/
	E_TWFS_DIR_LISTS_OWN_LNAME_OWNER,												// 79
	E_TWFS_DIR_AUTH_LISTS_OWN_LNAME_OWNER,											// 80

	E_TWFS_FILE_LISTS_OWN_LNAME_LDESC,
				/* [sname]/lists/my_list/[lname]/[owner]/description			*/	// 81
	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_LDESC,
				/* [authorized]/lists/my_list/[lname]/[owner]/description		*/	// 82
	E_TWFS_FILE_LISTS_OWN_LNAME_TL,
			/* [sname]/lists/my_list/[lname]/[owner]/timeline					*/	// 83
	
	/* lists/my_list/[list_name]/members/										*/	// 84
	E_TWFS_DIR_LISTS_OWN_LNAME_MEM,
	
	E_TWFS_DIR_LISTS_OWN_LNAME_MEM_OWNER,
			/* [sname]/lists/my_list/[lname]/[owner]/members/					*/	// 85
	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST,
			/* [sname]/lists/my_list/[lname]/[owner]/members/list				*/	// 86
	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST,
			/* [sname]/lists/my_list/[lname]/[owner]/members/.list				*/	// 87
	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_SNAME,
			/* [sname]/lists/my_list/[lname]/[owner]/members/[sname]			*/	// 88
	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_MEM_SNAME,
			/* [sname]/lists/my_list/[lname]/[owner]/members/[sname]			*/	// 89

	/* lists/my_list/[list_name]/[owner]/subscribers/							*/
	E_TWFS_DIR_LISTS_OWN_LNAME_SUB,
					/* [sname]/lists/my_list/[lname]/[owner]/subscribers/		*/	// 90
	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST,
			/* [sname]/lists/my_list/[lname]/[owner]/subscribers/list			*/	// 91
	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST,
			/* [sname]/lists/my_list/[lname]/[owner]/subscribers/.list			*/	// 92
	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_SNAME,
			/* [sname]/lists/my_list/[lname]/[owner]/subscribers/[sname]		*/	// 93
	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_SUB_SNAME,
			/* [sname]/lists/my_list/[lname]/[owner]/subscribers/[sname]		*/	// 94

	/* ------------------------------------------------------------------------ */
	/* lists/added/																*/
	/* ------------------------------------------------------------------------ */
	/* lists/added/																*/
	E_TWFS_DIR_LISTS_ADD,		/* [screen_name]/lists/added/					*/	// 95
	E_TWFS_FILE_LISTS_ADD_LIST,	/* [screen_name]/lists/added/list				*/	// 96
	E_TWFS_FILE_LISTS_ADD_DOT_LIST,
					/* [screen_name]/lists/added/list							*/	// 97

	/* lists/added/[list_name]/													*/
	E_TWFS_DIR_LISTS_ADD_LNAME,	/* [screen_name]/lists/added/[lname]/			*/	// 98

	/* lists/added/[list_name]/[owner]/											*/	// 99
	E_TWFS_DIR_LISTS_ADD_LNAME_OWNER,

	E_TWFS_FILE_LISTS_ADD_LNAME_LDESC,
					/* [sname]/lists/added/[lname]/[owner]/list_description		*/	// 100
	E_TWFS_FILE_LISTS_ADD_LNAME_TL,
					/* [sname]/lists/added/[lname]/[owner]/timeline				*/	// 101
	
	/* lists/added/[list_name]/[owner]/members/									*/	// 102
	E_TWFS_DIR_LISTS_ADD_LNAME_MEM,
			/* [sname]/lists/added/[lname]/[owner]/members/						*/	// 103
	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST,
			/* [sname]/lists/added/[lname]/[owner]/members/list					*/	// 104
	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST,
			/* [sname]/lists/added/[lname]/[owner]/members/.list				*/	// 105
	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_SNAME,
			/* [sname]/lists/added/[lname]/[owner]/members/[sname]				*/	// 106
	E_TWFS_FILE_AUTH_LISTS_ADD_LNAME_MEM_SNAME,
			/* [sname]/lists/added/[lname]/[owner]/members/[sname]				*/	// 107

	/* lists/added/[list_name]/[owner]/subscribers/								*/	// 108
	E_TWFS_DIR_LISTS_ADD_LNAME_SUB,
			/* [sname]/lists/added/[lname]/[owner]/subscribers/					*/	// 109
	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST,
			/* [sname]/lists/added/[lname]/[owner]/subscribers/list				*/	// 110
	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST,
			/* [sname]/lists/added/[lname]/[owner]/subscribers/.list			*/	// 111
	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_SNAME,
			/* [sname]/lists/added/[lname]/[owner]/subscribers/[sname]			*/	// 112
	E_TWFS_FILE_AUTH_LISTS_ADD_LNAME_SUB_SNAME,
			/* [sname]/lists/added/[lname]/[owner]/subscribers/[sname]			*/	// 113


}E_TWFS_FILE_TYPE;

/*
--------------------------------------------------------------------------------

	Twfs List File Type

--------------------------------------------------------------------------------
*/
/* read and write operation type for list file									*/
typedef enum
{
	E_LIST_TYPE_ERROR,
	E_LIST_TYPE_W_DOT_R_NORM,	// read tweet and write them to dot file
								// open a normal list file as read file
	E_LIST_TYPE_W_NORM_R_DOT,	// read tweet and write them to normal list
								// open a dot list file as read file
	E_LIST_TYPE_W_NORM_R_NORM,
	E_LIST_TYPE_W_DOT_R_DOT,
} E_LIST_TYPE;


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
	Function	:initTwfsInternal
	Input		:void
	Output		:void
	Return		:int
				 < status >
	Description	:initialize twfs internal
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initTwfsInternal( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyTwfsInternal
	Input		:void
	Output		:void
	Return		:void
	Description	:free all resource of twfs internal
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroyTwfsInternal( void );

/*
--------------------------------------------------------------------------------
	TWFS Utilities
--------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:allocTwfsFile
	Input		:void
	Output		:void
	Return		:void*
				 < allocate memory for twfs_file struct >
	Description	:allocate a memory to twfs_file struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void* allocTwfsFile( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:freeTwfsFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
	Output		:void
	Return		:void
	Description	:free memory of twfs file struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void freeTwfsFile( struct twfs_file *twfs_file );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:closeTwfsFile
	Input		:struct twfs_file **close_twfs_file
	Output		:struct twfs_file **close_twfs_file
				 < update to null >
	Return		:int
				 < status >
	Description	:close a twfs file information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int closeTwfsFile( struct twfs_file **close_twfs_file );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openTwfsFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 const char *path
				 < path name to open >
				 E_TWFS_FILE_TYPE file_type
				 < twfs file type >
				 const char *slug
				 < slug used for only lists operations >
	Output		:void
	Return		:int
				 < status >
	Description	:open, mmap twfs file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int openTwfsFile( struct twfs_file *twfs_file,
				  const char *path,
				  E_TWFS_FILE_TYPE file_type );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openTwfsTweetFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 const char *r_path
				 < absolute path name over root dir >
				 const char screen_name
				 < user screen name to get tweets >
				 E_TWFS_FILE_TYPE file_type
				 < analyzed file type to be opened >
				 const char* slug
				 < slug used for only lists operations >
				 const char* owner
				 < owner used for only lists operations >
	Output		:void
	Return		:int
				 < status >
	Description	:open, mmap twfs tweet file, direct message file,
				 following/follower list file, lists files
				 for reading operation
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int openTwfsTweetFile( struct twfs_file **open_twfs_file,
					   const char *r_path,
					   const char *screen_name,
					   E_TWFS_FILE_TYPE file_type,
					   const char *slug,
					   const char *owner );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:whichTwfsPath
	Input		:const char *path
				 < path over twitter file system >
				 char *first_sname
				 < first screen name >
				 char *second_sname
				 < second screen name >
				 char *third_sname
				 < third screen name >
	Output		:char *first_sname
				 < first screen name >
				 char *second_sname
				 < second screen name >
				 char *third_sname
				 < third screen name >
	Return		:E_TWFS_FILE_TYPE
				 < type of twfs special file >
	Description	:decide whether path is a special twfs file or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
E_TWFS_FILE_TYPE
whichTwfsPath( const char *path,
			   char *first_sname,
			   char *second_sname,
			   char *third_sname );


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getScreenNameFromPath
	Input		:const char *path
				 < path over twitter file system >
				 char *screen_name
				 < screen name buffer to output >
	Output		:char *screen_name
				 < first screen name >
	Return		:void
	Description	:get first screen name from path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void getScreenNameFromPath( const char *path, char *screen_name );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getDirName
	Input		:char *dir_name
				 < directory path >
				 const char *file_path
				 < file path to get directory paht >
	Output		:char *dir_name
				 < direcotry path >
	Return		:void
	Description	:get directory path from file path
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void getDirName( char *dir_name, const char *file_path );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwfsPathFromRootDir
	Input		:const char *root_path
				 < root absolute path >
				 char *twfs_path
				 < twfs absolute path from mount point >
	Outpu		:void
	Return		:int
				 < status >
	Description	:remove a directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwfsPathFromRootDir( const char *root_path, char *twfs_path );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwfsPathFromMountDir
	Input		:const char *mount_path
				 < mount absolute path >
				 char *twfs_path
				 < twfs absolute path from mount point >
	Outpu		:void
	Return		:int
				 < status >
	Description	:remove a directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwfsPathFromMountDir( const char *mount_path, char *twfs_path );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readTweet
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *screen_name
				 < screen name for user timeline >
				 const char *last
				 < last received tweet id >
				 const char *slug
				 < slug used for only lists operations >
	Output		:void
	Return		:int
				 < status >
	Description	:read home or user timeline of authenticated user
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readTweet( E_TWFS_REQ request,
			   struct ssl_session *session,
			   struct twfs_file *twfs_file,
			   const char *screen_name,
			   const char *last,
			   const char *slug );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readDM
	Input		:struct ssl_session *session
				 < ssl session >
				 struct twfs_file
				 < twfs file information >
				 const char *sent_last
				 < last received tweet id of sent dm >
				 const char *recv_last
				 < last received tweet id of received dm >
	Output		:void
	Return		:int
				 < status >
	Description	:read direct messages of authenticated user
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readDM( E_TWFS_REQ requset,
			struct ssl_session *session,
			struct twfs_file *twfs_file,
			const char *recv_last,
			const char *sent_last );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readUsers
	Input		:struct ssl_session *session
				 < ssl session >
				 struct twfs_file
				 < twfs file information >
				 const char *
				 < screen name >
				 const char *cursor
				 < cursor in the list >
				 const char *slug
				 < slug of list. used only for lists >
	Output		:void
	Return		:int
				 < status >
	Description	:read users of following/follower list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readUsers( E_TWFS_REQ request,
			   struct ssl_session *session,
			   struct twfs_file *twfs_file,
			   const char *screen_name,
			   const char *cursor,
			   const char *slug );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readLists
	Input		:E_TWFS_REQ request
				 < type of request >
				 struct ssl_session *session
				 < ssl session >
				 struct twfs_file
				 < twfs file information >
				 const char *
				 < screen name >
				 const char *cursor
				 < cursor in the list >
	Output		:void
	Return		:int
				 < status >
	Description	:read users of following/follower list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readLists( E_TWFS_REQ request,
			  struct ssl_session *session,
			  struct twfs_file *twfs_file,
			  const char *screen_name,
			  const char *cursor );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:makeLists
	Input		: E_TWFS_REQ request
				 < request of twitter api >
				 struct ssl_session *session
				 < ssl session >
				 const char *
				 < screen name >
				 const char *slug
				 < slug of list >
	Output		:void
	Return		:int
				 < status >
	Description	:clreate a list and make its description
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int makeLists( E_TWFS_REQ request,
			   struct ssl_session *session,
			   const char *screen_name,
			   const char *slug );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readProfile
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *
				 < screen name >
	Output		:void
	Return		:int
				 < status >
	Description	:read users of following/follower list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readProfile( struct ssl_session *session,
				 const char *screen_name );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:symlinkRetweet
	Input		:struct ssl_session *session
				 < ssl session >
				 const char *rtw_id
				 < tweet id to retweet >
	Output		:void
	Return		:int
				 < status >
	Description	:retweet, received its tweet object and do symlink
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int symlinkRetweet( struct ssl_session *session, const char *rtw_id );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getLatestIdFromTlFile
	Input		:const char *tl
				 < timeline file to be memory mapped by mmap >
				 size_t size
				 < file size >
				 char *id
				 < id : maximum length is DEF_TWAPI_MAX_USER_ID_LEN >
	Output		:char *id
				 < latest tweet id of timeline >
	Return		:int
				 < length of id. -1 means that record not found >
	Description	:get latest id from timeline, direct message file etc.
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getLatestIdFromTlFile( const char *tl, size_t size, char *id );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getLatestIdFromDmFile
	Input		:const char *dm
				 < direct message file to be memory mapped by mmap >
				 size_t size
				 < file size >
				 char *sent_id
				 < sent id : maximum length is DEF_TWAPI_MAX_USER_ID_LEN >
				 char *recv_id
				 < received id : maximum length is DEF_TWAPI_MAX_USER_ID_LEN >
	Output		:char *id
				 < latest tweet id of timeline >
	Return		:int
				 < length of id. -1 means that record not found.
				 's' : found latest sent id only
				 'r' : found latest received id only
				  0  : found both of id >
	Description	:get latest id from direct message file.
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getLatestIdFromDmFile( const char *dm,
						   size_t size,
						   char *sent_id,
						   char *recv_id );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTotalSizeOfTlFileFromFd
	Input		:int fd
				 < file descriptor  >
				 size_t *total_size
				 < total size of timeline file >
	Output		:size_t *total_size
				 < total size of timeline file >
	Return		:int
				 < -1 : timelin file doesn't have its file size >
	Description	:get timeline total file size from file descriptor
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTotalSizeOfTlFileFromFd( int fd, size_t *total_size );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTotalSizeOfTlFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
	Output		:struct twfs_read *twfs_read
				 < update tl_size >
	Return		:int
				 < -1 : timelin file doesn't have its file size >
	Description	:get timeline total file size from twfs file info
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTotalSizeOfTlFile( struct twfs_file *twfs_file );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getNextCursorOfListFileFromFd
	Input		:int fd
				 < file descriptor  >
				 char *next_cur
				 < next cursor >
	Output		:char *next_cur
				 < next cursor >
	Return		:int
				 < -1 : list file does not have its next cursor >
	Description	:get next cursor of list file from file descriptor
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getNextCursorOfListFileFromFd( int fd, char *next_cur );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getNextCursorListFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 char *next_cur
				 < next cursor buffer >
	Output		:struct twfs_read *twfs_read
				 < update tl_size >
				 char *next_cur
				 < result of getting next cursor >
	Return		:int
				 < -1 : timelin file doesn't have its file size >
	Description	:get next cursor of twfs list file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getNextCursorOfListFile( struct twfs_file *twfs_file, char *next_cur );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getTwfsFileOffset
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 struct twfs_read *twfs_read
				 < twfs file read information >
				 E_TWFS_FILE_TYPE file_type
				 < twfs file type >
				 off_t offset
				 < offset user reqeusted >
	Output		:struct twfs_read *twfs_read
				 < calculated offset >
	Return		:int
				 < -1 : EOF >
	Description	:calculate offset of twfs timeline file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getTwfsFileOffset( struct twfs_file *twfs_file,
					   struct twfs_read *twfs_read,
					   E_TWFS_FILE_TYPE file_type,
					   off_t offset );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readTimeLineFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 struct twfs_read *twfs_read
				 < twfs file read information >
				 E_TWFS_FILE_TYPE
				 < file type of twfs >
				 char *buf
				 < read buffer >
				 size_t size
				 < size of buffer >
				 off_t offset
				 < offset user reqeusted >
	Output		:char *buf
				 < file contents >
	Return		:int
				 < -1 : EOF, 0 < : read size >
	Description	:read a timeline file, direct message file, etc.
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
size_t readTimeLineFile( struct twfs_file *twfs_file,
						 struct twfs_read *twfs_read,
						 E_TWFS_FILE_TYPE file_type,
						 char *buf,
						 size_t size,
						 off_t offset );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:isTwitterId
	Input		:const char *id_file
				 < condidate file name of id or id string >
	Output		:void
	Return		:bool
				 < true: id >
	Description	:test whether file path name or id string is id file or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
bool isTwitterId( const char *id_file );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:makeUserListsSlugDir
	Input		:const char *screen_name
				 < screen name >
				 const char *slug
				 < slug of list >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/lists/my_list/slug directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int makeUserListsSlugDir( const char *screen_name, const char *slug );

#endif	// __TWFS_INTERNAL_H__

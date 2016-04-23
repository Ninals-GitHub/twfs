/*******************************************************************************
 File:twfs_internal.c
 Description:Operations of twfs internal

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>

#include <openssl/ssl.h>
#include <openssl/crypto.h>


#include "twfs.h"
#include "twitter_operation.h"
#include "twfs_internal.h"
#include "lib/utils.h"
#include "lib/json.h"
#include "lib/utf.h"
#include "lib/log.h"
#include "net/twitter_json.h"
#include "net/twitter_api.h"
#include "net/ssl.h"
#include "net/http.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int makeUserHomeDirectory( const char *screen_name, bool daemon );

int makeUserScreenNameDir( const char *screen_name, bool daemon );
static int makeUserDirectMessageDir( const char *screen_name,
									 const char *friends_name,
									 bool daemon );
int makeUserRetweetDir( const char *screen_name, bool daemon );
int makeUserAccountDir( const char *screen_name, bool daemon );
int makeUserNotificationsDir( const char *screen_name, bool daemon );
int makeUserBlocksDir( const char *screen_name, bool daemon );
int makeUserFavoritesDir( const char *screen_name, bool daemon );
int makeUserFollowerDir( const char *screen_name, bool daemon );
int makeUserFriendsDir( const char *screen_name, bool daemon );
int makeUserListsDir( const char *screen_name, bool daemon );
//int makeUserListsSlugDir( const char *screen_name, const char *slug );

int mmapTwfsListFile( struct twfs_file *twfs_file );

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
int initTwfsInternal( void )
{
	int		result;

	if( ( result = makeUserHomeDirectory( getTwapiScreenName( ), false ) ) < 0 )
	{
		return( result );
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroyTwfsInternal
	Input		:void
	Output		:void
	Return		:void
	Description	:free all resource of twfs internal
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroyTwfsInternal( void )
{
}

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
void* allocTwfsFile( void )
{
	struct twfs_file	*twfs_file;

	twfs_file = malloc( sizeof( struct twfs_file ) );

	if( twfs_file )
	{
		twfs_file->tl		= NULL;
		twfs_file->fd		= 0;
		twfs_file->size		= 0;
		twfs_file->tl_size	= 0;
	}

	return( ( void* )twfs_file );
}

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
void freeTwfsFile( struct twfs_file *twfs_file )
{
	free( twfs_file );
}

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
int closeTwfsFile( struct twfs_file **close_twfs_file )
{
	struct twfs_file *twfs_file;
	int		result = 0;

	twfs_file = *close_twfs_file;

	if( twfs_file )
	{
		if( twfs_file->tl && ( twfs_file != MAP_FAILED ) )
		{
			result = munmap( twfs_file->tl, twfs_file->size );
		}

		result = closeFile( twfs_file->fd );

		freeTwfsFile( twfs_file );
	}

	*close_twfs_file = NULL;

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openTwfsFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 const char *path
				 < path name to open >
				 E_TWFS_FILE_TYPE file_type
				 < twfs file type >
	Output		:void
	Return		:int
				 < status >
	Description	:open, mmap twfs file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int openTwfsFile( struct twfs_file *twfs_file,
				  const char *path,
				  E_TWFS_FILE_TYPE file_type )
{
	int				result;
	struct stat		twfs_stat;
	size_t			mmap_size;

	if( ( twfs_file->fd = openFile( path, O_CREAT | O_RDWR, 0660 ) ) < 0 )
	{
		logMessage( "cannot open %s\n", path );
		freeTwfsFile( twfs_file );
		return( -ENOMEM );
	}

	if( ( result = fstat( twfs_file->fd, &twfs_stat ) ) < 0 )
	{
		logMessage( "cannot fstat %s\n", path );
		freeTwfsFile( twfs_file );
		return( -ENOMEM );
	}

	twfs_file->size = twfs_stat.st_size;
	logMessage( "openTwfsFile:twfs_file->size[%zu]\n", twfs_file->size );
	switch( file_type )
	{
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
		if( twfs_file->size < DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN )
		{
			mmap_size = DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN
						+ ( DEF_TWFS_TL_RECORD_LEN
							* DEF_TWOPE_MAX_TWEET_COUNT );
		}
		else
		{
			mmap_size = twfs_file->size
						+ ( DEF_TWFS_TL_RECORD_LEN
							* DEF_TWOPE_MAX_TWEET_COUNT );
		}
		
		break;
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
		if( twfs_file->size < DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN )
		{
			mmap_size = DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN
						+ ( DEF_TWFS_TL_RECORD_LEN
							* DEF_TWOPE_MAX_DM_COUNT );
		}
		else
		{
			mmap_size = twfs_file->size
						+ ( DEF_TWFS_TL_RECORD_LEN
							* DEF_TWOPE_MAX_DM_COUNT );
		}
		break;
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		if( twfs_file->size < DEF_TWFS_HEAD_FF_LEN )
		{
			mmap_size = DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN
						+ ( DEF_TWFS_TL_RECORD_LEN
							* DEF_TWOPE_MAX_FF_COUNT );
		}
		else
		{
			mmap_size = twfs_file->size
						+ ( DEF_TWFS_TL_RECORD_LEN
							* DEF_TWOPE_MAX_FF_COUNT );
		}
		break;
	default:
		mmap_size = 0;
		twfs_file->size = 0;
		freeTwfsFile( twfs_file );
		return( 0 );
	}

	twfs_file->tl = ( char* )mmap( NULL, mmap_size,
								   PROT_WRITE, MAP_SHARED,
								   twfs_file->fd, 0 );

	if( twfs_file->tl == MAP_FAILED )
	{
		logMessage( "mmap failed at twfsOpen\n" );
		closeTwfsFile( &twfs_file );
		return( -ENOMEM );
	}

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openTwfsListFile
	Input		:struct twfs_file *twfs_file
				 < twfs file information >
				 const char *path
				 < path name to open >
				 const char *screen_name
				 < screen name >
				 E_TWFS_FILE_TYPE file_type
				 < twfs file type >
				 const char *slug
				 < slug of list. used for lists only >
				 const char *owner
				 < owner of list. used for lists only >
	Output		:struct twfs_file *twfs_file
				 < update file descriptor >
	Return		:E_LIST_TYPE
				 < read and write operation type for list file >
	Description	:open a follower/following list file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
E_LIST_TYPE openTwfsListFile( struct twfs_file *twfs_file,
					  const char *path,
					  const char *screen_name,
					  E_TWFS_FILE_TYPE file_type,
					  const char *slug,
					  const char *owner )
{
	int			result_path			= 0;
	int			result_res			= 0;
	char		reserved_path[ DEF_TWFS_PATH_MAX ];
	int			fd_res				= -1;
	int			fd_path				= -1;
	char		next_cur_path[ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD + 1 ];
	char		*next_cur_res;
	struct stat	stat_path;
	struct stat	stat_res;
	int			result_stat_path	= -1;
	int			result_stat_res		= -1;;

	E_LIST_TYPE	list_type;

	list_type = E_LIST_TYPE_W_DOT_R_NORM;

	fd_path = openFile( path, O_CREAT | O_RDWR, 0660 );

	switch( file_type )
	{
	default:
	case	E_TWFS_FILE_FL_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_FOLLOWERS,
				  DEF_TWFS_PATH_FF_DOT_LIST );
		break;
	case	E_TWFS_FILE_FR_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_FRIENDS,
				  DEF_TWFS_PATH_FF_DOT_LIST );
		break;
	case	E_TWFS_FILE_FL_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_FOLLOWERS,
				  DEF_TWFS_PATH_FF_LIST );
		break;
	case	E_TWFS_FILE_FR_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_FRIENDS,
				  DEF_TWFS_PATH_FF_LIST );
		break;
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_BLOCKS,
				  DEF_TWFS_PATH_DOT_BLOCK_LIST );
		break;
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_BLOCKS,
				  DEF_TWFS_PATH_BLOCK_LIST );
		break;
	case	E_TWFS_FILE_LISTS_SUB_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_SUB,
				  DEF_TWFS_PATH_SUB_DOT_LIST );
		break;
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_SUB,
				  DEF_TWFS_PATH_SUB_LIST );
		break;
	case	E_TWFS_FILE_LISTS_OWN_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  DEF_TWFS_PATH_OWN_DOT_LIST );
		break;
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  DEF_TWFS_PATH_OWN_LIST );
		break;
	case	E_TWFS_FILE_LISTS_ADD_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_ADD,
				  DEF_TWFS_PATH_ADD_DOT_LIST );
		break;
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_ADD,
				  DEF_TWFS_PATH_ADD_LIST );
		break;
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  slug,
				  owner,
				  DEF_TWFS_PATH_DIR_LNAME_MEM,
				  DEF_TWFS_PATH_LNAME_MEM_DOT_LIST );
		break;
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  slug,
				  owner,
				  DEF_TWFS_PATH_DIR_LNAME_MEM,
				  DEF_TWFS_PATH_LNAME_MEM_LIST );
		break;
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  slug,
				  owner,
				  DEF_TWFS_PATH_DIR_LNAME_SUB,
				  DEF_TWFS_PATH_LNAME_SUB_DOT_LIST );
		break;
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:
		snprintf( reserved_path, sizeof( reserved_path ),
				  "%s/%s/%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  screen_name,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  slug,
				  owner,
				  DEF_TWFS_PATH_DIR_LNAME_MEM,
				  DEF_TWFS_PATH_LNAME_SUB_LIST );
		break;
	}

	logMessage( "list norm path[%d] : %s\n",file_type, path );
	logMessage( "list resv path : %s\n", reserved_path );

	fd_res = openFile( reserved_path, O_CREAT | O_RDWR, 0660 );

	if( ( fd_path < 0 ) && ( fd_res < 0 ) )
	{
		return( E_LIST_TYPE_ERROR );
	}

	next_cur_res = reserved_path;

	if( 0 <= fd_path )
	{
		result_stat_path	= fstat( fd_path, &stat_path );
		logMessage( "cursor path<0>:[%d]%s\n", fd_path,next_cur_path );
		result_path	= getNextCursorOfListFileFromFd( fd_path, next_cur_path );
		logMessage( "cursor path<1>:[%d]%s\n", fd_path,next_cur_path );
	}
	if( 0 <= fd_res )
	{
		result_stat_res		= fstat( fd_res, &stat_res );
		logMessage( "cursor res<0>:[%d]%s\n", fd_res, next_cur_res );
		result_res	= getNextCursorOfListFileFromFd( fd_res, next_cur_res );
		logMessage( "cursor res<1>:[%d]%s\n", fd_res,next_cur_res );
	}

	if( result_stat_path < 0 )
	{
		closeFile( fd_path );
		fd_path = -1;
	}

	if( result_stat_res < 0 )
	{
		closeFile( fd_res );
		fd_res = -1;
	}

	next_cur_path[ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ]	= '\0';
	next_cur_res[ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ]		= '\0';

	logMessage( "cursor path[%d]:%s\n", result_path, next_cur_path );
	logMessage( "cursor rsvd[%d]:%s\n", result_res, next_cur_res );

	/* ------------------------------------------------------------------------ */
	/* error handling for file descriptor										*/
	/* ------------------------------------------------------------------------ */
	if( ( fd_path < 0 ) && ( 0 <= fd_res ) )
	{
		switch( file_type )
		{
		default:
		case	E_TWFS_FILE_FR_LIST:
		case	E_TWFS_FILE_FL_LIST:
			list_type		= E_LIST_TYPE_W_DOT_R_DOT;
			//closeFile( fd_path );
			break;
		case	E_TWFS_FILE_FR_DOT_LIST:
		case	E_TWFS_FILE_FL_DOT_LIST:
			list_type		= E_LIST_TYPE_W_NORM_R_NORM;
			//closeFile( fd_path );
			break;
		}

		twfs_file->fd	= fd_res;
		twfs_file->size	= stat_res.st_size;
		if( mmapTwfsListFile( twfs_file ) < 0 )
		{
			return( E_LIST_TYPE_ERROR );
		}
		return( list_type );
	}
	else if( ( 0 <= fd_path ) && ( fd_res < 0 ) )
	{
		switch( file_type )
		{
		default:
		case	E_TWFS_FILE_FR_LIST:
		case	E_TWFS_FILE_FL_LIST:
			list_type		= E_LIST_TYPE_W_NORM_R_NORM;
			//closeFile( fd_res );
			break;
		case	E_TWFS_FILE_FR_DOT_LIST:
		case	E_TWFS_FILE_FL_DOT_LIST:
			list_type		= E_LIST_TYPE_W_DOT_R_DOT;
			//closeFile( fd_res );
			break;
		}
		twfs_file->fd	= fd_path;
		twfs_file->size	= stat_path.st_size;
		if( mmapTwfsListFile( twfs_file ) < 0 )
		{
			return( E_LIST_TYPE_ERROR );
		}
		return( list_type );
	}
	else if( ( fd_path < 0 ) && ( fd_res < 0 ) )
	{
		return( E_LIST_TYPE_ERROR );
	}

	/* ------------------------------------------------------------------------ */
	/* error handling for next cursor											*/
	/* ------------------------------------------------------------------------ */
	if( ( result_path < 0 ) && ( result_res < 0 ) )
	{
		ftruncate( fd_path, 0 );
		ftruncate( fd_res, 0 );

		twfs_file->fd = fd_path;
		closeFile( fd_res );

		switch( file_type )
		{
		default:
		case	E_TWFS_FILE_FL_LIST:
		case	E_TWFS_FILE_FR_LIST:
			list_type = E_LIST_TYPE_W_NORM_R_NORM;
			break;
		case	E_TWFS_FILE_FL_DOT_LIST:
		case	E_TWFS_FILE_FR_DOT_LIST:
			list_type = E_LIST_TYPE_W_DOT_R_DOT;
			break;
		}

		//twfs_file->size	= stat_path.st_size;
		twfs_file->size	= 0;
		if( mmapTwfsListFile( twfs_file ) < 0 )
		{
			return( E_LIST_TYPE_ERROR );
		}
		logMessage( "<0>open path:%s\n", path );
		logMessage( "twfs_file->size:%zu\n", twfs_file->size );

		return( list_type );
	}
	else if( ( 0 < result_path ) && ( result_res < 0 ) )
	{
		ftruncate( fd_res, 0 );

		if( ( next_cur_path[ 0 ] == '0' ) && ( result_path == 1 ) )
		{
			switch( file_type )
			{
			default:
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FR_LIST:
				list_type = E_LIST_TYPE_W_DOT_R_NORM;
				break;
			case	E_TWFS_FILE_FL_DOT_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
				list_type = E_LIST_TYPE_W_NORM_R_DOT;
				break;
			}
			twfs_file->fd = fd_res;
			closeFile( fd_path );
			twfs_file->size	= stat_res.st_size;
			if( mmapTwfsListFile( twfs_file ) < 0 )
			{
				return( E_LIST_TYPE_ERROR );
			}

			logMessage( "<1>open reserve:%s\n", reserved_path );

			return( list_type );
		}

		switch( file_type )
		{
		default:
		case	E_TWFS_FILE_FL_LIST:
		case	E_TWFS_FILE_FR_LIST:
			list_type = E_LIST_TYPE_W_NORM_R_NORM;
			break;
		case	E_TWFS_FILE_FL_DOT_LIST:
		case	E_TWFS_FILE_FR_DOT_LIST:
			list_type = E_LIST_TYPE_W_DOT_R_DOT;
			break;
		}
		twfs_file->fd = fd_path;
		closeFile( fd_res );
		twfs_file->size	= stat_path.st_size;
		if( mmapTwfsListFile( twfs_file ) < 0 )
		{
			return( E_LIST_TYPE_ERROR );
		}

		logMessage( "<1>open path:%s\n", path );

		return( list_type );
	}
	else if( ( result_path < 0 ) && ( 0 < result_res ) )
	{
		ftruncate( fd_path, 0 );

		if( ( next_cur_res[ 0 ] == '-' ) &&
			( next_cur_res[ 1 ] == '1' ) &&
			( result_res == 2 ) )
		{
			switch( file_type )
			{
			default:
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FR_LIST:
				list_type = E_LIST_TYPE_W_NORM_R_DOT;
				break;
			case	E_TWFS_FILE_FL_DOT_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
				list_type = E_LIST_TYPE_W_DOT_R_NORM;
				break;
			}
			twfs_file->fd = fd_path;
			closeFile( fd_res );
			twfs_file->size	= stat_path.st_size;
			if( mmapTwfsListFile( twfs_file ) < 0 )
			{
				return( E_LIST_TYPE_ERROR );
			}
			logMessage( "<2>open path:%s\n", path );

			return( list_type );
		}

		switch( file_type )
		{
		default:
		case	E_TWFS_FILE_FL_LIST:
		case	E_TWFS_FILE_FR_LIST:
			list_type = E_LIST_TYPE_W_DOT_R_DOT;
			break;
		case	E_TWFS_FILE_FL_DOT_LIST:
		case	E_TWFS_FILE_FR_DOT_LIST:
			list_type = E_LIST_TYPE_W_NORM_R_NORM;
			break;
		}
		twfs_file->fd = fd_res;
		closeFile( fd_path );
		twfs_file->size	= stat_res.st_size;
		if( mmapTwfsListFile( twfs_file ) < 0 )
		{
			return( E_LIST_TYPE_ERROR );
		}
		logMessage( "<2>open reserve:%s\n", reserved_path );

		return( list_type );
	}
	/* ------------------------------------------------------------------------ */
	/* next cursor of both of path and res file has sane number					*/
	/* ------------------------------------------------------------------------ */
	else
	{
		logMessage( "next_cur_path:%s [%d]\n", next_cur_path, result_path );
		logMessage( "next_cur_res:%s [%d]\n", next_cur_res, result_res );
#if 0
		if( ( ( ( next_cur_res[ 0 ]		== '-' ) &&
				( next_cur_res[ 1 ]		== '1' ) &&
				( result_res == 2 )					) &&
			  ( ( next_cur_path[ 0 ]	== '-' ) &&
				( next_cur_path[ 1 ]	== '1' ) &&
				( result_path == 2 )				) ) ||
			( ( 1 < result_path ) && ( 1 < result_res ) ) )
#endif
		if( ( 1 < result_path ) && ( 1 < result_res ) )
		{
			if( stat_path.st_mtime <= stat_res.st_mtime )
			{
				switch( file_type )
				{
				default:
				case	E_TWFS_FILE_FL_LIST:
				case	E_TWFS_FILE_FR_LIST:
					//list_type = E_LIST_TYPE_W_NORM_R_DOT;
					list_type = E_LIST_TYPE_W_DOT_R_DOT;
					break;
				case	E_TWFS_FILE_FL_DOT_LIST:
				case	E_TWFS_FILE_FR_DOT_LIST:
					//list_type = E_LIST_TYPE_W_DOT_R_NORM;
					//list_type = E_LIST_TYPE_W_NORM_R_DOT;
					//list_type = E_LIST_TYPE_W_DOT_R_DOT;
					list_type = E_LIST_TYPE_W_NORM_R_NORM;
				}
				twfs_file->fd = fd_res;
				closeFile( fd_path );
				twfs_file->size	= stat_res.st_size;
				//twfs_file->size	= 0;
				if( mmapTwfsListFile( twfs_file ) < 0 )
				{
					return( E_LIST_TYPE_ERROR );
				}
				logMessage( "<6>open res:%s\n", reserved_path );
				logMessage( "twfs_file->size:%zu\n", twfs_file->size );

				return( list_type );
			}
			else
			{
				//ftruncate( fd_res, 0 );
				switch( file_type )
				{
				default:
				case	E_TWFS_FILE_FL_LIST:
				case	E_TWFS_FILE_FR_LIST:
					//list_type = E_LIST_TYPE_W_DOT_R_NORM;
					//list_type = E_LIST_TYPE_W_NORM_R_DOT;
					list_type = E_LIST_TYPE_W_NORM_R_NORM;
					break;
				case	E_TWFS_FILE_FL_DOT_LIST:
				case	E_TWFS_FILE_FR_DOT_LIST:
					//list_type = E_LIST_TYPE_W_NORM_R_DOT;
					//list_type = E_LIST_TYPE_W_DOT_R_NORM;
					list_type = E_LIST_TYPE_W_DOT_R_DOT;
				}
				twfs_file->fd = fd_path;
				closeFile( fd_res );
				twfs_file->size	= stat_path.st_size;
				//twfs_file->size	= 0;
				if( mmapTwfsListFile( twfs_file ) < 0 )
				{
					return( E_LIST_TYPE_ERROR );
				}
				logMessage( "<7>open path:%s\n", reserved_path );
				logMessage( "twfs_file->size:%zu\n", twfs_file->size );
				logMessage( "fd:[%d]\n", twfs_file->fd );

				return( list_type );
			}
		}
#if 0
		else if( ( next_cur_res[ 0 ] == '-' ) &&
				 ( next_cur_res[ 1 ] == '1' ) &&
				 ( result_res == 2 ) )
#endif
		else if( ( ( next_cur_res[ 0 ] == '0'	)	&&
				   ( result_res == 1 )			)	&&
				 ( 1 < result_path ) )
		{
			switch( file_type )
			{
			default:
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FR_LIST:
				list_type = E_LIST_TYPE_W_NORM_R_DOT;
				break;
			case	E_TWFS_FILE_FL_DOT_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
				list_type = E_LIST_TYPE_W_DOT_R_NORM;
				break;
			}
			twfs_file->fd = fd_path;
			closeFile( fd_res );
			twfs_file->size	= stat_path.st_size;
			if( mmapTwfsListFile( twfs_file ) < 0 )
			{
				return( E_LIST_TYPE_ERROR );
			}
			logMessage( "<8>open path:%s\n", path );
			
			return( list_type );
		}
		else if( ( ( next_cur_path[ 0 ] == '0'	)	&&
				   ( result_path == 1 )			)	&&
				 ( 1 < result_res ) )
		{
			switch( file_type )
			{
			default:
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FR_LIST:
				list_type = E_LIST_TYPE_W_DOT_R_NORM;
				break;
			case	E_TWFS_FILE_FL_DOT_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
				list_type = E_LIST_TYPE_W_NORM_R_DOT;
				break;
			}
			twfs_file->fd = fd_res;
			closeFile( fd_path );
			twfs_file->size	= stat_res.st_size;
			if( mmapTwfsListFile( twfs_file ) < 0 )
			{
				return( E_LIST_TYPE_ERROR );
			}
			logMessage( "<9>open reserved:%s\n", reserved_path );
			
			return( list_type );
		}
		else if( ( ( next_cur_res[ 0 ] == '0'	)	&&
				   ( result_res == 1 )			)	&&
				 ( ( next_cur_path[ 0 ] == '0'	)	&&
				   ( result_path == 1 )			) )
		{
			if( stat_path.st_mtime <= stat_res.st_mtime )
			{
				ftruncate( fd_path, 0 );
				switch( file_type )
				{
				default:
				case	E_TWFS_FILE_FL_LIST:
				case	E_TWFS_FILE_FR_LIST:
					//list_type = E_LIST_TYPE_W_DOT_R_NORM;
					list_type = E_LIST_TYPE_W_NORM_R_DOT;
					break;
				case	E_TWFS_FILE_FL_DOT_LIST:
				case	E_TWFS_FILE_FR_DOT_LIST:
					//list_type = E_LIST_TYPE_W_NORM_R_DOT;
					list_type = E_LIST_TYPE_W_DOT_R_NORM;
				}
				twfs_file->fd = fd_path;
				closeFile( fd_res );
				//twfs_file->size	= stat_res.st_size;
				twfs_file->size	= 1;
				if( mmapTwfsListFile( twfs_file ) < 0 )
				{
					return( E_LIST_TYPE_ERROR );
				}
				logMessage( "<11>open path:%s\n", path );
				logMessage( "twfs_file->size:%zu\n", twfs_file->size );
				logMessage( "fd:[%d]\n", twfs_file->fd );

				return( list_type );
			}
			else
			{
				ftruncate( fd_res, 0 );
				switch( file_type )
				{
				default:
				case	E_TWFS_FILE_FL_LIST:
				case	E_TWFS_FILE_FR_LIST:
					//list_type = E_LIST_TYPE_W_NORM_R_DOT;
					list_type = E_LIST_TYPE_W_DOT_R_NORM;
					break;
				case	E_TWFS_FILE_FL_DOT_LIST:
				case	E_TWFS_FILE_FR_DOT_LIST:
					//list_type = E_LIST_TYPE_W_DOT_R_NORM;
					list_type = E_LIST_TYPE_W_NORM_R_DOT;
				}
				twfs_file->fd = fd_res;
				closeFile( fd_path );
				//twfs_file->size	= stat_path.st_size;
				twfs_file->size	= 1;
				logMessage( "twfs_file->size:%zu\n", twfs_file->size );
				if( mmapTwfsListFile( twfs_file ) < 0 )
				{
					return( E_LIST_TYPE_ERROR );
				}
				logMessage( "<10>open resv:%s\n", reserved_path );
				logMessage( "twfs_file->size:%zu\n", twfs_file->size );

				return( list_type );
			}
		}
		
#if 0
		else if kokokara next_cur ga 0 and path_cur ga 0 no toki
#endif
	}
	twfs_file->fd = fd_path;
	closeFile( fd_res );
	twfs_file->size	= stat_path.st_size;
	if( mmapTwfsListFile( twfs_file ) < 0 )
	{
		return( E_LIST_TYPE_ERROR );
	}
	logMessage( "<12>open path:%s\n", path );

	return( list_type );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openTwfsTweetFile
	Input		:struct twfs_file **open_twfs_file
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
	Output		:struct twfs_file **open_twfs_file
				 < opened twfs file information >
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
					   const char *owner )
{
	int					result;
	char				last[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sent_last[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	struct twfs_file	*twfs_file;
	E_LIST_TYPE			list_type;

	if( ( twfs_file = ( struct twfs_file* )allocTwfsFile( ) ) == NULL )
	{
		logMessage( "cannot alloc\n" );
		return( -ENOMEM );
	}

	switch( file_type )
	{
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		list_type = openTwfsListFile( twfs_file,
									  r_path,
									  screen_name,
									  file_type,
									  slug,
									  owner );
		if( list_type == E_LIST_TYPE_ERROR )
		{
			return( -ENOMEM );
		}
		break;
	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
	default:
		if( ( result = openTwfsFile( twfs_file, r_path, file_type ) ) < 0 )
		{
			return( -ENOMEM );
		}
		break;
	}

	if( ( result = getTotalSizeOfTlFile( twfs_file ) ) < 0 )
	{
		logMessage( "gettotalsizeoftlfile failed:%d\n", result );
		return( -EACCES );
	}
	logMessage( "total size of timelin file : %zu\n", twfs_file->tl_size );

	/* ------------------------------------------------------------------------ */
	/* timeline file															*/
	/* ------------------------------------------------------------------------ */
	switch( file_type )
	{
	default:
		logMessage( "get timeline statuses\n" );
		if( getLatestIdFromTlFile( twfs_file->tl, twfs_file->size, last ) < 0 )
		{
			switch( file_type )
			{
			case	E_TWFS_FILE_TL:
				/* nothing to do												*/
				break;
			case	E_TWFS_FILE_USER_TL:
			case	E_TWFS_FILE_AUTH_USER_TL:
				result = readTweet( E_TWFS_REQ_READ_USER_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, NULL, NULL );
				logMessage( "readTweet result:%d\n", result );
				break;
			case	E_TWFS_FILE_AUTH_TL:
				result = readTweet( E_TWFS_REQ_READ_HOME_TL,
									getCurrentSSLSession( ),
									twfs_file,
									NULL, NULL, NULL );
				break;
			case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
				result = readTweet( E_TWFS_REQ_READ_MENTIONS_TL,
									getCurrentSSLSession( ),
									twfs_file,
									NULL, NULL, NULL );
				break;
			case	E_TWFS_FILE_AUTH_NOTI_RTW:
				result = readTweet( E_TWFS_REQ_READ_RTW_OF_ME_TL,
									getCurrentSSLSession( ),
									twfs_file,
									NULL, NULL, NULL );
				break;
			case	E_TWFS_FILE_FAV_LIST:
				result = readTweet( E_TWFS_REQ_READ_FAV_LIST,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, NULL, NULL );
				break;
			case	E_TWFS_FILE_AUTH_FAV_LIST:
				result = readTweet( E_TWFS_REQ_READ_AUTH_FAV_LIST,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, NULL, NULL );
				break;
			case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
				result = readTweet( E_TWFS_REQ_READ_LISTS_SUB_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, NULL, slug );
				break;
			case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
				result = readTweet( E_TWFS_REQ_READ_LISTS_OWN_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, NULL, slug );
				break;
			case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
				result = readTweet( E_TWFS_REQ_READ_LISTS_ADD_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, NULL, slug );
				break;
			default:
				/* nothing to do												*/
				break;
			}
		}
		else
		{
			switch( file_type )
			{
			case	E_TWFS_FILE_TL:
				/* nothing to do												*/
				break;
			case	E_TWFS_FILE_USER_TL:
			case	E_TWFS_FILE_AUTH_USER_TL:
				result = readTweet( E_TWFS_REQ_READ_USER_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, last, NULL );
				break;
			case	E_TWFS_FILE_AUTH_TL:
				result = readTweet( E_TWFS_REQ_READ_HOME_TL,
									getCurrentSSLSession( ),
									twfs_file,
									NULL, last, NULL );
				break;
			case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
				result = readTweet( E_TWFS_REQ_READ_MENTIONS_TL,
									getCurrentSSLSession( ),
									twfs_file,
									NULL, last, NULL );
				break;
			case	E_TWFS_FILE_AUTH_NOTI_RTW:
				result = readTweet( E_TWFS_REQ_READ_RTW_OF_ME_TL,
									getCurrentSSLSession( ),
									twfs_file,
									NULL, last, NULL );
				break;
			case	E_TWFS_FILE_FAV_LIST:
				result = readTweet( E_TWFS_REQ_READ_FAV_LIST,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, last, NULL );
				break;
			case	E_TWFS_FILE_AUTH_FAV_LIST:
				result = readTweet( E_TWFS_REQ_READ_AUTH_FAV_LIST,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, last, NULL );
				break;
			case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
				result = readTweet( E_TWFS_REQ_READ_LISTS_SUB_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, last, slug );
				break;
			case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
				result = readTweet( E_TWFS_REQ_READ_LISTS_OWN_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, last, slug );
				break;
			case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
				result = readTweet( E_TWFS_REQ_READ_LISTS_ADD_TL,
									getCurrentSSLSession( ),
									twfs_file,
									screen_name, last, slug );
				break;
			default:
				/* nothing to do												*/
				break;
			}
		}
		break;
	/* ------------------------------------------------------------------------ */
	/* authenticated direct message file										*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_AUTH_DM_MSG:
		result = getLatestIdFromDmFile( twfs_file->tl,
										twfs_file->size,
										sent_last, last );

		if( result < 0 )
		{
			result = readDM( E_TWFS_REQ_READ_DM,
							 getCurrentSSLSession( ),
							 twfs_file,
							 NULL, NULL );
		}
		else if( result == 's' )
		{
			result = readDM( E_TWFS_REQ_READ_DM,
							 getCurrentSSLSession( ),
							 twfs_file,
							 sent_last, NULL );
		}
		else if( result == 'r' )
		{
			result = readDM( E_TWFS_REQ_READ_DM,
							 getCurrentSSLSession( ),
							 twfs_file,
							 NULL, last );
		}
		else
		{
			result = readDM( E_TWFS_REQ_READ_DM,
							 getCurrentSSLSession( ),
							 twfs_file,
							 sent_last, last );
		}
		break;
	/* ------------------------------------------------------------------------ */
	/* authenticated direct message file from friends							*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
		/* nothing to do														*/
		result = 0;
		break;
	/* ------------------------------------------------------------------------ */
	/* follower list file														*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<0>next cursor:(null)\n" );
			result = readUsers( E_TWFS_REQ_READ_FOLLOWER_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL,
								NULL );
		}
		else
		{
			if( last[ 0 ] == '0' && last[ 1 ] == '\0' )
			{
				last[ 0 ] = '-';
				last[ 1 ] = '1';
				last[ 2 ] = '2';
			}
			logMessage( "<0>next cursor:%s\n", last );
			result = readUsers( E_TWFS_REQ_READ_FOLLOWER_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last,		// last for using cursor in the list
								NULL );
		}
		break;
	/* ------------------------------------------------------------------------ */
	/* following list file														*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readUsers( E_TWFS_REQ_READ_FOLLOWING_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL,
								NULL );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readUsers( E_TWFS_REQ_READ_FOLLOWING_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last,		// last for using cursor in the list
								NULL );
		}
		break;
	/* ------------------------------------------------------------------------ */
	/* blocks list file															*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readUsers( E_TWFS_REQ_READ_AUTH_BLOCK_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL,
								NULL );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readUsers( E_TWFS_REQ_READ_AUTH_BLOCK_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last,		// last for using cursor in the list
								NULL );
		}
		break;
	/* ------------------------------------------------------------------------ */
	/* lists list file															*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readLists( E_TWFS_REQ_READ_LISTS_SUB_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readLists( E_TWFS_REQ_READ_LISTS_SUB_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last );		// last for using cursor in the list
		}
		break;
	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readLists( E_TWFS_REQ_READ_LISTS_OWN_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readLists( E_TWFS_REQ_READ_LISTS_OWN_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last );		// last for using cursor in the list
		}
		break;
	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readLists( E_TWFS_REQ_READ_LISTS_ADD_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readLists( E_TWFS_REQ_READ_LISTS_ADD_LIST,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last );		// last for using cursor in the list
		}
		break;
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readUsers( E_TWFS_REQ_READ_LISTS_MEMBERS,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL,
								slug );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readUsers( E_TWFS_REQ_READ_LISTS_MEMBERS,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last,		// last for using cursor in the list
								slug );
		}
		break;
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		if( getNextCursorOfListFile( twfs_file, last ) <= 0 )
		{
			logMessage( "<1>next cursor:(null)\n" );
			result = readUsers( E_TWFS_REQ_READ_LISTS_SUBSCRIBERS,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								NULL,
								slug );
		}
		else
		{
			logMessage( "<1>next cursor:%s\n", last );
			result = readUsers( E_TWFS_REQ_READ_LISTS_SUBSCRIBERS,
								getCurrentSSLSession( ),
								twfs_file,
								screen_name,
								last,		// last for using cursor in the list
								slug );
		}
		break;
	}

	/* ------------------------------------------------------------------------ */
	/* following procedure is only for twfs list files							*/
	/* ------------------------------------------------------------------------ */
	char	new_list_file[ DEF_TWFS_PATH_MAX ];
	switch( file_type )
	{
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		if( list_type == E_LIST_TYPE_W_NORM_R_NORM )
		{
			if( ( file_type == E_TWFS_FILE_FL_LIST )					||
				( file_type == E_TWFS_FILE_FR_LIST )					||
				( file_type == E_TWFS_FILE_AUTH_BLOCKS_LIST )			||
				( file_type == E_TWFS_FILE_LISTS_SUB_LIST )				||
				( file_type == E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_OWN_LIST )				||
				( file_type == E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_ADD_LIST )				||
				( file_type == E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST ) )
			{
				/* already dot file is opened									*/
				break;
			}
		}
		else if( list_type == E_LIST_TYPE_W_DOT_R_DOT )
		{
			if( ( file_type == E_TWFS_FILE_FL_DOT_LIST )					||
				( file_type == E_TWFS_FILE_FR_DOT_LIST )					||
				( file_type == E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST )			||
				( file_type == E_TWFS_FILE_LISTS_SUB_DOT_LIST )				||
				( file_type == E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_OWN_DOT_LIST )				||
				( file_type == E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_ADD_DOT_LIST )				||
				( file_type == E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST )	||
				( file_type == E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST ) )
			{
				/* already dot file is opened									*/
				break;
			}
		}
		else
		{
			/* already dot file is opened										*/
			//break;
		}

		/* -------------------------------------------------------------------- */
		/* this path may be through under following conditions.					*/
		/* condition 1)															*/
		/* 	list_type = E_LIST_TYPE_W_NORM_R_DOT &&								*/
		/* 	file_type = E_TWFS_FILE_FL_DOT_LIST or FR_DOT_LIST					*/
		/* condition 2)															*/
		/* 	list_type = E_LIST_TYPE_W_DOT_R_NORM &&								*/
		/* 	file_type = E_TWFS_FILE_FL_LIST or FR_LIST							*/
		/* -------------------------------------------------------------------- */
	
		logMessage( "close written list file.[%d]\n", twfs_file->fd );
		closeTwfsFile( &twfs_file );
		logMessage( "and open read list file\n" );
		if( ( twfs_file = ( struct twfs_file* )allocTwfsFile( ) ) == NULL )
		{
			logMessage( "cannot alloc\n" );
			return( -ENOMEM );
		}
		

#if 0
		if( twfs_file->tl && ( twfs_file != MAP_FAILED ) )
		{
			munmap( twfs_file->tl, twfs_file->size );
		}

		closeFile( twfs_file->fd );
#endif

		if( ( list_type == E_LIST_TYPE_W_NORM_R_DOT ) ||
			( list_type == E_LIST_TYPE_W_DOT_R_DOT ) )
		{
			switch( file_type )
			{
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FL_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_FOLLOWERS,
						  DEF_TWFS_PATH_FF_DOT_LIST );
				file_type = E_TWFS_FILE_FL_DOT_LIST;
				break;
			case	E_TWFS_FILE_FR_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_FRIENDS,
						  DEF_TWFS_PATH_FF_DOT_LIST );
				file_type = E_TWFS_FILE_FR_DOT_LIST;
				break;
			case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
			case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_BLOCKS,
						  DEF_TWFS_PATH_DOT_BLOCK_LIST );
				file_type = E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST;
				break;
			case	E_TWFS_FILE_LISTS_SUB_LIST:
			case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  DEF_TWFS_PATH_SUB_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_SUB_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  DEF_TWFS_PATH_LNAME_MEM_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_SUB,
						  DEF_TWFS_PATH_LNAME_SUB_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_OWN_LIST:
			case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  DEF_TWFS_PATH_OWN_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_OWN_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  DEF_TWFS_PATH_LNAME_MEM_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_SUB,
						  DEF_TWFS_PATH_LNAME_SUB_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_ADD_LIST:
			case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  DEF_TWFS_PATH_ADD_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_ADD_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  DEF_TWFS_PATH_LNAME_MEM_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST;
				break;
			case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_SUB,
						  DEF_TWFS_PATH_LNAME_SUB_DOT_LIST );
				file_type = E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST;
				break;
			default:
				/* just update file information								*/
				*open_twfs_file = twfs_file;
				return( 0 );
			}
		}
		else if( ( list_type == E_LIST_TYPE_W_DOT_R_NORM ) ||
				 ( list_type == E_LIST_TYPE_W_NORM_R_NORM ) )
		{
			switch( file_type )
			{
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FL_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_FOLLOWERS,
						  DEF_TWFS_PATH_FF_LIST );
				file_type = E_TWFS_FILE_FL_LIST;
				break;
			case	E_TWFS_FILE_FR_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_FRIENDS,
						  DEF_TWFS_PATH_FF_LIST );
				file_type = E_TWFS_FILE_FR_LIST;
				break;
			case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
			case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_BLOCKS,
						  DEF_TWFS_PATH_BLOCK_LIST );
				file_type = E_TWFS_FILE_AUTH_BLOCKS_LIST;
				break;
			case	E_TWFS_FILE_LISTS_SUB_LIST:
			case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  DEF_TWFS_PATH_SUB_LIST );
				file_type = E_TWFS_FILE_LISTS_SUB_LIST;
				break;
			case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  DEF_TWFS_PATH_LNAME_MEM_LIST );
				file_type = E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST;
				break;
			case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_SUB,
						  DEF_TWFS_PATH_LNAME_SUB_LIST );
				file_type = E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST;
				break;
			case	E_TWFS_FILE_LISTS_OWN_LIST:
			case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  DEF_TWFS_PATH_OWN_LIST );
				file_type = E_TWFS_FILE_LISTS_OWN_LIST;
				break;
			case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  DEF_TWFS_PATH_LNAME_MEM_LIST );
				file_type = E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST;
				break;
			case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_SUB,
						  DEF_TWFS_PATH_LNAME_SUB_LIST );
				file_type = E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST;
				break;
			case	E_TWFS_FILE_LISTS_ADD_LIST:
			case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  DEF_TWFS_PATH_ADD_LIST );
				file_type = E_TWFS_FILE_LISTS_ADD_LIST;
				break;
			case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  DEF_TWFS_PATH_LNAME_MEM_LIST );
				file_type = E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST;
				break;
			case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
				snprintf( new_list_file, sizeof( new_list_file ),
						  "%s/%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  slug,
						  owner,
						  DEF_TWFS_PATH_DIR_LNAME_SUB,
						  DEF_TWFS_PATH_LNAME_SUB_LIST );
				file_type = E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST;
				break;
			default:
				/* just update file information								*/
				*open_twfs_file = twfs_file;
				return( 0 );
			}
		}

		if( ( result = openTwfsFile( twfs_file, new_list_file, file_type ) ) < 0 )
		{
			return( -ENOMEM );
		}
		if( ( result = getTotalSizeOfTlFile( twfs_file ) ) < 0 )
		{
			logMessage( "gettotalsizeoftlfile failed:%d\n", result );
			return( -EACCES );
		}
		logMessage( "<12>open :%s\n", new_list_file );
		break;
	default:
		break;
	}

	if( result < 0 )
	{
		logMessage( "read failed at twfsOpen\n" );
		//closeTwfsFile( &twfs_file );
		//return( -EACCES );
	}

	/* update file information													*/
	*open_twfs_file = twfs_file;
	
	return( 0 );
}

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
			   char *third_sname )
{
	int					i, j;
	int					total;
	char				path_cmp[ DEF_TWFS_PATH_MAX ];
	bool				auth_user	= false;
	const char			*user_path	= path;
	E_TWFS_FILE_TYPE	file_type	= E_TWFS_FILE_REG;

	i		= 0;
	total	= 0;

	/* ------------------------------------------------------------------------ */
	/* skip "/"																	*/
	/* ------------------------------------------------------------------------ */
	user_path++;
	total++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}

	/* ------------------------------------------------------------------------ */
	/* check whethre authorized user directory									*/
	/* ------------------------------------------------------------------------ */
	getScreenNameFromPath( path, first_sname );

	if( strncmp( first_sname, getTwapiScreenName( ), DEF_TWFS_PATH_MAX ) == 0 )
	{
		auth_user = true;
	}

	/* ------------------------------------------------------------------------ */
	/* skip "screen_name"														*/
	/* ------------------------------------------------------------------------ */
	while( *user_path != '/' )
	{
		if( ( *user_path == '\0' ) || ( DEF_TWFS_PATH_MAX <= total ) )
		{
			return( E_TWFS_FILE_FIRST_SNAME );
		}

		user_path++;
		total++;
	}

	/* skip "/"		*/
	user_path++;
	total++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}


	/* ------------------------------------------------------------------------ */
	/* check second file/directory												*/
	/* ------------------------------------------------------------------------ */
	i = 0;
	while( 1 )
	{
		if( DEF_TWFS_PATH_MAX <= total )
		{
			return( E_TWFS_FILE_REG );
		}
		/* -------------------------------------------------------------------- */
		/* check tweet, timeline files											*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '\0' )
		{
			path_cmp[ i ] = *user_path;
			if( strncmp( path_cmp, DEF_TWFS_PATH_TWEET, DEF_TWFS_PATH_MAX ) == 0 )
			{
				if( auth_user )
				{
					return( E_TWFS_FILE_AUTH_TWEET );
				}
				
				return( E_TWFS_FILE_TWEET );
			}
			if( strncmp( path_cmp, DEF_TWFS_PATH_TL, DEF_TWFS_PATH_MAX ) == 0 )
			{
				if( auth_user )
				{
					return( E_TWFS_FILE_AUTH_TL );
				}

				return( E_TWFS_FILE_TL );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_USER_TL,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				if( auth_user )
				{
					return( E_TWFS_FILE_AUTH_USER_TL );
				}

				return( E_TWFS_FILE_USER_TL );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_BLOCKS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				return( E_TWFS_DIR_BLOCKS );
			}

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_NOTI,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				return( E_TWFS_DIR_NOTI );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_FAV,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				return( E_TWFS_DIR_FAV );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_RETWEET,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				return( E_TWFS_DIR_RTW );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_DM,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				//if( auth_user )
				//{
				//	return( E_TWFS_DIR_AUTH_DM );
				//}
				return( E_TWFS_DIR_DM );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_FOLLOWERS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				//if( auth_user )
				//{
				//	return( E_TWFS_DIR_AUTH_FL );
				//}
				return( E_TWFS_DIR_FL );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_FRIENDS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				//if( auth_user )
				//{
				//	return( E_TWFS_DIR_AUTH_FR );
				//}
				return( E_TWFS_DIR_FR );
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_LISTS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				return( E_TWFS_DIR_LISTS );
			}

			return( E_TWFS_FILE_REG );
		}
		/* -------------------------------------------------------------------- */
		/* check follower, following, direct_messages, lists directories		*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '/' )
		{
			path_cmp[ i ] = '\0';
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_STATUS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_STATUS;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_DM,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_DM;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_FOLLOWERS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_FL;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_FRIENDS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_FR;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_LISTS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_LISTS;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_NOTI,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_NOTI;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_RETWEET,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_RTW;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_ACCOUNT,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_ACCOUNT;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_FAV,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_FAV;
				break;
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_BLOCKS,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				file_type = E_TWFS_DIR_BLOCKS;
				break;
			}
		}
		path_cmp[ i ] = *user_path;
		i++;
		total++;
		user_path++;
	}

	/* skip "/"		*/
	user_path++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}

	/* ------------------------------------------------------------------------ */
	/* check third path															*/
	/* ------------------------------------------------------------------------ */
	i = 0;
	while( 1 )
	{
		if( DEF_TWFS_PATH_MAX <= total )
		{
			return( E_TWFS_FILE_REG );
		}

		/* -------------------------------------------------------------------- */
		/* check status, lists, direct_message/send, recv files					*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '\0' )
		{
			path_cmp[ i ] = '\0';
			/* copy second screen name or file	*/
			for( j = 0 ; j < i ; j++ )
			{
				if( DEF_TWAPI_MAX_SCREEN_NAME_LEN <= j )
				{
					j--;
					break;
				}
				second_sname[ j ] = path_cmp[ j ];
			}
			second_sname[ j ] = '\0';

			if( file_type == E_TWFS_DIR_STATUS )
			{
				if( isTwitterId( path_cmp ) )
				{
					if( auth_user )
					{
						/* [authorized]/status/[any]							*/
						return( E_TWFS_FILE_AUTH_STATUS );
					}
					/* [screen_name]/status/[any]								*/
					return( E_TWFS_FILE_STATUS );
				}

				return( E_TWFS_FILE_REG );
			}
			/* [screen_name]/lists/[any]										*/
			if( file_type == E_TWFS_DIR_LISTS )
			{
#if 0
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/lists/list								*/
						return( E_TWFS_FILE_AUTH_LISTS_LIST );
					}

					return( E_TWFS_FILE_LISTS_LIST );
				}
#endif
				/* [screen_name]/lists/[any]									*/
				return( E_TWFS_FILE_REG );
			}

			if( file_type == E_TWFS_DIR_DM )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DM_MSG,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/direct_message/message					*/
						return( E_TWFS_FILE_AUTH_DM_MSG );
					}

					/* [screen_name]/direct_message/message						*/
					return( E_TWFS_FILE_DM_MSG );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DM_SEND_TO,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/direct_message/send_to					*/
						return( E_TWFS_FILE_AUTH_DM_SEND_TO );
					}

					/* [screen_name]/direct_message/send_to						*/
					return( E_TWFS_FILE_DM_SEND_TO );
				}
				return( E_TWFS_FILE_REG );
			}

			if( file_type == E_TWFS_DIR_FL )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_FF_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					/* [screen_name]/follower/list								*/
					/* [authorized]/follower/list								*/
					return( E_TWFS_FILE_FL_LIST );
				}

				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_FF_DOT_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FL_DOT_LIST );
				}
				/* [authorized]/follower/[follower]/							*/
				if( auth_user )
				{
					return( E_TWFS_DIR_AUTH_FL_FL );
				}
				/* [screen_name]/follower/[follower]/							*/
				return( E_TWFS_DIR_FL_FL );
			}
			if( file_type == E_TWFS_DIR_FR )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_FF_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					/* [screen_name]/following/list								*/
					/* [authorized]/following/list								*/
					return( E_TWFS_FILE_FR_LIST );
				}
				
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_FF_DOT_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FR_DOT_LIST );
				}
				/* [authorized]/following/[follower]/							*/
				if( auth_user )
				{
					return( E_TWFS_DIR_AUTH_FR_FR );
				}
				/* [screen_name]/following/[follower]/							*/
				return( E_TWFS_DIR_FR_FR );
			}

			if( file_type == E_TWFS_DIR_NOTI )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_AT_TW,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/notifications/@tweet					*/
						return( E_TWFS_FILE_AUTH_NOTI_AT_TW );
					}

					/* [screen_name]/notifications/@tweet						*/
					return( E_TWFS_FILE_NOTI_AT_TW );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_RTW,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/notifications/retweet_of_me				*/
						return( E_TWFS_FILE_AUTH_NOTI_RTW );
					}

					/* [screen_name]/notifications/retweet_of_me				*/
					return( E_TWFS_FILE_NOTI_RTW );
				}
				return( E_TWFS_FILE_REG );
			}

			if( file_type == E_TWFS_DIR_RTW )
			{
				if( isTwitterId( path_cmp ) )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_RTW );
					}

					return( E_TWFS_FILE_RTW );
				}
				return( E_TWFS_FILE_REG );
			}

			if( file_type == E_TWFS_DIR_ACCOUNT )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_PROFILE,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/account/profile							*/
						return( E_TWFS_FILE_AUTH_ACC_PROFILE );
					}

					/* [screen_name]/account/profile							*/
					return( E_TWFS_FILE_ACC_PROFILE );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_SETTINGS,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/account/settings						*/
						return( E_TWFS_FILE_AUTH_ACC_SETTING );
					}

					/* [screen_name]/account/settings							*/
					return( E_TWFS_FILE_ACC_SETTING );
				}
				return( E_TWFS_FILE_REG );
			}

			if( file_type == E_TWFS_DIR_FAV )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_FAV_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/favorites/list							*/
						return( E_TWFS_FILE_AUTH_FAV_LIST );
					}

					/* [screen_name]/favorites/list								*/
					return( E_TWFS_FILE_FAV_LIST );
				}

				if( isTwitterId( path_cmp ) )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_FAV_TWEET );
					}

					return( E_TWFS_FILE_FAV_TWEET );
				}

				return( E_TWFS_FILE_REG );
			}
			if( file_type == E_TWFS_DIR_BLOCKS )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_BLOCK_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/blocks/list								*/
						return( E_TWFS_FILE_AUTH_BLOCKS_LIST );
					}

					/* [screen_name]/blocks/list								*/
					return( E_TWFS_FILE_BLOCKS_LIST );
				}

				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DOT_BLOCK_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						/* [authorized]/blocks/.list							*/
						return( E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST );
					}

					/* [screen_name]/blocks/.list								*/
					return( E_TWFS_FILE_DOT_BLOCKS_LIST );
				}

				if( auth_user )
				{
					return( E_TWFS_FILE_AUTH_BLOCKS );
				}
			}
			return( E_TWFS_FILE_REG );
		}
		/* -------------------------------------------------------------------- */
		/* check screen_name directory											*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '/' )
		{
			path_cmp[ i ] = '\0';
			switch( file_type )
			{
			/* skip screen_name folder		*/
			case	E_TWFS_DIR_DM:
				file_type = E_TWFS_DIR_DM_FR;
				break;
			case	E_TWFS_DIR_FL:
				file_type = E_TWFS_DIR_FL_FL;
				break;
			case	E_TWFS_DIR_FR:
				file_type = E_TWFS_DIR_FR_FR;
				break;
			case	E_TWFS_DIR_LISTS:
				/* [screen_name]/lists/subscriptions/							*/
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DIR_SUB,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					file_type = E_TWFS_DIR_LISTS_SUB;
				}
				/* [screen_name]/lists/my_list/									*/
				else if( strncmp( path_cmp,
								  DEF_TWFS_PATH_DIR_OWN,
								  DEF_TWFS_PATH_MAX ) == 0 )
				{
					file_type = E_TWFS_DIR_LISTS_OWN;
				}
				else if( strncmp( path_cmp,
								  DEF_TWFS_PATH_DIR_ADD,
								  DEF_TWFS_PATH_MAX ) == 0 )
				{
					file_type = E_TWFS_DIR_LISTS_ADD;
				}
				else
				{
					return( E_TWFS_FILE_REG );
				}

				break;
			default:
				return( E_TWFS_FILE_REG );
			}
			/* copy second screen name	*/
			for( j = 0 ; j < i ; j++ )
			{
				if( DEF_TWAPI_MAX_SCREEN_NAME_LEN <= j )
				{
					j--;
					break;
				}
				second_sname[ j ] = path_cmp[ j ];
			}
			second_sname[ j ] = '\0';
			break;
		}

		path_cmp[ i ] = *user_path;
		i++;
		total++;
		user_path++;
	}

	/* skip "/"		*/
	user_path++;
	total++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}

	/* ------------------------------------------------------------------------ */
	/* check forth path															*/
	/* ------------------------------------------------------------------------ */
	i = 0;
	while( 1 )
	{
		if( DEF_TWFS_PATH_MAX <= total )
		{
			return( E_TWFS_FILE_REG );
		}

		/* -------------------------------------------------------------------- */
		/* check direct_message/{send, recv}, {follower, friends}/{tweet, tl}	*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '\0' )
		{
			path_cmp[ i ] = '\0';
			if( file_type == E_TWFS_DIR_DM_FR )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DM_MSG,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_DM_FR_MSG );
					}

					return( E_TWFS_FILE_DM_FR_MSG );
				}

				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DM_SEND_TO,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_DM_FR_SEND_TO );
					}

					return( E_TWFS_FILE_DM_FR_SEND_TO );
				}

				if( isTwitterId( path_cmp ) )
				{
					for( int len = 0 ;
						 len < DEF_TWAPI_MAX_SCREEN_NAME_LEN ;
						 len++ )
					{
						second_sname[ len ] = path_cmp[ len ];
						if( path_cmp[ len ] == '\0' )
						{
							break;
						}
					}
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_DM_FR_STATUS );
					}

					return( E_TWFS_FILE_DM_FR_STATUS );
				}
				
				//return( E_TWFS_DIR_DM_FR );
			}
			if( file_type == E_TWFS_DIR_FL_FL )
			{
#if 0
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_TWEET,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FL_TWEET );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_TL,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FL_TL );
				}
#endif
				//return( E_TWFS_DIR_FL_FL );
			}
			if( file_type == E_TWFS_DIR_FR_FR )
			{
#if 0
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_TWEET,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FR_TWEET );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_TL,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FR_TL );
				}
#endif
				//return( E_TWFS_DIR_FR_FR );
			}

			if( file_type == E_TWFS_DIR_FL_FL )
			{
#if 0
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_TWEET,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FL_TWEET );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_TL,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					return( E_TWFS_FILE_FL_TL );
				}
#endif
				//return( E_TWFS_DIR_FL_FL );
			}

#if 0
			if( file_type == E_TWFS_DIR_LISTS_SUB )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_SUB_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_SUB_LIST );
					}
					return( E_TWFS_FILE_LISTS_SUB_LIST );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_SUB_SUBSCRIBERS,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_SUB_SUB );
					}
					return( E_TWFS_FILE_LISTS_SUB_SUB );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_SUB_MEMBERSHIPS,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_SUB_MEM );
					}
					return( E_TWFS_FILE_LISTS_SUB_MEM );
				}
			}

			if( file_type == E_TWFS_DIR_LISTS_OWN )
			{
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_OWN_LIST,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_OWN_LIST );
					}
					return( E_TWFS_FILE_LISTS_OWN_LIST );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_OWN_SUBSCRIBERS,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_OWN_SUB );
					}
					return( E_TWFS_FILE_LISTS_OWN_SUB );
				}
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_OWN_MEMBERSHIPS,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_OWN_MEM );
					}
					return( E_TWFS_FILE_LISTS_OWN_MEM );
				}
			}
#endif
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_SUB_LIST,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [screen_name]/lists/subscriptions/list						*/
				if( file_type == E_TWFS_DIR_LISTS_SUB )
				{
					return( E_TWFS_FILE_LISTS_SUB_LIST );
				}
				/* [screen_name]/lists/my_list/list								*/
				if( file_type == E_TWFS_DIR_LISTS_OWN )
				{
					return( E_TWFS_FILE_LISTS_OWN_LIST );
				}
				/* [screen_name]/lists/added/list								*/
				if( file_type == E_TWFS_DIR_LISTS_ADD )
				{
					return( E_TWFS_FILE_LISTS_ADD_LIST );
				}
			}

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_SUB_DOT_LIST,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [screen_name]/lists/subscriptions/.list						*/
				if( file_type == E_TWFS_DIR_LISTS_SUB )
				{
					return( E_TWFS_FILE_LISTS_SUB_DOT_LIST );
				}
				/* [screen_name]/lists/my_list/.list							*/
				if( file_type == E_TWFS_DIR_LISTS_OWN )
				{
					return( E_TWFS_FILE_LISTS_OWN_DOT_LIST );
				}
				/* [screen_name]/lists/added/.list								*/
				if( file_type == E_TWFS_DIR_LISTS_ADD )
				{
					return( E_TWFS_FILE_LISTS_ADD_DOT_LIST );
				}
			}

			if( ( file_type == E_TWFS_DIR_LISTS_SUB ) ||
				( file_type == E_TWFS_DIR_LISTS_OWN ) ||
				( file_type == E_TWFS_DIR_LISTS_ADD ) )
			{
				//for( j = 0 ; j < DEF_TWAPI_MAX_SCREEN_NAME_LEN ; j++ )
				for( j = 0 ; j < DEF_REST_ACTUAL_SLUG_MAX_LENGTH ; j++ )
				{
					second_sname[ j ] = path_cmp[ j ];
					if( path_cmp[ j ] == '\0' )
					{
						break;
					}
				}

#if 0
				for( int len = j ; 0 < len ; len++ )
				{
					if( second_sname[ len ] == '@' )
					{
						second_sname[ len ] = '\0';
						break;
					}
				}
#endif

				if( file_type == E_TWFS_DIR_LISTS_SUB )
				{
					return( E_TWFS_DIR_LISTS_SUB_LNAME );
				}

				if( file_type == E_TWFS_DIR_LISTS_OWN )
				{
					if( auth_user )
					{
						return( E_TWFS_DIR_AUTH_LISTS_OWN_LNAME );
					}
					return( E_TWFS_DIR_LISTS_OWN_LNAME );
				}

				if( file_type == E_TWFS_DIR_LISTS_ADD )
				{
					return( E_TWFS_DIR_LISTS_ADD_LNAME );
				}
			}

			return( E_TWFS_FILE_REG );
		}

		/* -------------------------------------------------------------------- */
		/* check {follower, friends}/direct_message								*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '/' )
		{
			path_cmp[ i ] = '\0';
			if( file_type == E_TWFS_DIR_FL_FL )
			{
#if 0
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DIR_DM,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					file_type = E_TWFS_DIR_FL_FL_DM;
					break;
				}
#endif
				return( file_type );
			}
			if( file_type == E_TWFS_DIR_FR_FR )
			{
#if 0
				if( strncmp( path_cmp,
							 DEF_TWFS_PATH_DIR_DM,
							 DEF_TWFS_PATH_MAX ) == 0 )
				{
					file_type = E_TWFS_DIR_FR_FR_DM;
					break;
				}
#endif
				return( file_type );
			}

			if( ( file_type == E_TWFS_DIR_LISTS_SUB ) ||
				( file_type == E_TWFS_DIR_LISTS_OWN ) ||
				( file_type == E_TWFS_DIR_LISTS_ADD ) )
			{
				/* copy list slug name to second screen_name					*/
				for( j = 0 ; j < i ; j++ )
				{
					//if( DEF_TWAPI_MAX_SCREEN_NAME_LEN <= j )
					if( DEF_REST_ACTUAL_SLUG_MAX_LENGTH <= j )
					{
						j--;
						break;
					}
					second_sname[ j ] = path_cmp[ j ];
				}

				second_sname[ j ] = '\0';

#if 0
				for( int len = j ; 0 < len ; len++ )
				{
					if( second_sname[ len ] == '@' )
					{
						second_sname[ len ] = '\0';
						break;
					}
				}
#endif

				switch( file_type )
				{
				case	E_TWFS_DIR_LISTS_SUB:
					file_type = E_TWFS_DIR_LISTS_SUB_LNAME;
					break;
				case	E_TWFS_DIR_LISTS_OWN:
					file_type = E_TWFS_DIR_LISTS_OWN_LNAME;
					break;
				case	E_TWFS_DIR_LISTS_ADD:
					file_type = E_TWFS_DIR_LISTS_ADD_LNAME;
				default:
					break;
				}

				/* go to fifth path name										*/
				break;
			}

			return( E_TWFS_FILE_REG );
		}

		path_cmp[ i ] = *user_path;
		i++;
		total++;
		user_path++;
	}

	if( ( file_type != E_TWFS_DIR_LISTS_SUB_LNAME ) &&
		( file_type != E_TWFS_DIR_LISTS_OWN_LNAME ) &&
		( file_type != E_TWFS_DIR_LISTS_ADD_LNAME ) )
	{
		return( E_TWFS_FILE_REG );
	}

#if 0
	/* skip "/"		*/
	user_path++;
	total++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}

	/* ------------------------------------------------------------------------ */
	/* check fifth path															*/
	/* ------------------------------------------------------------------------ */
	i = 0;
	while( 1 )
	{
		if( DEF_TWFS_PATH_MAX <= total )
		{
			return( E_TWFS_FILE_REG );
		}

		/* -------------------------------------------------------------------- */
		/* check lists/{my_list, subs, added}/[lname]/[any]						*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '\0' )
		{
			path_cmp[ i ] = '\0';

			return( E_TWFS_FILE_REG );
		}
		/* -------------------------------------------------------------------- */
		/* check [sname]/lists/{subs, my_list, added}/[lname]/[owner]			*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '/' )
		{
			path_cmp[ i ] = '\0';

#if 0
			/* copy list owner to first screen_name								*/
			for( j = 0 ; j < i ; j++ )
			{
				if( DEF_TWAPI_MAX_SCREEN_NAME_LEN <= j )
				{
					j--;
					break;
				}
				third_sname[ j ] = path[ j ];
			}
			third_sname[ j ] = '\0';
#endif

			/* [sname]/lists/subscriptions/[lname]/[owner]						*/
			if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
			{
				file_type = E_TWFS_DIR_LISTS_SUB_LNAME_OWNER;
				break;
			}
			/* [sname]/lists/my_list/[lname]/[owner]							*/
			else if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
			{
				file_type = E_TWFS_DIR_LISTS_OWN_LNAME_OWNER;
				break;
			}
			/* [sname]/lists/added/[lname]/[owner]								*/
			else if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
			{
				file_type = E_TWFS_DIR_LISTS_ADD_LNAME_OWNER;
				break;
			}

			return( E_TWFS_FILE_REG );
		}

		path_cmp[ i ] = *user_path;
		i++;
		total++;
		user_path++;
	}
#endif

#if 1
	/* skip "/"		*/
	user_path++;
	total++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}

	/* ------------------------------------------------------------------------ */
	/* check fifth path															*/
	/* ------------------------------------------------------------------------ */
	i = 0;
	while( 1 )
	{
		if( DEF_TWFS_PATH_MAX <= total )
		{
			return( E_TWFS_FILE_REG );
		}

		/* -------------------------------------------------------------------- */
		/* check lists/{subs, my_list, added}/[lname]/[owner]/					*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '\0' )
		{
			path_cmp[ i ] = '\0';

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_LNAME_LDESC,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/description				*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
				{
					return( E_TWFS_FILE_LISTS_SUB_LNAME_LDESC );
				}
				/* [sname]/lists/my_list/[lname]/description					*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
				{
					if( auth_user )
					{
						return( E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_LDESC );
					}

					return( E_TWFS_FILE_LISTS_OWN_LNAME_LDESC );
				}
				/* [sname]/lists/added/[lname]/description						*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
				{
					return( E_TWFS_FILE_LISTS_ADD_LNAME_LDESC );
				}

				return( E_TWFS_FILE_REG );
			}

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_LNAME_TL,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/timeline					*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
				{
					return( E_TWFS_FILE_LISTS_SUB_LNAME_TL );
				}
				/* [sname]/lists/my_list/[lname]/timeline						*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
				{
					return( E_TWFS_FILE_LISTS_OWN_LNAME_TL );
				}
				/* [sname]/lists/added/[lname]/timeline							*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
				{
					return( E_TWFS_FILE_LISTS_ADD_LNAME_TL );
				}
			}

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_LNAME_SUB,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/subscribers				*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
				{
					return( E_TWFS_DIR_LISTS_SUB_LNAME_SUB );
				}
				/* [sname]/lists/my_list/[lname]/subscribers					*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
				{
					return( E_TWFS_DIR_LISTS_OWN_LNAME_SUB );
				}
				/* [sname]/lists/added/[lname]/subscribers						*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
				{
					return( E_TWFS_DIR_LISTS_ADD_LNAME_SUB );
				}
			}

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_LNAME_MEM,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/members					*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
				{
					return( E_TWFS_DIR_LISTS_SUB_LNAME_MEM );
				}
				/* [sname]/lists/my_list/[lname]/members						*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
				{
					return( E_TWFS_DIR_LISTS_OWN_LNAME_MEM );
				}
				/* [sname]/lists/added/[lname]/members							*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
				{
					return( E_TWFS_DIR_LISTS_ADD_LNAME_MEM );
				}
			}

			return( E_TWFS_FILE_REG );
		}
		/* -------------------------------------------------------------------- */
		/* check [sname]/lists/{subs, my_list, added}/[lname]					*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '/' )
		{
			path_cmp[ i ] = '\0';
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_LNAME_SUB,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/subscribers				*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
				{
					file_type = E_TWFS_DIR_LISTS_SUB_LNAME_SUB;
					break;
				}
				/* [sname]/lists/my_list/[lname]/subscribers					*/
				else if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
				{
					file_type = E_TWFS_DIR_LISTS_OWN_LNAME_SUB;
					break;
				}
				/* [sname]/lists/added/[lname]/subscribers						*/
				else if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
				{
					file_type = E_TWFS_DIR_LISTS_ADD_LNAME_SUB;
					break;
				}
			}
			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_DIR_LNAME_MEM,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/members					*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME )
				{
					file_type = E_TWFS_DIR_LISTS_SUB_LNAME_MEM;
					break;
				}
				/* [sname]/lists/my_list/[lname]/members						*/
				else if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
				{
					file_type = E_TWFS_DIR_LISTS_OWN_LNAME_MEM;
					break;
				}
				/* [sname]/lists/added/[lname]/members							*/
				else if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME )
				{
					file_type = E_TWFS_DIR_LISTS_ADD_LNAME_MEM;
					break;
				}
			}

			return( E_TWFS_FILE_REG );
		}

		path_cmp[ i ] = *user_path;
		i++;
		total++;
		user_path++;
	}

	/* skip "/"		*/
	user_path++;

	if( *user_path == '\0' )
	{
		return( E_TWFS_FILE_REG );
	}

	/* ------------------------------------------------------------------------ */
	/* check sixth path															*/
	/* ------------------------------------------------------------------------ */
	i = 0;
	while( 1 )
	{
		if( DEF_TWFS_PATH_MAX <= total )
		{
			return( E_TWFS_FILE_REG );
		}

		/* -------------------------------------------------------------------- */
		/* check lists/{subs, my_list, added}/[lname]/[owner]/{subs,mem}		*/
		/* -------------------------------------------------------------------- */
		if( *user_path == '\0' )
		{
			path_cmp[ i ] = '\0';

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_LNAME_MEM_LIST,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/[owner]/
													subscribers/list			*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME_SUB )
				{
					return( E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST );
				}
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/list		*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME_MEM )
				{
					return( E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST );
				}

				/* [sname]/lists/my_list/[lname]/[owner]/subscribers/list		*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME_SUB )
				{
					return( E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST );
				}
				/* [sname]/lists/my_list/[lname]/[owner]/members/list			*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME_MEM )
				{
					return( E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST );
				}

				/* [sname]/lists/added/[lname]/[owner]/subscribers/list			*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME_SUB )
				{
					return( E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST );
				}
				/* [sname]/lists/my_list/[lname]/[owner]/members/list			*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME_MEM )
				{
					return( E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST );
				}
			}

			if( strncmp( path_cmp,
						 DEF_TWFS_PATH_LNAME_MEM_DOT_LIST,
						 DEF_TWFS_PATH_MAX ) == 0 )
			{
				/* [sname]/lists/subscriptions/[lname]/[owner]/
														subscribers/.list		*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME_SUB )
				{
					return( E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST );
				}
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/.list	*/
				if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME_MEM )
				{
					return( E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST );
				}

				/* [sname]/lists/my_list/[lname]/[owner]/subscribers/.list		*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME_SUB )
				{
					return( E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST );
				}
				/* [sname]/lists/my_list/[lname]/[owner]/members/.list			*/
				if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME_MEM )
				{
					return( E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST );
				}

				/* [sname]/lists/added/[lname]/[owner]/subscribers/.list		*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME_SUB )
				{
					return( E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST );
				}
				/* [sname]/lists/my_list/[lname]/[owner]/members/.list			*/
				if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME_MEM )
				{
					return( E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST );
				}
			}

			/* copy members/subscribers screen name to third screen name		*/
			for( int len = 0 ;
				 len < DEF_TWAPI_MAX_SCREEN_NAME_LEN ;
				 len++ )
			{
				third_sname[ len ] = path_cmp[ len ];
				if( path_cmp[ len ] == '\0' )
				{
					break;
				}
			}

			if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME_SUB )
			{
				/* [sname]/lists/subscriptions/[lname]/[owner]
													/subscribers/[sname]		*/
				return( E_TWFS_FILE_LISTS_SUB_LNAME_SUB_SNAME );
			}
			if( file_type == E_TWFS_DIR_LISTS_SUB_LNAME_MEM )
			{
				/* [sname]/lists/subscriptions/[lname]/[owner]/members/[sname]	*/
				return( E_TWFS_FILE_LISTS_SUB_LNAME_MEM_SNAME );
			}

			if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME_SUB )
			{
				/* [sname]/lists/my_list/[lname]/[owner]/subscribers/[sname]	*/
				return( E_TWFS_FILE_LISTS_OWN_LNAME_SUB_SNAME );
			}
			if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME_MEM )
			{
				/* [sname]/lists/my_list/[lname]/[owner]/members/[sname]		*/
				if( auth_user )
				{
					return( E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_MEM_SNAME );
				}
				return( E_TWFS_FILE_LISTS_OWN_LNAME_MEM_SNAME );
			}
			
			if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME_SUB )
			{
				/* [sname]/lists/added/[lname]/[owner]/subscribers/[sname]		*/
				return( E_TWFS_FILE_LISTS_ADD_LNAME_SUB_SNAME );
			}
			if( file_type == E_TWFS_DIR_LISTS_ADD_LNAME_MEM )
			{
				/* [sname]/lists/added/[lname]/[owner]/members/[sname]			*/
				return( E_TWFS_FILE_LISTS_ADD_LNAME_MEM_SNAME );
			}

			return( E_TWFS_FILE_REG );
		}

		if( *user_path == '/' )
		{
			path_cmp[ i ] = '\0';
			/* copy members/subscribers screen name to third screen name		*/
			for( int len = 0 ;
				 len < DEF_TWAPI_MAX_SCREEN_NAME_LEN ;
				 len++ )
			{
				third_sname[ len ] = path_cmp[ len ];
				if( path_cmp[ len ] == '\0' )
				{
					break;
				}
			}
			return( E_TWFS_FILE_REG );
		}

		path_cmp[ i ] = *user_path;
		i++;
		total++;
		user_path++;
	}

	return( E_TWFS_FILE_REG );
#endif
}

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
void getScreenNameFromPath( const char *path, char *screen_name )
{
	int			i;
	const char	*user_path = path;

	i = 0;
	
	/* skip root	*/
	user_path++;

	while( *user_path != '/' )
	{
		if( DEF_TWAPI_MAX_SCREEN_NAME_LEN <= i )
		{
			screen_name[ i - 1 ] = '\0';
			return;
		}
		if( *user_path == '\0' )
		{
			screen_name[ i ] = '\0';
			return;
		}

		screen_name[ i ] = *user_path;
		i++;
		user_path++;
	}

	screen_name[ i ] = '\0';
}

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
void getDirName( char *dir_name, const char *file_path )
{
	int		file_path_len;
	int		i;
	char	*p;

	file_path_len = strnlen( file_path, DEF_TWFS_PATH_MAX );

	if( ( file_path[ 0 ] == '/' ) && file_path_len == 1 )
	{
		dir_name[ 0 ] = file_path[ 0 ];
		dir_name[ 1 ] = '\0';
	}

	for( i = 0 ; i < file_path_len ; i++ )
	{
		dir_name[ i ] = file_path[ i ];
	}

	dir_name[ i ] = '\0';
	p = &dir_name[ i ];

	while( p-- )
	{
		if( *p == '/' )
		{
			break;
		}
	}

	*( p + 1 ) = '\0';
}

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
int getTwfsPathFromRootDir( const char *root_path, char *twfs_path )
{
	int			i;
	int			root_len;
	const char	*root_dir	= getRootDirPath( );
	const char	*path		= root_path;
	
	root_len = getRootDirLength( );
	i = 0;
	while( *root_dir == *path )
	{
		if( ( *root_dir == '\0' ) || ( *path == '\0' ) )
		{
			break;
		}
		root_dir++;
		path++;
		i++;
	}
	
	if( ( root_len != i ) || ( *path != '/' ) )
	{
		return( -1 );
	}
	
	twfs_path[ 0 ] = '/';
	i = 0;
	while( *path != '\0' )
	{
		twfs_path[ i ] = *path;
		i++;
		path++;
	}

	if( 0 < i )
	{
		if( 1 < i )
		{
			if( twfs_path[ i - 1 ] == '/' )
			{
				twfs_path[ i - 1 ] = '\0';
			}
		}
		twfs_path[ i ] = '\0';
	}
	
	return( 0 );
}

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
int getTwfsPathFromMountDir( const char *mount_path, char *twfs_path )
{
	int			i;
	int			mount_len;
	const char	*mount_dir	= getMountDirPath( );
	const char	*path		= mount_path;
	
	mount_len = getMountDirLength( );
	i = 0;
	while( *mount_dir == *path )
	{
		if( ( *mount_dir == '\0' ) || ( *path == '\0' ) )
		{
			break;
		}
		mount_dir++;
		path++;
		i++;
	}
	
	if( ( mount_len != i ) || ( *path != '/' ) )
	{
		return( -1 );
	}
	
	twfs_path[ 0 ] = '/';
	i = 0;
	while( *path != '\0' )
	{
		twfs_path[ i ] = *path;
		i++;
		path++;
	}

	if( 0 < i )
	{
		if( 1 < i )
		{
			if( twfs_path[ i - 1 ] == '/' )
			{
				twfs_path[ i - 1 ] = '\0';
			}
		}
		twfs_path[ i ] = '\0';
	}
	
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readTweet
	Input		:struct ssl_session *session
				 < ssl session >
				 struct twfs_file
				 < twfs file information >
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
			   const char *slug )
{
	typedef enum
	{
		E_TW_CREATED_AT,			// created_at
		E_TW_ID_STR,				// id_str
		E_TW_IN_RE_TO_SNAME,		// in_reply_to_screen_name
		E_TW_IN_RE_TO_SIDS,			// in_reply_to_status_id_str
		E_TW_RETWEETED,				// retweeted
		E_TW_FAVORITED,				// favorited
		E_TW_RTW_STATUS,			// retweeted_status/
		E_TW_FAV_CNT,				// favorite_count
		E_TW_RTW_CNT,				// retweet_count
		E_TW_RTW_STATUS_CREATED_AT,	// retweeted_status/created_at
		E_TW_RTW_STATUS_ID_STR,		// retweeted_status/id_str
		E_TW_RTW_STATUS_TEXT,		// retweeted_status/text
		E_TW_RTW_STATUS_FAV_CNT,	// retweeted_status/favorite_count
		E_TW_RTW_STATUS_RTW_CNT,	// retweeted_status/retweet_count
		E_TW_RTW_STATUS_USR,		// retweeted_status/user/
		E_TW_RTW_STATUS_USR_ID_STR,	// retweeted_status/user/id_str
		E_TW_RTW_STATUS_USR_NAME,	// retweeted_status/user/name
		E_TW_RTW_STATUS_USR_SNAME,	// retweeted_status/user/sname
		E_TW_TEXT,					// text
		E_TW_USER,					// user/
		E_TW_USR_ID_STR,			// user/id_str
		E_TW_USR_NAME,				// user/name
		E_TW_USR_SNAME,				// user/screen_name
		E_TW_NUM,
	} E_OBJ;

	struct new_tws
	{
		size_t			length;		// length of all texts except for rtw message
		uint8_t			*tweets;
		int				num_tws;
		int				text_len;	// current text length
	};

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	int					fd;
	struct new_tws		new_tws = { 0, NULL, 0, 0};
	struct jnode		root;
	struct jnode		node[ E_TW_NUM ];
	char				rs_created_at[ ]	= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_TW_CREATED_AT;
	char				rs_id_str[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_TW_ID_STR;
	char				rs_text[ ]			= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_TW_TEXT;
	char				rs_fav_cnt[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_TW_FAV_CNT;
	char				rs_rtw_cnt[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_TW_RTW_CNT;
	char				rs_usr[ ]			= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR;
	char				rs_usr_idstr[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				rs_usr_name[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_NAME;
	char				rs_usr_screen_name[ ]="/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_SNAME;
	char				usr_idstr[ ]		= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				usr_name[ ]			= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_NAME;
	char				usr_screen_name[ ]	= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_SNAME;

	/* ------------------------------------------------------------------------ */
	/* request get statuses/home_timeline										*/
	/* ------------------------------------------------------------------------ */
	switch( request )
	{
	case	E_TWFS_REQ_READ_HOME_TL:
	default:
		if( ( result = getHomeTimeLine( session, &hctx, last ) ) < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_HOME_TWEET:
		if( ( result = getMentionsTimeLine( session, &hctx, last ) ) < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_USER_TL:
		if( ( result = getUserTimeLine( session, &hctx,
										screen_name, last ) ) < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_MENTIONS_TL:
		if( ( result = getMentionsTimeLine( session, &hctx, last ) ) < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_RTW_OF_ME_TL:
		if( ( result = getRetweetOfMeTimeLine( session, &hctx, last ) ) < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_FAV_LIST:
	case	E_TWFS_REQ_READ_AUTH_FAV_LIST:
		if( ( result = getFavoritesList( session, &hctx, last ) ) < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_LISTS_SUB_TL:
	case	E_TWFS_REQ_READ_LISTS_OWN_TL:
	case	E_TWFS_REQ_READ_LISTS_ADD_TL:
		logMessage( " get lists timeline\n" );
		if( ( result = getListsTimeLine( session,
										 &hctx,
										 slug,
										 screen_name,
										 last ) ) < 0 )
		{
			return( result );
		}
		break;
	}

	if( hctx.content_length == 0 )
	{
		return( 0 );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for new tweets													*/
	/* ------------------------------------------------------------------------ */
	new_tws.tweets = mmap( NULL, hctx.content_length + 1,
						   PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

	if( new_tws.tweets == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )rs_created_at, 1 ) < 0 )
			{
				break;
			}
		}

		logMessage( "cannot map [new_tws]%s\n", strerror( errno ) );

		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive tweets of json structure											*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_tws.tweets,
						hctx.content_length ) < 0 )
	{
		munmap( new_tws.tweets, hctx.content_length + 1 );
		//disconnectSSLServer( session );
		return( -1 );
	}

	//disconnectSSLServer( session );

	//logMessage( "%s\n", new_tws );

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );

	/* created_at																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_CREATED_AT,
					 &node[ E_TW_CREATED_AT				]	);
	/* id_str																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_ID_STR,
					 &node[ E_TW_ID_STR					]	);
	/* in_reply_to_screen_name													*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_IN_RE_TO_SNAME,
					 &node[ E_TW_IN_RE_TO_SNAME			]	);
	/* in_reply_to_id_str														*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_IN_RE_TO_SIDS,
					 &node[ E_TW_IN_RE_TO_SIDS			]	);
	/* retweeted																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_RETWEETED,
					 &node[ E_TW_RETWEETED				]	);
	/* favorited																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_FAVORITED,
					 &node[ E_TW_FAVORITED				]	);
	/* retweeted_status/														*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_RTW_STATUS,
					 &node[ E_TW_RTW_STATUS				]	);
	/* favorite_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_FAV_CNT,
					 &node[ E_TW_FAV_CNT				]	);
	/* retweet_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_RTW_CNT,
					 &node[ E_TW_RTW_CNT				]	);
	/* retweeted_status/created_at												*/
	insertJsonNodes( &root, rs_created_at,
					 &node[ E_TW_RTW_STATUS_CREATED_AT	]	);
	/* retweeted_status/id_str													*/
	insertJsonNodes( &root, rs_id_str,
					 &node[ E_TW_RTW_STATUS_ID_STR		]	);
	/* retweeted_status/text													*/
	insertJsonNodes( &root, rs_text,
					 &node[ E_TW_RTW_STATUS_TEXT		]	);
	/* retweeted_status/favorite_count											*/
	insertJsonNodes( &root, rs_fav_cnt,
					 &node[ E_TW_RTW_STATUS_FAV_CNT		]	);
	/* retweeted_status/retweet_count											*/
	insertJsonNodes( &root, rs_rtw_cnt,
					 &node[ E_TW_RTW_STATUS_RTW_CNT		]	);
	/* retweeted_status/usr/													*/
	insertJsonNodes( &root, rs_usr,
					 &node[ E_TW_RTW_STATUS_USR			]	);
	/* retweeted_status/usr/id_str												*/
	insertJsonNodes( &root, rs_usr_idstr,
					 &node[ E_TW_RTW_STATUS_USR_ID_STR	]	);
	/* retweeted_status/usr/name												*/
	insertJsonNodes( &root, rs_usr_name,
					 &node[ E_TW_RTW_STATUS_USR_NAME	]	);
	/* retweeted_status/usr/screen_name											*/
	insertJsonNodes( &root, rs_usr_screen_name,
					 &node[ E_TW_RTW_STATUS_USR_SNAME	]	);
	/* text																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_TEXT,
					 &node[ E_TW_TEXT					]	);
	/* user/																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_USER,
					 &node[ E_TW_USER					]	);
	/* user/id_str																*/
	insertJsonNodes( &root, usr_idstr,
					 &node[ E_TW_USR_ID_STR				]	);
	/* user/name																*/
	insertJsonNodes( &root, usr_name,
					 &node[ E_TW_USR_NAME				]	);
	/* user/screen_name															*/
	insertJsonNodes( &root, usr_screen_name,
					 &node[ E_TW_USR_SNAME				]	);
	
	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		char	buffer[ 1024 ];
		int		buf_len;
		int		ana_result;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_tws.tweets,
								  &ana, &root,
								  0,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_TW_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( new_tws.tweets, hctx.content_length + 1 );
			return( -1 );
		}

		if( ( node[ E_TW_USR_SNAME ].value	== NULL ) ||
			( node[ E_TW_ID_STR ].value		== NULL ) ||
			( node[ E_TW_USR_NAME ].value	== NULL ) ||
			( node[ E_TW_RTW_CNT ].value	== NULL ) ||
			( node[ E_TW_FAV_CNT ].value	== NULL ) )
		{
			for( i = 0 ; i < E_TW_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}

			if( ana_result == 0 )
			{
				if( new_tws.num_tws == 0 )
				{
					logMessage( "faile to readTweet\n" );
					munmap( new_tws.tweets, hctx.content_length + 1 );
					return( -1 );
				}
				else
				{
					break;
				}
			}
			continue;
		}

		for( i = 0 ; i < E_TW_NUM ; i++ )
		{
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/status/[tweet id]								*/
		/* -------------------------------------------------------------------- */
		if( node[ E_TW_RTW_STATUS_USR_SNAME ].value != NULL )
		{
			/* retweet															*/
			snprintf( buffer, sizeof( buffer ), "%s/%s/%s",
					  getRootDirPath( ),
					  node[ E_TW_RTW_STATUS_USR_SNAME ].value,
					  DEF_TWFS_PATH_DIR_STATUS );
			if( ( result = isDirectory( buffer ) ) < 0 )
			{
				result =
				makeUserHomeDirectory( node[ E_TW_RTW_STATUS_USR_SNAME ].value,
									   true );
				//logMessage( "==========================\n" );
				//logMessage( "make [%s] home\n", node[ E_TW_RTW_STATUS_USR_SNAME ].value );

				if( result < 0 )
				{
					for( i = 0 ; i < E_TW_NUM ; i++ )
					{
						free( node[ i ].value );
						node[ i ].value = NULL;
					}

					if( ana_result == 0 )
					{
						if( new_tws.num_tws == 0 )
						{
							munmap( new_tws.tweets, hctx.content_length + 1 );
							return( -1 );
						}
						break;
					}

					/* try next tweet											*/
					continue;
				}
			}
		}

		/* tweet itself															*/
		snprintf( buffer, sizeof( buffer ), "%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_TW_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_STATUS );

		//logMessage( "-----------------------------------\n" );
		//logMessage( "%s\n", buffer );

		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserHomeDirectory( node[ E_TW_USR_SNAME ].value, true );
			//logMessage( "make [%s] home\n", node[ E_TW_USR_SNAME ].value );

			if( result < 0 )
			{
				for( i = 0 ; i < E_TW_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}
				
				if( ana_result == 0 )
				{
					if( new_tws.num_tws == 0 )
					{
						munmap( new_tws.tweets, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		if( ( node[ E_TW_RTW_STATUS_ID_STR ].value != NULL ) &&
			( node[ E_TW_RTW_STATUS_USR_SNAME ].value != NULL ) )
		{
			snprintf( buffer, sizeof( buffer ), "%s/%s/%s/%s",
					  getRootDirPath( ),
					  node[ E_TW_RTW_STATUS_USR_SNAME ].value,
					  DEF_TWFS_PATH_DIR_STATUS,
					  node[ E_TW_RTW_STATUS_ID_STR ].value );
			logMessage( "Retweet file:%s\n", buffer );
		}
		else
		{
			snprintf( buffer, sizeof( buffer ), "%s/%s/%s/%s",
					  getRootDirPath( ),
					  node[ E_TW_USR_SNAME ].value,
					  DEF_TWFS_PATH_DIR_STATUS,
					  node[ E_TW_ID_STR ].value );
			//logMessage( "tweet file:%s\n", buffer );
		}

		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			char	favorited;
			char	retweeted;
			/* ---------------------------------------------------------------- */
			/* tweet id															*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"id:%s\n",
								node[ E_TW_ID_STR ].value );
#if 0
			if( node[ E_TW_RTW_STATUS_USR_ID_STR ].value == NULL )
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"id:%s\n",
									node[ E_TW_ID_STR ].value );
			}
			else
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"id:%s\n",
									node[ E_TW_RTW_STATUS_ID_STR ].value );
			}
#endif
			writeFile( fd, ( const void* )buffer, buf_len );
			new_tws.text_len = buf_len;
			/* copy id															*/
			memcpy( new_tws.tweets + new_tws.length, buffer + 3, buf_len - 4 );
			new_tws.length += buf_len - 4;	// without '\n'
			/* fill residual space												*/
			buf_len = DEF_TWFS_ID_FIELD + DEF_TWFS_ID_FIELD_NEXT - ( buf_len - 4 );
			memset( new_tws.tweets + new_tws.length, 0x00, buf_len );
			new_tws.length += buf_len;
#if 0
			/* ---------------------------------------------------------------- */
			/* if retweeted														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_TW_RTW_STATUS_USR_ID_STR ].value != NULL )
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"Retweeted by %s\n",
									node[ E_TW_USR_NAME ].value );
				writeFile( fd, ( const void* )buffer, buf_len );
			}
#endif
			/* ---------------------------------------------------------------- */
			/* [user name] @ [screen name]										*/
			/* ---------------------------------------------------------------- */
			if( ( node[ E_TW_RTW_STATUS_USR_NAME ].value == NULL ) ||
				( node[ E_TW_RTW_STATUS_USR_SNAME ].value == NULL ) )
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"%s @%s\n",
									node[ E_TW_USR_NAME ].value,
									node[ E_TW_USR_SNAME ].value );
				writeFile( fd, ( const void* )buffer, buf_len );
				new_tws.text_len += buf_len;
				/* copy screeen name											*/
				buf_len = node[ E_TW_USR_SNAME ].length;
				memcpy( new_tws.tweets + new_tws.length,
						node[ E_TW_USR_SNAME ].value,
						buf_len );
				new_tws.length += buf_len;
				/* fill residual space											*/
				buf_len = DEF_TWFS_SNAME_FIELD
						  + DEF_TWFS_SNAME_FIELD_NEXT
						  - buf_len;
				memset( new_tws.tweets + new_tws.length, 0x00, buf_len );
				new_tws.length += buf_len;
				/* fill RT flag													*/
				*( new_tws.tweets + new_tws.length++ ) = 'N';
				*( new_tws.tweets + new_tws.length++ ) = 0x00;
				*( new_tws.tweets + new_tws.length++ ) = '0';
				*( new_tws.tweets + new_tws.length++ ) = '0';
				*( new_tws.tweets + new_tws.length++ ) = 0x00;
			}
			else
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"%s @%s\n",
									node[ E_TW_RTW_STATUS_USR_NAME ].value,
									node[ E_TW_RTW_STATUS_USR_SNAME ].value );
				writeFile( fd, ( const void* )buffer, buf_len );
				new_tws.text_len += buf_len;
				/* copy screeen name											*/
				//buf_len = node[ E_TW_RTW_STATUS_USR_SNAME ].length;
				buf_len = node[ E_TW_USR_SNAME ].length;
				memcpy( new_tws.tweets + new_tws.length,
						//node[ E_TW_RTW_STATUS_USR_SNAME ].value,
						node[ E_TW_USR_SNAME ].value,
						buf_len );
				new_tws.length += buf_len;
				/* fill residual space											*/
				buf_len = DEF_TWFS_SNAME_FIELD
						  + DEF_TWFS_SNAME_FIELD_NEXT
						  - buf_len;
				memset( new_tws.tweets + new_tws.length, 0x00, buf_len );
				new_tws.length += buf_len;
				/* fill RT flag													*/
				*( new_tws.tweets + new_tws.length++ ) = 'R';
				*( new_tws.tweets + new_tws.length++ ) = 0x00;
				//buf_len = node[ E_TW_RTW_STATUS_USR_SNAME ].length;
				buf_len = node[ E_TW_USR_SNAME ].length;
				buf_len += sizeof( DEF_TWFS_RTW_MESSAGE ) - 1;
				buf_len++;		// for '\n'
				logMessage( "retweet text len :%d\n", buf_len );
				buf_len = snprintf( ( char *)( new_tws.tweets + new_tws.length ),
									DEF_TWFS_RTW_LEN_FIELD + 1,	// +1 for null
									"%02d", buf_len );
				new_tws.length += buf_len;
				*( new_tws.tweets + new_tws.length++ ) = 0x00;
			}
			/* ---------------------------------------------------------------- */
			/* [text]															*/
			/* ---------------------------------------------------------------- */
			if( node[ E_TW_RTW_STATUS_TEXT ].value == NULL )
			{
				buf_len = snprintf( buffer, sizeof( buffer ), "%s\n",
									node[ E_TW_TEXT ].value );
			}
			else
			{
				buf_len = snprintf( buffer, sizeof( buffer ), "%s\n",
									node[ E_TW_RTW_STATUS_TEXT ].value );
			}
			writeFile( fd, ( const void* )buffer, buf_len );
			new_tws.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* RTWEETS:[retweets count]  FAVORITES:[favorites count]			*/
			/* ---------------------------------------------------------------- */
			if( node[ E_TW_RETWEETED ].value != NULL )
			{
				if( ( *( node[ E_TW_RETWEETED ].value ) == 't' ) ||
					( *( node[ E_TW_RETWEETED ].value ) == 'T' ) )
				{
					retweeted = '*';
				}
				else
				{
					retweeted = ' ';
				}
			}
			else
			{
				retweeted = ' ';
			}

			if( node[ E_TW_FAVORITED ].value != NULL )
			{
				if( ( *( node[ E_TW_FAVORITED ].value ) == 't' ) ||
					( *( node[ E_TW_FAVORITED ].value ) == 'T' ) )
				{
					favorited = '*';
				}
				else
				{
					favorited = ' ';
				}
			}
			else
			{
				favorited = ' ';
			}

			if( ( node[ E_TW_RTW_STATUS_RTW_CNT ].value == NULL ) ||
				( node[ E_TW_RTW_STATUS_FAV_CNT ].value == NULL ) )
			{
				int	res_len;

				buf_len = snprintf( buffer, sizeof( buffer ),
									"[%c]RETWEETS:%s",
									retweeted,
									node[ E_TW_RTW_CNT ].value );
				res_len = DEF_REST_INT_MAX_LENGTH - node[ E_TW_RTW_CNT ].length;
				memset( &buffer[ buf_len ], ' ', res_len + 1 );	// + 1 for separate
				buf_len += res_len + 1;

				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "[%c]FAVORITES:%s",
									 favorited,
									 node[ E_TW_FAV_CNT ].value );
				res_len = DEF_REST_INT_MAX_LENGTH - node[ E_TW_FAV_CNT ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
				buffer[ buf_len++ ] = '\n';
			}
			else
			{
				int	res_len;

				buf_len = snprintf( buffer, sizeof( buffer ),
									"[%c]RETWEETS:%s",
									retweeted,
									node[ E_TW_RTW_STATUS_RTW_CNT ].value );
				res_len = DEF_REST_INT_MAX_LENGTH
						  - node[ E_TW_RTW_STATUS_RTW_CNT ].length;
				memset( &buffer[ buf_len ], ' ', res_len + 1 );	// + 1 for separate
				buf_len += res_len + 1;

				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "[%c]FAVORITES:%s",
									 favorited,
									 node[ E_TW_RTW_STATUS_FAV_CNT ].value );
				res_len = DEF_REST_INT_MAX_LENGTH
						  - node[ E_TW_RTW_STATUS_FAV_CNT ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
				buffer[ buf_len++ ] = '\n';
			}
			writeFile( fd, ( const void* )buffer, buf_len );
			new_tws.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* [created_at]														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_TW_RTW_STATUS_CREATED_AT ].value == NULL )
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"%s\n\n",
									node[ E_TW_CREATED_AT ].value );
			}
			else
			{
				buf_len = snprintf( buffer, sizeof( buffer ),
									"%s\n\n",
									node[ E_TW_RTW_STATUS_CREATED_AT ].value );
			}
			writeFile( fd, ( const void* )buffer, buf_len );
			new_tws.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"----------------------------------------\n" );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_tws.text_len += buf_len;

			closeFile( fd );

			/* ---------------------------------------------------------------- */
			/* link retweet file												*/
			/* ---------------------------------------------------------------- */
			if( ( node[ E_TW_RTW_STATUS_USR_SNAME ].value != NULL ) &&
				( node[ E_TW_RTW_STATUS_ID_STR ].value ) )
			{
				char	new_path[ DEF_TWFS_PATH_MAX ];
				snprintf( new_path, sizeof( new_path ), "%s/%s/%s/%s",
						  getRootDirPath( ),
						  node[ E_TW_USR_SNAME ].value,
						  DEF_TWFS_PATH_DIR_RETWEET,
						  node[ E_TW_ID_STR ].value );

				logMessage( "readTweet from link:%s\n", new_path );

				logMessage( "node[ E_TW_RTW_STATUS_USR_SNAME ].value : %s\n",
							 node[ E_TW_RTW_STATUS_USR_SNAME ].value );
				logMessage( "node[ E_TW_RTW_STATUS_ID_STR ].value : %s\n",
							 node[ E_TW_RTW_STATUS_ID_STR ].value );

				snprintf( buffer, DEF_TWFS_PATH_MAX, "../../%s/%s/%s",
						  node[ E_TW_RTW_STATUS_USR_SNAME ].value,
						  DEF_TWFS_PATH_DIR_STATUS,
						  node[ E_TW_RTW_STATUS_ID_STR ].value );

				logMessage( "readTweet to link:%s\n", buffer );

				symlink( buffer, new_path );

				/* update length for "Retweeted by [screen_name]"			*/
				twfs_file->tl_size += sizeof( DEF_TWFS_RTW_MESSAGE ) - 1;
				twfs_file->tl_size += node[ E_TW_USR_SNAME ].length;
				twfs_file->tl_size += 1;	// for '\n'
			}

			/* ---------------------------------------------------------------- */
			/* link favorite file												*/
			/* ---------------------------------------------------------------- */
			if( request == E_TWFS_REQ_READ_AUTH_FAV_LIST )
			{
				if( ( node[ E_TW_RTW_STATUS_USR_SNAME ].value != NULL ) &&
					( node[ E_TW_RTW_STATUS_ID_STR ].value ) )
				{
					char	new_path[ DEF_TWFS_PATH_MAX ];

					snprintf( new_path, sizeof( new_path ), "%s/%s/%s/%s",
							  getRootDirPath( ),
							  screen_name,
							  DEF_TWFS_PATH_DIR_FAV,
							  node[ E_TW_RTW_STATUS_ID_STR ].value );

					snprintf( buffer, DEF_TWFS_PATH_MAX, "../../%s/%s/%s",
						  node[ E_TW_RTW_STATUS_USR_SNAME ].value,
						  DEF_TWFS_PATH_DIR_STATUS,
						  node[ E_TW_RTW_STATUS_ID_STR ].value );

					symlink( buffer, new_path );
				}
				else
				{
					char	new_path[ DEF_TWFS_PATH_MAX ];

					snprintf( new_path, sizeof( new_path ), "%s/%s/%s/%s",
							  getRootDirPath( ),
							  screen_name,
							  DEF_TWFS_PATH_DIR_FAV,
							  node[ E_TW_ID_STR ].value );

					snprintf( buffer, DEF_TWFS_PATH_MAX, "../../%s/%s/%s",
						  node[ E_TW_USR_SNAME ].value,
						  DEF_TWFS_PATH_DIR_STATUS,
						  node[ E_TW_ID_STR ].value );

					symlink( buffer, new_path );
				}
			}

			/* fill text length												*/
			buf_len = snprintf( ( char* )( new_tws.tweets + new_tws.length ),
								DEF_TWFS_TEXT_LEN_FIELD,
								"%d", new_tws.text_len );
			new_tws.length += buf_len;
			/* update total size of timeline file							*/
			twfs_file->tl_size += new_tws.text_len;
			/* fill residual space											*/
			buf_len = DEF_TWFS_TEXT_LEN_FIELD - buf_len;
			memset( new_tws.tweets + new_tws.length, 0x00, buf_len );
			new_tws.length += buf_len;
			*( new_tws.tweets + new_tws.length++ ) = '\n';

			new_tws.num_tws++;
		}

		for( i = 0 ; i < E_TW_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_tws.num_tws == 0 )
			{
				munmap( new_tws.tweets, hctx.content_length + 1 );
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}

	if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
	{
		result = ftruncate( twfs_file->fd,
							twfs_file->size
							+ ( DEF_TWFS_TL_RECORD_LEN * new_tws.num_tws ) );
	}
	else
	{
		result = ftruncate( twfs_file->fd,
							DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN
							+ ( DEF_TWFS_TL_RECORD_LEN * new_tws.num_tws ) );
	}
	
	if( result < 0 )
	{
		munmap( new_tws.tweets, hctx.content_length + 1 );
		logMessage( "failed to ftruncat at readTweet[%zu]\n", twfs_file->size );
		return( -1 );
	}
	*( new_tws.tweets + new_tws.length ) = '\0';

	/* ------------------------------------------------------------------------ */
	/* body of a timeline file is updated										*/
	/* reverse new_twfs.tweets and save them									*/
	/* ------------------------------------------------------------------------ */
	for( i = 0 ; i < new_tws.num_tws ; i++ )
	{
		if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
		{
			memcpy( twfs_file->tl
					+ twfs_file->size
					+ ( DEF_TWFS_TL_RECORD_LEN * ( new_tws.num_tws - i )
					- DEF_TWFS_TL_RECORD_LEN ),
					new_tws.tweets + ( DEF_TWFS_TL_RECORD_LEN * i ),
					DEF_TWFS_TL_RECORD_LEN );
		}
		else
		{
			memcpy( twfs_file->tl
					+ twfs_file->size
					+ DEF_TWFS_OFFSET_BODY_OF_TL
					+ ( DEF_TWFS_TL_RECORD_LEN * ( new_tws.num_tws - i )
					- DEF_TWFS_TL_RECORD_LEN ),
					new_tws.tweets + ( DEF_TWFS_TL_RECORD_LEN * i ),
					DEF_TWFS_TL_RECORD_LEN );
		}
	}

	/* ------------------------------------------------------------------------ */
	/* update total timeline file size											*/
	/* ------------------------------------------------------------------------ */
	if( new_tws.num_tws )
	{
		result = snprintf( twfs_file->tl,
						   DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN,
						   "%zu", twfs_file->tl_size );

		for( i = result ; i < ( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN - 1 ) ; i++ )
		{
			*( twfs_file->tl + i ) = 0x00;
		}
		*( twfs_file->tl + i ) = '\n';
	
		if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
		{
			/* actual timeline file is updated									*/
			twfs_file->size += ( DEF_TWFS_TL_RECORD_LEN * new_tws.num_tws );
		}
		else
		{
			/* actual timeline file is updated									*/
			twfs_file->size = DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN;
			twfs_file->size += ( DEF_TWFS_TL_RECORD_LEN * new_tws.num_tws );
		}
	}

	logMessage( "twfs_file->size :%d\n", twfs_file->size );
	logMessage( "num :%d\n", new_tws.num_tws );
	logMessage( "new_tws.length : %d\n", new_tws.length );
	logMessage( "------------------- timeline file ------------------ \n" );

	if( new_tws.num_tws )
	{
		msync( twfs_file->tl, twfs_file->size, MS_SYNC );
	}

	munmap( new_tws.tweets, hctx.content_length + 1 );

	return( 0 );
}

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
			const char *sent_last )
{
	typedef enum
	{
		E_DM_CREATED_AT,			// created_at
		E_DM_ID_STR,				// id_str
		E_DM_RECP,					// recipient
		E_DM_RECP_ID_STR,			// recipient/id_str
		E_DM_RECP_NAME,				// recipient/name
		E_DM_RECP_PROTECTED,		// recipient/protected
		E_DM_RECP_SNAME,			// recipient/screen_name
		E_DM_SEND,					// sender/
		E_DM_SEND_ID_STR,			// sender/id_str
		E_DM_SEND_NAME,				// sender/name
		E_DM_SEND_PROTECTED,		// sender/protected
		E_DM_SEND_SNAME,			// sender/screen_name
		E_DM_TEXT,					// text
		E_DM_NUM,
	} E_OBJ;

	struct new_dms
	{
		uint8_t			*dms;
		size_t			length;
		size_t			text_total_len;
		int				text_len;
		int				num_dms;
		int				unmap_length;
	};

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	int					fd;
	int					snd_index			= 0;
	int					rcp_index			= 0;
	int					twfs_index			= 0;
	struct new_dms		new_recp_dms		= { NULL, 0, 0, 0, 0 };
	struct new_dms		new_send_dms		= { NULL, 0, 0, 0, 0 };
	struct jnode		root;
	struct jnode		node[ E_DM_NUM ];
	/* for received dm															*/
	char				rc_id_str[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_ID_STR;
	char				rc_name[ ]			= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_NAME;
	char				rc_protected[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_PROTECTED;
	char				rc_sname[ ]			= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_SNAME;
	/* for sent dm																*/
	char				rc_id_str2[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_ID_STR;
	char				rc_name2[ ]			= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_NAME;
	char				rc_protected2[ ]	= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_PROTECTED;
	char				rc_sname2[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_RECP
											  "/" DEF_TWAPI_OBJ_DM_USR_SNAME;

	/* for received dm															*/
	char				sd_id_str[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_ID_STR;
	char				sd_name[ ]			= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_NAME;
	char				sd_protected[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_PROTECTED;
	char				sd_sname[ ]			= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_SNAME;
	/* for sent dm																*/
	char				sd_id_str2[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_ID_STR;
	char				sd_name2[ ]			= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_NAME;
	char				sd_protected2[ ]	= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_PROTECTED;
	char				sd_sname2[ ]		= "/" DEF_TWAPI_OBJ_DM_USR_SEND
											  "/" DEF_TWAPI_OBJ_DM_USR_SNAME;

	if( ( result = getDirectMessages( session, &hctx, recv_last ) ) < 0 )
	{
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for received new dm tweets										*/
	/* ------------------------------------------------------------------------ */
	new_send_dms.dms = mmap( NULL, hctx.content_length + 1,
							 PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );
	
	if( new_send_dms.dms == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )sd_id_str, 1 ) < 0 )
			{
				break;
			}
		}

		logMessage( "cannot map [new_send_dms]%s\n", strerror( errno ) );

		return( -1 );
	}

	new_send_dms.unmap_length = hctx.content_length + 1;

	/* ------------------------------------------------------------------------ */
	/* receive recieved dm tweets of json structure								*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_send_dms.dms,
						hctx.content_length ) < 0 )
	{
		munmap( new_send_dms.dms, hctx.content_length );
		//disconnectSSLServer( session );
		logMessage( "failed to recv ssl message at readDM\n" );
		return( -1 );
	}

	for( i = 0 ; i < E_DM_NUM ; i++ )
	{
		node[ i ].value = NULL;
	}

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );

	/* created_at																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_CREATED_AT,
					 &node[ E_DM_CREATED_AT		] );
	/* id_str																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_ID_STR,
					 &node[ E_DM_ID_STR			] );
	/* sender																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_USR_SEND,
					 &node[ E_DM_SEND			] );
	/* sender/id_str															*/
	insertJsonNodes( &root, sd_id_str,
					 &node[ E_DM_SEND_ID_STR	] );
	/* sender/name																*/
	insertJsonNodes( &root, sd_name,
					 &node[ E_DM_SEND_NAME		] );
	/* sender/protected															*/
	insertJsonNodes( &root, sd_protected,
					 &node[ E_DM_SEND_PROTECTED	] );
	/* sender/screen_name														*/
	insertJsonNodes( &root, sd_sname,
					 &node[ E_DM_SEND_SNAME		] );
	/* text																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_TEXT,
					 &node[ E_DM_TEXT			] );
	/* recipient																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_USR_RECP,
					 &node[ E_DM_RECP			] );
	/* recipient/id_str															*/
	insertJsonNodes( &root, rc_id_str,
					 &node[ E_DM_RECP_ID_STR	] );
	/* recipient/name															*/
	insertJsonNodes( &root, rc_name,
					 &node[ E_DM_RECP_NAME		] );
	/* recipient/protected														*/
	insertJsonNodes( &root, rc_protected,
					 &node[ E_DM_RECP_PROTECTED	] );
	/* recipietn/screen_name													*/
	insertJsonNodes( &root, rc_sname,
					 &node[ E_DM_RECP_SNAME		] );

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		char	buffer[ 1024 ];
		int		buf_len;
		int		ana_result;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_send_dms.dms,
								  &ana, &root,
								  0,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_DM_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			return( -1 );
		}

		if( ( node[ E_DM_SEND_ID_STR	].value == NULL ) ||
			( node[ E_DM_SEND_NAME		].value == NULL ) ||
			( node[ E_DM_SEND_SNAME		].value == NULL ) ||
			( node[ E_DM_RECP_NAME		].value == NULL ) ||
			( node[ E_DM_RECP_SNAME		].value == NULL ) )
		{
			for( i = 0 ; i < E_DM_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}

			if( ana_result == 0 )
			{
				if( new_send_dms.num_dms == 0 )
				{
					logMessage( " node is null at readDM\n" );
					return( -1 );
				}
				break;
			}
			continue;
		}

#if 0
		for( i = 0 ; i < E_DM_NUM ; i++ )
		{
			if( node[ i ].value != NULL )
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}
#endif

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/direct_message/[screen_name]/[tweet id]		*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, sizeof( buffer ), "%s/%s/%s/%s",
				  getRootDirPath( ),
				  getTwapiScreenName( ),
				  DEF_TWFS_PATH_DIR_DM,
				  node[ E_DM_SEND_SNAME ].value );
		
		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserDirectMessageDir( getTwapiScreenName( ),
											   node[ E_DM_SEND_SNAME ].value,
											   true );

			if( result < 0 )
			{
				for( i = 0 ; i < E_DM_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}
				
				if( ana_result == 0 )
				{
					if( new_send_dms.num_dms == 0 )
					{
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		snprintf( buffer, sizeof( buffer ), "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  getTwapiScreenName( ),
				  DEF_TWFS_PATH_DIR_DM,
				  node[ E_DM_SEND_SNAME ].value,
				  node[ E_DM_ID_STR ].value );

		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			/* ---------------------------------------------------------------- */
			/* tweet id															*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"id:%s\n",
								node[ E_DM_ID_STR ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_send_dms.text_len = buf_len;
			/* copy id															*/
			memcpy( new_send_dms.dms + new_send_dms.length,
					buffer + 3,
					buf_len - 4 );
			new_send_dms.length += buf_len - 4;	// without '\n'
			/* fill residual space												*/
			buf_len = DEF_TWFS_ID_FIELD + DEF_TWFS_ID_FIELD_NEXT - ( buf_len - 4 );
			memset( new_send_dms.dms + new_send_dms.length, 0x00, buf_len );
			new_send_dms.length += buf_len;
			/* ---------------------------------------------------------------- */
			/* [user name] @ [screen name]										*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"To  :%s @%s\nFrom:%s @%s\n",
								node[ E_DM_RECP_NAME ].value,
								node[ E_DM_RECP_SNAME ].value,
								node[ E_DM_SEND_NAME ].value,
								node[ E_DM_SEND_SNAME ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_send_dms.text_len += buf_len;
			/* copy screen name													*/
			buf_len = node[ E_DM_SEND_SNAME ].length;
			memcpy( new_send_dms.dms + new_send_dms.length,
					node[ E_DM_SEND_SNAME ].value,
					buf_len );
			new_send_dms.length += buf_len;
			/* fill residual space											*/
			buf_len = DEF_TWFS_SNAME_FIELD
					  + DEF_TWFS_SNAME_FIELD_NEXT
					  - buf_len;
			memset( new_send_dms.dms + new_send_dms.length, 0x00, buf_len );
			new_send_dms.length += buf_len;
			/* fill SEND flag													*/
			*( new_send_dms.dms + new_send_dms.length ++ ) = 's';
			*( new_send_dms.dms + new_send_dms.length ++ ) = 0x00;
			*( new_send_dms.dms + new_send_dms.length ++ ) = '0';
			*( new_send_dms.dms + new_send_dms.length ++ ) = '0';
			*( new_send_dms.dms + new_send_dms.length ++ ) = 0x00;

			/* ---------------------------------------------------------------- */
			/* [text]															*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ), "%s\n",
								node[ E_DM_TEXT ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_send_dms.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* [created_at]														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ), "%s\n\n",
								node[ E_DM_CREATED_AT ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_send_dms.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"---------------------------------------------\n" );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_recp_dms.text_len += buf_len;

			closeFile( fd );

			/* fill text length													*/
			buf_len = snprintf( ( char* )( new_send_dms.dms + new_send_dms.length ),
								DEF_TWFS_TEXT_LEN_FIELD,
								"%d", new_send_dms.text_len );
			new_send_dms.length += buf_len;
			/* update total size of message file								*/
			twfs_file->tl_size			+= new_send_dms.text_len;
			new_send_dms.text_total_len	+= new_send_dms.text_len;
			/* fill residual space												*/
			buf_len = DEF_TWFS_TEXT_LEN_FIELD - buf_len;
			memset( new_send_dms.dms + new_send_dms.length, 0x00, buf_len );
			new_send_dms.length += buf_len;
			*( new_send_dms.dms + new_send_dms.length++ ) = '\n';

			new_send_dms.num_dms++;
		}

		for( i = 0 ; i < E_DM_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_send_dms.num_dms == 0 )
			{
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}

	if( ( result = getDirectMessagesSent( session, &hctx, sent_last ) ) < 0 )
	{
		munmap( new_send_dms.dms, hctx.content_length );
		//disconnectSSLServer( session );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for received new dm tweets										*/
	/* ------------------------------------------------------------------------ */
	new_recp_dms.dms = mmap( NULL, hctx.content_length + 1,
							PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );
	
	if( new_recp_dms.dms == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )sd_id_str, 1 ) < 0 )
			{
				break;
			}
		}

		logMessage( "cannot map [new_recp_dms]%s\n", strerror( errno ) );
		munmap( new_send_dms.dms, hctx.content_length );
		//disconnectSSLServer( session );

		return( -1 );
	}

	new_recp_dms.unmap_length = hctx.content_length + 1;

	/* ------------------------------------------------------------------------ */
	/* receive send dm tweets of json structure									*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_recp_dms.dms,
						hctx.content_length ) < 0 )
	{
		munmap( new_send_dms.dms, hctx.content_length );
		munmap( new_recp_dms.dms, hctx.content_length );
		//disconnectSSLServer( session );

		return( -1 );
	}

	//disconnectSSLServer( session );

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );

	/* created_at																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_CREATED_AT,
					 &node[ E_DM_CREATED_AT		] );
	/* id_str																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_ID_STR,
					 &node[ E_DM_ID_STR			] );
	/* recipient																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_USR_RECP,
					 &node[ E_DM_RECP			] );
	/* recipient/id_str															*/
	insertJsonNodes( &root, rc_id_str2,
					 &node[ E_DM_RECP_ID_STR	] );
	/* recipient/name															*/
	insertJsonNodes( &root, rc_name2,
					 &node[ E_DM_RECP_NAME		] );
	/* recipient/protected														*/
	insertJsonNodes( &root, rc_protected2,
					 &node[ E_DM_RECP_PROTECTED	] );
	/* recipietn/screen_name													*/
	insertJsonNodes( &root, rc_sname2,
					 &node[ E_DM_RECP_SNAME		] );
	/* text																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_TEXT,
					 &node[ E_DM_TEXT			] );
	/* sender																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_DM_USR_SEND,
					 &node[ E_DM_SEND			] );
	/* sender/id_str															*/
	insertJsonNodes( &root, sd_id_str2,
					 &node[ E_DM_SEND_ID_STR	] );
	/* sender/name																*/
	insertJsonNodes( &root, sd_name2,
					 &node[ E_DM_SEND_NAME		] );
	/* sender/protected															*/
	insertJsonNodes( &root, sd_protected2,
					 &node[ E_DM_SEND_PROTECTED	] );
	/* sender/screen_name														*/
	insertJsonNodes( &root, sd_sname2,
					 &node[ E_DM_SEND_SNAME		] );

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		char	buffer[ 1024 ];
		int		buf_len;
		int		ana_result;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_recp_dms.dms,
								  &ana, &root,
								  0,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_DM_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( new_send_dms.dms, hctx.content_length );
			munmap( new_recp_dms.dms, hctx.content_length );
			return( -1 );;
		}

		if( ( node[ E_DM_RECP_ID_STR	].value == NULL ) ||
			( node[ E_DM_RECP_NAME		].value == NULL ) ||
			( node[ E_DM_RECP_SNAME		].value == NULL ) ||
			( node[ E_DM_SEND_NAME		].value == NULL ) ||
			( node[ E_DM_SEND_SNAME		].value == NULL ) )
		{
			for( i = 0 ; i < E_DM_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}

			if( ana_result == 0 )
			{
				if( new_recp_dms.num_dms == 0 )
				{
					munmap( new_send_dms.dms, hctx.content_length );
					munmap( new_recp_dms.dms, hctx.content_length );
					return( -1 );
				}
				break;
			}
			continue;
		}

#if 0
		for( i = 0 ; i < E_DM_NUM ; i++ )
		{
			if( node[ i ].value != NULL )
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}
#endif

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/direct_message/[screen_name]/[tweet id]		*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, sizeof( buffer ), "%s/%s/%s/%s",
				  getRootDirPath( ),
				  getTwapiScreenName( ),
				  DEF_TWFS_PATH_DIR_DM,
				  node[ E_DM_RECP_SNAME ].value );
		
		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserDirectMessageDir( getTwapiScreenName( ),
											   node[ E_DM_RECP_SNAME ].value,
											   true );

			if( result < 0 )
			{
				for( i = 0 ; i < E_DM_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}
				
				if( ana_result == 0 )
				{
					if( new_recp_dms.num_dms == 0 )
					{
						munmap( new_send_dms.dms, hctx.content_length );
						munmap( new_recp_dms.dms, hctx.content_length );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		snprintf( buffer, sizeof( buffer ), "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  getTwapiScreenName( ),
				  DEF_TWFS_PATH_DIR_DM,
				  node[ E_DM_RECP_SNAME ].value,
				  node[ E_DM_ID_STR ].value );

		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			/* ---------------------------------------------------------------- */
			/* tweet id															*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"id:%s\n",
								node[ E_DM_ID_STR ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_recp_dms.text_len = buf_len;
			/* copy id															*/
			memcpy( new_recp_dms.dms + new_recp_dms.length,
					buffer + 3,
					buf_len - 4 );
			new_recp_dms.length += buf_len - 4;	// without '\n'
			/* fill residual space												*/
			buf_len = DEF_TWFS_ID_FIELD + DEF_TWFS_ID_FIELD_NEXT - ( buf_len - 4 );
			memset( new_recp_dms.dms + new_recp_dms.length, 0x00, buf_len );
			new_recp_dms.length += buf_len;
			/* ---------------------------------------------------------------- */
			/* [user name] @ [screen name]										*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"To  :%s @%s\nFrom:%s @%s\n",
								node[ E_DM_RECP_NAME ].value,
								node[ E_DM_RECP_SNAME ].value,
								node[ E_DM_SEND_NAME ].value,
								node[ E_DM_SEND_SNAME ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_recp_dms.text_len += buf_len;
			/* copy screen name													*/
			buf_len = node[ E_DM_RECP_SNAME ].length;
			memcpy( new_recp_dms.dms + new_recp_dms.length,
					node[ E_DM_RECP_SNAME ].value,
					buf_len );
			new_recp_dms.length += buf_len;
			/* fill residual space											*/
			buf_len = DEF_TWFS_SNAME_FIELD
					  + DEF_TWFS_SNAME_FIELD_NEXT
					  - buf_len;
			memset( new_recp_dms.dms + new_recp_dms.length, 0x00, buf_len );
			new_recp_dms.length += buf_len;
			/* fill SEND flag													*/
			*( new_recp_dms.dms + new_recp_dms.length ++ ) = 'r';
			*( new_recp_dms.dms + new_recp_dms.length ++ ) = 0x00;
			*( new_recp_dms.dms + new_recp_dms.length ++ ) = '0';
			*( new_recp_dms.dms + new_recp_dms.length ++ ) = '0';
			*( new_recp_dms.dms + new_recp_dms.length ++ ) = 0x00;

			/* ---------------------------------------------------------------- */
			/* [text]															*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ), "%s\n",
								node[ E_DM_TEXT ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_recp_dms.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* [created_at]														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ), "%s\n\n",
								node[ E_DM_CREATED_AT ].value );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_recp_dms.text_len += buf_len;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( buffer, sizeof( buffer ),
								"---------------------------------------------\n" );
			writeFile( fd, ( const void* )buffer, buf_len );
			new_recp_dms.text_len += buf_len;

			closeFile( fd );

			/* fill text length													*/
			buf_len = snprintf( ( char* )( new_recp_dms.dms + new_recp_dms.length ),
								DEF_TWFS_TEXT_LEN_FIELD,
								"%d", new_recp_dms.text_len );
			new_recp_dms.length += buf_len;
			/* update total size of message file								*/
			twfs_file->tl_size			+= new_recp_dms.text_len;
			new_send_dms.text_total_len	+= new_recp_dms.text_len;
			/* fill residual space												*/
			buf_len = DEF_TWFS_TEXT_LEN_FIELD - buf_len;
			memset( new_recp_dms.dms + new_recp_dms.length, 0x00, buf_len );
			new_recp_dms.length += buf_len;
			*( new_recp_dms.dms + new_recp_dms.length++ ) = '\n';

			new_recp_dms.num_dms++;
		}

		for( i = 0 ; i < E_DM_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_recp_dms.num_dms == 0 )
			{
				munmap( new_send_dms.dms, hctx.content_length );
				munmap( new_recp_dms.dms, hctx.content_length );
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}

	if( ( new_send_dms.num_dms == 0 ) && ( new_recp_dms.num_dms == 0 ) )
	{
		
		munmap( new_send_dms.dms, new_send_dms.unmap_length );
		munmap( new_recp_dms.dms, new_recp_dms.unmap_length );
		return( 0 );
	}

#if 1
	*( new_send_dms.dms + new_send_dms.length ) = '\0';
	*( new_recp_dms.dms + new_recp_dms.length ) = '\0';
	logMessage( "------------------- send message -----------------\n" );
	for( i = 0 ; i < new_send_dms.length ; i++ )
	{
		logMessage( "%c", *( new_send_dms.dms + i ) );
	}
	logMessage( "\n" );
	logMessage( "------------------- recp message -----------------\n" );
	for( i = 0 ; i < new_recp_dms.length ; i++ )
	{
		logMessage( "%c", *( new_recp_dms.dms + i ) );
	}
	logMessage( "\n" );
#endif

	snd_index = new_send_dms.num_dms - 1;
	rcp_index = new_recp_dms.num_dms - 1;
	{
		int		save_snd_index	= INT_MIN;
		int		save_rcp_index	= INT_MIN;
		int		cmp_result;
		char	snd_id[ DEF_TWAPI_MAX_USER_ID_LEN ] = "99999999999999999999";
		char	rcp_id[ DEF_TWAPI_MAX_USER_ID_LEN ] = "99999999999999999999";

		//while( ( snd_index < new_send_dms.num_dms ) ||
		//	   ( rcp_index < new_recp_dms.num_dms ) )
		while( ( 0 <= snd_index ) ||
			   ( 0 <= rcp_index ) )
		{
			if( ( snd_index != save_snd_index ) &&
				( 0 <= snd_index ) )
			{
				for( i = 0 ; i < DEF_TWFS_ID_FIELD ; i++ )
				{
					char	s;

					s = *( new_send_dms.dms
						   + ( DEF_TWFS_TL_RECORD_LEN * snd_index )
						   + DEF_TWFS_OFFSET_ID_FIELD
						   + i );

					if( s == 0x00 )
					{
						snd_id[ i ] = '\0';
						break;
					}
					else
					{
						snd_id[ i ] = s;
					}
				}

				if( DEF_TWFS_ID_FIELD <= i )
				{
					snd_index--;
					continue;
				}

				save_snd_index = snd_index;
			}

			if( ( rcp_index != save_rcp_index ) &&
				( 0 <= rcp_index ) )
			{
				for( i = 0 ; i < DEF_TWFS_ID_FIELD ; i++ )
				{
					char	s;

					s = *( new_recp_dms.dms
						   + ( DEF_TWFS_TL_RECORD_LEN * rcp_index )
						   + DEF_TWFS_OFFSET_ID_FIELD
						   + i );

					if( s == 0x00 )
					{
						rcp_id[ i ] = '\0';
						break;
					}
					else
					{
						rcp_id[ i ] = s;
					}
				}

				if( DEF_TWFS_ID_FIELD <= i )
				{
					rcp_index--;
					continue;
				}

				save_rcp_index = rcp_index;
			}

			if( ( 0 <= snd_index ) &&
				( 0 <= rcp_index ) )
			{
				cmp_result = strncmp( snd_id, rcp_id, sizeof( snd_id ) );
			}
			else if( snd_index < 0 )
			{
				cmp_result = 1;		// go to recipient result
			}
			else if( rcp_index < 0 )
			{
				cmp_result = -1;		// go to sender result
			}
			else
			{
				break;
			}

			/* sender result													*/
			if( cmp_result < 0 )
			{
				/* ------------------------------------------------------------ */
				/*	save twfs_file contents to new_send_dms.dms temporarily		*/
				/*	@ DEF_TWFS_OSSET_BODY_OF_TL +								*/
				/*	( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT )			*/
				/* ------------------------------------------------------------ */
				memcpy( new_send_dms.dms
						+ DEF_TWFS_OFFSET_BODY_OF_TL
						+ ( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT )
						+ ( DEF_TWFS_TL_RECORD_LEN * twfs_index ),
							new_send_dms.dms
							+ ( DEF_TWFS_TL_RECORD_LEN * snd_index ),
							DEF_TWFS_TL_RECORD_LEN );

				twfs_index++;
				snd_index--;
			}
			/* recipient result													*/
			else
			{
				/* ------------------------------------------------------------ */
				/*	save twfs_file contents to new_send_dms.dms temporarily		*/
				/*	@ DEF_TWFS_OSSET_BODY_OF_TL +								*/
				/*	( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT )			*/
				/* ------------------------------------------------------------ */
				memcpy( new_send_dms.dms
						+ DEF_TWFS_OFFSET_BODY_OF_TL
						+ ( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT )
						+ ( DEF_TWFS_TL_RECORD_LEN * twfs_index ),
							new_recp_dms.dms
							+ ( DEF_TWFS_TL_RECORD_LEN * rcp_index ),
							DEF_TWFS_TL_RECORD_LEN );
				/* sender = recipient											*/
				if( cmp_result == 0 )
				{
					int		sender_text_len = 0;
					char	text_len_1;
					int		tf_ind;

					/* -------------------------------------------------------- */
					/* update DM flag to loop back flag 'l'						*/
					/* -------------------------------------------------------- */
					*( new_send_dms.dms
					   + DEF_TWFS_OFFSET_BODY_OF_TL
					   + ( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT )
					   + ( DEF_TWFS_TL_RECORD_LEN * twfs_index )
					   + DEF_TWFS_OFFSET_DM_FIELD ) = 'l';

					/* -------------------------------------------------------- */
					/* update total text length									*/
					/* -------------------------------------------------------- */
					for( tf_ind = 0 ;
						 tf_ind < DEF_TWFS_TEXT_LEN_FIELD ;
						 tf_ind++ )
					{
						text_len_1 = ( char )( *( new_send_dms.dms
												 + ( DEF_TWFS_TL_RECORD_LEN
												     * snd_index )
												 + DEF_TWFS_OFFSET_TEXT_LEN_FIELD
												 + tf_ind ) );
						if( text_len_1 == 0x00 )
						{
							twfs_file->tl_size -= sender_text_len;
							break;
						}
						else
						{
							sender_text_len = sender_text_len * 10
											  + ( text_len_1 - '0' );
						}
					}
					snd_index--;
				}
				twfs_index++;
				rcp_index--;
			}
		}
	}

	/* ------------------------------------------------------------------------ */
	/* expand twfs_file to save twfs_index record								*/
	/* ------------------------------------------------------------------------ */
	if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
	{
		result = ftruncate( twfs_file->fd,
							twfs_file->size
							+ ( DEF_TWFS_TL_RECORD_LEN * twfs_index ) );
	}
	else
	{
		result = ftruncate( twfs_file->fd,
							DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN
							+ ( DEF_TWFS_TL_RECORD_LEN * twfs_index ) );
	}

	/* ------------------------------------------------------------------------ */
	/* copy temporary contents of twfs_file record to twfs_file					*/
	/* ------------------------------------------------------------------------ */
	if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
	{
		memcpy( twfs_file->tl + twfs_file->size,
				new_send_dms.dms
				+ DEF_TWFS_OFFSET_BODY_OF_TL
				+ ( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT ),
				DEF_TWFS_TL_RECORD_LEN * twfs_index );
	}
	else
	{
		memcpy( twfs_file->tl
				 + twfs_file->size
				 + DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN,
				new_send_dms.dms
				 + DEF_TWFS_OFFSET_BODY_OF_TL
				 + ( DEF_TWFS_TL_RECORD_LEN * DEF_TWOPE_MAX_DM_COUNT ),
				DEF_TWFS_TL_RECORD_LEN * twfs_index );
	}


	/* new_recp_dms.dms is not used from here								*/
	munmap( new_recp_dms.dms, new_recp_dms.unmap_length );

	/* new_send_dms.dms is used for temporary buffer								*/
	for( i = 0 ; i < twfs_index ; i++ )
	{
		new_send_dms.dms[ i ] = 0x00;
	}

	snd_index = 0;
	rcp_index = 0;
	for( i = 0 ; i < twfs_index ; i++ )
	{
		char		header[ DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN ];
		char		dm_sname[ DEF_TWFS_SNAME_FIELD ];
		char		dm_file[ DEF_TWFS_PATH_MAX ];
		int			dm_sname_len;
		int			j;
		struct stat	dm_file_stat;
		char		dm_field;
		off_t		dm_file_offset;
		size_t		dm_text_total = 0;
		size_t		dm_text_len;
		size_t		dm_current_size = 0;

		/* -------------------------------------------------------------------- */
		/* if end of record is 's', its record already written to a file		*/
		/* -------------------------------------------------------------------- */
		if( new_send_dms.dms[ i ] != 0x00 )
		{
			continue;
		}

		for( j = 0 ; j < DEF_TWFS_SNAME_FIELD ; j++ )
		{
			char	s;

			if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
			{
				s = *( twfs_file->tl
					   + twfs_file->size
					   + ( DEF_TWFS_TL_RECORD_LEN * i )
					   + DEF_TWFS_OFFSET_SNAME_FIELD
					   + j );
			}
			else
			{
				s = *( twfs_file->tl
					   + twfs_file->size
					   + DEF_TWFS_OFFSET_BODY_OF_TL
					   + ( DEF_TWFS_TL_RECORD_LEN * i )
					   + DEF_TWFS_OFFSET_SNAME_FIELD
					   + j );
			}

			if( s == 0x00 )
			{
				dm_sname[ j ] = '\0';
				dm_sname_len = j;
				break;
			}
			else
			{
				dm_sname[ j ] = s;
			}
		}

		/* -------------------------------------------------------------------- */
		/* open [authed_name]/direct_message/[screen_name]/message				*/
		/* -------------------------------------------------------------------- */
		snprintf( dm_file, sizeof( dm_file ),
				  "%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  getTwapiScreenName( ),
				  DEF_TWFS_PATH_DIR_DM,
				  dm_sname,
				  DEF_TWFS_PATH_DM_MSG );

		logMessage( "--------------------------------\n" );
		logMessage( "%d\n", i );
		logMessage( "%s\n", dm_file );

		logMessage( "--------------------------------\n" );

		fd = openFile( dm_file, O_RDWR | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			result = fstat( fd, &dm_file_stat );
		}

		if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
		{
			dm_field = *( twfs_file->tl
						  + twfs_file->size
						  + ( DEF_TWFS_TL_RECORD_LEN * i )
						  + DEF_TWFS_OFFSET_DM_FIELD );
		}
		else
		{
			dm_field = *( twfs_file->tl
						  + twfs_file->size
						  + DEF_TWFS_OFFSET_BODY_OF_TL
						  + ( DEF_TWFS_TL_RECORD_LEN * i )
						  + DEF_TWFS_OFFSET_DM_FIELD );
		}

		if( fd < 0 || result < 0 )
		{
			if( dm_field == 's' )
			{
				snd_index++;
			}
			else
			{
				rcp_index++;
			}
			if( fd < 0 )
			{
				closeFile( fd );
			}
			/* saved flag is updated											*/
			new_send_dms.dms[ i ] = 's';
			continue;
		}

		if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < dm_file_stat.st_size )
		{
			dm_file_offset = dm_file_stat.st_size;
			getTotalSizeOfTlFileFromFd( fd, &dm_current_size );
		}
		else
		{
			dm_file_offset = DEF_TWFS_OFFSET_BODY_OF_TL;
		}

		if( dm_field == 's' )
		{
			dm_file_offset += ( DEF_TWFS_TL_RECORD_LEN * snd_index++ );
		}
		else
		{
			dm_file_offset += ( DEF_TWFS_TL_RECORD_LEN * rcp_index++ );
		}

		/* -------------------------------------------------------------------- */
		/* update [authed_name]/direct_message/[screen_name]/message file		*/
		/* -------------------------------------------------------------------- */
		if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
		{
			pwriteFile( fd,
						twfs_file->tl
						 + twfs_file->size
						 + ( DEF_TWFS_TL_RECORD_LEN * i ),
						DEF_TWFS_TL_RECORD_LEN,
						dm_file_offset );
		}
		else
		{
			pwriteFile( fd,
						twfs_file->tl
						 + twfs_file->size
						 + DEF_TWFS_OFFSET_BODY_OF_TL
						 + ( DEF_TWFS_TL_RECORD_LEN * i ),
						DEF_TWFS_TL_RECORD_LEN,
						dm_file_offset );
		}

		/* -------------------------------------------------------------------- */
		/* calc size of [authed_name]/direct_message/[screen_name]/message file	*/
		/* -------------------------------------------------------------------- */
		for( int l = 0 ; l < DEF_TWFS_TEXT_LEN_FIELD ; l++ )
		{
			if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
			{
				dm_text_len = *( twfs_file->tl
								 + twfs_file->size
								 + ( DEF_TWFS_TL_RECORD_LEN * i )
								 + DEF_TWFS_OFFSET_TEXT_LEN_FIELD
								 + l );
			}
			else
			{
				dm_text_len = *( twfs_file->tl
								 + twfs_file->size
								 + DEF_TWFS_OFFSET_BODY_OF_TL
								 + ( DEF_TWFS_TL_RECORD_LEN * i )
								 + DEF_TWFS_OFFSET_TEXT_LEN_FIELD
								 + l );
			}

			if( dm_text_len == 0x00 )
			{
				break;
			}
			dm_text_total = dm_text_total * 10 + ( dm_text_len - '0' );
		}

		dm_text_total += dm_current_size;

		/* -------------------------------------------------------------------- */
		/* search same [screen_name] and write its record						*/
		/* -------------------------------------------------------------------- */
		for( j = i + 1 ; j < twfs_index ; j++ )
		{
			int		k;
			bool	match;

			match = false;

			if( new_send_dms.dms[ j ] != 0x00 )
			{
				continue;
			}

			for( k = 0 ; k < DEF_TWFS_SNAME_FIELD ; k++ )
			{
				char	s;

				if( dm_sname_len + 1 < k )
				{
					break;
				}

				if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
				{
					s = *( twfs_file->tl
						   + twfs_file->size
						   + ( DEF_TWFS_TL_RECORD_LEN * j )
						   + DEF_TWFS_OFFSET_SNAME_FIELD
						   + k );
				}
				else
				{
					s = *( twfs_file->tl
						   + twfs_file->size
						   + DEF_TWFS_OFFSET_BODY_OF_TL
						   + ( DEF_TWFS_TL_RECORD_LEN * j )
						   + DEF_TWFS_OFFSET_SNAME_FIELD
						   + k );
				}

				if( s == 0x00 && dm_sname[ k ] == '\0' )
				{
					match = true;
					break;
				}
				else if( s != dm_sname[ k ] )
				{
					break;
				}
			}

			if( match )
			{
				int		this_record_text_len = 0;
				if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < dm_file_stat.st_size )
				{
					dm_file_offset = dm_file_stat.st_size;
				}
				else
				{
					dm_file_offset = DEF_TWFS_OFFSET_BODY_OF_TL;
				}

				if( dm_field == 's' )
				{
					dm_file_offset += ( DEF_TWFS_TL_RECORD_LEN * snd_index++ );
				}
				else
				{
					dm_file_offset += ( DEF_TWFS_TL_RECORD_LEN * rcp_index++ );
				}

				/* update [authed_name]/direct_message/[screen_name]/message file		*/
				if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
				{
					pwriteFile( fd,
								twfs_file->tl
								 + twfs_file->size
								 + ( DEF_TWFS_TL_RECORD_LEN * j ),
								DEF_TWFS_TL_RECORD_LEN,
								dm_file_offset );
				}
				else
				{
					pwriteFile( fd,
								twfs_file->tl
								 + twfs_file->size
								 + DEF_TWFS_OFFSET_BODY_OF_TL
								 + ( DEF_TWFS_TL_RECORD_LEN * j ),
								DEF_TWFS_TL_RECORD_LEN,
								dm_file_offset );
				}
				/* ------------------------------------------------------------ */
				/* size [authed_name]/direct_message/[screen_name]/message file	*/
				/* ------------------------------------------------------------ */
				for( int l = 0 ; l < DEF_TWFS_TEXT_LEN_FIELD ; l++ )
				{
					if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
					{
						dm_text_len = *( twfs_file->tl
										 + twfs_file->size
										 + ( DEF_TWFS_TL_RECORD_LEN * j )
										 + DEF_TWFS_OFFSET_TEXT_LEN_FIELD
										 + l );
					}
					else
					{
						dm_text_len = *( twfs_file->tl
										 + twfs_file->size
										 + DEF_TWFS_OFFSET_BODY_OF_TL
										 + ( DEF_TWFS_TL_RECORD_LEN * j )
										 + DEF_TWFS_OFFSET_TEXT_LEN_FIELD
										 + l );
					}

					if( dm_text_len == 0x00 )
					{
						break;
					}
					this_record_text_len = this_record_text_len * 10
											+ ( dm_text_len - '0' );
				}
				dm_text_total += this_record_text_len;
				/* update saved flag											*/
				new_send_dms.dms[ j ] = 's';
			}
		}

		result = snprintf( header, sizeof( header ), "%zu", dm_text_total );

		for( j = result ; j < ( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN - 1 ) ; j++ )
		{
			header[ j ] = 0x00;
		}
		header[ j ] = '\n';

		pwriteFile( fd, header, DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN, 0 );

		closeFile( fd );
	}

	/* update total timeline file size											*/
	if( new_send_dms.num_dms + new_recp_dms.num_dms )
	{
		result = snprintf( twfs_file->tl,
						   DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN,
						   "%zu", twfs_file->tl_size );

		for( i = result ; i < ( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN - 1 ) ; i++ )
		{
			*( twfs_file->tl + i ) = 0x00;
		}
		*( twfs_file->tl + i ) = '\n';
	
		if( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN < twfs_file->size )
		{
			/* actual timeline file is updated									*/
			twfs_file->size += ( DEF_TWFS_TL_RECORD_LEN * twfs_index );
		}
		else
		{
			/* actual timeline file is updated									*/
			twfs_file->size = DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN;
			twfs_file->size += ( DEF_TWFS_TL_RECORD_LEN * twfs_index );
		}
	}

	if( twfs_index )
	{
		msync( twfs_file->tl, twfs_file->size, MS_SYNC );
	}

	munmap( new_send_dms.dms, new_send_dms.unmap_length );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readUsers
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
			   const char *slug )
{
	typedef enum
	{
		E_LIST_PREV_CUR,			// previous_cursor_str
		E_LIST_NEXT_CUR,			// next_cursor_str

		E_USR_USR,					// user/
		E_USR_CREATED_AT,			// user/created_at
		E_USR_DESCRIPTION,			// user/description [nullable]
		E_USR_FAV_CNT,				// user/favourites_count
		E_USR_FOLLOWING,			// user/following [nullable]
		E_USR_FOLLOWERS_CNT,		// user/followers_count
		E_USR_FRIENDS_CNT,			// user/friends_count
		E_USR_ID_STR,				// user/id_str
		E_USR_LISTED_CNT,			// user/listed_count
		E_USR_LOCATION,				// user/location [nullable]
		E_USR_NAME,					// user/name
		E_USR_SNAME,				// user/screen_name
		E_USR_STATUSES_CNT,			// user/statuses_count
		E_USR_URL,					// user/url [nullable]
		E_USR_VERIFIED,				// user/verified
		E_LIST_NUM,
	} E_OBJ;

	struct new_usr
	{
		uint8_t		*users;			// buffer for json object
		int			num_usrs;		// number of users
		size_t		length;			// length of all text
		int			text_len;		// current text length
	};

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	int					fd;
	struct new_usr		new_usr = { NULL, 0, 0, 0};
	struct jnode		root;
	struct jnode		node[ E_LIST_NUM ];
	char				usr_created_at[ ]	= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_CREATED_AT;
	char				usr_description[ ]	= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_DESCRIPTION;
	char				usr_fav_cnt[ ]		= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_FAV_CNT;
	char				usr_follwing[ ]		= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_FOLLOWING;
	char				usr_followers_cnt[ ]= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_FOLLOWERS_CNT;
	char				usr_friends_cnt[ ]	= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_FRIENDS_CNT;
	char				usr_id_str[ ]		= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				usr_listed_cnt[ ]	= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_LISTED_CNT;
	char				usr_location[ ]		= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_LOCATION;
	char				usr_name[ ]			= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_NAME;
	char				usr_sname[ ]		= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_SNAME;
	char				usr_statuses_cnt[ ]	= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_STATUSES_CNT;
	char				usr_url[ ]			= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_URL;
	char				usr_verified[ ]		= "/" DEF_TWAPI_OBJ_USRS
											  "/" DEF_TWAPI_OBJ_USR_VERIFIED;

	/* ------------------------------------------------------------------------ */
	/* request get following/follower list										*/
	/* ------------------------------------------------------------------------ */
	switch( request )
	{
	case	E_TWFS_REQ_READ_FOLLOWING_LIST:
	default:
		result = getFollowingList( session, &hctx, screen_name, cursor );
		if( result < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_FOLLOWER_LIST:
		result = getFollowerList( session, &hctx, screen_name, cursor );
		if( result < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_AUTH_BLOCK_LIST:
		result = getBlockList( session, &hctx, cursor );
		if( result < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_LISTS_MEMBERS:
		result = getListsMembers( session, &hctx, slug, screen_name, cursor );
		if( result < 0 )
		{
			return( result );
		}
		break;
	case	E_TWFS_REQ_READ_LISTS_SUBSCRIBERS:
		result = getListsSubscribers( session, &hctx, slug, screen_name, cursor );
		if( result < 0 )
		{
			return( result );
		}
		break;
	}

	if( hctx.content_length == 0 )
	{
		return( 0 );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for new users														*/
	/* ------------------------------------------------------------------------ */
	new_usr.users = mmap( NULL, hctx.content_length + 1,
						   PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

	if( new_usr.users == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )usr_created_at, 1 ) < 0 )
			{
				break;
			}
		}

		logMessage( "cannot map [new_tws]%s\n", strerror( errno ) );

		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive tweets of json structure											*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_usr.users,
						hctx.content_length ) < 0 )
	{
		munmap( new_usr.users, hctx.content_length );
		//disconnectSSLServer( session );
		return( -1 );
	}

	//disconnectSSLServer( session );

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );
	
	/* previous_cursor_str														*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LIST_PREV_CUR_STR,
					 &node[ E_LIST_PREV_CUR		]	);
	/* next_cursor_str															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LIST_NEXT_CUR_STR,
					 &node[ E_LIST_NEXT_CUR		]	);
	/* user/																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USRS,
					 &node[ E_USR_USR			]	);
	/* user/created_at															*/
	insertJsonNodes( &root, usr_created_at,
					 &node[ E_USR_CREATED_AT	]	);
	/* user/description															*/
	insertJsonNodes( &root, usr_description,
					 &node[ E_USR_DESCRIPTION	]	);
	/* user/favourites_count													*/
	insertJsonNodes( &root, usr_fav_cnt,
					 &node[ E_USR_FAV_CNT		]	);
	/* user/following															*/
	insertJsonNodes( &root, usr_follwing,
					 &node[ E_USR_FOLLOWING		]	);
	/* user/followers_count														*/
	insertJsonNodes( &root, usr_followers_cnt,
					 &node[ E_USR_FOLLOWERS_CNT	]	);
	/* user/friends_count														*/
	insertJsonNodes( &root, usr_friends_cnt,
					 &node[ E_USR_FRIENDS_CNT	]	);
	/* user/id_str																*/
	insertJsonNodes( &root, usr_id_str,
					 &node[ E_USR_ID_STR		]	);
	/* user/listed_count														*/
	insertJsonNodes( &root, usr_listed_cnt,
					 &node[ E_USR_LISTED_CNT	]	);
	/* user/location															*/
	insertJsonNodes( &root, usr_location,
					 &node[ E_USR_LOCATION		]	);
	/* user/name																*/
	insertJsonNodes( &root, usr_name,
					 &node[ E_USR_NAME			]	);
	/* user/screen_name															*/
	insertJsonNodes( &root, usr_sname,
					 &node[ E_USR_SNAME			]	);
	/* user/statuses_count														*/
	insertJsonNodes( &root, usr_statuses_cnt,
					 &node[ E_USR_STATUSES_CNT	]	);
	/* user/url																	*/
	insertJsonNodes( &root, usr_url,
					 &node[ E_USR_URL			]	);
	/* user/verified															*/
	insertJsonNodes( &root, usr_verified,
					 &node[ E_USR_VERIFIED		]	);

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* allocat space for next_cursor, previsou_cursor							*/
	/* ------------------------------------------------------------------------ */
	new_usr.length = DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
					 + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN;

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		//char	buffer[ 1024 ];
		char	buffer[ DEF_PROF_TEXT_LEN + 1 ];
		int		buf_len;
		int		ana_result;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_usr.users,
								  &ana, &root,
								  1,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_LIST_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( new_usr.users, hctx.content_length + 1 );
			return( -1 );
		}

		if( ( node[ E_USR_CREATED_AT ].value	== NULL ) ||
			( node[ E_USR_FAV_CNT ].value		== NULL ) ||
			( node[ E_USR_FOLLOWERS_CNT ].value	== NULL ) ||
			( node[ E_USR_FRIENDS_CNT ].value	== NULL ) ||
			( node[ E_USR_LISTED_CNT ].value	== NULL ) ||
			( node[ E_USR_STATUSES_CNT ].value	== NULL ) ||
			( node[ E_USR_SNAME ].value			== NULL ) ||
			( node[ E_USR_ID_STR ].value		== NULL ) ||
			( node[ E_USR_NAME ].value			== NULL ) )
			//( node[ E_LIST_PREV_CUR ].value		== NULL ) ||
			//( node[ E_LIST_NEXT_CUR ].value		== NULL ) )
		{
			logMessage( "\ndetected json null value\n" );
			if( node[ E_LIST_NEXT_CUR ].value	!= NULL )
			{
				int		res_len;
				/* first new_usr.users area are temporaly saved next cursor			*/
				memcpy( new_usr.users,
						node[ E_LIST_NEXT_CUR ].value,
						node[ E_LIST_NEXT_CUR ].length );
				/* fill residual space											*/
				res_len = DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD
						  - node[ E_LIST_NEXT_CUR ].length;
				memset( new_usr.users
						+ node[ E_LIST_NEXT_CUR ].length,
						0x00, res_len );
				*( new_usr.users
				   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ) = '\n';
			}
			if( node[ E_LIST_PREV_CUR ].value	!= NULL )
			{
				int		res_len;
				/* first new_usr.users area are temporaly saved next cursor		*/
				memcpy( new_usr.users
						+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN,
						node[ E_LIST_PREV_CUR ].value,
						node[ E_LIST_PREV_CUR ].length );
				/* fill residual space											*/
				res_len = DEF_TWFS_HEAD_FF_PREV_CUR_FIELD
						  - node[ E_LIST_PREV_CUR ].length;
				memset( new_usr.users
						+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
						+ node[ E_LIST_PREV_CUR ].length,
						0x00, res_len );
				*( new_usr.users
				   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
				   + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD ) = '\n';
			}

			for( i = 0 ; i < E_LIST_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}

			if( ana_result == 0 )
			{
				logMessage( "analyzed finish(%d)!!!\n", new_usr.num_usrs );
				if( new_usr.num_usrs == 0 )
				{
					munmap( new_usr.users, hctx.content_length + 1 );
					return( -1 );
				}
				break;
			}
			logMessage( "continue \n" );
			continue;
		}

		for( i = 0 ; i < E_LIST_NUM ; i++ )
		{
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]												*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_ACCOUNT );

		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserHomeDirectory( node[ E_USR_SNAME ].value, true );

			if( result < 0 )
			{
				for( i = 0 ; i < E_LIST_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}

				if( ana_result == 0 )
				{
					if( new_usr.num_usrs == 0 )
					{
						munmap( new_usr.users, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		/* -------------------------------------------------------------------- */
		/* link to [screen_name]/account										*/
		/* -------------------------------------------------------------------- */
		{
			char	sym_path[ DEF_TWFS_PATH_MAX ];

			switch( request )
			{
			case	E_TWFS_REQ_READ_FOLLOWING_LIST:
				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_FRIENDS,
						  node[ E_USR_SNAME ].value );
				snprintf( sym_path, sizeof( sym_path ), "../../%s",
						  node[ E_USR_SNAME ].value );

				symlink( sym_path, buffer );
				break;
			case	E_TWFS_REQ_READ_FOLLOWER_LIST:
				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_FOLLOWERS,
						  node[ E_USR_SNAME ].value );
				snprintf( sym_path, sizeof( sym_path ), "../../%s",
						  node[ E_USR_SNAME ].value );

				symlink( sym_path, buffer );
				break;
			case	E_TWFS_REQ_READ_AUTH_BLOCK_LIST:
				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_BLOCKS,
						  node[ E_USR_SNAME ].value );
				snprintf( sym_path, sizeof( sym_path ), "../../%s",
						  node[ E_USR_SNAME ].value );

				symlink( sym_path, buffer );
				break;
			case	E_TWFS_REQ_READ_LISTS_MEMBERS:
				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  slug,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  node[ E_USR_SNAME ].value );
				snprintf( sym_path, sizeof( sym_path ), "../../../../../%s",
						  node[ E_USR_SNAME ].value );

				symlink( sym_path, buffer );
				break;
			case	E_TWFS_REQ_READ_LISTS_SUBSCRIBERS:
				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  slug,
						  DEF_TWFS_PATH_DIR_LNAME_MEM,
						  node[ E_USR_SNAME ].value );
				snprintf( sym_path, sizeof( sym_path ), "../../../../../%s",
						  node[ E_USR_SNAME ].value );

				symlink( sym_path, buffer );
				break;
			default:
				break;
			}
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/account/profile								*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_ACCOUNT,
				  DEF_TWFS_PATH_PROFILE );

		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			int		res_len;
			char	now_following;
			char	followed_by;
			char	verified;
			buf_len = 0;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( &buffer[ buf_len ],
								sizeof( buffer ) - buf_len + 1,
								"%s\n", DEF_PROF_SEPARATOR );
			/* ---------------------------------------------------------------- */
			/* id																*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_ID,
								 node[ E_USR_ID_STR ].value );
			/* copy id															*/
			memcpy( new_usr.users + new_usr.length,
					node[ E_USR_ID_STR ].value,
					node[ E_USR_ID_STR ].length );
			new_usr.length += node[ E_USR_ID_STR ].length;
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_PROF_ID_FIELD - node[ E_USR_ID_STR ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			memset( new_usr.users + new_usr.length, 0x00, res_len );
			new_usr.length += res_len;
			*( new_usr.users + new_usr.length++ ) = 0x00;

			/* ---------------------------------------------------------------- */
			/* created at														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s\n",
								 DEF_PROF_CREATED_AT,
								 node[ E_USR_CREATED_AT ].value );
			/* ---------------------------------------------------------------- */
			/* name																*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_NAME,
								 node[ E_USR_NAME ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_PROF_NAME_FIELD - node[ E_USR_NAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* screen name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_SNAME,
								 node[ E_USR_SNAME ].value );
			/* copy screen name													*/
			memcpy( new_usr.users + new_usr.length,
					node[ E_USR_SNAME ].value,
					node[ E_USR_SNAME ].length );
			new_usr.length += node[ E_USR_SNAME ].length;
			/* fill residual space of sreccn name in 'fd' file					*/
			res_len = DEF_PROF_SNAME_FIELD - node[ E_USR_SNAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			memset( new_usr.users + new_usr.length, 0x00, res_len );
			new_usr.length += res_len;
			*( new_usr.users + new_usr.length++ ) = 0x00;

			/* ---------------------------------------------------------------- */
			/* location															*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_LOCATION ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_PROF_LOCATION,
									 node[ E_USR_LOCATION ].value );


				/* fill residual space of location in 'fd' file					*/
				res_len = DEF_PROF_LOCATION_FIELD - node[ E_USR_LOCATION ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_PROF_LOCATION );
				memset( &buffer[ buf_len ], ' ', DEF_PROF_LOCATION_FIELD );
				buf_len += DEF_PROF_LOCATION_FIELD;
			}
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* url																*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_URL ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_PROF_URL,
									 node[ E_USR_URL ].value );

				/* fill residual space of url in 'fd' file						*/
				res_len = DEF_PROF_URL_FIELD - node[ E_USR_URL ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_PROF_URL );
				memset( &buffer[ buf_len ], ' ', DEF_PROF_URL_FIELD );
				buf_len += DEF_PROF_URL_FIELD;
			}
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* description														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_DESCRIPTION ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_PROF_DESCRIPTION,
									 node[ E_USR_DESCRIPTION ].value );
				/* fill residual space of description in 'fd' file				*/
				res_len = DEF_PROF_DESC_FIELD - node[ E_USR_DESCRIPTION ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
				buffer[ buf_len++ ] = '\n';
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_PROF_DESCRIPTION );
				memset( &buffer[ buf_len ], ' ', DEF_PROF_DESC_FIELD );
				buf_len += DEF_PROF_DESC_FIELD;
				buffer[ buf_len++ ] = '\n';
			}
			
			/* '\n' is separator												*/
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* tweets															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_STATUSES,
								 node[ E_USR_STATUSES_CNT ].value );

			/* fill residual space of tweets in 'fd' file						*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_STATUSES_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* favorites														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_FAVS,
								 node[ E_USR_FAV_CNT ].value );

			/* fill residual space of favorites in 'fd' file					*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_FAV_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* following														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_FOLLOWING,
								 node[ E_USR_FRIENDS_CNT ].value );

			/* fill residual space of following in 'fd' file					*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_FRIENDS_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* followers														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_FOLLOWERS,
								 node[ E_USR_FOLLOWERS_CNT ].value );

			/* fill residual space of following in 'fd' file					*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_FOLLOWERS_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* listed															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_LISTED_CNT,
								 node[ E_USR_LISTED_CNT ].value );

			/* fill residual space of listed in 'fd' file						*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_LISTED_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* '\n' is separator												*/
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* follwing [*]														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_FOLLOWING ].value != NULL )
			{
				if( ( node[ E_USR_FOLLOWING ].value[ 0 ] == 't' ) ||
					( node[ E_USR_FOLLOWING ].value[ 0 ] == 'T' ) )
				{
					now_following = '*';
				}
				else
				{
					now_following = ' ';
				}
			}
			else
			{
				now_following = ' ';
			}
			
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s [%c]",
								 DEF_PROF_NOW_FOLLOWING,
								 now_following );
			/* fill space after following item								*/
			memset( &buffer[ buf_len ], ' ', DEF_PROF_SPACE_FR_AND_FL );
			buf_len += DEF_PROF_SPACE_FR_AND_FL;
			
			/* ---------------------------------------------------------------- */
			/* followed [*]														*/
			/* ---------------------------------------------------------------- */
			followed_by = ' ';

			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s [%c]",
								 DEF_PROF_FOLLOWED_BY,
								 followed_by );
			/* fill space after followed item									*/
			memset( &buffer[ buf_len ], ' ', DEF_PROF_SPACE_FL_AND_VERI );
			buf_len += DEF_PROF_SPACE_FL_AND_VERI;

			/* ---------------------------------------------------------------- */
			/* verified [*]														*/
			/* ---------------------------------------------------------------- */
			if( ( node[ E_USR_VERIFIED ].value[ 0 ] == 't' ) ||
				( node[ E_USR_VERIFIED ].value[ 0 ] == 'T' ) )
			{
				verified = '*';
			}
			else
			{
				verified = ' ';
			}

			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s [%c]\n",
								 DEF_PROF_VERIFIED,
								 verified );

			/* ---------------------------------------------------------------- */
			/* text length is determined ( the size of each text of				*/
			/* ffollower/fllowing list file may be always fixed )				*/
			/* ---------------------------------------------------------------- */
			new_usr.text_len = buf_len;
			twfs_file->tl_size += new_usr.text_len;

			/* ---------------------------------------------------------------- */
			/* write to profle file												*/
			/* ---------------------------------------------------------------- */
			writeFile( fd, ( const void* )buffer, buf_len );
			//logMessage( "%s\n", buffer );
			//buffer[ buf_len ] = '\0';
			//logMessage( "%s\n", buffer );

			closeFile( fd );

			/* ---------------------------------------------------------------- */
			/* update list file													*/
			/* ---------------------------------------------------------------- */
			*( new_usr.users + new_usr.length++ ) = 'L';
			*( new_usr.users + new_usr.length++ ) = 0x00;
			*( new_usr.users + new_usr.length++ ) = '0';
			*( new_usr.users + new_usr.length++ ) = '0';
			*( new_usr.users + new_usr.length++ ) = 0x00;

			buf_len = snprintf( ( char* )( new_usr.users + new_usr.length ),
								DEF_TWFS_TEXT_LEN_FIELD,
								"%d", buf_len );

			new_usr.length += buf_len;
			
			/* fill residual space											*/
			buf_len = DEF_TWFS_TEXT_LEN_FIELD - buf_len;
			memset( new_usr.users + new_usr.length, 0x00, buf_len );
			new_usr.length += buf_len;
			*( new_usr.users + new_usr.length++ ) = '\n';

			new_usr.num_usrs++;
		}

		if( node[ E_LIST_NEXT_CUR ].value	!= NULL )
		{
			int		res_len;
			/* first new_usr.users area are temporaly saved next cursor			*/
			memcpy( new_usr.users,
					node[ E_LIST_NEXT_CUR ].value,
					node[ E_LIST_NEXT_CUR ].length );
			/* fill residual space												*/
			res_len = DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD
					  - node[ E_LIST_NEXT_CUR ].length;
			memset( new_usr.users
					+ node[ E_LIST_NEXT_CUR ].length,
					0x00, res_len );
			*( new_usr.users
			   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ) = '\n';
		}
		if( node[ E_LIST_PREV_CUR ].value	!= NULL )
		{
			int		res_len;
			/* first new_usr.users area are temporaly saved next cursor			*/
			memcpy( new_usr.users
					+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN,
					node[ E_LIST_PREV_CUR ].value,
					node[ E_LIST_PREV_CUR ].length );
			/* fill residual space												*/
			res_len = DEF_TWFS_HEAD_FF_PREV_CUR_FIELD
					  - node[ E_LIST_PREV_CUR ].length;
			memset( new_usr.users
					+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
					+ node[ E_LIST_PREV_CUR ].length,
					0x00, res_len );
			*( new_usr.users
			   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
			   + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD ) = '\n';
		}

		for( i = 0 ; i < E_LIST_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_usr.num_usrs == 0 )
			{
				munmap( new_usr.users, hctx.content_length + 1 );
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}
	logMessage( "user num is :%d\n", new_usr.num_usrs );
#if 1
	if( DEF_TWFS_HEAD_FF_LEN < twfs_file->size )
	{
		result = ftruncate( twfs_file->fd,
							twfs_file->size
							+ ( DEF_TWFS_TL_RECORD_LEN * new_usr.num_usrs ) );
	}
	else
	{
		result = ftruncate( twfs_file->fd,
							DEF_TWFS_HEAD_FF_LEN
							+ ( DEF_TWFS_TL_RECORD_LEN * new_usr.num_usrs ) );
	}

	if( result < 0 )
	{
		munmap( new_usr.users, hctx.content_length + 1 );
		return( -1 );
	}

	*( new_usr.users + new_usr.length ) = '\0';

	/* ------------------------------------------------------------------------ */
	/* update next/prev cursor													*/
	/* ------------------------------------------------------------------------ */
	memcpy( twfs_file->tl + DEF_TWFS_OFFSET_FF_NEXT_CUR_FIELD,
			new_usr.users,
			DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
			+ DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN );
	
	/* ------------------------------------------------------------------------ */
	/* body of a timeline file is updated										*/
	/* ------------------------------------------------------------------------ */
	if( DEF_TWFS_HEAD_FF_LEN < twfs_file->size )
	{
		memcpy( twfs_file->tl + twfs_file->size,
				new_usr.users
				+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
				+ DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN,
				new_usr.num_usrs * DEF_TWFS_TL_RECORD_LEN );
	}
	else
	{
		memcpy( twfs_file->tl + DEF_TWFS_OFFSET_BODY_OF_FF,
				new_usr.users
				+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
				+ DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN,
				new_usr.num_usrs * DEF_TWFS_TL_RECORD_LEN );
	}
	/* ------------------------------------------------------------------------ */
	/* update total follwer/following list file size							*/
	/* ------------------------------------------------------------------------ */
	if( new_usr.num_usrs )
	{
		result = snprintf( twfs_file->tl,
						   DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN,
						   "%zu", twfs_file->tl_size );

		for( i = result ; i < ( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN - 1 ) ; i++ )
		{
			*( twfs_file->tl + i ) = 0x00;
		}
		*( twfs_file->tl + i ) = '\n';
	
		if( DEF_TWFS_HEAD_FF_LEN < twfs_file->size )
		{
			/* actual timeline file is updated									*/
			twfs_file->size += ( DEF_TWFS_TL_RECORD_LEN * new_usr.num_usrs );
		}
		else
		{
			/* actual timeline file is updated									*/
			twfs_file->size = DEF_TWFS_HEAD_FF_LEN;
			twfs_file->size += ( DEF_TWFS_TL_RECORD_LEN * new_usr.num_usrs );
		}
	

		msync( twfs_file->tl, twfs_file->size, MS_SYNC );
	}
#endif
	munmap( new_usr.users, hctx.content_length + 1 );
	logMessage( "twfs_file->size :%d\n", twfs_file->size );

	return( 0 );
}


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
int symlinkRetweet( struct ssl_session *session, const char *rtw_id )
{
	typedef enum
	{
		E_TW_ID_STR,				// id_str
		E_TW_RTW_STATUS,			// retweeted_status/
		E_TW_RTW_STATUS_ID_STR,		// retweeted_status/id_str
		E_TW_RTW_STATUS_USR,		// retweeted_status/user/
		E_TW_RTW_STATUS_USR_ID_STR,	// retweeted_status/user/id_str
		E_TW_RTW_STATUS_USR_SNAME,	// retweeted_status/user/sname
		E_TW_USER,					// user/
		E_TW_USR_ID_STR,			// user/id_str
		E_TW_USR_SNAME,				// user/screen_name
		E_TW_NUM,
	} E_OBJ;

	uint8_t			*tweets;

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	struct jnode		root;
	struct jnode		node[ E_TW_NUM ];
	char				rs_id_str[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_TW_ID_STR;
	char				rs_usr[ ]			= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR;
	char				rs_usr_idstr[ ]		= "/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				rs_usr_screen_name[ ]="/" DEF_TWAPI_OBJ_TW_RTW_STATUS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_SNAME;
	char				usr_idstr[ ]		= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				usr_screen_name[ ]	= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_SNAME;
	

	result = retweet( session, &hctx, rtw_id );


	if( hctx.content_length == 0 )
	{
		return( 0 );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for new tweets													*/
	/* ------------------------------------------------------------------------ */
	tweets = mmap( NULL, hctx.content_length + 1,
				   PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

	if( tweets == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )rs_id_str, 1 ) < 0 )
			{
				break;
			}
		}

		logMessage( "cannot map [tweets]%s\n", strerror( errno ) );

		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive tweets of json structure											*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )tweets,
						hctx.content_length ) < 0 )
	{
		munmap( tweets, hctx.content_length + 1 );
		//disconnectSSLServer( session );
		return( -1 );
	}

	//disconnectSSLServer( session );

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );

	/* id_str																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_ID_STR,
					 &node[ E_TW_ID_STR					]	);
	/* retweeted_status/														*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_RTW_STATUS,
					 &node[ E_TW_RTW_STATUS				]	);
	/* retweeted_status/id_str													*/
	insertJsonNodes( &root, rs_id_str,
					 &node[ E_TW_RTW_STATUS_ID_STR		]	);
	/* retweeted_status/usr/													*/
	insertJsonNodes( &root, rs_usr,
					 &node[ E_TW_RTW_STATUS_USR			]	);
	/* retweeted_status/usr/id_str												*/
	insertJsonNodes( &root, rs_usr_idstr,
					 &node[ E_TW_RTW_STATUS_USR_ID_STR	]	);
	/* retweeted_status/usr/screen_name											*/
	insertJsonNodes( &root, rs_usr_screen_name,
					 &node[ E_TW_RTW_STATUS_USR_SNAME	]	);
	/* user/																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_TW_USER,
					 &node[ E_TW_USER					]	);
	/* user/id_str																*/
	insertJsonNodes( &root, usr_idstr,
					 &node[ E_TW_USR_ID_STR				]	);
	/* user/screen_name															*/
	insertJsonNodes( &root, usr_screen_name,
					 &node[ E_TW_USR_SNAME				]	);

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		char	buffer[ 1024 ];
		char	new_path[ DEF_TWFS_PATH_MAX ];
		int		ana_result;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )tweets,
								  &ana, &root,
								  0,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_TW_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( tweets, hctx.content_length + 1 );
			return( -1 );
		}

		if( ( node[ E_TW_USR_SNAME ].value				== NULL ) ||
			( node[ E_TW_ID_STR ].value					== NULL ) ||
			( node[ E_TW_RTW_STATUS_USR_SNAME ].value	== NULL ) ||
			( node[ E_TW_RTW_STATUS_ID_STR ].value		== NULL ) )
		{
			for( i = 0 ; i < E_TW_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}

			if( ana_result == 0 )
			{
				munmap( tweets, hctx.content_length + 1 );
				return( 0 );
			}
			continue;
		}

		for( i = 0 ; i < E_TW_NUM ; i++ )
		{
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}

		snprintf( new_path, sizeof( new_path ), "%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_TW_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_RETWEET,
				  node[ E_TW_ID_STR ].value );

		logMessage( "readTweet from link:%s\n", new_path );

		snprintf( buffer, DEF_TWFS_PATH_MAX, "../../%s/%s/%s",
				  node[ E_TW_RTW_STATUS_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_STATUS,
				  node[ E_TW_RTW_STATUS_ID_STR ].value );

		logMessage( "readTweet to link:%s\n", buffer );

		result = symlink( buffer, new_path );

		for( i = 0 ; i < E_TW_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			result = 0;
			logMessage( "analysis is done!\n" );
			munmap( tweets, hctx.content_length + 1 );
			break;
		}
	}

	munmap( tweets, hctx.content_length + 1 );

	return( result );
}

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
			  const char *cursor )
{
	typedef enum
	{
		E_LIST_PREV_CUR,			// previous_cursor_str
		E_LIST_NEXT_CUR,			// next_cursor_str

		E_LISTS,					// lists/
		E_LISTS_NAME,				// lists/name
		E_LISTS_SLUG,				// lists/slug
		E_LISTS_ID,					// lists/id_str
		E_LISTS_SUB_CNT,			// lists/subscriber_count
		E_LISTS_MEM_CNT,			// lists/member_count
		E_LISTS_DESC,				// lists/description
		E_LISTS_USR,				// lists/user
		E_LISTS_USR_ID_STR,			// lists/user/id_str
		E_LISTS_USR_NAME,			// lists/user/name
		E_LISTS_USR_SNAME,			// lists/user/screen_name
		E_LISTS_NUM,
	} E_OBJ;

	struct new_lst
	{
		uint8_t		*lists;			// buffer for json object
		int			num_lsts;		// number of lists
		size_t		length;			// length of all text
		int			text_len;		// current text length
	};

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	int					fd;
	struct new_lst		new_lst = { NULL, 0, 0, 0 };
	struct jnode		root;
	struct jnode		node[ E_LISTS_NUM ];
	char				lst_name[ ]			= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_LISTS_NAME;
	char				lst_slug[ ]			= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_LISTS_SLUG;
	char				lst_id_str[ ]		= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_LISTS_ID_STR;
	char				lst_sub_cnt[ ]		= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_LISTS_SUB_CNT;
	char				lst_mem_cnt[ ]		= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_LISTS_MEM_CNT;
	char				lst_desc[ ]			= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_LISTS_DESC;
	char				lst_usr[ ]			= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_USR;
	char				lst_usr_id_str[ ]	= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				lst_usr_name[ ]		= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_NAME;
	char				lst_usr_sname[ ]	= "/" DEF_TWAPI_OBJ_LISTS
											  "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_SNAME;

	/* ------------------------------------------------------------------------ */
	/* request get lists/{subscriptions, memberships, ownerships}				*/
	/* ------------------------------------------------------------------------ */
	switch( request )
	{
	case	E_TWFS_REQ_READ_LISTS_SUB_LIST:
		result = getListsSubscriptions( session, &hctx, screen_name, cursor );
		break;
	case	E_TWFS_REQ_READ_LISTS_OWN_LIST:
		result = getListsOwnerships( session, &hctx, screen_name, cursor );
		break;
	case	E_TWFS_REQ_READ_LISTS_ADD_LIST:
		result = getListsMemberships( session, &hctx, screen_name, cursor );
		break;
	default:
		return( -1 );
	}

	if( result < 0 )
	{
		return( result );
	}

	if( hctx.content_length == 0 )
	{
		return( 0 );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for new lists														*/
	/* ------------------------------------------------------------------------ */
	new_lst.lists = mmap( NULL, hctx.content_length + 1,
						  PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

	if( new_lst.lists == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )lst_name, 1 ) < 0 )
			{
				break;
			}
		}

		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive tweets of json structure											*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_lst.lists,
						hctx.content_length ) < 0 )
	{
		munmap( new_lst.lists, hctx.content_length + 1 );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );

	/* previous_cursor_str														*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LIST_PREV_CUR_STR,
					 &node[ E_LIST_PREV_CUR		]	);
	/* next_cursor_str															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LIST_NEXT_CUR_STR,
					 &node[ E_LIST_NEXT_CUR		]	);
	/* lists/																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS,
					 &node[ E_LISTS				]	);
	/* lists/name																*/
	insertJsonNodes( &root, lst_name,
					 &node[ E_LISTS_NAME		]	);
	/* lists/slug																*/
	insertJsonNodes( &root, lst_slug,
					 &node[ E_LISTS_SLUG		]	);
	/* lists/id_str																*/
	insertJsonNodes( &root, lst_id_str,
					 &node[ E_LISTS_ID			]	);
	/* lists/subscriber_count													*/
	insertJsonNodes( &root, lst_sub_cnt,
					 &node[ E_LISTS_SUB_CNT		]	);
	/* lists/member_count														*/
	insertJsonNodes( &root, lst_mem_cnt,
					 &node[ E_LISTS_MEM_CNT		]	);
	/* lists/descriptions														*/
	insertJsonNodes( &root, lst_desc,
					 &node[ E_LISTS_DESC		]	);
	/* lists/user																*/
	insertJsonNodes( &root, lst_usr,
					 &node[ E_LISTS_USR			]	);
	/* lists/user/id_str														*/
	insertJsonNodes( &root, lst_usr_id_str,
					 &node[ E_LISTS_USR_ID_STR	]	);
	/* lists/user/name															*/
	insertJsonNodes( &root, lst_usr_name,
					 &node[ E_LISTS_USR_NAME	]	);
	/* lists/user/screen_name													*/
	insertJsonNodes( &root, lst_usr_sname,
					 &node[ E_LISTS_USR_SNAME	]	);

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* allocat space for next_cursor, previsou_cursor							*/
	/* ------------------------------------------------------------------------ */
	new_lst.length = DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
					 + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN;

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		//char	buffer[ 1024 ];
		char	buffer[ DEF_LDESC_TEXT_LEN ];
		int		buf_len;
		int		ana_result;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_lst.lists,
								  &ana, &root,
								  1,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_LISTS_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( new_lst.lists, hctx.content_length + 1 );
			return( -1 );
		}

		if( ( node[ E_LISTS_ID ].value			== NULL ) ||
			( node[ E_LISTS_SLUG ].value		== NULL ) ||
			( node[ E_LISTS_USR_ID_STR ].value	== NULL ) ||
			( node[ E_LISTS_USR_NAME ].value	== NULL ) ||
			( node[ E_LISTS_USR_SNAME ].value	== NULL ) ||
			( node[ E_LISTS_SUB_CNT ].value		== NULL ) ||
			( node[ E_LISTS_MEM_CNT ].value		== NULL ) )
		{
			logMessage( "\ndetected json null value\n" );
			if( node[ E_LIST_NEXT_CUR ].value	!= NULL )
			{
				int		res_len;
				/* first new_usr.users area are temporaly saved next cursor		*/
				memcpy( new_lst.lists,
						node[ E_LIST_NEXT_CUR ].value,
						node[ E_LIST_NEXT_CUR ].length );
				/* fill residual space											*/
				res_len = DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD
						  - node[ E_LIST_NEXT_CUR ].length;
				memset( new_lst.lists
						+ node[ E_LIST_NEXT_CUR ].length,
						0x00, res_len );
				*( new_lst.lists
				   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ) = '\n';
			}
			if( node[ E_LIST_PREV_CUR ].value	!= NULL )
			{
				int		res_len;
				/* first new_lst.lists area are temporaly saved next cursor		*/
				memcpy( new_lst.lists
						+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN,
						node[ E_LIST_PREV_CUR ].value,
						node[ E_LIST_PREV_CUR ].length );
				/* fill residual space											*/
				res_len = DEF_TWFS_HEAD_FF_PREV_CUR_FIELD
						  - node[ E_LIST_PREV_CUR ].length;
				memset( new_lst.lists
						+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
						+ node[ E_LIST_PREV_CUR ].length,
						0x00, res_len );
				*( new_lst.lists
				   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
				   + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD ) = '\n';
			}

			for( i = 0 ; i < E_LISTS_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			if( ana_result == 0 )
			{
				if( new_lst.num_lsts == 0 )
				{
					munmap( new_lst.lists, hctx.content_length + 1 );
					return( -1 );
				}
				break;
			}

			logMessage( "continue\n" );
			continue;
		}

		for( i = 0 ; i < E_LISTS_NUM ; i++ )
		{
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/lists											*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_LISTS_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_LISTS );

		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserHomeDirectory( node[ E_LISTS_USR_SNAME ].value, true );

			if( result < 0 )
			{
				for( i = 0 ; i < E_LISTS_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}

				if( ana_result == 0 )
				{
					if( new_lst.num_lsts == 0 )
					{
						munmap( new_lst.lists, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/lists/[list name]								*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_LISTS_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  node[ E_LISTS_SLUG ].value,
				  DEF_TWFS_PATH_LNAME_LDESC );

		if( ( result = isRegularFile( buffer ) ) < 0 )
		{
			result = makeUserListsSlugDir( node[ E_LISTS_USR_SNAME ].value,
										   node[ E_LISTS_SLUG ].value );

			if( result < 0 )
			{
				for( i = 0 ; i < E_LISTS_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}

				if( ana_result == 0 )
				{
					if( new_lst.num_lsts == 0 )
					{
						munmap( new_lst.lists, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		/* -------------------------------------------------------------------- */
		/* link to my_list/[slug]												*/
		/* -------------------------------------------------------------------- */
		switch( request )
		{
		case	E_TWFS_REQ_READ_LISTS_SUB_LIST:
			{
				char	sym_path[ DEF_TWFS_PATH_MAX ];

				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_SUB,
						  node[ E_LISTS_SLUG ].value );

				snprintf( sym_path, sizeof( sym_path ),
						  "../../../%s/%s/%s/%s",
						  node[ E_LISTS_USR_SNAME ].value,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  node[ E_LISTS_SLUG ].value );

				logMessage( "sym_path : %s\n", sym_path );
				logMessage( "buffer : %s\n\n", buffer );
				symlink( sym_path, buffer );
			}
			break;
		case	E_TWFS_REQ_READ_LISTS_ADD_LIST:
			{
				char	sym_path[ DEF_TWFS_PATH_MAX ];

				snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  screen_name,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_ADD,
						  node[ E_LISTS_SLUG ].value );

				snprintf( sym_path, sizeof( sym_path ),
						  "../../../%s/%s/%s/%s",
						  node[ E_LISTS_USR_SNAME ].value,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  node[ E_LISTS_SLUG ].value );
				logMessage( "sym_path : %s\n", sym_path );
				logMessage( "buffer : %s\n\n", buffer );

				symlink( sym_path, buffer );
			}
			break;
		default:
			break;
		}

		/* -------------------------------------------------------------------- */
		/* path to [slug]/descriptions											*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_LISTS_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  node[ E_LISTS_SLUG ].value,
				  DEF_TWFS_PATH_LNAME_LDESC );
		
		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			int		res_len;

			buf_len = 0;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( &buffer[ buf_len ],
								sizeof( buffer ) - buf_len + 1,
								"%s\n", DEF_LDESC_SEPARATOR );
			/* ---------------------------------------------------------------- */
			/* list id															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_ID,
								 node[ E_LISTS_ID ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_ID_FIELD - node[ E_LISTS_ID ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* list name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s\n",
								 DEF_LDESC_NAME,
								 node[ E_LISTS_SLUG ].value );
			/* copy list name													*/
			memcpy( new_lst.lists + new_lst.length,
					node[ E_LISTS_SLUG ].value,
					node[ E_LISTS_SLUG ].length );
			new_lst.length += node[ E_LISTS_SLUG ].length;
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_NAME_FIELD - node[ E_LISTS_SLUG ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			memset( new_lst.lists + new_lst.length, 0x00, res_len );
			new_lst.length += res_len;
			*( new_lst.lists + new_lst.length++ ) = 0x00;
			/* ---------------------------------------------------------------- */
			/* own id															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_OWN_ID,
								 node[ E_LISTS_USR_ID_STR ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_OWN_ID_FIELD - node[ E_LISTS_USR_ID_STR ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* owner name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_OWN_NAME,
								 node[ E_LISTS_USR_NAME ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_OWN_NAME_FIELD - node[ E_LISTS_USR_NAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* own screen name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_OWN_SNAME,
								 node[ E_LISTS_USR_SNAME ].value );
			/* copy screen name													*/
			memcpy( new_lst.lists + new_lst.length,
					node[ E_LISTS_USR_SNAME ].value,
					node[ E_LISTS_USR_SNAME ].length );
			new_lst.length += node[ E_LISTS_USR_SNAME ].length;
			/* fill residual space of sreccn name in 'fd' file					*/
			res_len = DEF_LDESC_OWN_SNAME_FIELD
					  - node[ E_LISTS_USR_SNAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			memset( new_lst.lists + new_lst.length, 0x00, res_len );
			new_lst.length += res_len;
			*( new_lst.lists + new_lst.length++ ) = 0x00;

			/* ---------------------------------------------------------------- */
			/* description														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_LISTS_DESC ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_LDESC_DESC,
									 node[ E_LISTS_DESC ].value );


				/* fill residual space of location in 'fd' file					*/
				res_len = DEF_LDESC_DESC_FIELD - node[ E_LISTS_DESC ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_LDESC_DESC );
				memset( &buffer[ buf_len ], ' ', DEF_LDESC_DESC_FIELD );
				buf_len += DEF_LDESC_DESC_FIELD;
			}
			buffer[ buf_len++ ] = '\n';

			/* '\n' is separator												*/
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* subscribers														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_SUB,
								 node[ E_LISTS_SUB_CNT ].value );

			/* fill residual space of tweets in 'fd' file						*/
			res_len = DEF_LDESC_SUB_FIELD - node[ E_LISTS_SUB_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* members															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_MEM,
								 node[ E_LISTS_MEM_CNT ].value );

			/* fill residual space of tweets in 'fd' file						*/
			res_len = DEF_LDESC_MEM_FIELD - node[ E_LISTS_MEM_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* text length is determined ( the size of each text of				*/
			/* lists description file may be always fixed )						*/
			/* ---------------------------------------------------------------- */
			new_lst.text_len = buf_len;
			twfs_file->tl_size += new_lst.text_len;

			/* ---------------------------------------------------------------- */
			/* write to description file										*/
			/* ---------------------------------------------------------------- */
			writeFile( fd, ( const void* )buffer, buf_len );

			closeFile( fd );

			/* ---------------------------------------------------------------- */
			/* update list file													*/
			/* ---------------------------------------------------------------- */
			*( new_lst.lists + new_lst.length++ ) = 'F';
			*( new_lst.lists + new_lst.length++ ) = 0x00;
			*( new_lst.lists + new_lst.length++ ) = '0';
			*( new_lst.lists + new_lst.length++ ) = '0';
			*( new_lst.lists + new_lst.length++ ) = 0x00;

			buf_len = snprintf( ( char* )( new_lst.lists + new_lst.length ),
								DEF_TWFS_TEXT_LEN_FIELD,
								"%d", buf_len );

			new_lst.length += buf_len;
			
			/* fill residual space											*/
			buf_len = DEF_TWFS_TEXT_LEN_FIELD - buf_len;
			memset( new_lst.lists + new_lst.length, 0x00, buf_len );
			new_lst.length += buf_len;
			*( new_lst.lists + new_lst.length++ ) = '\n';

			new_lst.num_lsts++;
		}

		if( node[ E_LIST_NEXT_CUR ].value	!= NULL )
		{
			int		res_len;
			/* first new_usr.users area are temporaly saved next cursor			*/
			memcpy( new_lst.lists,
					node[ E_LIST_NEXT_CUR ].value,
					node[ E_LIST_NEXT_CUR ].length );
			/* fill residual space												*/
			res_len = DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD
					  - node[ E_LIST_NEXT_CUR ].length;
			memset( new_lst.lists
					+ node[ E_LIST_NEXT_CUR ].length,
					0x00, res_len );
			*( new_lst.lists
			   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ) = '\n';
		}
		if( node[ E_LIST_PREV_CUR ].value	!= NULL )
		{
			int		res_len;
			/* first new_usr.users area are temporaly saved next cursor			*/
			memcpy( new_lst.lists
					+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN,
					node[ E_LIST_PREV_CUR ].value,
					node[ E_LIST_PREV_CUR ].length );
			/* fill residual space												*/
			res_len = DEF_TWFS_HEAD_FF_PREV_CUR_FIELD
					  - node[ E_LIST_PREV_CUR ].length;
			memset( new_lst.lists
					+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
					+ node[ E_LIST_PREV_CUR ].length,
					0x00, res_len );
			*( new_lst.lists
			   + DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
			   + DEF_TWFS_HEAD_FF_PREV_CUR_FIELD ) = '\n';
		}

		for( i = 0 ; i < E_LISTS_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_lst.num_lsts == 0 )
			{
				munmap( new_lst.lists, hctx.content_length + 1 );
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}

	logMessage( "lists num is :%d\n", new_lst.num_lsts );

	if( DEF_TWFS_HEAD_FF_LEN < twfs_file->size )
	{
		result = ftruncate( twfs_file->fd,
							twfs_file->size
							+ ( DEF_TWFS_LISTS_RECORD_LEN * new_lst.num_lsts ) );
	}
	else
	{
		result = ftruncate( twfs_file->fd,
							DEF_TWFS_HEAD_FF_LEN
							+ ( DEF_TWFS_LISTS_RECORD_LEN * new_lst.num_lsts ) );
	}

	if( result < 0 )
	{
		munmap( new_lst.lists, hctx.content_length + 1 );
		return( -1 );
	}

	*( new_lst.lists + new_lst.length ) = '\0';

	/* ------------------------------------------------------------------------ */
	/* update next/prev cursor													*/
	/* ------------------------------------------------------------------------ */
	memcpy( twfs_file->tl + DEF_TWFS_OFFSET_FF_NEXT_CUR_FIELD,
			new_lst.lists,
			DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
			+ DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN );
	
	/* ------------------------------------------------------------------------ */
	/* body of a lists file is updated											*/
	/* ------------------------------------------------------------------------ */
	if( DEF_TWFS_HEAD_FF_LEN < twfs_file->size )
	{
		memcpy( twfs_file->tl + twfs_file->size,
				new_lst.lists
				+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
				+ DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN,
				new_lst.num_lsts * DEF_TWFS_LISTS_RECORD_LEN );
	}
	else
	{
		memcpy( twfs_file->tl + DEF_TWFS_OFFSET_BODY_OF_FF,
				new_lst.lists
				+ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD_LEN
				+ DEF_TWFS_HEAD_FF_PREV_CUR_FIELD_LEN,
				new_lst.num_lsts * DEF_TWFS_LISTS_RECORD_LEN );
	}
	/* ------------------------------------------------------------------------ */
	/* update total lists list file size										*/
	/* ------------------------------------------------------------------------ */
	if( new_lst.num_lsts )
	{
		result = snprintf( twfs_file->tl,
						   DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN,
						   "%zu", twfs_file->tl_size );

		for( i = result ; i < ( DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN - 1 ) ; i++ )
		{
			*( twfs_file->tl + i ) = 0x00;
		}
		*( twfs_file->tl + i ) = '\n';
	
		if( DEF_TWFS_HEAD_FF_LEN < twfs_file->size )
		{
			/* actual lists file is updated										*/
			twfs_file->size += ( DEF_TWFS_LISTS_RECORD_LEN * new_lst.num_lsts );
		}
		else
		{
			/* actual lists file is updated										*/
			twfs_file->size = DEF_TWFS_HEAD_FF_LEN;
			twfs_file->size += ( DEF_TWFS_LISTS_RECORD_LEN * new_lst.num_lsts );
		}
	

		msync( twfs_file->tl, twfs_file->size, MS_SYNC );
	}

	munmap( new_lst.lists, hctx.content_length + 1 );
	logMessage( "Read Lists: twfs_file->size :%d\n", twfs_file->size );

	return( 0 );
}

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
			   const char *slug )
{
	typedef enum
	{
		E_CREATED_AT,				// created_at
		E_ID_STR,					// id_str
		E_SLUG,						// slug
		E_NAME,						// name
		E_DESCRIPTION,				// description
		E_MODE,						// mode
		E_SUB_CNT,					// subscriber_count
		E_MEM_CNT,					// member_count
		E_USR,						// user
		E_USR_ID_STR,				// user/id_str
		E_USR_NAME,					// user/name
		E_USR_SNAME,				// user/screen_name
		E_OBJ_NUM,
	} E_OBJ;

	struct new_lst
	{
		uint8_t		*lists;			// buffer for json object
		int			num_lsts;		// number of lists
		size_t		length;			// length of all text
		int			text_len;		// current text length
	};

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	int					fd;
	struct new_lst		new_lst = { NULL, 0, 0, 0 };
	struct jnode		root;
	struct jnode		node[ E_OBJ_NUM ];
	char				obj_usr_id_str[ ]	= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_ID_STR;
	char				obj_usr_name[ ]		= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_NAME;
	char				obj_usr_sname[ ]	= "/" DEF_TWAPI_OBJ_USR
											  "/" DEF_TWAPI_OBJ_USR_SNAME;

	/* ------------------------------------------------------------------------ */
	/* request get lists/{subscriptions, memberships, ownerships}				*/
	/* ------------------------------------------------------------------------ */
	switch( request )
	{
	case	E_TWFS_REQ_CREATE_LISTS:
		result = createLists( session, &hctx, slug, NULL );
		break;
	case	E_TWFS_REQ_SHOW_LISTS:
		result = showLists( session, &hctx, screen_name, slug );
		break;
	default:
		return( -1 );
	}

	if( result < 0 )
	{
		return( result );
	}

	if( hctx.content_length == 0 )
	{
		return( 0 );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for new lists														*/
	/* ------------------------------------------------------------------------ */
	new_lst.lists = mmap( NULL, hctx.content_length + 1,
						  PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

	if( new_lst.lists == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )obj_usr_name, 1 ) < 0 )
			{
				break;
			}
		}

		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive tweets of json structure											*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_lst.lists,
						hctx.content_length ) < 0 )
	{
		munmap( new_lst.lists, hctx.content_length + 1 );
		return( -1 );
	}

	logMessage( "%s\n", new_lst.lists );

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );

	/* created_at																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_CREATED_AT,
					 &node[ E_CREATED_AT		]	);
	/* id_str																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_ID_STR,
					 &node[ E_ID_STR			]	);
	/* slug																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_SLUG,
					 &node[ E_SLUG				]	);
	/* name																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_NAME,
					 &node[ E_NAME				]	);
	/* descriptions																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_DESC,
					 &node[ E_DESCRIPTION		]	);
	/* mode																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_MODE,
					 &node[ E_MODE				]	);
	/* subscriber_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_SUB_CNT,
					 &node[ E_SUB_CNT			]	);
	/* member_count																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_LISTS_MEM_CNT,
					 &node[ E_MEM_CNT			]	);
	/* user/																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR,
					 &node[ E_USR				]	);
	/* user/id_str																*/
	insertJsonNodes( &root, obj_usr_id_str,
					 &node[ E_USR_ID_STR		]	);
	/* user/name																*/
	insertJsonNodes( &root, obj_usr_name,
					 &node[ E_USR_NAME			]	);
	/* user/screen_name															*/
	insertJsonNodes( &root, obj_usr_sname,
					 &node[ E_USR_SNAME			]	);

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		//char	buffer[ 1024 ];
		char	buffer[ DEF_LDESC_TEXT_LEN ];
		int		buf_len;
		int		ana_result;
		int		analysis_count = 0;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_lst.lists,
								  &ana, &root,
								  1,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_OBJ_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( new_lst.lists, hctx.content_length + 1 );
			return( -1 );
		}

		if( ( node[ E_ID_STR ].value		== NULL ) ||
			( node[ E_SLUG ].value			== NULL ) ||
			( node[ E_NAME ].value			== NULL ) ||
			( node[ E_USR_ID_STR ].value	== NULL ) ||
			( node[ E_USR_NAME ].value		== NULL ) ||
			( node[ E_USR_SNAME ].value		== NULL ) ||
			( node[ E_SUB_CNT ].value		== NULL ) ||
			( node[ E_MEM_CNT ].value		== NULL ) )
		{
			analysis_count++;
			if( analysis_count < 100 )
			{
				continue;
			}
			logMessage( "\ndetected json null value\n" );
			for( i = 0 ; i < E_OBJ_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			if( ana_result == 0 )
			{
				if( new_lst.num_lsts == 0 )
				{
					munmap( new_lst.lists, hctx.content_length + 1 );
					return( -1 );
				}
				break;
			}

			logMessage( "continue\n" );
			continue;
		}

		for( i = 0 ; i < E_OBJ_NUM ; i++ )
		{
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/lists											*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_LISTS );

		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserHomeDirectory( node[ E_USR_SNAME ].value, true );

			if( result < 0 )
			{
				for( i = 0 ; i < E_OBJ_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}

				if( ana_result == 0 )
				{
					if( new_lst.num_lsts == 0 )
					{
						munmap( new_lst.lists, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/lists/[list name]								*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  node[ E_SLUG ].value,
				  DEF_TWFS_PATH_LNAME_LDESC );

		if( ( result = isRegularFile( buffer ) ) < 0 )
		{
			result = makeUserListsSlugDir( node[ E_USR_SNAME ].value,
										   node[ E_SLUG ].value );

			if( result < 0 )
			{
				for( i = 0 ; i < E_OBJ_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}

				if( ana_result == 0 )
				{
					if( new_lst.num_lsts == 0 )
					{
						munmap( new_lst.lists, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		/* -------------------------------------------------------------------- */
		/* path to [slug]/descriptions											*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_LISTS,
				  DEF_TWFS_PATH_DIR_OWN,
				  node[ E_SLUG ].value,
				  DEF_TWFS_PATH_LNAME_LDESC );
		
		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT, 0660 );

		if( 0 <= fd )
		{
			int		res_len;

			buf_len = 0;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( &buffer[ buf_len ],
								sizeof( buffer ) - buf_len + 1,
								"%s\n", DEF_LDESC_SEPARATOR );
			/* ---------------------------------------------------------------- */
			/* list id															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_ID,
								 node[ E_ID_STR ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_ID_FIELD - node[ E_ID_STR ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* list name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s\n",
								 DEF_LDESC_NAME,
								 node[ E_SLUG ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_NAME_FIELD - node[ E_SLUG ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* own id															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_OWN_ID,
								 node[ E_USR_ID_STR ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_OWN_ID_FIELD - node[ E_USR_ID_STR ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* owner name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_OWN_NAME,
								 node[ E_USR_NAME ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_LDESC_OWN_NAME_FIELD - node[ E_USR_NAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';
			/* ---------------------------------------------------------------- */
			/* own screen name													*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_OWN_SNAME,
								 node[ E_USR_SNAME ].value );
			/* fill residual space of sreccn name in 'fd' file					*/
			res_len = DEF_LDESC_OWN_SNAME_FIELD
					  - node[ E_USR_SNAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* description														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_DESCRIPTION ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_LDESC_DESC,
									 node[ E_DESCRIPTION ].value );


				/* fill residual space of location in 'fd' file					*/
				res_len = DEF_LDESC_DESC_FIELD - node[ E_DESCRIPTION ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_LDESC_DESC );
				memset( &buffer[ buf_len ], ' ', DEF_LDESC_DESC_FIELD );
				buf_len += DEF_LDESC_DESC_FIELD;
			}
			buffer[ buf_len++ ] = '\n';

			/* '\n' is separator												*/
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* subscribers														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_SUB,
								 node[ E_SUB_CNT ].value );

			/* fill residual space of tweets in 'fd' file						*/
			res_len = DEF_LDESC_SUB_FIELD - node[ E_SUB_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* members															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_LDESC_MEM,
								 node[ E_MEM_CNT ].value );

			/* fill residual space of tweets in 'fd' file						*/
			res_len = DEF_LDESC_MEM_FIELD - node[ E_MEM_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* text length is determined ( the size of each text of				*/
			/* lists description file may be always fixed )						*/
			/* ---------------------------------------------------------------- */
			new_lst.text_len = buf_len;

			/* ---------------------------------------------------------------- */
			/* write to description file										*/
			/* ---------------------------------------------------------------- */
			writeFile( fd, ( const void* )buffer, buf_len );

			closeFile( fd );

			//new_lst.num_lsts++;
		}

		for( i = 0 ; i < E_OBJ_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_lst.num_lsts == 0 )
			{
				munmap( new_lst.lists, hctx.content_length + 1 );
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}

	munmap( new_lst.lists, hctx.content_length + 1 );

	return( 0 );
}

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
				 const char *screen_name )
{
	typedef enum
	{
		E_USR_CREATED_AT,			// created_at
		E_USR_DESCRIPTION,			// description [nullable]
		E_USR_FAV_CNT,				// favourites_count
		E_USR_FOLLOWING,			// following [nullable]
		E_USR_FOLLOWERS_CNT,		// followers_count
		E_USR_FRIENDS_CNT,			// friends_count
		E_USR_ID_STR,				// id_str
		E_USR_LISTED_CNT,			// listed_count
		E_USR_LOCATION,				// location [nullable]
		E_USR_NAME,					// name
		E_USR_SNAME,				// screen_name
		E_USR_STATUSES_CNT,			// statuses_count
		E_USR_URL,					// url [nullable]
		E_USR_VERIFIED,				// verified
		E_LIST_NUM,
	} E_OBJ;

	struct new_usr
	{
		uint8_t		*users;			// buffer for json object
		int			num_usrs;		// number of users
		size_t		length;			// length of all text
		int			text_len;		// current text length
	};

	struct http_ctx		hctx;
	struct json_ana		ana;
	int					result;
	int					recv_length;
	int					i;
	int					fd;
	struct new_usr		new_usr = { NULL, 0, 0, 0};
	struct jnode		root;
	struct jnode		node[ E_LIST_NUM ];

	/* ------------------------------------------------------------------------ */
	/* request get users/show 													*/
	/* ------------------------------------------------------------------------ */
	result = getUserProfile( session, &hctx, screen_name );

	if( result < 0 )
	{
		return( result );
	}

	if( hctx.content_length == 0 )
	{
		return( 0 );
	}

	/* ------------------------------------------------------------------------ */
	/* buffer for new users														*/
	/* ------------------------------------------------------------------------ */
	new_usr.users = mmap( NULL, hctx.content_length + 1,
						   PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

	if( new_usr.users == MAP_FAILED )
	{
		recv_length = hctx.content_length;
		while( recv_length-- )
		{
			if( recvSSLMessage( session, ( unsigned char* )node[ 0 ].value, 1 ) < 0 )
			{
				break;
			}
		}

		logMessage( "cannot map [new_tws]%s\n", strerror( errno ) );

		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* receive tweets of json structure											*/
	/* ------------------------------------------------------------------------ */
	if( recvSSLMessage( session,
						( unsigned char* )new_usr.users,
						hctx.content_length ) < 0 )
	{
		munmap( new_usr.users, hctx.content_length );
		//disconnectSSLServer( session );
		return( -1 );
	}

	//disconnectSSLServer( session );

	/* ------------------------------------------------------------------------ */
	/* make json lookup structure												*/
	/* ------------------------------------------------------------------------ */
	initJsonRoot( &root );
	
	/* created_at																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_CREATED_AT,
					 &node[ E_USR_CREATED_AT	]	);
	/* description																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_DESCRIPTION,
					 &node[ E_USR_DESCRIPTION	]	);
	/* favorites_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_FAV_CNT,
					 &node[ E_USR_FAV_CNT		]	);
	/* following																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_FOLLOWING,
					 &node[ E_USR_FOLLOWING		]	);
	/* followers_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_FOLLOWERS_CNT,
					 &node[ E_USR_FOLLOWERS_CNT	]	);
	/* friends_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_FRIENDS_CNT,
					 &node[ E_USR_FRIENDS_CNT	]	);
	/* id_str																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_ID_STR,
					 &node[ E_USR_ID_STR		]	);
	/* listed_count																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_LISTED_CNT,
					 &node[ E_USR_LISTED_CNT	]	);
	/* location																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_LOCATION,
					 &node[ E_USR_LOCATION		]	);
	/* name																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_NAME,
					 &node[ E_USR_NAME			]	);
	/* screen_name																*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_SNAME,
					 &node[ E_USR_SNAME			]	);
	/* statuses_count															*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_STATUSES_CNT,
					 &node[ E_USR_STATUSES_CNT	]	);
	/* url																		*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_URL,
					 &node[ E_USR_URL			]	);
	/* verified																	*/
	insertJsonNodes( &root, "/" DEF_TWAPI_OBJ_USR_VERIFIED,
					 &node[ E_USR_VERIFIED		]	);

	/* ------------------------------------------------------------------------ */
	/* prepare for analyzing json structure										*/
	/* ------------------------------------------------------------------------ */
	initJsonAnalysisCtx( &ana );

	/* ------------------------------------------------------------------------ */
	/* receive body and analyze json structre									*/
	/* ------------------------------------------------------------------------ */
	while( ana.length < hctx.content_length )
	{
		//char	buffer[ 1024 ];
		char	buffer[ DEF_PROF_TEXT_LEN + 1 ];
		int		buf_len;
		int		ana_result;
		int		analysis_count = 0;

		ana_result = analyzeJson( session, &hctx,
								  ( uint8_t* )new_usr.users,
								  &ana, &root,
								  1,
								  ( uint8_t* )buffer, sizeof( buffer ) );

		if( ana_result < 0 )
		{
			for( i = 0 ; i < E_LIST_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}
			logMessage( "error analyzing json\n" );
			munmap( new_usr.users, hctx.content_length + 1 );
			return( -1 );
		}

		if( ( node[ E_USR_CREATED_AT ].value	== NULL ) ||
			( node[ E_USR_FAV_CNT ].value		== NULL ) ||
			( node[ E_USR_FOLLOWERS_CNT ].value	== NULL ) ||
			( node[ E_USR_FRIENDS_CNT ].value	== NULL ) ||
			( node[ E_USR_LISTED_CNT ].value	== NULL ) ||
			( node[ E_USR_STATUSES_CNT ].value	== NULL ) ||
			( node[ E_USR_SNAME ].value			== NULL ) ||
			( node[ E_USR_ID_STR ].value		== NULL ) ||
			( node[ E_USR_NAME ].value			== NULL ) )
			//( node[ E_LIST_PREV_CUR ].value		== NULL ) ||
			//( node[ E_LIST_NEXT_CUR ].value		== NULL ) )
		{
			analysis_count++;
			if( analysis_count < 100 )
			{
				continue;
			}
			logMessage( "\ndetected json null value\n" );

			for( i = 0 ; i < E_LIST_NUM ; i++ )
			{
				free( node[ i ].value );
				node[ i ].value = NULL;
			}

			if( ana_result == 0 )
			{
				logMessage( "analyzed finish(%d)!!!\n", new_usr.num_usrs );
				if( new_usr.num_usrs == 0 )
				{
					munmap( new_usr.users, hctx.content_length + 1 );
					return( -1 );
				}
				break;
			}
			logMessage( "continue \n" );
			continue;
		}

		for( i = 0 ; i < E_LIST_NUM ; i++ )
		{
			logMessage( "%s:%s\n", node[i].obj, node[i ].value );
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]												*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_ACCOUNT );

		if( ( result = isDirectory( buffer ) ) < 0 )
		{
			result = makeUserHomeDirectory( node[ E_USR_SNAME ].value, true );

			if( result < 0 )
			{
				for( i = 0 ; i < E_LIST_NUM ; i++ )
				{
					free( node[ i ].value );
					node[ i ].value = NULL;
				}

				if( ana_result == 0 )
				{
					if( new_usr.num_usrs == 0 )
					{
						munmap( new_usr.users, hctx.content_length + 1 );
						return( -1 );
					}
					break;
				}
				/* try next tweet												*/
				continue;
			}
		}

		/* -------------------------------------------------------------------- */
		/* path to [screen_name]/account/profile								*/
		/* -------------------------------------------------------------------- */
		snprintf( buffer, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
				  getRootDirPath( ),
				  node[ E_USR_SNAME ].value,
				  DEF_TWFS_PATH_DIR_ACCOUNT,
				  DEF_TWFS_PATH_PROFILE );

		fd = openFile( buffer, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC, 0660 );

		if( 0 <= fd )
		{
			int		res_len;
			char	now_following;
			char	followed_by;
			char	verified;
			buf_len = 0;
			/* ---------------------------------------------------------------- */
			/* separator														*/
			/* ---------------------------------------------------------------- */
			buf_len = snprintf( &buffer[ buf_len ],
								sizeof( buffer ) - buf_len + 1,
								"%s\n", DEF_PROF_SEPARATOR );
			/* ---------------------------------------------------------------- */
			/* id																*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_ID,
								 node[ E_USR_ID_STR ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_PROF_ID_FIELD - node[ E_USR_ID_STR ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* created at														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s\n",
								 DEF_PROF_CREATED_AT,
								 node[ E_USR_CREATED_AT ].value );
			/* ---------------------------------------------------------------- */
			/* name																*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_NAME,
								 node[ E_USR_NAME ].value );
			/* fill residual space of name in 'fd' file							*/
			res_len = DEF_PROF_NAME_FIELD - node[ E_USR_NAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* screen name														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_SNAME,
								 node[ E_USR_SNAME ].value );
			/* fill residual space of sreccn name in 'fd' file					*/
			res_len = DEF_PROF_SNAME_FIELD - node[ E_USR_SNAME ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* location															*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_LOCATION ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_PROF_LOCATION,
									 node[ E_USR_LOCATION ].value );


				/* fill residual space of location in 'fd' file					*/
				res_len = DEF_PROF_LOCATION_FIELD - node[ E_USR_LOCATION ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_PROF_LOCATION );
				memset( &buffer[ buf_len ], ' ', DEF_PROF_LOCATION_FIELD );
				buf_len += DEF_PROF_LOCATION_FIELD;
			}
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* url																*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_URL ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_PROF_URL,
									 node[ E_USR_URL ].value );

				/* fill residual space of url in 'fd' file						*/
				res_len = DEF_PROF_URL_FIELD - node[ E_USR_URL ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_PROF_URL );
				memset( &buffer[ buf_len ], ' ', DEF_PROF_URL_FIELD );
				buf_len += DEF_PROF_URL_FIELD;
			}
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* description														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_DESCRIPTION ].value != NULL )
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s%s",
									 DEF_PROF_DESCRIPTION,
									 node[ E_USR_DESCRIPTION ].value );
				/* fill residual space of description in 'fd' file				*/
				res_len = DEF_PROF_DESC_FIELD - node[ E_USR_DESCRIPTION ].length;
				memset( &buffer[ buf_len ], ' ', res_len );
				buf_len += res_len;
				buffer[ buf_len++ ] = '\n';
			}
			else
			{
				buf_len += snprintf( &buffer[ buf_len ],
									 sizeof( buffer ) - buf_len + 1,
									 "%s",
									 DEF_PROF_DESCRIPTION );
				memset( &buffer[ buf_len ], ' ', DEF_PROF_DESC_FIELD );
				buf_len += DEF_PROF_DESC_FIELD;
				buffer[ buf_len++ ] = '\n';
			}
			
			/* '\n' is separator												*/
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* tweets															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_STATUSES,
								 node[ E_USR_STATUSES_CNT ].value );

			/* fill residual space of tweets in 'fd' file						*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_STATUSES_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* favorites														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_FAVS,
								 node[ E_USR_FAV_CNT ].value );

			/* fill residual space of favorites in 'fd' file					*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_FAV_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* following														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_FOLLOWING,
								 node[ E_USR_FRIENDS_CNT ].value );

			/* fill residual space of following in 'fd' file					*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_FRIENDS_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* followers														*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_FOLLOWERS,
								 node[ E_USR_FOLLOWERS_CNT ].value );

			/* fill residual space of following in 'fd' file					*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_FOLLOWERS_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* listed															*/
			/* ---------------------------------------------------------------- */
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s%s",
								 DEF_PROF_LISTED_CNT,
								 node[ E_USR_LISTED_CNT ].value );

			/* fill residual space of listed in 'fd' file						*/
			res_len = DEF_PROF_COUNT_FIELD - node[ E_USR_LISTED_CNT ].length;
			memset( &buffer[ buf_len ], ' ', res_len );
			buf_len += res_len;
			buffer[ buf_len++ ] = '\n';

			/* '\n' is separator												*/
			buffer[ buf_len++ ] = '\n';

			/* ---------------------------------------------------------------- */
			/* follwing [*]														*/
			/* ---------------------------------------------------------------- */
			if( node[ E_USR_FOLLOWING ].value != NULL )
			{
				if( ( node[ E_USR_FOLLOWING ].value[ 0 ] == 't' ) ||
					( node[ E_USR_FOLLOWING ].value[ 0 ] == 'T' ) )
				{
					now_following = '*';
				}
				else
				{
					now_following = ' ';
				}
			}
			else
			{
				now_following = ' ';
			}
			
			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s [%c]",
								 DEF_PROF_NOW_FOLLOWING,
								 now_following );
			/* fill space after following item								*/
			memset( &buffer[ buf_len ], ' ', DEF_PROF_SPACE_FR_AND_FL );
			buf_len += DEF_PROF_SPACE_FR_AND_FL;
			
			/* ---------------------------------------------------------------- */
			/* followed [*]														*/
			/* ---------------------------------------------------------------- */
			followed_by = ' ';

			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s [%c]",
								 DEF_PROF_FOLLOWED_BY,
								 followed_by );
			/* fill space after followed item									*/
			memset( &buffer[ buf_len ], ' ', DEF_PROF_SPACE_FL_AND_VERI );
			buf_len += DEF_PROF_SPACE_FL_AND_VERI;

			/* ---------------------------------------------------------------- */
			/* verified [*]														*/
			/* ---------------------------------------------------------------- */
			if( ( node[ E_USR_VERIFIED ].value[ 0 ] == 't' ) ||
				( node[ E_USR_VERIFIED ].value[ 0 ] == 'T' ) )
			{
				verified = '*';
			}
			else
			{
				verified = ' ';
			}

			buf_len += snprintf( &buffer[ buf_len ],
								 sizeof( buffer ) - buf_len + 1,
								 "%s [%c]\n",
								 DEF_PROF_VERIFIED,
								 verified );

			/* ---------------------------------------------------------------- */
			/* text length is determined ( the size of each text of				*/
			/* ffollower/fllowing list file may be always fixed )				*/
			/* ---------------------------------------------------------------- */
			new_usr.text_len = buf_len;

			/* ---------------------------------------------------------------- */
			/* write to profle file												*/
			/* ---------------------------------------------------------------- */
			writeFile( fd, ( const void* )buffer, buf_len );
			//logMessage( "%s\n", buffer );
			//buffer[ buf_len ] = '\0';
			//logMessage( "%s\n", buffer );

			closeFile( fd );

			new_usr.num_usrs++;
		}

		for( i = 0 ; i < E_LIST_NUM ; i++ )
		{
			free( node[ i ].value );
			node[ i ].value = NULL;
		}

		if( ana_result == 0 )
		{
			if( new_usr.num_usrs == 0 )
			{
				munmap( new_usr.users, hctx.content_length + 1 );
				return( -1 );
			}
			/* ---------------------------------------------------------------- */
			/* analysis is done!												*/
			/* ---------------------------------------------------------------- */
			logMessage( "analysis is done!\n" );
			break;
		}
	}
	logMessage( "user num is :%d\n", new_usr.num_usrs );


	munmap( new_usr.users, hctx.content_length + 1 );

	return( 0 );
}
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
	Description	:get latest id from timeline, etc.
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int getLatestIdFromTlFile( const char *tl, size_t size, char *id )
{
	off_t	offset;
	int		i;

	if( size < DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN )
	{
		id[ 0 ] = '\0';
		return( -1 );
	}

	offset = size;

	while( 0 < offset )
	{
		
		offset = offset - DEF_TWFS_TL_RECORD_LEN;

		if( offset < 0 )
		{
			id[ 0 ] = '\0';
			return( -1 );
		}
		
		if( ( ( unsigned int )( *( tl + offset ) ) - '0' ) < 10u )
		{
			*id = *( tl + offset );
			logMessage( "%c", *id );
			for( i = 1 ;
				 i < ( DEF_TWFS_ID_FIELD + DEF_TWFS_ID_FIELD_NEXT ) ;
				 i++ )
			{
				if( *( tl + offset + i ) == 0x00 )
				{
					break;
				}
				else if( ( ( unsigned int )( *( tl + offset + i ) ) - '0' ) < 10u )
				{
					*( id + i ) = *( tl + offset + i );
				}
				logMessage( "%c", *( id + i ) );
			}

			if( ( DEF_TWFS_ID_FIELD + DEF_TWFS_ID_FIELD_NEXT ) <= i )
			{
				*id = '\0';
				return( -1 );
			}
			else
			{
				*( id + i ) = '\0';
				return( i );
			}
		}
	}

	*id = '\0';

	return( -1 );
}

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
						   char *recv_id )
{
	off_t	offset;
	int		result;
	char	latest;
	char	dm_field;

	if( size < DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN )
	{
		sent_id[ 0 ] = '\0';
		recv_id[ 0 ] = '\0';
		return( -1 );
	}

	offset			= size - DEF_TWFS_TL_RECORD_LEN;
	latest			= '\0';
	sent_id[ 0 ]	= '\0';
	recv_id[ 0 ]	= '\0';

	while( 0 < offset )
	{
		dm_field =  *( dm + offset + DEF_TWFS_OFFSET_DM_FIELD );
		if( latest == '\0' )
		{
			if( dm_field == 's' )
			{
				result = getLatestIdFromTlFile( dm,
												offset + DEF_TWFS_TL_RECORD_LEN,
												sent_id );
				latest = 's';
			}
			else if( dm_field == 'r' )
			{
				result = getLatestIdFromTlFile( dm,
												offset + DEF_TWFS_TL_RECORD_LEN,
												recv_id );
				latest = 'r';
			}
			else
			{
				result = getLatestIdFromTlFile( dm,
												offset + DEF_TWFS_TL_RECORD_LEN,
												recv_id );
				latest = 'l';
				memcpy( sent_id, recv_id, DEF_TWFS_SNAME_FIELD );
				return( 0 );
			}
		}
		else if( latest == 's' )
		{
			if( dm_field == 's' )
			{
				result = 0;		// continue
			}
			else
			{
				result = getLatestIdFromTlFile( dm,
												offset + DEF_TWFS_TL_RECORD_LEN,
												recv_id );
				if( result < 0 )
				{
					return( 's' );
				}
				return( 0 );
			}
		}
		else
		{
			if( dm_field == 's' || dm_field == 'l' )
			{
				result = getLatestIdFromTlFile( dm,
												offset + DEF_TWFS_TL_RECORD_LEN,
												sent_id );
				if( result < 0 )
				{
					return( 'r' );
				}
				return( 0 );
			}
			else
			{
				result = 0;		// continue
			}
		}

		if( result < 0 )
		{
			return( result );
		}
		offset = offset - DEF_TWFS_TL_RECORD_LEN;
	}

	if( latest != '\0' )
	{
		return( latest );
	}

	return( -1 );
}

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
int getTotalSizeOfTlFileFromFd( int fd, size_t *total_size )
{
	int		i;
	char	buf[ DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN ];

	*total_size = 0;

	if( preadFile( fd, buf, DEF_TWFS_HEAD_TL_SIZE_FIELD, 0 ) < 0 )
	{
		return( -1 );
	}

	for( i = 0 ; i < DEF_TWFS_HEAD_TL_SIZE_FIELD ; i++ )
	{
		unsigned int	size;

		size = ( unsigned int )( *( buf + i ) );

		if( ( size - '0' ) < 10u )
		{

			*total_size = *total_size * 10 + ( size - '0' );
		}
		else if( size == 0x00 )
		{
			break;
		}
		else
		{
			return( -1 );
		}
	}

	return( i );
}
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
int getTotalSizeOfTlFile( struct twfs_file *twfs_file )
{
	int		i;

	twfs_file->tl_size = 0;

	if( twfs_file->size < DEF_TWFS_HEAD_TL_SIZE_FIELD )
	{
		logMessage( "unorganized tl file[%zu]\n", twfs_file->size );
		twfs_file->tl_size = 0;
		return( 0 );
	}

	for( i = 0 ; i < DEF_TWFS_HEAD_TL_SIZE_FIELD ; i++ )
	{
		unsigned int	size;

		size = ( unsigned int )( *( twfs_file->tl + i ) );

		if( ( size - '0' ) < 10u )
		{

			twfs_file->tl_size = twfs_file->tl_size * 10 + ( size - '0' );
		}
		else if( size == 0x00 )
		{
			break;
		}
		else
		{
			return( -1 );
		}
	}

	return( i );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getNextCursorOfTlFileFromFd
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
int getNextCursorOfListFileFromFd( int fd, char *next_cur )
{
	int		i;
	char	buf[ DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ];

	if( preadFile( fd, buf,
				   DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD,
				   DEF_TWFS_OFFSET_FF_NEXT_CUR_FIELD ) < 0 )
	{
		next_cur[ 0 ] = '-';
		next_cur[ 1 ] = '1';
		next_cur[ 2 ] = '\0';
		return( -1 );
	}

	for( i = 0 ; i < DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ; i++ )
	{
		unsigned int	cur;

		cur = ( unsigned int )( *( buf + i ) );

		if( ( cur - '0' ) < 10u )
		{
			next_cur[ i ] = ( char )cur;
		}
		else if( cur == 0x00 )
		{
			next_cur[ i ] = '\0';
			break;
		}
		else
		{
			next_cur[ 0 ] = '-';
			next_cur[ 1 ] = '1';
			next_cur[ 2 ] = '\0';
			return( -1 );
		}
	}

#if 0
	if( i < 1 )
	{
		next_cur[ 0 ] = '-';
		next_cur[ 1 ] = '1';
		next_cur[ 2 ] = '\0';
		return( -1 );
	}

	if( i == 1 && next_cur[ 0 ] == '0' && next_cur[ 1 ] == '\0' )
	{
		next_cur[ 0 ] = '-';
		next_cur[ 1 ] = '1';
		next_cur[ 2 ] = '\0';
		return( 2 );
	}
	
#endif

	return( i );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getNextCursorOfListFile
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
int getNextCursorOfListFile( struct twfs_file *twfs_file, char *next_cur )
{
	int		i;
	char	*tl;

	if( twfs_file->size < DEF_TWFS_HEAD_FF_LEN )
	{
		next_cur[ 0 ] = '-';
		next_cur[ 1 ] = '1';
		next_cur[ 2 ] = '\0';
		logMessage( "<0>getNextCursorFromTwfsFileFromId[%s]%d\n", next_cur, DEF_TWFS_HEAD_FF_LEN );
		return( -1 );
	}

	tl = twfs_file->tl + DEF_TWFS_OFFSET_FF_NEXT_CUR_FIELD;

	for( i = 0 ; i < DEF_TWFS_HEAD_FF_NEXT_CUR_FIELD ; i++ )
	{
		unsigned int	size;

		size = ( unsigned int )( *( tl + i ) );

		if( ( size - '0' ) < 10u )
		{
			next_cur[ i ] = ( char )size;
		}
		else if( size == 0x00 )
		{
			next_cur[ i ] = '\0';
			break;
		}
		else
		{
			/* initialize a cursor												*/
			i = 1;
			break;
		}
	}
	logMessage( "<1>getNextCursorFromTwfsFileFromId[%s]\n", next_cur );

#if 0
	if( i < 1 )
	{
		if( next_cur[ 0 ] == '0' )
		{
			next_cur[ 0 ] = '-';
			next_cur[ 1 ] = '1';
			next_cur[ 2 ] = '\0';
			return( 0 );
		}
	}
	logMessage( "<2>getNextCursorFromTwfsFileFromId[%s]\n", next_cur );
#endif
#if 0
	if( i == 1 && next_cur[ 0 ] == '0' && next_cur[ 1 ] == '\0' )
	{
		next_cur[ 0 ] = '-';
		next_cur[ 1 ] = '1';
		next_cur[ 2 ] = '\0';
		return( 2 );
	}

	logMessage( "<3>getNextCursorFromTwfsFileFromId[%s]\n", next_cur );

#endif
	return( i );
}

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
					   off_t offset )
{
	char				*tl;
	int					i;
	unsigned int		rtw_len;
	unsigned int		text_len;
	int					offset_of_body;
	bool				isLists;
	int					tl_record_len;
	int					text_len_offset;

	twfs_read->twfs_offset	= 0;
	twfs_read->file_size	= 0;
	twfs_read->state		= E_TWFS_READ_NONE;


	if( twfs_file->size <= 1 )
	{
		return( -1 );
	}

	switch( file_type )
	{
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		isLists			= false;
		tl_record_len	= DEF_TWFS_TL_RECORD_LEN;
		text_len_offset	= DEF_TWFS_OFFSET_TEXT_LEN_FIELD;
		offset_of_body	= DEF_TWFS_OFFSET_BODY_OF_FF;
		break;

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
		isLists			= true;
		tl_record_len	= DEF_TWFS_LISTS_RECORD_LEN;
		text_len_offset	= DEF_TWFS_OFFSET_LISTS_TEXT_LEN;
		offset_of_body	= DEF_TWFS_OFFSET_BODY_OF_FF;
		break;

	default:
		isLists			= false;
		tl_record_len	= DEF_TWFS_TL_RECORD_LEN;
		text_len_offset	= DEF_TWFS_OFFSET_TEXT_LEN_FIELD;
		offset_of_body	= DEF_TWFS_OFFSET_BODY_OF_TL;
		break;
	}
	

	tl = twfs_file->tl + offset_of_body;
	logMessage( "getTwfsFileOffset:twfs_file->size:%zu\n", twfs_file->size );
	//twfs_read->twfs_offset = offset_of_body;

	while( ( twfs_read->twfs_offset + offset_of_body )
		   < twfs_file->size )
	{
		rtw_len = 0;
		/* -------------------------------------------------------------------- */
		/* calculate "Retweeted by [screen_name]'\n'" length					*/
		/* -------------------------------------------------------------------- */
		if( !isLists )
		{
			if( *( tl + twfs_read->twfs_offset + DEF_TWFS_OFFSET_RTW_FIELD ) == 'R' )
			{
				unsigned int	rtw_len_10;

				rtw_len_10 = ( unsigned int )( *( tl
												  + twfs_read->twfs_offset
												  + DEF_TWFS_OFFSET_RTW_LEN_FIELD
												  + 0 ) ) - '0';
				if( rtw_len_10 < 10u )
				{
					rtw_len_10 = rtw_len_10 * 10;
				}
				else
				{
					twfs_read->twfs_offset += DEF_TWFS_TL_RECORD_LEN;
					continue;
				}

				rtw_len = ( unsigned int )( *( tl
											   + twfs_read->twfs_offset
											   + DEF_TWFS_OFFSET_RTW_LEN_FIELD
											   + 1 ) ) - '0';
				if( rtw_len < 10u )
				{
					rtw_len = rtw_len_10 + rtw_len;
				}
				else
				{
					twfs_read->twfs_offset += DEF_TWFS_TL_RECORD_LEN;
					continue;
				}

				if( offset < rtw_len )
				{
					twfs_read->state = E_TWFS_READ_NONE;
					return( 0 );
				}

				twfs_read->file_size += rtw_len;

				if( offset <= twfs_read->file_size )
				{
					twfs_read->state = E_TWFS_READ_OFFSET_IN_RTW;
					return( 0 );
				}
			}
		}

		/* -------------------------------------------------------------------- */
		/* calculate text length												*/
		/* -------------------------------------------------------------------- */
		text_len = 0;
		for( i = 0 ; i < DEF_TWFS_TEXT_LEN_FIELD + 1 ; i++ )
		{
			unsigned int	text_len_1;
			text_len_1 = ( unsigned int )( *( tl
											  + twfs_read->twfs_offset
											  + text_len_offset
											  + i ) );
			if( ( text_len_1  - '0' ) < 10u )
			{
				text_len_1	= text_len_1 - '0';
				text_len	= text_len * 10 + text_len_1;
				continue;
			}
			else if( text_len_1 == 0x00 )
			{
				break;
			}
		}

		if( DEF_TWFS_TEXT_LEN_FIELD < i )
		{
			twfs_read->file_size -= rtw_len;
			twfs_read->twfs_offset += tl_record_len;
			continue;
		}

		if( offset < text_len )
		{
			logMessage( "read offset none\n" );
			twfs_read->state = E_TWFS_READ_NONE;
			return( 0 );
		}

		twfs_read->file_size += text_len;
		logMessage( "calcoffset:%zu %d\n", twfs_read->file_size, text_len );

		if( offset <= twfs_read->file_size )
		{
			twfs_read->state = E_TWFS_READ_OFFSET_IN_TEXT;
			return( 0 );
		}

		twfs_read->twfs_offset += tl_record_len;
	}

	/* EOF																		*/
	return( -1 );
}
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
	Return		:size_t
				 < -1 : EOF, 0 < : read size >
	Description	:read a timeline file, direct message file, etc.
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
size_t readTimeLineFile( struct twfs_file *twfs_file,
						 struct twfs_read *twfs_read,
						 E_TWFS_FILE_TYPE file_type,
						 char *buf,
						 size_t size,
						 off_t offset )
{
	struct read_head
	{
		size_t	off_read_len;
		size_t	read_len;
	};

	struct read_head	read_head;
	char				*tl;
	size_t				save_size;
	int					offset_of_body;
	int					tl_record_len;
	int					text_len_offset;
	int					id_offset;
	int					sname_offset;
	int					id_len;
	bool				isLists;

	read_head.off_read_len	= twfs_read->file_size - offset;
	read_head.read_len		= 0;
	save_size				= size;

	switch( file_type )
	{
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		isLists			= false;
		tl_record_len	= DEF_TWFS_TL_RECORD_LEN;
		text_len_offset	= DEF_TWFS_OFFSET_TEXT_LEN_FIELD;
		id_offset		= DEF_TWFS_OFFSET_ID_FIELD;
		id_len			= DEF_TWFS_ID_FIELD;
		sname_offset	= DEF_TWFS_OFFSET_SNAME_FIELD;
		offset_of_body	= DEF_TWFS_OFFSET_BODY_OF_FF;
		break;
	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
		isLists			= true;
		tl_record_len	= DEF_TWFS_LISTS_RECORD_LEN;
		text_len_offset	= DEF_TWFS_OFFSET_LISTS_TEXT_LEN;
		id_offset		= DEF_TWFS_OFFSET_SLUG_FIELD;
		id_len			= DEF_TWFS_SLUG_FIELD;
		sname_offset	= DEF_TWFS_OFFSET_LISTS_SNAME;
		offset_of_body	= DEF_TWFS_OFFSET_BODY_OF_FF;
		break;
	default:
		isLists			= false;
		tl_record_len	= DEF_TWFS_TL_RECORD_LEN;
		text_len_offset	= DEF_TWFS_OFFSET_TEXT_LEN_FIELD;
		id_offset		= DEF_TWFS_OFFSET_ID_FIELD;
		id_len			= DEF_TWFS_ID_FIELD;
		sname_offset	= DEF_TWFS_OFFSET_SNAME_FIELD;
		offset_of_body = DEF_TWFS_OFFSET_BODY_OF_TL;
		break;
	}
	logMessage( "offset_of_body:%d\n", offset_of_body );
	tl = twfs_file->tl + offset_of_body;

#if 1
	/* ------------------------------------------------------------------------ */
	/* read contents from the offset point										*/
	/* ------------------------------------------------------------------------ */
	//while( twfs_file->size
	//	   < ( twfs_read->twfs_offset + offset_of_body ) )
	while( ( twfs_read->twfs_offset + offset_of_body ) < twfs_file->size )
	{
		if( twfs_read->state == E_TWFS_READ_OFFSET_IN_RTW )
		{
			char	rtw_msg[ sizeof( DEF_TWFS_RTW_MESSAGE ) + DEF_TWFS_SNAME_FIELD ];
			int		msg_len;
			int		i;

			msg_len = snprintf( rtw_msg, sizeof( rtw_msg ),
								"%s", DEF_TWFS_RTW_MESSAGE );
		
			for( i = 0 ; i < DEF_TWFS_SNAME_FIELD ; i ++ )
			{
				unsigned int	sname;
				sname = ( unsigned int )( *( tl
											 + twfs_read->twfs_offset
											 + DEF_TWFS_OFFSET_SNAME_FIELD
											 + i ) );
				if( sname == 0x00 )
				{
					rtw_msg[ msg_len + i ] = '\0';
					msg_len += i;
					break;
				}
				rtw_msg[ msg_len + i ] = ( char )sname;
			}

			if( DEF_TWFS_SNAME_FIELD < i )
			{
				/* go to next record												*/
				twfs_read->state = E_TWFS_READ_NONE;
				twfs_read->twfs_offset += DEF_TWFS_TL_RECORD_LEN;
				continue;
			}
			else
			{
				int		rtw_offset;
				i = 0;
				rtw_offset = msg_len - ( msg_len -  read_head.off_read_len );

				while( ( msg_len - rtw_offset ) )
				{
					*( buf + read_head.read_len ) = rtw_msg[ i + rtw_offset ];
					read_head.read_len++;
					size--;
					i++;
					msg_len--;
					if( !size )
					{
						return( read_head.read_len );
					}
				}

				read_head.off_read_len = 0;
			}

			twfs_read->state = E_TWFS_READ_NONE;
			twfs_read->twfs_offset += DEF_TWFS_TL_RECORD_LEN;
			break;
		}
		else if( twfs_read->state == E_TWFS_READ_OFFSET_IN_TEXT )
		{
			int		text_len;
			int		i;
			int		fd;
			//char	id[ DEF_TWFS_ID_FIELD + 1 ];
			char	id[ DEF_TWFS_SLUG_FIELD + 1 ];
			char	sname[ DEF_TWFS_SNAME_FIELD + 1 ];
			char	path[ DEF_TWFS_PATH_MAX ];

			//if( ( twfs_read->twfs_offset + offset_of_body )
			//	<= twfs_file->size )
			if( twfs_file->size <= ( twfs_read->twfs_offset + offset_of_body ) )
			{
				/* EOF															*/
				if( save_size == size )
				{
					logMessage( "<1>end of file\n" );
					return( 0 );
				}
				else
				{
					return( read_head.read_len );
				}
			}
			text_len = 0;
			/* ---------------------------------------------------------------- */
			/* read text length													*/
			/* ---------------------------------------------------------------- */
			for( i = 0 ; i < DEF_TWFS_TEXT_LEN_FIELD + 1 ; i++ )
			{
				unsigned int	text_len_1;
				text_len_1 = ( unsigned int )( *( tl
												  + twfs_read->twfs_offset
												  + text_len_offset
												  + i ) );
				if( ( text_len_1 - '0' ) < 10u )
				{
					text_len_1	= text_len_1 - '0';
					text_len	= text_len * 10 + text_len_1;
					continue;
				}
				else if( text_len_1 == 0x00 )
				{
					break;
				}
				else
				{
					/* go to next record										*/
					i = DEF_TWFS_TEXT_LEN_FIELD + 1;
				}
			}

			if( DEF_TWFS_TEXT_LEN_FIELD < i )
			{
				/* go to next record											*/
				twfs_read->twfs_offset += tl_record_len;
				twfs_read->state = E_TWFS_READ_NONE;
				continue;
			}

			/* ---------------------------------------------------------------- */
			/* read id															*/
			/* ---------------------------------------------------------------- */
			for( i = 0 ; i < id_len + 1 ; i++ )
			{
				unsigned int	temp_id;
				temp_id = ( unsigned int )( *( tl
											   + twfs_read->twfs_offset
											   + id_offset
											   + i ) );
				if( ( temp_id - '0' ) < 10u )
				{
					id[ i ] = ( char )temp_id;
					continue;
				}
				else if( temp_id == 0x00 )
				{
					id[ i ] = '\0';
					break;
				}
				else
				{
					/* go to next record										*/
					i = id_len + 1;
					break;
				}
			}

			if( id_len < i )
			{
				/* go to next record											*/
				twfs_read->twfs_offset += tl_record_len;
				twfs_read->state = E_TWFS_READ_NONE;
				continue;
			}
	
			/* ---------------------------------------------------------------- */
			/* read screen name													*/
			/* ---------------------------------------------------------------- */
			for( i = 0 ; i < DEF_TWFS_SNAME_FIELD + 1 ; i++ )
			{
				unsigned int	temp_sname;
				temp_sname = ( unsigned int )( *( tl
												  + twfs_read->twfs_offset
												  + sname_offset
												  + i ) );
				if( temp_sname != 0x00 )
				{
					sname[ i ] = ( char )temp_sname;
				}
				else
				{
					sname[ i ] = '\0';
					break;
				}
			}

			if( DEF_TWFS_SNAME_FIELD < i )
			{
				/* go to next record											*/
				twfs_read->twfs_offset += tl_record_len;
				twfs_read->state = E_TWFS_READ_NONE;
				continue;
			}

			/* ---------------------------------------------------------------- */
			/* read text 														*/
			/* ---------------------------------------------------------------- */
			switch( file_type )
			{
			case	E_TWFS_FILE_AUTH_DM_FR_MSG:
			case	E_TWFS_FILE_AUTH_DM_MSG:
				snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s",
												   getRootDirPath( ),
												   getTwapiScreenName( ),
												   DEF_TWFS_PATH_DIR_DM,
												   sname,
												   id );
				break;
			case	E_TWFS_FILE_FL_LIST:
			case	E_TWFS_FILE_FL_DOT_LIST:
			case	E_TWFS_FILE_FR_LIST:
			case	E_TWFS_FILE_FR_DOT_LIST:
			case	E_TWFS_FILE_BLOCKS_LIST:
			case	E_TWFS_FILE_DOT_BLOCKS_LIST:
			case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
			case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

			case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

			case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

			case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
				snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
												   getRootDirPath( ),
												   sname,
												   DEF_TWFS_PATH_DIR_ACCOUNT,
												   DEF_TWFS_PATH_PROFILE );
				break;
			
			case	E_TWFS_FILE_LISTS_SUB_LIST:
			case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
			case	E_TWFS_FILE_LISTS_OWN_LIST:
			case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
			case	E_TWFS_FILE_LISTS_ADD_LIST:
			case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
				snprintf( path, DEF_TWFS_PATH_MAX,
						  "%s/%s/%s/%s/%s/%s",
						  getRootDirPath( ),
						  sname,
						  DEF_TWFS_PATH_DIR_LISTS,
						  DEF_TWFS_PATH_DIR_OWN,
						  id,
						  DEF_TWFS_PATH_LNAME_LDESC );

				break;
			default:
				if( *( tl
					   + twfs_read->twfs_offset
					   + DEF_TWFS_OFFSET_RTW_FIELD ) == 'R' )
				{
					snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
													   getRootDirPath( ),
													   sname,
													   DEF_TWFS_PATH_DIR_RETWEET,
													   id );
				}
				else
				{
					snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
													   getRootDirPath( ),
													   sname,
													   DEF_TWFS_PATH_DIR_STATUS,
													   id );
				}
				break;
			}

			fd = openFile( path, O_RDONLY, 0666 );

			if( fd < 0 )
			{
				int		text_offset;
				//text_offset = text_len - ( text_len - read_head.off_read_len );
				text_offset = text_len - read_head.off_read_len;
				logMessage( "<0>read offset %s\n", path );
				logMessage( "*********************************\n" );
				logMessage( "text_offset : %d\n", text_offset );
				/* ------------------------------------------------------------ */
				/* unable to open or file is removed							*/
				/* ------------------------------------------------------------ */
				while( ( text_len - text_offset ) )
				{
					*( buf + read_head.read_len ) = ' ';
					size--;
					text_len--;
					if( ( text_len - text_offset ) <= 0 )
					{
						*( buf + read_head.read_len ) = '\n';
					}
					if( !size )
					{
						return( read_head.read_len );
					}
					read_head.read_len++;
				}

				read_head.off_read_len = 0;
				break;
			}
			else
			{
				int		sfile_len;
				int		text_offset;
				logMessage( "read offset\n" );
				logMessage( "*********************************\n" );
				logMessage( "read_head.off_read_len:%zu\n", read_head.off_read_len );
				logMessage( "text_len:%d\n", text_len );
				//text_offset = text_len - ( text_len - read_head.off_read_len );
				text_offset = text_len - read_head.off_read_len;
				logMessage( "text_offset : %d\n", text_offset );
				//logMessage( "text_len - text_offset:%d\n", text_len - text_offset );
				logMessage( "size:%zu\n", size );

				if( ( text_len - read_head.off_read_len ) < size )
				{
					sfile_len = preadFile( fd,
										   &buf[ read_head.read_len ],
										   text_len - text_offset,
										   text_offset );
					logMessage( "pread:%s\n", &buf[ read_head.read_len ] );
				}
				else
				{
					sfile_len = preadFile( fd,
										   &buf[ read_head.read_len ],
										   size,
										   text_offset );
					closeFile( fd );
					logMessage( "pread2:%s\n", &buf[ read_head.read_len ] );
					
					/* read is completed										*/
					return( read_head.read_len + size );
				}

				read_head.read_len		+= sfile_len;
				read_head.off_read_len	= 0;
				size					-= sfile_len;
				closeFile( fd );
				if( sfile_len < 0 )
				{
					sfile_len = 0;
				}

				/* here residential content is read, go to next					*/
				twfs_read->twfs_offset += tl_record_len;
				//twfs_read->twfs_offset += text_len - text_offset;
				twfs_read->state = E_TWFS_READ_NONE;
				break;
			}
		}
		else
		{
			break;
		}
	}

	if( twfs_file->size
		<= ( twfs_read->twfs_offset + offset_of_body ) )
	{
		if( save_size == size )
		{
			logMessage( "<2>end of file\n" );
			return( 0 );
		}
		else
		{
			logMessage( "<3>?????\n" );
			return( read_head.read_len );
		}
	}
#endif
	/* ------------------------------------------------------------------------ */
	/* read contents 															*/
	/* ------------------------------------------------------------------------ */
	while( 0 < size )
	{
		int		text_len;
		int		i;
		int		fd;
		int		result;
		//char	id[ DEF_TWFS_ID_FIELD + 1 ];
		char	id[ DEF_TWFS_SLUG_FIELD + 1 ];
		char	sname[ DEF_TWFS_SNAME_FIELD + 1 ];
		char	path[ DEF_TWFS_PATH_MAX ];

		/* -------------------------------------------------------------------- */
		/* read text length														*/
		/* -------------------------------------------------------------------- */
		text_len = 0;
		for( i = 0 ; i < DEF_TWFS_TEXT_LEN_FIELD + 1 ; i++ )
		{
			unsigned int	text_len_1;
			text_len_1 = ( unsigned int )( *( tl
											  + twfs_read->twfs_offset
											  + text_len_offset
											  + i ) );
			if( ( text_len_1 - '0' ) < 10u )
			{
				text_len_1	= text_len_1 - '0';
				text_len	= text_len * 10 + text_len_1;
				continue;
			}
			else if( text_len_1 == 0x00 )
			{
				break;
			}
			else
			{
				/* go to next record												*/
				i = DEF_TWFS_TEXT_LEN_FIELD + 1;
				break;
			}
		}

		if( DEF_TWFS_TEXT_LEN_FIELD < i )
		{
			/* go to next record												*/
			logMessage( "go to next record<0>\n" );
			twfs_read->twfs_offset += tl_record_len;
			continue;
		}

		/* -------------------------------------------------------------------- */
		/* read id																*/
		/* -------------------------------------------------------------------- */
		switch( file_type )
		{
		case	E_TWFS_FILE_FL_LIST:
		case	E_TWFS_FILE_FL_DOT_LIST:
		case	E_TWFS_FILE_FR_LIST:
		case	E_TWFS_FILE_FR_DOT_LIST:
		case	E_TWFS_FILE_BLOCKS_LIST:
		case	E_TWFS_FILE_DOT_BLOCKS_LIST:
		case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
		case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

		case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
		case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
		case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
		case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

		case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

		case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
			id[ 0 ] = '\0';
			break;
		default:
			if( isLists )
			{
				for( i = 0 ; i < id_len + 1 ; i++ )
				{
					id[ i ] = *( tl
								 + twfs_read->twfs_offset
								 + id_offset
								 + i );

					if( id[ i ] == 0x00 )
					{
						break;
					}
				}
			}
			else
			{
				for( i = 0 ; i < id_len + 1 ; i++ )
				{
					unsigned int	temp_id;
					temp_id = ( unsigned int )( *( tl
												   + twfs_read->twfs_offset
												   + id_offset
												   + i ) );
					if( ( temp_id - '0' ) < 10u )
					{
						id[ i ] = ( char )temp_id;
						continue;
					}
					else if( temp_id == 0x00 )
					{
						id[ i ] = '\0';
						break;
					}
					else
					{
						/* go to next record									*/
						i = id_len + 1;
						break;
					}
				}
			}
		
			if( id_len < i )
			{
				/* go to next record											*/
				logMessage( "go to next record<1>\n" );
				twfs_read->twfs_offset += tl_record_len;
				continue;
			}

			break;
		}

		/* -------------------------------------------------------------------- */
		/* read screen name														*/
		/* -------------------------------------------------------------------- */
		for( i = 0 ; i < DEF_TWFS_SNAME_FIELD + 1 ; i++ )
		//for( i = 0 ; i < 20 + 1 ; i++ )
		{
			unsigned int	temp_sname;
			temp_sname = ( unsigned int )( *( tl
											  + twfs_read->twfs_offset
											  + sname_offset
											  + i ) );

			if( temp_sname != 0x00 )
			{
				sname[ i ] = ( char )temp_sname;
			}
			else
			{
				sname[ i ] = '\0';
				break;
			}
		}

		if( DEF_TWFS_SNAME_FIELD < i )
		{
			/* go to next record												*/
			twfs_read->twfs_offset += tl_record_len;
			continue;
		}

		/* -------------------------------------------------------------------- */
		/* copy retweeted message 												*/
		/* -------------------------------------------------------------------- */
		if( !isLists )
		{
			if( *( tl + twfs_read->twfs_offset + DEF_TWFS_OFFSET_RTW_FIELD ) == 'R' )
			{
				char	rtw_msg[ sizeof( DEF_TWFS_RTW_MESSAGE ) + DEF_TWFS_SNAME_FIELD + 1 ];
				int		msg_len;

				msg_len = snprintf( rtw_msg, sizeof( rtw_msg ),
									"%s%s\n", DEF_TWFS_RTW_MESSAGE, sname );

				i = 0;
				while( msg_len-- )
				{
					*( buf + read_head.read_len++ ) = rtw_msg[ i++ ];
					size--;
					if( !size )
					{
						logMessage( "<4>?????\n" );
						return( read_head.read_len );
					}
				}
			}
		}

		/* -------------------------------------------------------------------- */
		/* read text 															*/
		/* -------------------------------------------------------------------- */
		switch( file_type )
		{
		case	E_TWFS_FILE_AUTH_DM_FR_MSG:
		case	E_TWFS_FILE_AUTH_DM_MSG:
			snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s/%s",
											   getRootDirPath( ),
											   getTwapiScreenName( ),
											   DEF_TWFS_PATH_DIR_DM,
											   sname,
											   id );
			break;
		case	E_TWFS_FILE_FL_LIST:
		case	E_TWFS_FILE_FL_DOT_LIST:
		case	E_TWFS_FILE_FR_LIST:
		case	E_TWFS_FILE_FR_DOT_LIST:
		case	E_TWFS_FILE_BLOCKS_LIST:
		case	E_TWFS_FILE_DOT_BLOCKS_LIST:
		case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
		case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

		case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
		case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
		case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
		case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

		case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

		case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
			snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
											   getRootDirPath( ),
											   sname,
											   DEF_TWFS_PATH_DIR_ACCOUNT,
											   DEF_TWFS_PATH_PROFILE );
			break;
		case	E_TWFS_FILE_LISTS_SUB_LIST:
		case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
		case	E_TWFS_FILE_LISTS_OWN_LIST:
		case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
		case	E_TWFS_FILE_LISTS_ADD_LIST:
		case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
			snprintf( path, DEF_TWFS_PATH_MAX,
					  "%s/%s/%s/%s/%s/%s",
					  getRootDirPath( ),
					  sname,
					  DEF_TWFS_PATH_DIR_LISTS,
					  DEF_TWFS_PATH_DIR_OWN,
					  id,
					  DEF_TWFS_PATH_LNAME_LDESC );
			break;
		default:
			if( *( tl
				   + twfs_read->twfs_offset
				   + DEF_TWFS_OFFSET_RTW_FIELD ) == 'R' )
			{
				snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
												   getRootDirPath( ),
												   sname,
												   DEF_TWFS_PATH_DIR_RETWEET,
												   id );
			}
			else
			{
				snprintf( path, DEF_TWFS_PATH_MAX, "%s/%s/%s/%s",
												   getRootDirPath( ),
												   sname,
												   DEF_TWFS_PATH_DIR_STATUS,
												   id );
			}
			break;
		}


		fd = openFile( path, O_RDONLY, 0000 );
		if( fd < 0 )
		{
			logMessage( "missing path :%s\n", path );
			while( text_len-- )
			{
				if( text_len )
				{
					*( buf + read_head.read_len++ ) = ' ';
				}
				else
				{
					*( buf + read_head.read_len++ ) = '\n';
				}
				size--;
				if( !size )
				{
					logMessage( "<5>?????\n" );
					return( read_head.read_len );
				}
			}
		}
		else
		{
			if( size <= text_len )
			{
				result = preadFile( fd, &buf[ read_head.read_len ], size, 0 );
				closeFile( fd );
				if( result < 0 )
				{
					result = 0;
				}
				result = size;

				/* read is completed											*/
				logMessage( "<6>?????\n" );
				return( read_head.read_len + result );
			}
			else
			{
				result = preadFile( fd, &buf[ read_head.read_len ], text_len, 0 );
				if( result < 0 )
				{
					result = 0;
				}

				closeFile( fd );
				//result = text_len;

				read_head.read_len += result;
				size -= result;
			}
		}

		/* go to next record													*/
		twfs_read->twfs_offset += tl_record_len;

		if( twfs_file->size
			<= ( twfs_read->twfs_offset + offset_of_body ) )
		{
			break;
		}
	}

	logMessage( "eof : size : %d\n", read_head.read_len );

	/* EOF																		*/
	if( save_size == size )
	{
		return( 0 );
	}
	else
	{
		//return( save_size - read_head.read_len );
		//buf[ read_head.read_len + 1] = 0x00;
		return( read_head.read_len );
	}
}

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
bool isTwitterId( const char *id_file )
{
	int		i;

	for( i = 0 ; i < DEF_TWAPI_MAX_USER_ID_LEN ; i++ )
	{
		unsigned int	digi;
		digi = ( unsigned int )( *( id_file + i ) );

		if( digi == 0 || digi == ' ' )
		{
			return( true );
		}
		else
		{
			if( 10 <= ( digi - '0' ) )
			{
				return( false );
			}
		}
	}

	return( false );
}

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
int makeUserListsSlugDir( const char *screen_name, const char *slug )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make slug directory														*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, true ) ) < 0 )
	{
		logMessage( "Cannot create a directory : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/members directory												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_DIR_LNAME_MEM );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, true ) ) < 0 )
	{
		logMessage( "Cannot create a directory : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/subscribers directory											*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_DIR_LNAME_SUB );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, true ) ) < 0 )
	{
		logMessage( "Cannot create a directory : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/descriptions file												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_LNAME_LDESC );

	if( ( result = createNewFile( ( const char* )path_dir, 0660, true ) ) < 0 )
	{
		logMessage( "Cannot create a file : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/timeline file													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_LNAME_TL );

	if( ( result = createNewFile( ( const char* )path_dir, 0660, true ) ) < 0 )
	{
		logMessage( "Cannot create a file : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/members/list file												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_DIR_LNAME_MEM,
					   DEF_TWFS_PATH_LNAME_MEM_LIST );

	if( ( result = createNewFile( ( const char* )path_dir, 0660, true ) ) < 0 )
	{
		logMessage( "Cannot create a file : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/members/.list file												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_DIR_LNAME_MEM,
					   DEF_TWFS_PATH_LNAME_MEM_DOT_LIST );

	if( ( result = createNewFile( ( const char* )path_dir, 0660, true ) ) < 0 )
	{
		logMessage( "Cannot create a file : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/subscribers/list file											*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_DIR_LNAME_SUB,
					   DEF_TWFS_PATH_LNAME_SUB_LIST );

	if( ( result = createNewFile( ( const char* )path_dir, 0660, true ) ) < 0 )
	{
		logMessage( "Cannot create a file : %s\n", path_dir );
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make slug/subscribers/.list file											*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   slug,
					   DEF_TWFS_PATH_DIR_LNAME_SUB,
					   DEF_TWFS_PATH_LNAME_SUB_DOT_LIST );

	if( ( result = createNewFile( ( const char* )path_dir, 0660, true ) ) < 0 )
	{
		logMessage( "Cannot create a file : %s\n", path_dir );
		return( result );
	}

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
	Function	:makeUserHomeDirectory
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make user home directory
================================================================================
*/
static int makeUserHomeDirectory( const char *screen_name, bool daemon )
{
	int			result;

	/* ------------------------------------------------------------------------ */
	/* make user screen name directory											*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserScreenNameDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* make direct_message directory											*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserDirectMessageDir( screen_name,
											 screen_name,
											 daemon ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* make retweet directory													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserRetweetDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* make account directory													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserAccountDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}

	if( strncmp( screen_name,
				 getTwapiScreenName( ),
				 DEF_TWAPI_MAX_SCREEN_NAME_LEN ) == 0 )
	{
		/* -------------------------------------------------------------------- */
		/* make notifications directory											*/
		/* -------------------------------------------------------------------- */
		if( ( result = makeUserNotificationsDir( screen_name, daemon ) ) < 0 )
		{
			return( result );
		}
		/* -------------------------------------------------------------------- */
		/* make blocks directory												*/
		/* -------------------------------------------------------------------- */
		if( ( result = makeUserBlocksDir( screen_name, daemon ) ) < 0 )
		{
			return( result );
		}
	}
	/* ------------------------------------------------------------------------ */
	/* make favorites directory													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserFavoritesDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* make follower directory													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserFollowerDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* make following directory													*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserFriendsDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* make lists directory														*/
	/* ------------------------------------------------------------------------ */
	if( ( result = makeUserListsDir( screen_name, daemon ) ) < 0 )
	{
		return( result );
	}
	
	return( 0 );
}

/*
================================================================================
	Function	:makeUserScreenNameDirectory
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name] directory
================================================================================
*/
int makeUserScreenNameDir( const char *screen_name, bool daemon )
{
	int			result;
	char		path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make screen name directory												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s",
					   getRootDirPath( ), screen_name );

	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
	}

	/* ------------------------------------------------------------------------ */
	/* make status directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name, DEF_TWFS_PATH_DIR_STATUS );

	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
	}
#if 1
	/* ------------------------------------------------------------------------ */
	/* make tweet file															*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name, DEF_TWFS_PATH_TWEET );
	
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		
		if( daemon )
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}
#endif

	/* ------------------------------------------------------------------------ */
	/* make home_timeline file													*/
	/* ------------------------------------------------------------------------ */
	if( strncmp( screen_name,
				 getTwapiScreenName( ),
				 DEF_TWAPI_MAX_SCREEN_NAME_LEN ) == 0 )
	{
		result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
						   getRootDirPath( ), screen_name, DEF_TWFS_PATH_TL );
	
		if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
		{
			if( daemon )
			{
				logMessage( "Cannot create a file : %s\n", path_dir );
			}
			else
			{
				printf( "Cannot create a file : %s\n", path_dir );
			}
			return( result );
		}
	}

	/* ------------------------------------------------------------------------ */
	/* make user_timeline file													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name, DEF_TWFS_PATH_USER_TL );
	
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}
/*
================================================================================
	Function	:makeUserDirectMessageDir
	Input		:const char *screen_name
				 < screen name >
				 const char *frineds_name
				 < screen name of friends >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/direct_message/[friends_name] directory
================================================================================
*/
static int makeUserDirectMessageDir( const char *screen_name,
									 const char *friends_name,
									 bool daemon )
{
	int			name_len;
	int			result;
	char		path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make direct_massage directory											*/
	/* ------------------------------------------------------------------------ */
	name_len = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
						 getRootDirPath( ), screen_name, DEF_TWFS_PATH_DIR_DM );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make direct_massage/[friends_name] directory								*/
	/* ------------------------------------------------------------------------ */
	name_len = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
						 getRootDirPath( ),screen_name,
						 DEF_TWFS_PATH_DIR_DM, friends_name );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make direct_massage/[friends_name]/message file							*/
	/* ------------------------------------------------------------------------ */
	name_len = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
						 getRootDirPath( ),screen_name,
						 DEF_TWFS_PATH_DIR_DM, friends_name,
						 DEF_TWFS_PATH_DM_MSG );
	
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make direct_massage/[friends_name]/send_to file							*/
	/* ------------------------------------------------------------------------ */
	name_len = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
						 getRootDirPath( ),screen_name,
						 DEF_TWFS_PATH_DIR_DM, friends_name,
						 DEF_TWFS_PATH_DM_SEND_TO );
	
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make direct_massage/message file											*/
	/* ------------------------------------------------------------------------ */
	name_len = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
						 getRootDirPath( ),screen_name,
						 DEF_TWFS_PATH_DIR_DM,
						 DEF_TWFS_PATH_DM_MSG );
	
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make direct_massage/send_to file											*/
	/* ------------------------------------------------------------------------ */
	name_len = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
						 getRootDirPath( ),screen_name,
						 DEF_TWFS_PATH_DIR_DM,
						 DEF_TWFS_PATH_DM_SEND_TO );
	
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}
	return( result );
}

/*
================================================================================
	Function	:makeUserRetweetDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/retweet directory
================================================================================
*/
int makeUserRetweetDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make profile directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_RETWEET );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}

/*
================================================================================
	Function	:makeUserAccountDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/account directory
================================================================================
*/
int makeUserAccountDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make profile directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_ACCOUNT );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make profile file														*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_ACCOUNT,
					   DEF_TWFS_PATH_PROFILE );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make settings file														*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_ACCOUNT,
					   DEF_TWFS_PATH_SETTINGS );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}

/*
================================================================================
	Function	:makeUserNotificationsDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/notifications directory
================================================================================
*/
int makeUserNotificationsDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make notifications directory												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_NOTI );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make @tweet file															*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_NOTI,
					   DEF_TWFS_PATH_AT_TW );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make retweets_of_me file													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_NOTI,
					   DEF_TWFS_PATH_RTW );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}

/*
================================================================================
	Function	:makeUserBlocksDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/blocks directory
================================================================================
*/
int makeUserBlocksDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make blocks directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_BLOCKS );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make list file															*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_BLOCKS,
					   DEF_TWFS_PATH_BLOCK_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make .list file															*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_BLOCKS,
					   DEF_TWFS_PATH_DOT_BLOCK_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}
/*
================================================================================
	Function	:makeUserFavoritesDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/favorites directory
================================================================================
*/
int makeUserFavoritesDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make favorites directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FAV );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make list file															*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FAV,
					   DEF_TWFS_PATH_FAV_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}
/*
================================================================================
	Function	:makeUserFollowerDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/follwer directory
================================================================================
*/
int makeUserFollowerDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make follower directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FOLLOWERS );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make follower list file													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FOLLOWERS,
					   DEF_TWFS_PATH_FF_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make follower list file													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FOLLOWERS,
					   DEF_TWFS_PATH_FF_DOT_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}

/*
================================================================================
	Function	:makeUserFriendsDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/following directory
================================================================================
*/
int makeUserFriendsDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make following directory													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FRIENDS );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make following list file													*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FRIENDS,
					   DEF_TWFS_PATH_FF_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make following .list file												*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_FRIENDS,
					   DEF_TWFS_PATH_FF_DOT_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}

/*
================================================================================
	Function	:makeUserListsDir
	Input		:const char *screen_name
				 < screen name >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make [screen_name]/lists directory
================================================================================
*/
int makeUserListsDir( const char *screen_name, bool daemon )
{
	int		result;
	char	path_dir[ DEF_TWFS_PATH_MAX ];

	/* ------------------------------------------------------------------------ */
	/* make lists directory														*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/list file										*/
	/* ------------------------------------------------------------------------ */
#if 0
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}
#endif
	
	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/subscriptions/ directory						*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_SUB );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/subscriptions/list file							*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_SUB,
					   DEF_TWFS_PATH_SUB_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

#if 0
	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/subscriptions/subscribers file					*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_SUB,
					   DEF_TWFS_PATH_SUB_SUBSCRIBERS );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/subscriptions/members file						*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_SUB,
					   DEF_TWFS_PATH_SUB_MEMBERSHIPS );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}
#endif

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/my_list/ directory								*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/my_list/list file								*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   DEF_TWFS_PATH_OWN_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

#if 0
	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/my_list/subscribers file						*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   DEF_TWFS_PATH_OWN_SUBSCRIBERS );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/my_list/members file							*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_OWN,
					   DEF_TWFS_PATH_OWN_MEMBERSHIPS );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}
#endif

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/added/ directory								*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_ADD );
	
	if( ( result = makeDirectory( ( const char* )path_dir, 0770, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a directory : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a directory : %s\n", path_dir );
		}
		return( result );
	}

	/* ------------------------------------------------------------------------ */
	/* make [screen_name]/lists/added/list file									*/
	/* ------------------------------------------------------------------------ */
	result = snprintf( path_dir, sizeof( path_dir ), "%s/%s/%s/%s/%s",
					   getRootDirPath( ), screen_name,
					   DEF_TWFS_PATH_DIR_LISTS,
					   DEF_TWFS_PATH_DIR_ADD,
					   DEF_TWFS_PATH_ADD_LIST );
	if( ( result = createNewFile( ( const char* )path_dir, 0660, daemon ) ) < 0 )
	{
		if( daemon )
		{
			logMessage( "Cannot create a file : %s\n", path_dir );
		}
		else
		{
			printf( "Cannot create a file : %s\n", path_dir );
		}
		return( result );
	}

	return( result );
}



/*
================================================================================
	Function	:mmapTwfsListFile
	Input		:struct *twfs_file
				 < twfs file information >
	Output		:void
	Return		:int
				 < status >
	Description	:mmap list file
================================================================================
*/
int mmapTwfsListFile( struct twfs_file *twfs_file )
{
	size_t	mmap_size;

	if( twfs_file->size < DEF_TWFS_HEAD_FF_LEN )
	{
		mmap_size = DEF_TWFS_HEAD_TL_SIZE_FIELD_LEN
					+ ( DEF_TWFS_TL_RECORD_LEN
						* DEF_TWOPE_MAX_FF_COUNT );
	}
	else
	{
		mmap_size = twfs_file->size
					+ ( DEF_TWFS_TL_RECORD_LEN
						* DEF_TWOPE_MAX_FF_COUNT );
	}
	twfs_file->tl = ( char* )mmap( NULL, mmap_size,
								   PROT_WRITE, MAP_SHARED,
								   twfs_file->fd, 0 );

	if( twfs_file->tl == MAP_FAILED )
	{
		logMessage( "mmap failed at twfsOpen\n" );
		closeTwfsFile( &twfs_file );
		return( -ENOMEM );
	}

	return( 0 );
}

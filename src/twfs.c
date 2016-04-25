/*******************************************************************************
 File:twfs.c
 Description:Operations of twitter pesudo filesystem

*******************************************************************************/
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <libgen.h>
#include <sys/xattr.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "twfs.h"
#include "twfs_internal.h"
#include "twitter_operation.h"
#include "lib/log.h"
#include "lib/utils.h"
#include "lib/utf.h"
#include "net/twitter_api.h"
#include "net/network.h"

/* fuse.h must be included here	*/
#include <fuse.h>

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int twfsGetAttributes( const char *path, struct stat *statbuf );
static int twfsReadLink( const char *path, char *link, size_t size );
static int twfsMakeNode( const char *path, mode_t mode, dev_t dev );
static int twfsMakeDirectory( const char *path, mode_t mode );
static int twfsUnlink( const char *path );
static int twfsRemoveDirectory( const char *path );
static int twfsSymbolicLink( const char *path, const char *link );
static int twfsRename( const char *path, const char *newpath );
static int twfsHardLink( const char *path, const char *hlink );
static int twfsChangeMode( const char *path, mode_t mode );
static int twfsChangeOwner( const char *path, uid_t uid, gid_t gid );
static int twfsTruncate( const char *path, off_t new_size );
static int twfsUtime( const char *path, struct utimbuf *ubuf );
static int twfsOpen( const char *path, struct fuse_file_info *fi );
static int twfsRead( const char *path,
					 char *buf,
					 size_t size,
					 off_t offset,
					 struct fuse_file_info *fi );
static int twfsWrite( const char *path,
					  const char *buf,
					  size_t size,
					  off_t offset,
					  struct fuse_file_info *fi );
static int twfsStatisticsFileSystem( const char *path, struct statvfs *statv );
static int twfsFlush( const char *path, struct fuse_file_info *fi );
static int twfsRelease( const char *path, struct fuse_file_info *fi );
static int
twfsFileSync( const char *path, int datasync, struct fuse_file_info *fi );
static int
twfsSetExtendedAttributes( const char *path,
						   const char *name,
						   const char *value,
						   size_t size,
						   int flags );
static int
twfsGetExtendedAttributes( const char *path,
						   const char *name,
						   char *value,
						   size_t size );
static int
twfsListExtendedAttributes( const char *path,
							char *list,
							size_t size );
static int
twfsRemoveExtendedAttributes( const char *path,
							  const char *name );
static int
twfsOpenDirectory( const char *path, struct fuse_file_info *fi );
static int twfsReadDirectory( const char *path,
							  void *buf,
							  fuse_fill_dir_t filler,
							  off_t offset,
							  struct fuse_file_info *fi );
static int
twfsReleaseDirectory( const char *path, struct fuse_file_info *fi );
static int
twfsFileSyncDirectory ( const char *path,
						int datasync,
						struct fuse_file_info *fi );
static void* initTwfs( struct fuse_conn_info *conn );
static void destroyTwfs( void *userdata );
static int twfsAccess( const char *path, int mask );
static int
twfsCreate( const char *path, mode_t mode, struct fuse_file_info *fi );
static int
twfsFileTruncate( const char *path, off_t offset, struct fuse_file_info *fi );
static int
twfsFileGetAttributes( const char *path,
					   struct stat *statbuf,
					   struct fuse_file_info *fi );

/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_LOGICAL_BLOCK_SIZE			512


/*
================================================================================

	Management

================================================================================
*/
//static char root_dir[ DEF_TWFS_PATH_MAX ];
struct root_dir
{
	char	*path;
	int		length;
};
static struct root_dir root_dir;

struct mount_dir
{
	char	*path;
	int		length;
};
static struct mount_dir mount_dir;
static void ( *destroy )( void );
static struct fuse_operations* getTwfsOperations( void );

/*
==================================================================================

	TWFS file system operations

==================================================================================
*/
struct fuse_operations twfs_operations =
{
	.getattr		= twfsGetAttributes,
	.readlink		= twfsReadLink,
	// no .getdir. that is deprecated
	.getdir			= NULL,
	.mknod			= twfsMakeNode,
	.mkdir			= twfsMakeDirectory,
	.unlink			= twfsUnlink,
	.rmdir			= twfsRemoveDirectory,
	.symlink		= twfsSymbolicLink,
	.rename			= twfsRename,
	.link			= twfsHardLink,
	.chmod			= twfsChangeMode,
	.chown			= twfsChangeOwner,
	.truncate		= twfsTruncate,
	.utime			= twfsUtime,
	.open			= twfsOpen,
	.read			= twfsRead,
	.write			= twfsWrite,
	.statfs			= twfsStatisticsFileSystem,
	.flush			= twfsFlush,
	.release		= twfsRelease,
	.fsync			= twfsFileSync,
	.setxattr		= twfsSetExtendedAttributes,
	.getxattr		= twfsGetExtendedAttributes,
	.listxattr		= twfsListExtendedAttributes,
	.removexattr	= twfsRemoveExtendedAttributes,
	.opendir		= twfsOpenDirectory,
	.readdir		= twfsReadDirectory,
	.releasedir		= twfsReleaseDirectory,
	.fsyncdir		= twfsFileSyncDirectory,
	.init			= initTwfs,
	.destroy		= destroyTwfs,
	.access			= twfsAccess,
	.create			= twfsCreate,
	.ftruncate		= twfsFileTruncate,
	.fgetattr		= twfsFileGetAttributes,
};
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
int initTwitterFileSystem( void( *call_back )( void ) )
{
	int		result;

	setTwfsDestroy( call_back );

	result = initTwfsInternal( );

	return( result );
}

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
void setTwfsDestroy( void ( *call_back )( void ) )
{
	destroy = call_back;
}

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
int startTwfs( int argc, char *argv[ ] )
{
	int		result;

	printf( "mount dir : %s\n" , argv[ 3 ] );

	argv[ 1 ] = argv[ 3 ];
	argv[ 2 ] = argv[ 3 ] = NULL;
	argc = argc - 2;

	result =  fuse_main( argc, argv, &twfs_operations, NULL );

	return( result );
}

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
void getRootAbsPath( char *r_path, const char *path )
{
	strcpy( r_path, root_dir.path );
	strncat( r_path, path, DEF_TWFS_PATH_MAX );
}

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
void getMountAbsPath( char *r_path, const char *path )
{
	strcpy( r_path, mount_dir.path );
	strncat( r_path, path, DEF_TWFS_PATH_MAX );
}


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
int setRootDirPath( const char *root )
{
	int		result;
	if( !root )
	{
		return( -1 );
	}

	result = strlen( root );

	if( DEF_TWFS_PATH_MAX <= result )
	{
		return( -1 );
	}

	root_dir.path	= realpath( root, NULL );
	root_dir.length	= strlen( root_dir.path );

	return( 0 );
}

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
int setMountDirPath( const char *mount )
{
	int		result;
	if( !mount )
	{
		return( -1 );
	}

	result = strlen( mount );

	if( DEF_TWFS_PATH_MAX <= result )
	{
		return( -1 );
	}

	mount_dir.path		= realpath( mount, NULL );

	if (!mount_dir.path) {
		if (errno == ENOTCONN) {
			printf("You may already mounted %s directory.\n", mount);
			printf("Try \"fusermount -u %s\" command and then revoke Twitter Filesystem!\n", mount);
			return(-1);
		}
		printf("unknown error occured. at %s\n", __FUNCTION__);
		return(-1);
	}

	mount_dir.length	= strlen( mount_dir.path );

	return( 0 );
}

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
const char* getRootDirPath( void )
{
	return( root_dir.path );
}

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
const char* getMountDirPath( void )
{
	return( mount_dir.path );
}

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
int getRootDirLength( void )
{
	return( root_dir.length );
}

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
int getMountDirLength( void )
{
	return( mount_dir.length );
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
	Function	:twfsGetAttributes
	Input		:const char *path
				 < path to file >
				 struct stat *statbuf
				 < statistics information buffer >
	Outpu		:struct stat *statbuf
				 < statistics information buffer >
	Return		:int
				 < status >
	Description	:get file attributes
==================================================================================
*/
static int twfsGetAttributes( const char *path, struct stat *statbuf )
{
	int					result;
	int					fd;
	size_t				total_size;
	int					at_name_len;
	char				r_path[ DEF_TWFS_PATH_MAX ];
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	blkcnt_t	phys_blks;
	blksize_t	phys_blksize;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );
	
	getRootAbsPath( r_path, path );

	result = lstat( r_path, statbuf );

	logMessage( "getattr:%s\n", r_path );

	if( result < 0 )
	{
		return( -errno );
	}

	switch( file_type )
	{
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		fd = openFile( r_path, O_RDONLY, 0600 );

		logMessage( "file type[%d]\n", file_type );

		if( fd < 0 )
		{
			result = -errno;
			break;
		}

		if( getTotalSizeOfTlFileFromFd( fd, &total_size ) < 0 )
		{
			total_size = 1;
		}

		if( total_size == 0 )
		{
			total_size = 1;
		}
		logMessage( "total_size:%zu\n", total_size );

		closeFile( fd );

		statbuf->st_size = total_size;
		phys_blks = ( statbuf->st_size + ( statbuf->st_blksize - 1 ) )
					/ statbuf->st_blksize;
		phys_blksize = statbuf->st_blksize * phys_blks;
		statbuf->st_blocks = ( phys_blksize + ( DEF_LOGICAL_BLOCK_SIZE - 1 ) )
							 / DEF_LOGICAL_BLOCK_SIZE;

		break;
	case	E_TWFS_FILE_TWEET:
	case	E_TWFS_FILE_DM_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_SEND_TO:
	case	E_TWFS_FILE_DM_FR_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_FR_SEND_TO:
		at_name_len			= strnlen( sname1,
									   DEF_TWAPI_MAX_SCREEN_NAME_LEN );
		// 1 for @, 1 for space -> @screen_name[space]
		at_name_len			+= 2;
		statbuf->st_size	= at_name_len;
		
		phys_blks = ( statbuf->st_size + ( statbuf->st_blksize - 1 ) )
					/ statbuf->st_blksize;
		phys_blksize = statbuf->st_blksize * phys_blks;
		statbuf->st_blocks = ( phys_blksize + ( DEF_LOGICAL_BLOCK_SIZE - 1 ) )
							 / DEF_LOGICAL_BLOCK_SIZE;

		logMessage( "st_size : %zu\n", statbuf->st_size );
		logMessage( "st_blocks : %zu\n", statbuf->st_blocks );
		logMessage( "st_blksize : %zu\n", statbuf->st_blksize );
		break;
	case	E_TWFS_FILE_AUTH_TWEET:
		statbuf->st_blksize = 0;
		break;
	default:
		break;
	}
	
	logMessage( "file type[%d]\n", file_type );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsReadLink
	Input		:const char *path
				 < path to file >
				 char *link
				 < link information buffer >
				 size_t size
				 < size of buffer >
	Outpu		:char *link
				 < link information buffer >
	Return		:int
				 < status >
	Description	:read link
==================================================================================
*/
static int twfsReadLink( const char *path, char *link, size_t size )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "read link path:%s\n", path );
	logMessage( "readlink:%s\n", r_path );
	
	if( ( result = readlink( r_path, link, size - 1 ) ) < 0 )
	{
		/* nothing to do	*/
		return( -errno );
	}
	else
	{
		link[ result ] = '\0';
		result = 0;
	}

	logMessage( "readlink link:%s\n", link );

	
	return( result );
}

/*
==================================================================================
	Function	:twfsMakeNode
	Input		:const char *path
				 < path to file >
				 mode_t mode
				 < file mode >
				 dev_t dev
				 < device >
	Outpu		:void
	Return		:int
				 < status >
	Description	:make node 
==================================================================================
*/
static int twfsMakeNode( const char *path, mode_t mode, dev_t dev )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	result = 0;
	getRootAbsPath( r_path, path );

	logMessage( "mknod:%s\n", r_path );
	
	if( S_ISREG( mode ) )
	{
		if( ( result = open( r_path, O_CREAT | O_EXCL | O_WRONLY, mode ) ) < 0 )
		{
			/* nothing to do	*/
		}
		else
		{
			result = close( result );
		}
	}
	else if( S_ISFIFO( mode ) )
	{
		result = mkfifo( r_path, mode );
	}
	else
	{
		result = mknod( r_path, mode, dev );
	}
	
	return( result );
}

/*
==================================================================================
	Function	:twfsMakeDirectory
	Input		:const char *path
				 < path to file >
				 mode_t mode
				 < file mode >
	Outpu		:void
	Return		:int
				 < status >
	Description	:make directory
==================================================================================
*/
static int twfsMakeDirectory( const char *path, mode_t mode )
{
	int					result;
	char				r_path[ DEF_TWFS_PATH_MAX ];
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;

	getRootAbsPath( r_path, path );

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );

	logMessage( "mkdir:%s\n", r_path );

	switch( file_type )
	{
	case	E_TWFS_DIR_AUTH_LISTS_OWN_LNAME:
#if 0
		result = makeUserListsSlugDir( sname1, sname2 );
		if( 0 <= result )
		{
			result = createLists( getCurrentSSLSession( ),
								  sname2,
								  NULL );
		}
#endif
#if 0
		result = readLists( E_TWFS_REQ_CREATE_LISTS,
							getCurrentSSLSession( ),
							NULL,
							sname1,
							NULL,
							sname2 );
#endif
		result = makeLists( E_TWFS_REQ_CREATE_LISTS,
							getCurrentSSLSession( ),
							sname1,
							sname2 );
		if( result < 0 )
		{
			return( -EACCES );
		}
		break;
	default:
		result = mkdir( r_path, mode );
		break;
	}

	
	return( result );
}

/*
==================================================================================
	Function	:twfsUnlink
	Input		:const char *path
				 < path to file >
	Outpu		:void
	Return		:int
				 < status >
	Description	:unlink a file
==================================================================================
*/
static int twfsUnlink( const char *path )
{
	int					result = 0;
	char				r_path[ DEF_TWFS_PATH_MAX ];
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	
	logMessage( "path:%s\n", path );

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );

	switch( file_type )
	{
	case	E_TWFS_FILE_AUTH_STATUS:
		logMessage( "auth status file![%s]\n", sname2 );
		/* sname2 has tweet id which is status file name						*/
		result = removeTweet( getCurrentSSLSession( ), sname2 );

		break;
	case	E_TWFS_FILE_AUTH_DM_FR_STATUS:
		logMessage( "auth dm to friends status[%s]\n", sname2 );
		/* sname2 is saved as direct message at whichTwfsPath					*/
		result = removeDirectMessage( getCurrentSSLSession( ), sname2 );

		break;
	case	E_TWFS_DIR_AUTH_FR_FR:
		if( sname2[ 0 ] != '\0' )
		{
			result = requestUnfollow( getCurrentSSLSession( ), sname2 );
		}
		break;
	case	E_TWFS_FILE_AUTH_FAV_TWEET:
		logMessage( "auth fav tweet[%s]\n", sname2 );
		/* sname2 is saved as tweet id at whichTwfsPath							*/
		result = requestUnFavorite( getCurrentSSLSession( ), sname2 );
		break;
	case	E_TWFS_FILE_AUTH_RTW:
		logMessage( "auth rettweet[%s]\n", sname2 );
		/* sname2 is saved as tweet id at whichTwfsPath							*/
		result = removeTweet( getCurrentSSLSession( ), sname2 );
		break;
	case	E_TWFS_FILE_AUTH_BLOCKS:
		logMessage( "auth blocks[%s]\n", sname2 );
		/* sname2 is saved as screen name at whichTwfsPath						*/
		result = requestUnblock( getCurrentSSLSession( ), sname2 );
		break;
	case	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_MEM_SNAME:
		result = destroyListsMembers( getCurrentSSLSession( ),
									  sname2,
									  sname1,
									  sname3 );
		break;
	case	E_TWFS_DIR_LISTS_SUB_LNAME:
		result = 0;
		if( strncmp( sname1,
					 getTwapiScreenName( ),
					 DEF_TWAPI_MAX_SCREEN_NAME_LEN ) == 0 )
		{
			result = stopSubscribeLists( getCurrentSSLSession( ),
										 sname2,
										 sname1 );
		}

		break;
	default:
		break;
	}
	
	getRootAbsPath( r_path, path );

	logMessage( "unlink:%s\n", r_path );
	
	result = unlink( r_path );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsRemoveDirectory
	Input		:const char *path
				 < path to file >
	Outpu		:void
	Return		:int
				 < status >
	Description	:remove a directory
==================================================================================
*/
static int twfsRemoveDirectory( const char *path )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );
	
	getRootAbsPath( r_path, path );

	logMessage( "rmdir:%s\n", r_path );
	
	result = rmdir( r_path );

	switch( file_type )
	{
	case	E_TWFS_DIR_AUTH_LISTS_OWN_LNAME:
		/* sname2 = slug														*/
		result = deleteLists( getCurrentSSLSession( ), sname1, sname2 );
		break;
	default:
		break;
	}
	
	return( result );
}

/*
==================================================================================
	Function	:twfsSymbolicLink
	Input		:const char *path
				 < path to file >
				 const char *link
				 < link information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:create a symbolic link
==================================================================================
*/
static int twfsSymbolicLink( const char *path, const char *link )
{
	int					result;
	char				link_r_path[ DEF_TWFS_PATH_MAX ];
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	char				link_sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				link_sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				link_sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				link_sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	link_file_type;
	bool				symlink_ok = false;

	if( path[ 0 ] != '/' )
	{
		char			real_link[ DEF_TWFS_PATH_MAX ];
		/* real_link = root absolute path								*/
		getRootAbsPath( real_link, link );
		/* link_r_path = directory of root absolute path				*/
		getDirName( link_r_path, real_link );
		/* link_r_path = get relative link source path to absolute path	*/
		strncat( link_r_path, path, DEF_TWFS_PATH_MAX );
		/* real_link = get rid of .. or . representation				*/
		realpath( link_r_path, real_link );
		/* link_r_path = mount point path								*/
		result = getTwfsPathFromRootDir( real_link, link_r_path );
	}
	else
	{
		logMessage( "path:%s\n", path );
		result = getTwfsPathFromMountDir( path, link_r_path );
		logMessage( "link_r_path(%d):%s\n", result, link_r_path );
		logMessage( "mount:%s\n", getMountDirPath( ) );
	}

	if( result < 0 )
	{
		file_type	= whichTwfsPath( path, sname1, sname2, sname3 );
	}
	else
	{
		file_type	= whichTwfsPath( link_r_path, sname1, sname2, sname3 );
	}
	link_file_type	= whichTwfsPath( link, link_sname1, link_sname2, link_sname3 );

	logMessage( "link:%s\n", link );

	switch( file_type )
	{
	case	E_TWFS_FILE_FIRST_SNAME:
	case	E_TWFS_FILE_STATUS:
	case	E_TWFS_FILE_AUTH_STATUS:
	case	E_TWFS_FILE_RTW:
	case	E_TWFS_FILE_AUTH_RTW:
	//case	E_TWFS_DIR_LISTS_SUB_LNAME:
	//case	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_MEM_SNAME:
	case	E_TWFS_DIR_LISTS_OWN_LNAME:
		logMessage( "symlink:%s\n", link_r_path );
		logMessage( "file_type[%d] link_file_type[%d]\n", file_type, link_file_type );
		symlink_ok = true;
		break;
	default:
		logMessage( "NG symlink:%s\n", link_r_path );
		logMessage( "file_type[%d] link_file_type[%d]\n", file_type, link_file_type );
		symlink_ok = false;
		break;
	}
	
	result = 0;

	switch( link_file_type )
	{
	case	E_TWFS_DIR_AUTH_FR_FR:
		//if( symlink_ok )
		if( file_type == E_TWFS_FILE_FIRST_SNAME )
		{
			logMessage( "link_sname1:%s\n", link_sname1 );
			logMessage( "link_sname2:%s\n", link_sname2 );
			result = requestFollow( getCurrentSSLSession( ), link_sname2 );
		}
		break;
	case	E_TWFS_FILE_AUTH_FAV_TWEET:
		//if( symlink_ok )
		if( ( file_type == E_TWFS_FILE_STATUS ) ||
			( file_type == E_TWFS_FILE_AUTH_STATUS ) )
		{
			logMessage( "link_sname2:%s\n", link_sname2 );
			/* link_sname2 has teet id at whichTwfsPath							*/
			result = requestFavorite( getCurrentSSLSession( ), link_sname2 );
		}
		break;
	//case	E_TWFS_FILE_AUTH_STATUS:
	case	E_TWFS_FILE_AUTH_RTW:
		//if( symlink_ok )
		if( file_type == E_TWFS_FILE_STATUS )
		{
			if( file_type == E_TWFS_FILE_AUTH_STATUS )
			{
				return( -EACCES );
			}
			logMessage( "link_sname2:%s\n", link_sname2 );
			/* sname2 has teet id at whichTwfsPath								*/
			getRootAbsPath( link_r_path, link );
			//result = retweet( getCurrentSSLSession( ), sname2 );
			result = symlinkRetweet( getCurrentSSLSession( ), sname2 );
#if 0
			if( result < 0 )
			{
				return( -EACCES );
			}
#endif

			return( 0 );
		}
		break;
	case	E_TWFS_FILE_AUTH_BLOCKS:
		//if( symlink_ok )
		if( file_type == E_TWFS_FILE_FIRST_SNAME )
		{
			result = requestBlock( getCurrentSSLSession( ), link_sname2 );
		}
		break;
	case	E_TWFS_DIR_LISTS_SUB_LNAME:
		//if( symlink_ok )
		if( file_type == E_TWFS_DIR_LISTS_OWN_LNAME )
		{
			if( strncmp( link_sname1,
						 getTwapiScreenName( ),
						 DEF_TWAPI_MAX_SCREEN_NAME_LEN ) == 0 )
			{
				result = subscribeLists( getCurrentSSLSession( ),
										 link_sname2,
										 sname1 );
			}
			else
			{
				return( -EACCES );
			}
		}
		break;
	case	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_MEM_SNAME:
		//if( symlink_ok )
		if( file_type == E_TWFS_FILE_FIRST_SNAME )
		{
			if( strncmp( link_sname1,
						 getTwapiScreenName( ),
						 DEF_TWAPI_MAX_SCREEN_NAME_LEN ) == 0 )
			{
				result = createListsMembers( getCurrentSSLSession( ),
											 link_sname2,
											 link_sname1,
											 sname1 );
			}
			
		}
		break;
	default:
		logMessage( "file type[%d]\n", file_type );
		break;
	}

#if 1
	if( result < 0 )
	{
		return( -EACCES );
	}
#endif

	getRootAbsPath( link_r_path, link );
	logMessage( "symlink:%s %s\n", path, link_r_path );

	result = symlink( path, link_r_path );

	if( result < 0 )
	{
		return( -errno );
	}
	
	return( result );
}

/*
==================================================================================
	Function	:twfsRename
	Input		:const char *path
				 < path to file >
				 const char *newpath
				 < new path >
	Outpu		:void
	Return		:int
				 < status >
	Description	:rename a file name
==================================================================================
*/
static int twfsRename( const char *path, const char *newpath )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	char	new_r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );
	getRootAbsPath( new_r_path, newpath );

	logMessage( "rename:%s %s\n", r_path, new_r_path );
	
	result = rename( r_path, new_r_path );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsHardLink
	Input		:const char *path
				 < path to file >
				 const char *hlink
				 < hard link information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:create a hard link
==================================================================================
*/
static int twfsHardLink( const char *path, const char *hlink )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	char	link_r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );
	getRootAbsPath( link_r_path, hlink );

	logMessage( "link:%s %s\n", r_path, link_r_path );
	
	result = link( r_path, link_r_path );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsChangeMode
	Input		:const char *path
				 < path to file >
				 mode_t mode
				 < file mode >
	Outpu		:void
	Return		:int
				 < status >
	Description	:change file mode
==================================================================================
*/
static int twfsChangeMode( const char *path, mode_t mode )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "chmod:%s %o\n", r_path, mode );
	
	result = chmod( r_path, mode );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsChangeOwner
	Input		:const char *path
				 < path to file >
				 uid_t uid
				 < user id >
				 gid_t gid
				 < group id >
	Outpu		:void
	Return		:int
				 < status >
	Description	:change owner of a file
==================================================================================
*/
static int twfsChangeOwner( const char *path, uid_t uid, gid_t gid )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "chmod:%s %o %o\n", r_path, uid, gid );
	
	result = chown( r_path, uid, gid );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsTruncate
	Input		:const char *path
				 < path to file >
				 off_t new_size
				 < size of file size to truncate >
	Outpu		:void
	Return		:int
				 < status >
	Description	:change the size of a file
==================================================================================
*/
static int twfsTruncate( const char *path, off_t new_size )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );
	
	logMessage( "truncate:%s %d\n", r_path, new_size );
	
	result = truncate( r_path, new_size );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsUtime
	Input		:const char *path
				 < path to file >
				 utimbuf *ubuf
				 < buffer for utime >
	Outpu		:void
	Return		:int
				 < status >
	Description	:change access/modification times of a file
==================================================================================
*/
static int twfsUtime( const char *path, struct utimbuf *ubuf )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "utime:%s\n", r_path );
	
	result = utime( r_path, ubuf );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsOpen
	Input		:const char *path
				 < path to file >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:open a file. no creation/truncation with O_CREAT, O_EXCL, O_TRUNC
==================================================================================
*/
static int twfsOpen( const char *path, struct fuse_file_info *fi )
{
	int					fd;
	char				r_path[ DEF_TWFS_PATH_MAX ];
	int					result;
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	struct twfs_file	*twfs_file;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );

	getRootAbsPath( r_path, path );

	logMessage( "flags: %o\n", fi->flags );
	logMessage( "O_ACCMODE:%o\n", O_ACCMODE );

	switch( file_type )
	{
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
		if( ( fi->flags & O_ACCMODE ) != O_WRONLY )
		{
			result = openTwfsTweetFile( &twfs_file,
										r_path,
										sname1,
										file_type,
										sname2,
										sname3 );
			if( result < 0 )
			{
				return( -ENOMEM );
			}
			fi->fh = ( unsigned long long )twfs_file;
		}
		else
		{
			logMessage( "open:%s\n", r_path );
			fd = openFile( r_path, fi->flags, 0000 );
			fi->fh = fd;
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
		if( ( fi->flags & O_ACCMODE ) != O_WRONLY )
		{
			result = openTwfsTweetFile( &twfs_file,
										r_path,
										sname1,
										file_type,
										sname2,		// for slug
										sname3 );	// for owner
			if( result < 0 )
			{
				return( -ENOMEM );
			}
			fi->fh = ( unsigned long long )twfs_file;
		}
		else
		{
			return( -EACCES );
		}
		break;
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_DM_FR_MSG:
	case	E_TWFS_FILE_DM_FR_STATUS:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
		/* the user does not have the read permission						*/
		fi->fh = 0;
		return( -EACCES );
	case	E_TWFS_FILE_LISTS_SUB_LNAME_LDESC:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_LDESC:
	case	E_TWFS_FILE_AUTH_LISTS_OWN_LNAME_LDESC:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_LDESC:
		result = makeLists( E_TWFS_REQ_SHOW_LISTS,
							getCurrentSSLSession( ),
							sname1,
							sname2 );
		if( result < 0 )
		{
			result = 0;
		}
		logMessage( "open:%s\n", r_path );
		fd = openFile( r_path, fi->flags, 0000 );
		fi->fh = fd;
		if( fd < 0 )
		{
			return( -errno );
		}
		break;
	
	case	E_TWFS_FILE_ACC_PROFILE:
	case	E_TWFS_FILE_AUTH_ACC_PROFILE:
		result = readProfile( getCurrentSSLSession( ), sname1 );

		if( result < 0 )
		{
			return( -EACCES );
		}
		fd = openFile( r_path, fi->flags, 0000 );
		fi->fh = fd;
		if( fd < 0 )
		{
			return( -errno );
		}
		break;


	case	E_TWFS_FILE_TWEET:
	case	E_TWFS_FILE_AUTH_TWEET:
	case	E_TWFS_FILE_DM_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_SEND_TO:
	case	E_TWFS_FILE_DM_FR_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_FR_SEND_TO:
	default:
		logMessage( "open:%s\n", r_path );
		fd = openFile( r_path, fi->flags, 0000 );
		fi->fh = fd;
		if( fd < 0 )
		{
			return( -errno );
		}
		break;
	}

	return( 0 );
}

/*
==================================================================================
	Function	:twfsRead
	Input		:const char *path
				 < path to file >
				 char *buf
				 < read buffer >
				 size_t size
				 < size of read buffer >
				 off_t offset
				 < read position >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:read a file
==================================================================================
*/
static int twfsRead( const char *path,
					 char *buf,
					 size_t size,
					 off_t offset,
					 struct fuse_file_info *fi )
{
	int					i;
	int					result;
	int					sname_len;
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	struct twfs_file	*twfs_file;
	struct twfs_read	twfs_read;
	size_t				read_size;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );

	logMessage( "twfsRead:%s [%d]\n", path, file_type );


	switch( file_type )
	{
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:

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
		twfs_file = ( struct twfs_file* )fi->fh;
		logMessage( "read:twfs_file->size : %lu\n", twfs_file->size );
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
			if( twfs_file->size < DEF_TWFS_HEAD_FF_LEN )
			{
				return( 0 );
			}
			break;
		default:
			if( twfs_file->size < DEF_TWFS_HEAD_TL_SIZE_FIELD )
			{
				return( 0 );
			}
			break;
		}

		result = getTwfsFileOffset( twfs_file, &twfs_read, file_type, offset );
		logMessage( "twfs_read.twfs_offset : %lu\n", twfs_read.twfs_offset );
		logMessage( "twfs_read.file_size : %lu\n", twfs_read.file_size );
		logMessage( "twfs_file->size : %lu\n", twfs_file->size );
		if( result < 0 )
		{
			logMessage( "EOF(1)\n" );
			/* EOF															*/
			return( 0 );
		}
		logMessage( "size : %lu\n", size );
		logMessage( "offset : %lu\n", offset );
		read_size = readTimeLineFile( twfs_file,
									  &twfs_read,
									  file_type,
									  buf,
									  size,
									  offset );
		if( read_size == 0 )
		{
			logMessage( "EOF(2)\n" );
			/* EOF															*/
			return( 0 );
		}

		logMessage( "read_size:%zu\n", read_size );
#if 0
		if( 0 < read_size )
		{
			logMessage( "\nhead******************************************n" );
			for( int k = 0 ; k < 500 ; k++ )
			{
				logMessage( "%c", buf[ k ] );
			}
			logMessage( "\nlasti******************************************n" );
			for( int k = read_size - 1500 ; k < read_size ; k++ )
			{
				logMessage( "%c", buf[ k ] );
			}
			logMessage( "\n" );
		}
#endif

		return( ( int )read_size );
	case	E_TWFS_FILE_TWEET:
		sname_len = strnlen( sname1, DEF_TWAPI_MAX_SCREEN_NAME_LEN );
		if( sname_len < offset )
		{
			for( i = 0 ; i < size ; i++ )
			{
				*( buf + i ) = 0x00;
			}
			return( 0 );
		}
		*( buf + 0 ) = '@';
		for( i = 0 ; i < sname_len ; i++ )
		{
			*( buf + i + 1 ) = sname1[ i ];
		}
		*( buf + i + 1 ) = ' ';
		*( buf + i + 2 ) = '\0';
		logMessage( "read tweet:%s\n", buf );
		return( sname_len + 1 + 1 );	// sname + @ + ' '
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_TWEET:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_DM_FR_MSG:
	case	E_TWFS_FILE_DM_FR_STATUS:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
		logMessage( "read nothing:\n" );
		/* -------------------------------------------------------------------- */
		/* the user does not have the right for the file.						*/
		/* so, read operation return nothing									*/
		/* -------------------------------------------------------------------- */
		for( i = 0 ; i < size ; i++ )
		{
			*( buf + i ) = 0x00;
		}
		return( 0 );
	default:
		logMessage( " pread:\n" );
		logMessage( " size : %zu offset : %zu\n", size, offset );
		result = pread( fi->fh, buf, size, offset );
		break;
	}

	return( result );
}

/*
==================================================================================
	Function	:twfsWrite
	Input		:const char *path
				 < path to file >
				 const char *buf
				 < write buffer >
				 size_t size
				 < size of write buffer >
				 off_t offset
				 < write position >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:read a file
==================================================================================
*/
static int twfsWrite( const char *path,
					  const char *buf,
					  size_t size,
					  off_t offset,
					  struct fuse_file_info *fi )
{
	int					result;
	int					tweet_len;
	int					at_name_len;
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;

	result		= size;
	
	file_type	= whichTwfsPath( path, sname1, sname2, sname3 );
	
	switch( file_type )
	{
	/* ------------------------------------------------------------------------ */
	/* [authorized]/{tweet}														*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_AUTH_TWEET:
		logMessage( "message auth tweet: %s(%d)\n", buf,size );
		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf,
									 DEF_REST_ACTUAL_TWEETS_MAX_LEN );
		}
		else
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf, size );
		}

		if( DEF_REST_TWEETS_MAX_LENGTH < tweet_len )
		{
			return( size );
		}

		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			tweetTwapi( getCurrentSSLSession( ),
						buf,
						DEF_REST_ACTUAL_TWEETS_MAX_LEN );
		}
		else
		{
			tweetTwapi( getCurrentSSLSession( ), buf, size );
		}
		return( size );
	/* ------------------------------------------------------------------------ */
	/* [screen_name]/{tweet}													*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_TWEET:
		logMessage( "message tweet: %s(%d)\n", buf,size );
		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf,
									 DEF_REST_ACTUAL_TWEETS_MAX_LEN );
		}
		else
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf, size );
		}

		if( buf[ 0 ] != '@' )
		{
			at_name_len = strnlen( sname1, DEF_TWAPI_MAX_SCREEN_NAME_LEN );
			at_name_len += 2;	// 1 for @, 1 for space -> @screen_name[space]

			if( DEF_REST_TWEETS_MAX_LENGTH < ( tweet_len + at_name_len ) )
			{
				return( size );
			}

			if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < ( size + at_name_len ) )
			{
				char	to_buf[ DEF_REST_ACTUAL_TWEETS_MAX_LEN + 1 ];
				snprintf( to_buf, sizeof( to_buf ), "@%s %s", sname1, buf );
				tweetTwapi( getCurrentSSLSession( ),
							to_buf,
							DEF_REST_ACTUAL_TWEETS_MAX_LEN );
			}
			else
			{
				char	to_buf[ DEF_REST_ACTUAL_TWEETS_MAX_LEN + 1 ];
				snprintf( to_buf, sizeof( to_buf ), "@%s %s", sname1, buf );
				tweetTwapi( getCurrentSSLSession( ), to_buf, size + at_name_len );
			}
		}
		else
		{
			if( DEF_REST_TWEETS_MAX_LENGTH < tweet_len )
			{
				return( size );
			}
			
			if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
			{
				tweetTwapi( getCurrentSSLSession( ),
							buf,
							DEF_REST_ACTUAL_TWEETS_MAX_LEN );
			}
			else
			{
				tweetTwapi( getCurrentSSLSession( ),
							buf,
							size );
			}
		}
		return( size );
	/* ------------------------------------------------------------------------ */
	/* [authorized]/direct_message/send_to										*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_AUTH_DM_SEND_TO:
		result = clipTwopeScreenNameFromMsg( buf, sname1 );
		if( result < 0 )
		{
			return( size );
		}

		if( result != 0 )
		{
			if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < ( size - result - 1 ) )
			{
				tweet_len = utf8StrnLen( ( const uint8_t* )&buf[ result + 1 ],
										 DEF_REST_ACTUAL_TWEETS_MAX_LEN );
			}
			else
			{
				tweet_len = utf8StrnLen( ( const uint8_t* )&buf[ result + 1 ],
										 size - result - 1 );
			}
		}
		else
		{
			if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
			{
				tweet_len = utf8StrnLen( ( const uint8_t* )buf,
										 DEF_REST_ACTUAL_TWEETS_MAX_LEN );
			}
			else
			{
				tweet_len = utf8StrnLen( ( const uint8_t* )buf, size );
			}
		}

		if( DEF_REST_TWEETS_MAX_LENGTH < tweet_len )
		{
			return( size );
		}
		logMessage( "direct message : %s(%d)\n", sname1, result );
		if( result != 0 )
		{
			if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < ( size - result - 1 ) )
			{
				sendDirectMessage( getCurrentSSLSession( ),
								   sname1,
								   &buf[ result + 1 ],
								   DEF_REST_ACTUAL_TWEETS_MAX_LEN - result - 1 );
			}
			else
			{
				sendDirectMessage( getCurrentSSLSession( ),
								   sname1,
								   &buf[ result + 1 ],
								   size - result - 1 );
			}
		}
		else
		{
			if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
			{
				sendDirectMessage( getCurrentSSLSession( ),
								   sname1,
								   buf,
								   DEF_REST_ACTUAL_TWEETS_MAX_LEN );
			}
			else
			{
				sendDirectMessage( getCurrentSSLSession( ),
								   sname1,
								   buf,
								   size - 1 );
			}
		}
		return( size );
	/* ------------------------------------------------------------------------ */
	/* [authorized]/direct_message/[friends]/send_to							*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_AUTH_DM_FR_SEND_TO:
		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf,
									 DEF_REST_TWEETS_MAX_LENGTH
									 * DEF_UTF8_MAX_SIZE );
		}
		else
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf, size );
		}

		if( DEF_REST_TWEETS_MAX_LENGTH < tweet_len )
		{
			return( size );
		}

		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			sendDirectMessage( getCurrentSSLSession( ),
							   sname2,
							   buf,
							   DEF_REST_ACTUAL_TWEETS_MAX_LEN );
		}
		else
		{
			sendDirectMessage( getCurrentSSLSession( ), sname2, buf, size );
		}
		return( size );
	/* ------------------------------------------------------------------------ */
	/* [screen_name]/direct_message/send_to										*/
	/* ------------------------------------------------------------------------ */
	case	E_TWFS_FILE_DM_SEND_TO:
		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf,
									 DEF_REST_TWEETS_MAX_LENGTH
									 * DEF_UTF8_MAX_SIZE );
		}
		else
		{
			tweet_len = utf8StrnLen( ( const uint8_t* )buf, size );
		}

		if( DEF_REST_TWEETS_MAX_LENGTH < tweet_len )
		{
			return( size );
		}
		logMessage( "message: %s(%d)\n", buf,size );

		if( DEF_REST_ACTUAL_TWEETS_MAX_LEN < size )
		{
			sendDirectMessage( getCurrentSSLSession( ),
							   sname1,
							   buf,
							   DEF_REST_ACTUAL_TWEETS_MAX_LEN );
		}
		else
		{
			sendDirectMessage( getCurrentSSLSession( ), sname1, buf, size );
		}
		return( size );
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		return( size );
	/* ------------------------------------------------------------------------ */
	/* regular files															*/
	/* ------------------------------------------------------------------------ */
	default:
		logMessage( "write normal:%d\n", file_type );
		if( file_type == E_TWFS_FILE_REG )
		{
			result = pwrite( fi->fh, buf, size, offset );
			logMessage( "pwrite(%d)\n", result );
		}
		else
		{
			return( size );
		}
		break;
	}

	return( result );
}

/*
==================================================================================
	Function	:twfsStatisticsFileSystem
	Input		:const char *path
				 < path to file >
				 struct statvfs *statv
				 < statistics file system information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:get statistics of file sytem
==================================================================================
*/
static int twfsStatisticsFileSystem( const char *path, struct statvfs *statv )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "statvfs:%s\n", r_path );
	
	result = statvfs( r_path, statv );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsFlush
	Input		:const char *path
				 < path to file >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:flush read/memory buffer
==================================================================================
*/
static int twfsFlush( const char *path, struct fuse_file_info *fi )
{
	int					result;
#if 0
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	struct twfs_file	*twfs_file;
#endif

	logMessage( "flush:%s\n", path );
#if 0
	file_type = whichTwfsPath( path, sname1, sname2, sname3 );


	switch( file_type )
	{
	case	E_TWFS_FILE_TWEET:
	case	E_TWFS_FILE_AUTH_TWEET:
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_AUTH_DM_MSG:

		twfs_file = ( struct twfs_file* )fi->fh;
		logMessage( "twfs = %d\n", twfs_file );
		result = closeTwfsFile( &twfs_file );
		return( 0 );
		break;
	default:
#if 0
		if( fi->fh )
		{
			result = closeFile( fi->fh );
			fi->fh = 0;
		}
		else
		{
			result = 0;
		}
#endif
		result = 0;
		break;
	}
#endif
	result = 0;
	
	return( result );
}

/*
==================================================================================
	Function	:twfsRelease
	Input		:const char *path
				 < path to file >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:close a file
==================================================================================
*/
static int twfsRelease( const char *path, struct fuse_file_info *fi )
{
	int					result;
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	struct twfs_file	*twfs_file;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );

	switch( file_type )
	{
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	
	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		if( ( fi->flags & O_ACCMODE ) != O_WRONLY )
		{
			twfs_file = ( struct twfs_file* )fi->fh;
			logMessage( "twfs = %d\n", twfs_file );
			result = closeTwfsFile( &twfs_file );
		}
		else
		{
			if( fi->fh )
			{
				result = closeFile( fi->fh );
				fi->fh = 0;
			}
			else
			{
				result = 0;
			}
		}
		return( 0 );
		break;
	case	E_TWFS_FILE_TWEET:
	case	E_TWFS_FILE_AUTH_TWEET:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_DM_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_SEND_TO:
	case	E_TWFS_FILE_DM_FR_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_FR_SEND_TO:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	default:
		//result = closeFile( fi->fh );
#if 1
		if( fi->fh )
		{
			result = closeFile( fi->fh );
			fi->fh = 0;
		}
		else
		{
			result = 0;
		}
#endif
		break;
	}

	logMessage( "release:%s\n", path );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsFileSync
	Input		:const char *path
				 < path to file >
				 int datasync
				 < flag to sync >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:syncronize a file with buffer
==================================================================================
*/
static int
twfsFileSync( const char *path, int datasync, struct fuse_file_info *fi )
{
	int		result;
	
	if( datasync )
	{
		result = fdatasync( fi->fh );
		logMessage( "fdatasync:%s\n", path );
	}
	else
	{
		result = fsync( fi->fh );
		logMessage( "fsync:%s\n", path );
	}
	
	return( result );
}

/*
==================================================================================
	Function	:twfsSetExtendedAttributes
	Input		:const char *path
				 < path to file >
				 const char *name
				 < name of extended attribute >
				 const char *value
				 < value of extended attribute >
				 size_t size
				 < size of value >
				 int flags
				 < flags to set xattr >
	Outpu		:void
	Return		:int
				 < status >
	Description	:set extended attributes
==================================================================================
*/
static int
twfsSetExtendedAttributes( const char *path,
						   const char *name,
						   const char *value,
						   size_t size,
						   int flags )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );
	
	logMessage( "setxattr:%s\n", r_path );
	
	result = lsetxattr( r_path, name, value, size, flags );
	
	return( -errno );
}

/*
==================================================================================
	Function	:twfsGetExtendedAttributes
	Input		:const char *path
				 < path to file >
				 const char *name
				 < name of extended attribute >
				 char *value
				 < value of extended attribute >
				 size_t size
				 < size of value >
	Outpu		:void
	Return		:int
				 < status >
	Description	:get extended attributes
==================================================================================
*/
static int
twfsGetExtendedAttributes( const char *path,
						   const char *name,
						   char *value,
						   size_t size )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "getxattr:%s\n", r_path );
	
	result = lgetxattr( r_path, name, value, size );
	
	return( -errno );
}

/*
==================================================================================
	Function	:twfsListExtendedAttributes
	Input		:const char *path
				 < path to file >
				 char *list
				 < list of extended attribute >
				 size_t size
				 < size of value >
	Outpu		:char *list
				 < list of extended attribute >
	Return		:int
				 < status >
	Description	:list extended attributes
==================================================================================
*/
static int
twfsListExtendedAttributes( const char *path,
							char *list,
							size_t size )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "listxattr:%s\n", r_path );
	
	result = llistxattr( r_path, list, size );
	
	return( -errno );
}

/*
==================================================================================
	Function	:twfsRemoveExtendedAttributes
	Input		:const char *path
				 < path to file >
				 const char *name
				 < name of extended attribute >
	Outpu		:void
	Return		:int
				 < status >
	Description	:remove extended attributes
==================================================================================
*/
static int
twfsRemoveExtendedAttributes( const char *path,
							  const char *name )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );
	
	logMessage( "lrmovetxattr:%s\n", r_path );
	
	result = lremovexattr( r_path, name );
	
	return( -errno );
}

/*
==================================================================================
	Function	:twfsOpenDirectory
	Input		:const char *path
				 < path to file >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:open a directory fiel
==================================================================================
*/
static int
twfsOpenDirectory( const char *path, struct fuse_file_info *fi )
{
	DIR		*dp;
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "opendir:%s\n", r_path );
	
	dp = opendir( r_path );
	
	if( !dp )
	{
		result = -1;
	}
	else
	{
		result = 0;
	}
	
	fi->fh = ( intptr_t )dp;
	
	return( result );
}

/*
==================================================================================
	Function	:twfsReadDirectory
	Input		:const char *path
				 < path to file >
				 void *buf
				 < read buffer >
				 fuse_fill_dir_t filler
				 < filler >
				 off_t offset
				 < read position >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:read a directory fiel
==================================================================================
*/
static int twfsReadDirectory( const char *path,
							  void *buf,
							  fuse_fill_dir_t filler,
							  off_t offset,
							  struct fuse_file_info *fi )
{
	DIR				*dp;
	struct dirent	*de;
	
	dp = ( DIR* )( uintptr_t )fi->fh;

	de = readdir( dp );
	
	logMessage( "readdir:%s\n", de->d_name );
	
	if( !de )
	{
		return( -1 );
	}
	
	do
	{
		if( filler( buf, de->d_name, NULL, 0 ) != 0 )
		{
			return( -ENOMEM );
		}
	} while( ( de = readdir( dp ) ) != NULL );
	
	return( 0 );
}

/*
==================================================================================
	Function	:twfsReleaseDirectory
	Input		:const char *path
				 < path to file >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:close a directory file
==================================================================================
*/
static int
twfsReleaseDirectory( const char *path, struct fuse_file_info *fi )
{
	DIR		*dp;
	int		result;

	dp = ( DIR* )( uintptr_t )fi->fh;

	logMessage( "closedir:%s\n", path );
	
	result = closedir( dp );
	
	return( result );
}
/*
==================================================================================
	Function	:twfsFileSyncDirectory
	Input		:const char *path
				 < path to file >
				 int datasync
				 < flag to sync >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:close a directory file
==================================================================================
*/
static int
twfsFileSyncDirectory ( const char *path,
						int datasync,
						struct fuse_file_info *fi )
{
	logMessage( "fsync:%s\n", path );
	return( 0 );
}

/*
==================================================================================
	Function	:initTwfs
	Input		:struct fuse_conn_info *conn
				 < fuse connection information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:initialize twitter file system
==================================================================================
*/
static void* initTwfs( struct fuse_conn_info *conn )
{
	return( NULL );
}

/*
==================================================================================
	Function	:destroyTwfs
	Input		:void *userdata
				 < user data >
	Outpu		:void
	Return		:int
				 < status >
	Description	:close twitter file system
==================================================================================
*/
static void destroyTwfs( void *userdata )
{
	/* ------------------------------------------------------------------------ */
	/* destroy internals and twfs main resource									*/
	/* ------------------------------------------------------------------------ */
	destroyTwfsInternal( );

	destroy( );
}

/*
==================================================================================
	Function	:twfsAccess
	Input		:const char *path
				 < path to file >
				 int mask
				 < mask >
	Outpu		:void
	Return		:int
				 < status >
	Description	:check file access permissions
==================================================================================
*/
static int twfsAccess( const char *path, int mask )
{
	int		result;
	char	r_path[ DEF_TWFS_PATH_MAX ];
	
	getRootAbsPath( r_path, path );

	logMessage( "access:%s %o\n", r_path, mask );

	result = access( r_path, mask );

	if( result < 0 )
	{
		result = -errno;
	}

	return( result );
}

/*
==================================================================================
	Function	:twfsCreate
	Input		:const char *path
				 < path to file >
				 mode_t mode
				 < file mode >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:create and open a file
==================================================================================
*/
static int
twfsCreate( const char *path, mode_t mode, struct fuse_file_info *fi )
{
	int					fd;
	char				r_path[ DEF_TWFS_PATH_MAX ];
	int					result;
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	struct twfs_file	*twfs_file;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );
	
	getRootAbsPath( r_path, path );

	logMessage( "create:%s %o\n", r_path, mode );
	logMessage( "flags:%X\n", fi->flags );

	switch( file_type )
	{
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
		if( ( fi->flags & O_RDONLY ) != O_WRONLY )
		{
			result = openTwfsTweetFile( &twfs_file,
										r_path,
										sname1,
										file_type,
										NULL,
										NULL );
			if( result < 0 )
			{
				return( result );
			}
			fi->fh = ( unsigned long long )twfs_file;
		}
		else
		{
			return( -EACCES );
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
		if( ( fi->flags & O_RDONLY ) != O_WRONLY )
		{
			result = openTwfsTweetFile( &twfs_file,
										r_path,
										sname1,
										file_type,
										sname2,		// for slug
										sname3 );	// for owner
			if( result < 0 )
			{
				return( -ENOMEM );
			}
			fi->fh = ( unsigned long long )twfs_file;
		}
		else
		{
			return( -EACCES );
		}
		break;
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_DM_FR_MSG:
	case	E_TWFS_FILE_DM_FR_STATUS:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
		fi->fh = 0;
		return( -EACCES );
	case	E_TWFS_FILE_TWEET:
	case	E_TWFS_FILE_AUTH_TWEET:
	case	E_TWFS_FILE_DM_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_SEND_TO:
	case	E_TWFS_FILE_DM_FR_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_FR_SEND_TO:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_LDESC:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_LDESC:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_LDESC:
	default:
		fd = creat( r_path, mode );

		fi->fh = fd;

		if( fd < 0 )
		{
			return( -errno );
		}
		break;
	}


	return( 0 );
}
/*
==================================================================================
	Function	:twfsFileTruncate
	Input		:const char *path
				 < path to file >
				 off_t offset
				 < position >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status DOT_>
	Description	:change a opened file size
==================================================================================
*/
static int
twfsFileTruncate( const char *path, off_t offset, struct fuse_file_info *fi )
{
	int		result;

	logMessage( "ftruncate:%s\n", path );
	
	result = ftruncate( fi->fh, offset );
	
	return( result );
}

/*
==================================================================================
	Function	:twfsFileGetAttributes
	Input		:const char *path
				 < path to file >
				 struct stat *statbuf
				 < statistics buffer >
				 struct fuse_file_info *fi
				 < fuse file information >
	Outpu		:void
	Return		:int
				 < status >
	Description	:get attributes of opened file
==================================================================================
*/
static int
twfsFileGetAttributes( const char *path,
					   struct stat *statbuf,
					   struct fuse_file_info *fi )
{
	int					result;
	char				sname1[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	//char				sname2[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	char				sname2[ DEF_REST_ACTUAL_SLUG_MAX_LENGTH + 1 ];
	char				sname3[ DEF_TWAPI_MAX_SCREEN_NAME_LEN ];
	E_TWFS_FILE_TYPE	file_type;
	struct twfs_file	*twfs_file;
	size_t				total_size;
	blkcnt_t			phys_blks;
	blksize_t			phys_blksize;

	file_type = whichTwfsPath( path, sname1, sname2, sname3 );

	logMessage( "fstat:%s\n", path );

	switch( file_type )
	{
	case	E_TWFS_FILE_TL:
	case	E_TWFS_FILE_AUTH_TL:
	case	E_TWFS_FILE_USER_TL:
	case	E_TWFS_FILE_AUTH_USER_TL:
	case	E_TWFS_FILE_DM_MSG:
	case	E_TWFS_FILE_AUTH_DM_MSG:
	case	E_TWFS_FILE_DM_FR_MSG:
	case	E_TWFS_FILE_AUTH_DM_FR_MSG:
	case	E_TWFS_FILE_FL_LIST:
	case	E_TWFS_FILE_FL_DOT_LIST:
	case	E_TWFS_FILE_FR_LIST:
	case	E_TWFS_FILE_FR_DOT_LIST:
	case	E_TWFS_FILE_AUTH_NOTI_AT_TW:
	case	E_TWFS_FILE_AUTH_NOTI_RTW:
	case	E_TWFS_FILE_NOTI_AT_TW:
	case	E_TWFS_FILE_NOTI_RTW:
	case	E_TWFS_FILE_FAV_LIST:
	case	E_TWFS_FILE_AUTH_FAV_LIST:
	case	E_TWFS_FILE_BLOCKS_LIST:
	case	E_TWFS_FILE_DOT_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_BLOCKS_LIST:
	case	E_TWFS_FILE_AUTH_DOT_BLOCKS_LIST:

	case	E_TWFS_FILE_LISTS_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_TL:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_SUB_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_OWN_LIST:
	case	E_TWFS_FILE_LISTS_OWN_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_TL:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_OWN_LNAME_SUB_DOT_LIST:

	case	E_TWFS_FILE_LISTS_ADD_LIST:
	case	E_TWFS_FILE_LISTS_ADD_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_TL:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_MEM_DOT_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_LIST:
	case	E_TWFS_FILE_LISTS_ADD_LNAME_SUB_DOT_LIST:
		twfs_file = ( struct twfs_file* )fi->fh;

		result = fstat( twfs_file->fd, statbuf );

		if( result < 0 )
		{
			logMessage( "stat failed : %s\n", strerror( errno )  );
			break;
		}
		else
		{
			logMessage( "st_size : %zu\n", statbuf->st_size );
			logMessage( "st_blocks : %zu\n", statbuf->st_blocks );
			logMessage( "st_blksize : %zu\n", statbuf->st_blksize );
			getTotalSizeOfTlFileFromFd( twfs_file->fd, &total_size );
			logMessage( "total_size : %zu\n", total_size );

			if( total_size )
			{
				statbuf->st_size	= total_size;
			}
			else
			{
				statbuf->st_size	= 1;
			}
			phys_blks = ( statbuf->st_size + ( statbuf->st_blksize - 1 ) )
						/ statbuf->st_blksize;
			phys_blksize = statbuf->st_blksize * phys_blks;
			statbuf->st_blocks = ( phys_blksize + ( DEF_LOGICAL_BLOCK_SIZE - 1 ) )
								 / DEF_LOGICAL_BLOCK_SIZE;

			logMessage( "st_size : %zu\n", statbuf->st_size );
			logMessage( "st_blocks : %zu\n", statbuf->st_blocks );
			logMessage( "st_blksize : %zu\n", statbuf->st_blksize );
		}

		break;
	case	E_TWFS_FILE_TWEET:
	case	E_TWFS_FILE_DM_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_SEND_TO:
	case	E_TWFS_FILE_DM_FR_SEND_TO:
	case	E_TWFS_FILE_AUTH_DM_FR_SEND_TO:
		result = fstat( fi->fh, statbuf );
		if( ( fi->flags & O_ACCMODE ) == O_WRONLY )
		{
			statbuf->st_size	= 1;
		}
		else
		{
			int					at_name_len;
			at_name_len			= strnlen( sname1,
										   DEF_TWAPI_MAX_SCREEN_NAME_LEN );
			// 1 for @, 1 for space -> @screen_name[space]
			at_name_len			+= 2;
			statbuf->st_size	= at_name_len;
		}
		phys_blks = ( statbuf->st_size + ( statbuf->st_blksize - 1 ) )
					/ statbuf->st_blksize;
		phys_blksize = statbuf->st_blksize * phys_blks;
		statbuf->st_blocks = ( phys_blksize + ( DEF_LOGICAL_BLOCK_SIZE - 1 ) )
							 / DEF_LOGICAL_BLOCK_SIZE;

		logMessage( "st_size : %zu\n", statbuf->st_size );
		logMessage( "st_blocks : %zu\n", statbuf->st_blocks );
		logMessage( "st_blksize : %zu\n", statbuf->st_blksize );
		break;
	case	E_TWFS_FILE_AUTH_TWEET:
		result = fstat( fi->fh, statbuf );
		statbuf->st_size = 0;
		break;
	default:
		result = fstat( fi->fh, statbuf );
		if( result < 0 )
		{
			result = -errno;
		}
			logMessage( "fstat\n" );
			logMessage( "st_size : %zu\n", statbuf->st_size );
			logMessage( "st_blocks : %zu\n", statbuf->st_blocks );
			logMessage( "st_blksize : %zu\n", statbuf->st_blksize );
		break;
	}
	logMessage( "fstat : %zu\n", statbuf->st_size  );
	
	return( result );
}


/*
================================================================================
	Function	:getTwfsOperations
	Input		:void
	Output		:void
	Return		:struct fuse_operations*
				 < operations of twfs >
	Description	:get twfs operations
================================================================================
*/
static struct fuse_operations* getTwfsOperations( void )
{
	return( &twfs_operations );
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

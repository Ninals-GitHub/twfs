/*******************************************************************************
 File:utils.c
 Description:Operations of Twfs Utilities

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

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
#define	DEF_UTILS_MAX_RETRY			10
#define	DEF_UTILS_CLOSE_MAX_RETRY	DEF_UTILS_MAX_RETRY
#define	DEF_UTILS_WRITE_MAX_RETRY	DEF_UTILS_MAX_RETRY
#define	DEF_UTILS_READ_MAX_RETRY	DEF_UTILS_MAX_RETRY

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
	Function	:isDirectory
	Input		:const char *dir_path
				 < directory to create >
	Output		:void
	Return		:int
				 < status >
	Description	:is dir_path directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int isDirectory( const char *dir_path )
{
	struct stat		dir_stat = { 0 };
	int				result;

	if( ( result = stat( dir_path, &dir_stat ) ) == 0 )
	{
		if( !S_ISDIR( dir_stat.st_mode ) )
		{
			return( -1 );
		}

		if( !( S_IRUSR & dir_stat.st_mode ) || !( S_IXUSR & dir_stat.st_mode ) )
		{
			return( -1 );
		}
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:isRegularFile
	Input		:const char *file_path
				 < file to check >
	Output		:void
	Return		:int
				 < status >
	Description	:is file_path a regular file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int isRegularFile( const char *file_path )
{
	struct stat	file_stat = { 0 };
	int			result;

	if( ( result = stat( file_path, &file_stat ) ) == 0 )
	{
		return( S_ISREG( file_stat.st_mode ) );
	}

	return( -1 );
}
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:makeDirectory
	Input		:const *dir_path
				 < directory to create >
				 mode_t mode
				 < access permission for the directory >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:make directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int makeDirectory( const char *dir_path, mode_t mode, bool daemon )
{
	struct stat	dir_stat = { 0 };
	int			result;

	if( ( result = stat( dir_path, &dir_stat ) ) == 0 )
	{
		if( !S_ISDIR( dir_stat.st_mode ) )
		{
			if( !daemon )
			{
				printf( "Cannot create a directory : %s\n", dir_path );
			}
		}

		if( !( S_IRUSR & dir_stat.st_mode ) || !( S_IXUSR & dir_stat.st_mode ) )
		{
			if( ( result = chmod( dir_path, mode ) ) < 0 )
			{
				if( !daemon )
				{
					printf( "Credential for accessing the directory" );
					printf( " cannot be granted : %s\n", dir_path );
				}
			}
		}

		return( result );
	}

	if( ( result = mkdir( dir_path, mode ) ) < 0 )
	{
		if( !daemon )
		{
			printf( "Cannot create a directory : %s\n", dir_path );
			printf( "Error: errno = %d, %s\n", errno, strerror( errno ) );
		}
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:createNewFile
	Input		:const *file_path
				 < file path to create >
				 mode_t mode
				 < access permission for the new file >
				 bool daemon
				 < true:daemon mode >
	Output		:void
	Return		:int
				 < status >
	Description	:create a new file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int createNewFile( const char *file_path, mode_t mode, bool daemon )
{
	struct stat	file_stat = { 0 };
	int			result;

	if( ( result = stat( file_path, &file_stat ) ) == 0 )
	{
		if( !S_ISREG( file_stat.st_mode ) )
		{
			if( !daemon )
			{
				printf( "Cannot create a file : %s\n", file_path );
			}
		}

		if( !( S_IRUSR & file_stat.st_mode ) &&
			!( S_IWUSR & file_stat.st_mode ) )
		{
			if( ( result = chmod( file_path, mode ) ) < 0 )
			{
				if( !daemon )
				{
					printf( "Credential for accessing the file" );
					printf( " cannot be granted : %s\n", file_path );
				}
			}
		}

		return( result );
	}

	if( ( result = open( file_path, O_RDWR | O_CREAT, mode ) ) < 0 )
	{
		if( !daemon )
		{
			printf( "CAnnot create a file : %s\n", file_path );
			printf( "Error: errno = %d, %s\n", errno, strerror( errno ) );
		}
	}

	close( result );

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openFile
	Input		:const char *path
				 < path name >
				 int flags
				 < flags >
				 mode_t mode
				 < mode >
	Output		:void
	Return		:int
				 < file descriptor >
	Description	:open a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int openFile( const char *path, int flags, mode_t mode )
{
	int		result;
	int		retry;

	for( retry = 0 ; retry < DEF_UTILS_CLOSE_MAX_RETRY ; retry++ )
	{
		if( mode == 0000 )
		{
			result = open( path, flags );
		}
		else
		{
			result = open( path, flags, mode );
		}

		if( result < 0 )
		{
			if( errno == EINTR || errno == EIO )
			{
				continue;
			}
		}

		break;
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:closeFile
	Input		:int fd
				 < file descriptor >
	Output		:void
	Return		:int
				 < status >
	Description	:close a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int closeFile( int fd )
{
	int		result;
	int		retry;

	for( retry = 0 ; retry < DEF_UTILS_CLOSE_MAX_RETRY ; retry++ )
	{
		result = close( fd );

		if( result < 0 )
		{
			if( errno == EINTR || errno == EIO )
			{
				continue;
			}
		}

		break;
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:writeFile
	Input		:int fd
				 < file descriptor >
				 const void *buf
				 < buffer >
				 size_t count
				 < count to write >
	Output		:void
	Return		:int
				 < status >
	Description	:write a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int writeFile( int fd, const void *buf, size_t count )
{
	int		result;
	int		retry;

	for( retry = 0 ; retry < DEF_UTILS_WRITE_MAX_RETRY ; retry++ )
	{
		result = write( fd, buf, count );

		if( result < 0 )
		{
			if( ( errno == EAGAIN )			||
				( errno == EWOULDBLOCK )	||
				( errno == EIO )			||
				( errno == EINTR ) )
			{
				continue;
			}
		}
		break;
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:readFile
	Input		:int fd
				 < file descriptor >
				 void *buf
				 < buffer >
				 size_t count
				 < count to read >
	Output		:void
	Return		:int
				 < status >
	Description	:read a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int readFile( int fd, void *buf, size_t count )
{
	int		result;
	int		retry;

	for( retry = 0 ; retry < DEF_UTILS_READ_MAX_RETRY ; retry++ )
	{
		result = read( fd, buf, count );

		if( result < 0 )
		{
			if( ( errno == EAGAIN )			||
				( errno == EWOULDBLOCK )	||
				( errno == EIO )			||
				( errno == EINTR ) )
			{
				continue;
			}
		}
		break;
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:preadFile
	Input		:int fd
				 < file descriptor >
				 void *buf
				 < buffer >
				 size_t count
				 < count to read >
				 off_t offset
				 < offset in a file >
	Output		:void
	Return		:int
				 < status >
	Description	:pread a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int preadFile( int fd, void *buf, size_t count, off_t offset )
{
	int		result;
	int		retry;

	for( retry = 0 ; retry < DEF_UTILS_READ_MAX_RETRY ; retry++ )
	{
		result = pread( fd, buf, count, offset );

		if( result < 0 )
		{
			if( ( errno == EAGAIN )			||
				( errno == EWOULDBLOCK )	||
				( errno == EIO )			||
				( errno == EINTR ) )
			{
				continue;
			}
		}
		break;
	}

	return( result );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:pwriteFile
	Input		:int fd
				 < file descriptor >
				 const void *buf
				 < buffer >
				 size_t count
				 < count to write >
				 off_t offset
				 < offset in a file >
	Output		:void
	Return		:int
				 < status >
	Description	:pwrite a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int pwriteFile( int fd, const void *buf, size_t count, off_t offset )
{
	int		result;
	int		retry;

	for( retry = 0 ; retry < DEF_UTILS_READ_MAX_RETRY ; retry++ )
	{
		result = pwrite( fd, buf, count, offset );

		if( result < 0 )
		{
			if( ( errno == EAGAIN )			||
				( errno == EWOULDBLOCK )	||
				( errno == EIO )			||
				( errno == EINTR ) )
			{
				continue;
			}
		}
		break;
	}

	return( result );
}
/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
================================================================================
	Function	:void
	Input		:void
	Output		:void
	Return		:void
	Description	:void
================================================================================
*/


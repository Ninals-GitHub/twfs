/*******************************************************************************
 File:utils.h
 Description:Definitions of twfs utilities

*******************************************************************************/
#ifndef	__UTILS_H__
#define	__UTILS_H__

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
int isDirectory( const char *dir_path );

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
int isRegularFile( const char *file_path );

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
int makeDirectory( const char *dir_path, mode_t mode, bool daemon );

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
int createNewFile( const char *file_path, mode_t mode, bool daemon );

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
int openFile( const char *path, int flags, mode_t mode );

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
int closeFile( int fd );

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
int writeFile( int fd, const void *buf, size_t count );

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
int readFile( int fd, void *buf, size_t count );

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
int preadFile( int fd, void *buf, size_t count, off_t offset );

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
int pwriteFile( int fd, const void *buf, size_t count, off_t offset );

#endif	// __UTILS_H__

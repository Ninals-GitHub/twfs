/*******************************************************************************
 File:ssl.c
 Description:Procedures of SSL 3.1

*******************************************************************************/
#include <string.h>
#include <pthread.h>

#include "net/ssl.h"
#include "net/network.h"
#include "lib/hmac.h"
#include "lib/sha1.h"
#include "lib/md5.h"
#include "lib/log.h"

/*
================================================================================

	Prototype Statements

================================================================================
*/
static int initSSLThreadMutex( void );
static void lockSSLThreadMutex( int mode, int type, const char *file, int line );
static struct CRYPTO_dynlock_value *
createSSLDynamicThreadMutex( const char *file, int line );
static void
lockSSLDynamicThreadMutex( int mode,
						   struct CRYPTO_dynlock_value *dl,
						   const char *file,
						   int line );
static void getThreadId( CRYPTO_THREADID *id );
static void
destroySSLDynamicThreadMutex( struct CRYPTO_dynlock_value *dl,
							  const char *file,
							  int line );
#if 0
static int sslPrf( const unsigned char *secret,
				   int secret_size,
				   const char *label,
				   const unsigned char *seed,
				   int seed_size,
				   unsigned char *hash,
				   int hash_size );
#endif

/*
================================================================================

	DEFINES

================================================================================
*/
#define	DEF_SSL_SEND_RETRY			50
#define	DEF_SSL_RECV_RETRY			50

struct CRYPTO_dynlock_value
{
	pthread_mutex_t		mutex;
};

/*
================================================================================

	Management

================================================================================
*/
static struct ssl_session	*current;
static pthread_mutex_t		*mutex_locks;

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initSSL
	Input		:connection *con
				 < connection of ssl layer >
	Output		:void
	Return		:int
				 < status >
	Description	:initialize openssl library
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initSSL( void )
{
	int		result;
	/* ------------------------------------------------------------------------ */
	/* register the error strings for libcrypto & libssl						*/
	/* ------------------------------------------------------------------------ */
	SSL_load_error_strings( );
	/* ------------------------------------------------------------------------ */
	/* initialize threaded ssl resources										*/
	/* ------------------------------------------------------------------------ */
	if( ( result = initSSLThreadMutex( ) ) < 0 )
	{
		return( result );
	}
	/* ------------------------------------------------------------------------ */
	/* register the available ciphers and digests								*/
	/* ------------------------------------------------------------------------ */
	SSL_library_init( );

	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:openSSLSession
	Input		:struct ssl_session *session
				 < session of ssl to open >
	Output		:void
	Return		:int
				 < status >
	Description	:open a ssl session
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int openSSLSession( struct ssl_session *session )
{
	/* ------------------------------------------------------------------------ */
	/* new context saying we are a client, and using ssl 2 or 3					*/
	/* ------------------------------------------------------------------------ */
	session->ssl_context = SSL_CTX_new( TLSv1_1_client_method( ) );

	if( !session->ssl_context )
	{
		ERR_print_errors_fp( stderr );
		return( -1 );
	}

	//SSL_CTX_set_options( session->ssl_context, SSL_OP_ALL );

	/* ------------------------------------------------------------------------ */
	/* create an ssl struct for the session										*/
	/* ------------------------------------------------------------------------ */
	session->ssl_handle = SSL_new( session->ssl_context );

	if( !session->ssl_handle )
	{
		ERR_print_errors_fp( stderr );
		return( -1 );
	}

	session->connect = true;

	/* ------------------------------------------------------------------------ */
	/* set fd																	*/
	/* ------------------------------------------------------------------------ */
	if( !SSL_set_fd( session->ssl_handle, session->socket ) )
	{
		ERR_print_errors_fp( stderr );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* connect ssl																*/
	/* ------------------------------------------------------------------------ */
	if( SSL_connect( session->ssl_handle ) != 1 )
	{
		ERR_print_errors_fp( stderr );
		return( -1 );
	}

	current = session;
	
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:closeSSLSession
	Input		:struct ssl_session
				 < session of ssl to close >
	Output		:void
	Return		:void
	Description	:closea a ssl session
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void closeSSLSession( struct ssl_session *session )
{
	if( session->ssl_handle )
	{
		SSL_shutdown( session->ssl_handle );
		SSL_free( session->ssl_handle );
	}

	if( session->ssl_context )
	{
		SSL_CTX_free( session->ssl_context );
	}

	ERR_free_strings( );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroySSLResources
	Input		:void
	Output		:void
	Return		:void
	Description	:free all resource threaded ssl
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroySSLResources( void )
{
	int		i;

	CRYPTO_set_locking_callback( NULL );

	for( i = 0 ; i < CRYPTO_num_locks( ) ; i++ )
	{
		pthread_mutex_destroy( &( mutex_locks[ i ] ) );
	}

	free( mutex_locks );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:sendSSLMessage
	Input		:struct ssl_session *session
				 < session of ssl >
				 const unsgined char *message
				 < message to send >
				 int size
				 < size of message >
	Output		:void
	Return		:int
				 < size of send message >
	Description	:send a message using ssl layer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int sendSSLMessage( struct ssl_session *session,
					const unsigned char *message,
					int size )
{
	int		retry;
	int		length;
	int		org_size;

	int i;

	for( i = 0 ; i < size ; i++ )
	{
		if( ( char )message[ i ] != '\n' && ( char )message[ i ] != '\r' )
		{
		logMessage( "%c", (char)message[ i ] );
		}
		else if( (char) message[ i ] == '\n' )
		{
			logMessage( "<LF>\n" );
		}
		else{
			logMessage( "<CR>" );
		}
	}

	org_size = size;

	length = 0;
	
	length = SSL_write( session->ssl_handle,
						( void* )message,
						size );
	
	if( length < 0 )
	{
		logMessage( "[SSL:%s]", ERR_reason_error_string( ERR_get_error( ) ) );
	}

	return( length );

#if 0
	/* ------------------------------------------------------------------------ */
	/* send and retry															*/
	/* ------------------------------------------------------------------------ */
	for( retry = 0 ; retry < DEF_SSL_SEND_RETRY ; retry++ )
	{
		length = SSL_write( session->ssl_handle,
							( void* )&message[ length ],
							size );

		if( length < 0 )
		{
			perror( "send ssl message" );

			return( length );
		}

		if( size <= length )
		{
			return( org_size );
		}

		size = size - length;
	}
#endif

	/* all retries are failed													*/
	return( -1 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:recvSSLMessage
	Input		:struct ssl_session *session
				 < session of ssl >
				 unsgined char *message
				 < message to be stored a message >
				 int size
				 < size of a message to be received >
	Output		:void *message
				 < a received message >
	Return		:int
				 < size of a received message >
	Description	:receive a message using ssl layer
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int recvSSLMessage( struct ssl_session *session,
					unsigned char *message,
					int size )
{
	int		retry;
	int		length;
	int		org_size;
	int		r_indx;

	org_size	= size;
	r_indx		= 0;

	for( retry = 0 ; retry < DEF_SSL_RECV_RETRY ; retry++ )
	{
		length = SSL_read( session->ssl_handle,
						   &message[ r_indx ],
						   size );

		if( length < 0 )
		{
			logMessage( "[SSL:%s]", ERR_reason_error_string( ERR_get_error( ) ) );
			perror( "receive ssl message" );

			return( length );
		}
#if 1
		else if( length == 0 )
		{
			return( length );
		}
#endif

		if( size <= length )
		{
			return( org_size );
		}

		size	-= length;
		r_indx	+= length;
	}

	logMessage( "recv retry error:%d %d\n", length, size );

	/* failure to receive														*/
	return( -1 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:isSSLConnected
	Input		:struct ssl_session *session
				 < session of ssl >
	Output		:void
	Return		:int
				 < 0:connected -1:disconnected >
	Description	:test whether connected to server or not
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int isSSLConnected( struct ssl_session *session )
{
	return( isServerConnected( session->socket ) );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:reopenSSLSession
	Input		:struct ssl_session *session
				 < session of ssl >
	Output		:void
	Return		:int
				 < status >
	Description	:reopen a ssl session
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int reopenSSLSession( struct ssl_session *session )
{
	if( session->ssl_handle )
	{
		//SSL_clear( session->ssl_handle );
		//SSL_free( session->ssl_handle );
	}

	if( session->ssl_context )
	{
		//SSL_CTX_free( session->ssl_context );
	}

	if( session->connect )
	{
		logMessage( "disconnect \n" );
		disconnectServer( session->socket );
		session->connect = false;
	}

	if( ( session->socket = reconnectServer( session->socket ) ) < 0 )
	{
		logMessage( "reconnect error (%d)\n", session->socket );
		return( -1 );
	}

	session->connect = true;

	/* ------------------------------------------------------------------------ */
	/* set fd																	*/
	/* ------------------------------------------------------------------------ */
	if( !SSL_set_fd( session->ssl_handle, session->socket ) )
	{
		logMessage( "cannot set socket fd to ssl\n" );
		return( -1 );
	}

	/* ------------------------------------------------------------------------ */
	/* connect ssl																*/
	/* ------------------------------------------------------------------------ */
	if( SSL_connect( session->ssl_handle ) != 1 )
	{
		logMessage( "cannot connect ssl\n" );
		return( -1 );
	}
	
	return( 0 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:disconnectSSLServer
	Input		:struct ssl_session *session
				 < session of ssl >
	Output		:struct ssl_session *session
				 < opened session >
	Return		:void
	Description	:close socket
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int disconnectSSLServer( struct ssl_session *session )
{
	if( session->connect )
	{
		logMessage( "disconnect ssl server\n" );
		disconnectServer( session->socket );
		session->connect = false;
	}

	return( 0 );
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:getCurrentSSLSession
	Input		:void
	Output		:void
	Return		:struct ssl_session*
				 < current ssl connection >
	Description	:get current ssl session context
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
struct ssl_session* getCurrentSSLSession( void )
{
	return( current );
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
	Function	:initSSLThreadMutex
	Input		:void
	Output		:void
	Return		:int
				 < status >
	Description	:initialize mutexes for ssl thread safe
================================================================================
*/
static int initSSLThreadMutex( void )
{
	int		i;
	int		result;

	mutex_locks = ( pthread_mutex_t* )malloc( CRYPTO_num_locks( ) *
											   sizeof( pthread_mutex_t ) );
	
	if( !mutex_locks )
	{
		return( -1 );
	}
	
	for( i = 0 ; i < CRYPTO_num_locks( ) ; i++ )
	{
		if( ( result = pthread_mutex_init( &( mutex_locks[ i ] ), NULL ) ) < 0 )
		{
			return( result );
		}
	}

	if( ( result = CRYPTO_THREADID_set_callback( getThreadId ) ) < 0 )
	{
		return( result );
	}
	CRYPTO_set_locking_callback( lockSSLThreadMutex );
	CRYPTO_set_dynlock_create_callback( createSSLDynamicThreadMutex );
	CRYPTO_set_dynlock_lock_callback( lockSSLDynamicThreadMutex );
	CRYPTO_set_dynlock_destroy_callback( destroySSLDynamicThreadMutex );

	return( 0 );
}

/*
================================================================================
	Function	:lockSSLThreadMutex
	Input		:int mode
				 < mode for locking, unlocking >
				 int type
				 < lock type >
				 const char *file
				 < file name >
				 int line
				 < line >
	Output		:void
	Return		:int
				 < status >
	Description	:callback of ssl thread safe lock for static locks
================================================================================
*/
static void lockSSLThreadMutex( int mode, int type, const char *file, int line )
{
	int		result;
	( void )file;
	( void )line;

	if( mode & CRYPTO_LOCK )
	{
		if( ( result = pthread_mutex_lock( &( mutex_locks[ type ] ) ) ) < 0 )
		{
			/* logging here in the future		*/
		}
		//logMessage( "lock ssl(%d)\n", result );
	}
	else
	{
		if( ( result = pthread_mutex_unlock( &( mutex_locks[ type ] ) ) ) < 0 )
		{
			/* logging here in the future		*/
		}
		//logMessage( "unlock ssl(%d)\n", result );
	}
}

/*
================================================================================
	Function	:createSSLDynamicThreadMutex
	Input		:int mode
				 < mode for locking, unlocking >
				 const char *file
				 < file name >
				 int line
				 < line of file >
	Output		:void
	Return		:void
	Description	:create dynamic lock
================================================================================
*/
static struct CRYPTO_dynlock_value *
createSSLDynamicThreadMutex( const char *file, int line )
{
	int							result;
	struct CRYPTO_dynlock_value	*dl;

	dl = ( struct CRYPTO_dynlock_value* )
				malloc( sizeof( struct CRYPTO_dynlock_value ) );

	if( !dl )
	{
		return( dl );
	}

	if( ( result = pthread_mutex_init( &dl->mutex, NULL ) ) < 0 )
	{
		free( dl );
		return( NULL );
	}

	return( dl );
}

/*
================================================================================
	Function	:lockSSLDynamicThreadMutex
	Input		:int mode
				 < mode for locking, unlocking >
				 CRYPTO_dynlock_value *dl
				 < dynamic mutex information >
				 const char *file
				 < file name >
				 int line
				 < line of file >
	Output		:void
	Return		:void
	Description	:callbacl of ssl thread safe lock for dynamic locks
================================================================================
*/
static void
lockSSLDynamicThreadMutex( int mode,
						   struct CRYPTO_dynlock_value *dl,
						   const char *file,
						   int line )
{
	int		result;

	if( mode & CRYPTO_LOCK )
	{
		if( ( result = pthread_mutex_lock( &dl->mutex ) ) < 0 )
		{
			/* logging here in the future	*/
		}
	}
	else
	{
		if( ( result = pthread_mutex_unlock( &dl->mutex ) ) < 0 )
		{
			/* logging here in the future	*/
		}
	}
}

/*
================================================================================
	Function	:destroySSLDynamicThreadMutex
	Input		:struct CRYPTO_dynlock_value *dl
				 < dynamic lock to be freed >
				 const char *file
				 < file name >
				 int line
				 < line >
	Output		:void
	Return		:void
	Description	:free dynamic allocation memory for dynamic mutex
================================================================================
*/
static void
destroySSLDynamicThreadMutex( struct CRYPTO_dynlock_value *dl,
							  const char *file,
							  int line )
{
	int		result;

	if( ( result = pthread_mutex_destroy( &dl->mutex ) ) < 0 )
	{
		/* logging here in the future	*/
	}

	free( dl );
}
/*
================================================================================
	Function	:getThreadId
	Input		:CRYPTO_THREADID *id
				 < thread id information of ssl management >
	Output		:void
	Return		:void
	Description	:get an id of thread
================================================================================
*/
static void getThreadId( CRYPTO_THREADID *id )
{
	CRYPTO_THREADID_set_numeric( id, ( unsigned long )pthread_self( ) );
}
/*
================================================================================
	Function	:sslPrf
	Input		:const unsigned char *secret
				 < secret stream >
				 int secret_size
				 < size of secret >
				 const char *label
				 < label string >
				 const unsigned char *seed
				 < seed stream >
				 int seed_size
				 < size of seed >
				 unsigned char *hash
				 < hash value >
				 int hash_size
				 < hash size to need >
	Output		:unsigned char *hash
				 < result of pseudo-random function >
	Return		:int
				 < hash size >
	Description	:calculate pseudo-random function
				 < defined in rfc 2246 >
================================================================================
*/
#if 0
static int sslPrf( const unsigned char *secret,
				   int secret_size,
				   const char *label,
				   const unsigned char *seed,
				   int seed_size,
				   unsigned char *hash,
				   int hash_size )
{
	int				L_S1, L_S2;
	unsigned char	*S1, *S2;
	struct key_vec	md5_key_vec[ 3 ];
	struct key_vec	sha1_key_vec[ 3 ];
	unsigned char	md5_a[ DEF_MD5_DIGEST_LENGTH ];
	unsigned char	md5_p[ DEF_MD5_DIGEST_LENGTH ];
	unsigned char	sha1_a[ DEF_SHA1_DIGEST_LENGTH ];
	unsigned char	sha1_p[ DEF_SHA1_DIGEST_LENGTH ];
	int				md5_pos;
	int				sha1_pos;
	int				i;

	if( !secret || !label || !seed || !hash )
	{
		return( -1 );
	}

	if( ( secret_size <= 0 ) || ( seed_size <= 0 ) || ( hash_size <= 0 ) )
	{
		return( -1 );
	}

	L_S1	= ( secret_size + ( 2 - 1 ) ) / 2;
	L_S2	= L_S1;
	S1		= ( unsigned char* )secret;

	if( secret_size & 0x00000001 )
	{
		S2 = ( unsigned char* )( &secret[ L_S1 - 1 ] );
	}
	else
	{
		S2 = ( unsigned char* )( &secret[ L_S1 ] );
	}

	md5_key_vec[ 0 ].key		= md5_a;
	md5_key_vec[ 0 ].key_size	= sizeof( md5_a );
	md5_key_vec[ 1 ].key		= ( unsigned char* )label;
	md5_key_vec[ 1 ].key_size	= strlen( label );
	md5_key_vec[ 2 ].key		= ( unsigned char* )seed;
	md5_key_vec[ 2 ].key_size	= seed_size;

	sha1_key_vec[ 0 ].key		= sha1_a;
	sha1_key_vec[ 0 ].key_size	= sizeof( sha1_a );
	sha1_key_vec[ 1 ].key		= md5_key_vec[ 1 ].key;
	sha1_key_vec[ 1 ].key_size	= md5_key_vec[ 1 ].key_size;
	sha1_key_vec[ 2 ].key		= md5_key_vec[ 2 ].key;
	sha1_key_vec[ 2 ].key_size	= md5_key_vec[ 2 ].key_size;

	/* ------------------------------------------------------------------------ */
	/* calculate A(0)															*/
	/* ------------------------------------------------------------------------ */
	encodeHmacMd5Vector(  S1, L_S1, 2, &md5_key_vec[  1 ],  md5_key_vec[ 0 ].key );
	encodeHmacSha1Vector( S2, L_S2, 2, &sha1_key_vec[ 1 ], sha1_key_vec[ 0 ].key );

	/* ------------------------------------------------------------------------ */
	/* calculate A(i)															*/
	/* ------------------------------------------------------------------------ */
	md5_pos		= DEF_MD5_DIGEST_LENGTH;
	sha1_pos	= DEF_SHA1_DIGEST_LENGTH;
	
	for( i = 0 ; i < hash_size ; i++ )
	{
		if( DEF_MD5_DIGEST_LENGTH == md5_pos )
		{
			encodeHmacMd5Vector( S1, L_S1, 3, &md5_key_vec[ 0 ], md5_p );
			md5_pos = 0;
			encodeHmacMd5( S1,
						   L_S1,
						   md5_key_vec[ 0 ].key,
						   md5_key_vec[ 0 ].key_size,
						   md5_key_vec[ 0 ].key );
		}

		if( DEF_SHA1_DIGEST_LENGTH == sha1_pos )
		{
			encodeHmacSha1Vector( S2, L_S2, 3, &sha1_key_vec[ 0 ], sha1_p );
			sha1_pos = 0;
			encodeHmacSha1( S2,
							L_S2,
							sha1_key_vec[ 0 ].key,
							sha1_key_vec[ 0 ].key_size,
							sha1_key_vec[ 0 ].key );
		}

		hash[ i ] = md5_key_vec[  0 ].key[ md5_pos ] ^
					sha1_key_vec[ 0 ].key[ sha1_pos ];

		md5_pos++;
		sha1_pos++;
	}

	return( hash_size );

}

#endif

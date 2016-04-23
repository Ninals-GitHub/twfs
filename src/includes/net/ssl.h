/*******************************************************************************
 File:ssl.h
 Description:Definitions of SSL

*******************************************************************************/
#ifndef	__SSL_H__
#define	__SSL_H__

#include <stdint.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

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
--------------------------------------------------------------------------------
	Connection State
--------------------------------------------------------------------------------
*/
#define	DEF_SSL_MASTER_SECRET_SIZE		48
#define	DEF_SSL_CLIENT_RANDOM_SIZE		32
#define	DEF_SSL_SERVER_RANDOM_SIZE		32

typedef enum
{
	server,
	client,
} ConnectionEnd;

typedef enum
{
	tls_prf_sha256,
}PRFAlgorithm;

typedef enum
{
	blkcypher_null,
	blkcypher_rc4,
	blkcypher_rc2,
	blkcypher_des,
	blkcypher_3des,
	blkcypher_des40,
} BuldCipherAlgorithm;

typedef enum
{
	stream,
	block,
	aead,
} CipherType;

typedef enum
{
	mac_null,
	hmac_md5,
	hmac_sha1,
	hmac_sha256,
	hmac_sha384,
	hmac_sha512,
} MACAlgorithm;

typedef enum
{
	compress_null,
} CompressionMethod;

struct SecurityParameters
{
	ConnectionEnd			entity;
	PRFAlgorithm			prf_algorithm;
	BuldCipherAlgorithm		bulk_cipher_algorithm;
	CipherType				cipher_type;
	uint8_t					enc_key_length;
	uint8_t					block_length;
	uint8_t					fixed_iv_length;
	uint8_t					record_iv_length;
	MACAlgorithm			mac_algorithm;
	uint8_t					mac_length;
	uint8_t					mac_key_length;
	CompressionMethod		compression_algorithm;
	uint8_t					master_secret[ DEF_SSL_MASTER_SECRET_SIZE ];
	uint8_t					client_random[ DEF_SSL_CLIENT_RANDOM_SIZE ];
	uint8_t					server_random[ DEF_SSL_SERVER_RANDOM_SIZE ];
};

/*
--------------------------------------------------------------------------------
	Record Layer
--------------------------------------------------------------------------------
*/
struct ProtocolVersion
{
	uint8_t					major;
	uint8_t					minor;
};

typedef enum
{
	change_cipher_spec		= 20,
	alert					= 21,
	handshake				= 22,
	application_data		= 23,
} ContentType;

/* Fragmentation																*/
#define	DEF_SSL_MAX_SIZE_PLAIN_TEXT		( 16384 )	// 2 ^14
struct TLSPlaintext
{
	ContentType				type;
	struct ProtocolVersion	version;
	uint16_t				length;
	uint8_t					*fragment;	// less than or equal 2^14
};

/* Compression and Decompression												*/
#define	DEF_SSL_MAX_SIZE_COMPRESSED		( DEF_SSL_MAX_SIZE_PLAIN_TEXT + 1024 )
struct TLSCompressed
{
	ContentType				type;
	struct ProtocolVersion	version;
	uint16_t				length;
	uint8_t					*fragment;
};

/* Record payload protection													*/
struct TLSCiphertext
{
	ContentType					type;
	struct ProtocolVersion		version;
	uint16_t					length;
	uint8_t						*fragment;
};

/* Null or Standard Stream														*/
struct GenericStreamCipher
{
	uint8_t						*content;
	uint8_t						*MAC;
};

/* CBC Block Cipher																*/
struct GenericBlockCipher
{
	uint8_t						*content;
	uint8_t						*MAC;
	uint8_t						*padding;
	uint8_t						padding_length;
};

/* AEAD Cipher																	*/
struct GenericAEADCipher
{
	uint8_t						*nonce_explicit;
	struct aead_ciphered
	{
		uint8_t					*content;
	} aead;
};

/*
--------------------------------------------------------------------------------
	Alert Protocol
--------------------------------------------------------------------------------
*/
typedef enum
{
	warinig						= 1,
	fatal						= 2,
} AlertLevel;

typedef enum
{
	close_notify				= 0,
	unexpected_message			= 10,
	bad_record_mac				= 20,
	decryption_failed_RESERVED	= 21,
	record_overflow				= 22,
	decompression_failure		= 30,
	handshake_failure			= 40,
	no_certificate_RESERVED		= 41,
	bad_certificate				= 42,
	unsupported_cerificate		= 43,
	certificate_revoked			= 44,
	certificate_expired			= 45,
	certificate_unknown			= 46,
	illegal_parameter			= 47,
	unknown_ca					= 48,
	access_denied				= 49,
	decode_error				= 50,
	decrypt_error				= 51,
	export_restriction_RESERVED	= 60,
	protocol_version			= 70,
	insufficient_security		= 71,
	internal_error				= 80,
	user_canceled				= 90,
	no_renegotiation			= 100,
	unsupported_extension		= 110,
} AlertDescription;

struct Alert
{
	AlertLevel					level;
	AlertDescription			description;
};
/*
================================================================================

	Management

================================================================================
*/
#if 0
struct ssl_session
{
	int							compression_state;
	int							cipher_state;
	unsigned char				mac_key[ 20 ];
	unsigned int				seq_num;
	struct SecurityParameters	sec_param;
};
#endif
/*
--------------------------------------------------------------------------------
	SSL Session
--------------------------------------------------------------------------------
*/
struct ssl_session
{
	int			socket;
	bool		connect;
	SSL			*ssl_handle;
	SSL_CTX		*ssl_context;
};

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
int initSSL( void );

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
int openSSLSession( struct ssl_session *session );

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
void closeSSLSession( struct ssl_session *session );

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
					int size );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:recvSSLMessage
	Input		:struct ssl_session *session
				 < session of ssl >
				 unsigned char *message
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
					int size );

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
int isSSLConnected( struct ssl_session *session );

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
int reopenSSLSession( struct ssl_session *session );

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
int disconnectSSLServer( struct ssl_session *session );

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
struct ssl_session* getCurrentSSLSession( void );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:destroySSLResources
	Input		:void
	Output		:void
	Return		:void
	Description	:free all resource threaded ssl
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void destroySSLResources( void );

#endif	//__SSL_H__

/*******************************************************************************
 File:http.h
 Description:Definitions of HTTP

*******************************************************************************/
#ifndef	__HTTP_H__
#define	__HTTP_H__

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
-------------------------------------------------------------------------------
	HTTP Method
-------------------------------------------------------------------------------
*/
#define	DEF_HTTPH_HTTP						"HTTP"
#define	DEF_HTTPH_VERSION					"1.1"
#define	DEF_HTTPH_HTTP_VER					"HTTP/1.1"
#define	DEF_HTTPH_POST						"POST"
#define	DEF_HTTPH_GET						"GET"
#define	DEF_HTTPH_OPTIONS					"OPTIONS"
#define	DEF_HTTPH_HEAD						"HEAD"
#define	DEF_HTTPH_PUT						"PUT"
#define	DEF_HTTPH_DELETE					"DELETE"
#define	DEF_HTTPH_TRACE						"TRACE"
#define	DEF_HTTPH_CONNECT					"CONNECT"

/*
-------------------------------------------------------------------------------
	HTTP Status Code
-------------------------------------------------------------------------------
*/
#define	DEF_HTTPH_STATUS_CONTINUE			(100)
#define	DEF_HTTPH_STATUS_SWITCHING_PROTOCOL	(101)
#define	DEF_HTTPH_STATUS_OK					(200)
#define	DEF_HTTPH_STATUS_CREATED			(201)
#define	DEF_HTTPH_STATUS_ACCEPTED			(202)
#define	DEF_HTTPH_STATUS_NON_AUTO_INFO		(203)
#define	DEF_HTTPH_STATUS_NO_CONTENT			(204)
#define	DEF_HTTPH_STATUS_RESET_CONTENT		(205)
#define	DEF_HTTPH_STATUS_PARTIAL_CONTENT	(206)
#define	DEF_HTTPH_STATUS_MULTIPLE_CHOICES	(300)
#define	DEF_HTTPH_STATUS_MOVED_PERMANENTLY	(301)
#define	DEF_HTTPH_STATUS_FOUND				(302)
#define	DEF_HTTPH_STATUS_SEE_OTHER			(303)
#define	DEF_HTTPH_STATUS_NOT_MODIFIED		(304)
#define	DEF_HTTPH_STATUS_USE_PROXY			(305)
#define	DEF_HTTPH_STATUS_TEMPORARY_REDIRECT	(307)
#define	DEF_HTTPH_STATUS_BAD_REQUEST		(400)
#define	DEF_HTTPH_STATUS_UNAUTHORIZED		(401)
#define	DEF_HTTPH_STATUS_PAYMENT_REQUIRED	(402)
#define	DEF_HTTPH_STATUS_FORBIDDEN			(403)
#define	DEF_HTTPH_STATUS_NOT_FOUND			(404)
#define	DEF_HTTPH_STATUS_METHOD_NOT_ALLOWED	(405)
#define	DEF_HTTPH_STATUS_NOT_ACCEPTABLE		(406)
#define	DEF_HTTPH_STATUS_PROXY_AUTH_REQ		(407)
#define	DEF_HTTPH_STATUS_REQUEST_TIMEOUT	(408)
#define	DEF_HTTPH_STATUS_CONFLICT			(409)
#define	DEF_HTTPH_STATUS_GONE				(410)
#define	DEF_HTTPH_STATUS_LENGTH_REQUIRED	(411)
#define	DEF_HTTPH_STATUS_PRECOND_FAILED		(412)
#define	DEF_HTTPH_STATUS_REQ_ENT_TOOLARGE	(413)
#define	DEF_HTTPH_STATUS_REQ_URI_TOOLARGE	(414)
#define	DEF_HTTPH_STATUS_UNSUPPORTED_MTYPE	(415)
#define	DEF_HTTPH_STATUS_REQ_RANGE_NOT_SAT	(416)
#define	DEF_HTTPH_STATUS_EXPECTATION_FAILED	(417)
#define	DEF_HTTPH_STATUS_INTERNAL_SERV_ERR	(500)
#define	DEF_HTTPH_STATUS_NOT_IMPLEMENTED	(501)
#define	DEF_HTTPH_STATUS_BAD_GATEWAY		(502)
#define	DEF_HTTPH_STATUS_SERVICE_UNAVAIL	(503)
#define	DEF_HTTPH_STATUS_GATEWAY_TIMEOUT	(504)
#define	DEF_HTTPH_STATUS_HTTP_VER_NOT_SPPRT	(505)

/*
-------------------------------------------------------------------------------
	HTTP Header
-------------------------------------------------------------------------------
*/
#define	DEF_HTTPH_ACCEPT					"Accept:"
#define	DEF_HTTPH_ACCEPT_ASTA				"Accept: */*"
#define	DEF_HTTPH_ACCEPT_CHARSET			"Accept-Charset:"
#define	DEF_HTTPH_ACCEPT_ENCODING			"Accept-Encoding:"
#define	DEF_HTTPH_ACCEPT_LANGUAGE			"Accept-Language:"
#define	DEF_HTTPH_RES_ACCEPT_RANGES			"Accept-Ranges:"
#define	DEF_HTTPH_AGE						"Age:"
#define	DEF_HTTPH_ALLOW						"Allow:"
#define	DEF_HTTPH_AUTHORIZATION				"Authorization:"
#define	DEF_HTTPH_AUTHORIZATION_OAUTH		"Authorization: OAuth"
#define	DEF_HTTPH_CACHE_CONTROL				"Cache-Control:"
#define	DEF_HTTPH_CONNECTION				"Connection:"
#define	DEF_HTTPH_CONNECTION_CLOSE			"Connection: close"
#define	DEF_HTTPH_CONNECTION_ALIVE			"Connection: Keep-Alive"
#define	DEF_HTTPH_CONTENT_ENCODING			"Content-Encoding:"
#define	DEF_HTTPH_CONTENT_LANGUAGE			"Content-Language:"
#define	DEF_HTTPH_CONTENT_LENGTH			"Content-Length:"
#define	DEF_HTTPH_CONTENT_MD5				"Content-MD5:"
#define	DEF_HTTPH_CONTENT_RANGE				"Content-Range:"
#define	DEF_HTTPH_CONTENT_TYPE				"Content-Type:"
#define	DEF_HTTPH_CONTENT_TYPE_URLENC		"Content-Type: application/x-www-form-urlencoded; charset=utf-8"
#define	DEF_HTTPH_DATE						"Date:"
#define	DEF_HTTPH_ETAG						"ETag:"
#define	DEF_HTTPH_EXPECT					"Expect:"
#define	DEF_HTTPH_EXPIRES					"Expires:"
#define	DEF_HTTPH_FROM						"From:"
#define	DEF_HTTPH_HOST						"Host:"
#define	DEF_HTTPH_IF_MATCH					"If-Match:"
#define	DEF_HTTPH_IF_MODIFIED_SINCE			"If-Modified-Since:"
#define	DEF_HTTPH_IF_NONE_MATCH				"If-None-Match:"
#define	DEF_HTTPH_IF_RANGE					"If-Range:"
#define	DEF_HTTPH_IF_UNMODIFIED_SINCE		"If-Unmodified-Since:"
#define	DEF_HTTPH_LAST_MODIFIED				"Last-Modified:"
#define	DEF_HTTPH_LOCATION					"Location:"
#define	DEF_HTTPH_MAX_FORWARDS				"Max-Forwards:"
#define	DEF_HTTPH_PRAGMA					"Pragma:"
#define	DEF_HTTPH_PROXY_AUTHENTICATE		"Proxy-Authenticate:"
#define	DEF_HTTPH_PROXY_AUTHORIZATION		"Proxy-Authorization:"
#define	DEF_HTTPH_RANGE						"Range:"
#define	DEF_HTTPH_REFERE					"Referer:"
#define	DEF_HTTPH_RETRY_AFTER				"Retry-After:"
#define	DEF_HTTPH_SERVER					"Server:"
#define	DEF_HTTPH_TE						"TE:"
#define	DEF_HTTPH_TRAILER					"Trailer:"
#define	DEF_HTTPH_TRANSFER_ENCODING			"Transfer-Encoding:"
#define	DEF_HTTPH_UPGRADE					"Upgrade:"
#define	DEF_HTTPH_USER_AGENT				"User-Agent:"
#define	DEF_HTTPH_USER_AGENT_FS				"User-Agent: twfs"
#define	DEF_HTTPH_VARY						"Vary:"
#define	DEF_HTTPH_VIA						"Via:"
#define	DEF_HTTPH_WARNING					"Warning:"

#define	DEF_HTTPH_DELIMITOR					"\r\n"



/*
================================================================================

	Management

================================================================================
*/
/*
--------------------------------------------------------------------------------
	Http Context
--------------------------------------------------------------------------------
*/
struct http_ctx
{
	int		status_code;
	int		content_length;
};

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initHttpContext
	Input		:struct http_ctx *hctx
				 < initializee >
	Output		:struct http_ctx *hctx
				 < initializee >
	Return		:int
				 < status >
	Description	:intialize http context
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int initHttpContext( struct http_ctx *hctx );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:recvHttpHeader
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < context of http received data >
	Output		:struct http_ctx *hctx
				 < first body data block may be stored >
	Return		:int
				 < status >
	Description	:receive headers of http and interpret them
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int recvHttpHeader( struct ssl_session *session, struct http_ctx *hctx );

#endif	// __HTTP_H__

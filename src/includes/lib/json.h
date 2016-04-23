/*******************************************************************************
 File:json.h
 Description:Definitions of JSON

*******************************************************************************/
#ifndef	__JSON_H__
#define	__JSON_H__

#include "net/ssl.h"
#include "net/http.h"
#include "lib/utf.h"

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
#define	DEF_JSON_OBJ_NAME_MAX		256

struct jnode
{
	char			*obj;
	struct jnode	*child;
	struct jnode	*sibling;
	char			*value;
	int				length;
};

typedef enum
{
	E_JSON_ANA_OBJ,
	E_JSON_ANA_ARRAY,
	E_JSON_ANA_NAME,
	E_JSON_ANA_VALUE
} E_JSON_ANALYSIS;

typedef enum
{
	E_JSON_STATE_STRING,
	E_JSON_STATE_ESC,
	E_JSON_STATE_ESC_U,
	E_JSON_STATE_VALUE_VALUE,
	E_JSON_STATE_VALUE_STRING,
	E_JSON_STATE_VALUE_STRING_ESC,
	E_JSON_STATE_VALUE_STRING_ESC_U,
}E_JSON_STATE_VALUE;

typedef enum
{
	E_JSON_UTF_STATE_FIRST_INPUT,
	E_JSON_UTF_STATE_SECOND_INPUT,
}E_JSON_UTF_STATE;

#define	DEF_JSON_ANA_STACK_MAX		64

struct json_obj
{
	char	obj[ DEF_JSON_OBJ_NAME_MAX ];
	int		index;
};

struct json_ana
{
	E_JSON_ANALYSIS		stack[ DEF_JSON_ANA_STACK_MAX ];
	E_JSON_STATE_VALUE	state;
	struct
	{
		E_JSON_UTF_STATE	in_state;
		E_UTF16_CODE_POINT	state;
		int					count;
	} utf;
	int					stackp;
	int					obj_level;
	unsigned int		length;
	struct json_obj		obj_info;
};

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
	Function	:initJsonRoot
	Input		:struct jnode *root
				 < root of json tree >
	Output		:struct jnode *root
				 < root of json tree >
	Return		:void
	Description	:initialize a root of json tree
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initJsonRoot( struct jnode *root );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initJsonNode
	Input		:struct jnode *node
				 < node of json tree >
				 char *obj_name
				 < object name of json which the node represents >
				 char *value
				 < value of the object >
	Output		:struct jnoe *node
				 < node of json tree >
	Return		:void
	Description	:initialize a node of json tree
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initJsonNode( struct jnode *node, char *obj_name, char *value );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:insertJsonNode
	Input		:struct jnode *root
				 < root of json tree >
				 char *path
				 < path which represents a structure of json objects >
				 struct jnode *insertee
				 < the node to be inserted >
	Output		:struct jnode *root
				 < tree inserted a node >
	Return		:void
	Description	:insert a json node
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int insertJsonNodes( struct jnode *root,
					 char *path,
					 struct jnode *insertee );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:searchJsonNode
	Input		:struct jnode *root
				 < root of a json tree >
				 char *key
				 < key to search a tree >
	Output		:void
	Return		:struct jnode *
				 < looked up node >
	Description	:search a json tree
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
struct jnode *searchJsonNodes( struct jnode *root, char *key );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:initJsonAnalysisCtx
	Input		:struct json_ana *ana
				 < json analysis context >
	Output		:struct json_ana *ana
				 < initialized json analysis context >
	Return		:void
	Description	:initialize a json analysis context
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initJsonAnalysisCtx( struct json_ana *ana );

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:analyzeJson
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 int8_t *new_twfs
				 < buffer of json structure >
				 struct json_ana *ana
				 < json analysis context >
				 struct jnode *key
				 < root of json structre to search >
				 int break_level
				 < level of strucuter which you wanto break analyzing >
				 uint8_t *buffer
				 < buffer for temporary use >
				 int buf_len
				 < length of buffer >
	Output		:int *recv_lenth
				 < update received length >
	Return		:length
				 < received length >
	Description	:analyze json data
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
int analyzeJson( struct ssl_session *session,
				 struct http_ctx *hctx,
				 uint8_t *new_tws,
				 struct json_ana *ana,
				 struct jnode *key,
				 int break_level,
				 uint8_t *buffer,
				 int buf_len );

#endif	//__JSON_H__

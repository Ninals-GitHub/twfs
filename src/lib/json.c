/*******************************************************************************
 File:json.c
 Description:Operaions of JSON

*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "lib/json.h"
#include "lib/log.h"
#include "net/ssl.h"
#include "net/http.h"


/*
================================================================================

	Prototype Statements

================================================================================
*/
void cutLastElement( char *elements, int *length );
void analyzeUtf( struct json_ana *ana, uint8_t *buffer, int *length, int c );


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
	Function	:initJsonRoot
	Input		:struct jnode *root
				 < root of json tree >
	Output		:struct jnode *root
				 < root of json tree >
	Return		:void
	Description	:initialize a root of json tree
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
void initJsonRoot( struct jnode *root )
{
	initJsonNode( root, "/", NULL );
}

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
void initJsonNode( struct jnode *node, char *obj_name, char *value )
{
	node->obj		= obj_name;
	node->child		= NULL;
	node->sibling	= NULL;
	node->value		= value;
	node->length	= 0;
}

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
 					 struct jnode *insertee )
{
	char			*next;
	char			*p;
	int				node_index;
	struct jnode	*next_node;
	
	if( !root || !insertee || !path )
	{
		return( -1 );
	}
	
	node_index	= 0;
	next		= NULL;

	//printf( "%s\n", path );
	//logMessage( "%s\n", path );
	
	/* path is like a directory path representaion from root					*/
	/* ex. Twitter.Users.Entities -> /Twitter/User/Entities						*/
	
	/* root is represented as '/"												*/
	if( *path == '/' )
	{
		path++;
	}
	else
	{
		return( -1 );
	}

	
	/* insert root																*/
	next = strstr( ( const char* )path, "/" );

	
	if( next != NULL )
	{
		*next = '\0';
	}


	next_node = root;

	while( 1 )
	{
		//printf( "%s %s\n", next_node->obj, path );
		if( strcmp( next_node->obj, path ) == 0 )
		{
			break;
		}

		if( !next_node->sibling )
		{
			//printf( "%s\n", path );
			initJsonNode( insertee, path, NULL );
			next_node->sibling = insertee;
			next_node = next_node->sibling;
			return( 0 );
		}

		next_node = next_node->sibling;
	}
	
#if 1
	if( next == NULL )
	{
		return( 0 );
	}

	//printf( "root:%s\n", path );
	
	p = next + 1;
	
	/* sub-path																	*/
#if 1
	
	while( 1 )
	{
		next = strstr( ( const char* )p, "/" );
		
		if( next != NULL )
		{
			*next = '\0';
		}
		
		if( next_node->child )
		{
			next_node = next_node->child;
			
			while( 1 )
			{
				if( strcmp( next_node->obj, p ) == 0 )
				{
					//printf( "hit!:%s\n", p );
					if( !next_node->child )
					{
						initJsonNode( insertee, p, NULL );
						next_node->child = insertee;
					}
					next_node = next_node->child;
					break;
				}
				
				if( !next_node->sibling )
				{
					//printf( "sibling:%s ", next_node->obj );
					//printf( "child0:%s\n", p );
					initJsonNode( insertee, p, NULL );
					next_node->sibling = insertee;
					next_node = next_node->child;
					return( 0 );
				}
				
				next_node = next_node->sibling;
			}
		}
		else
		{
			//printf( "parent:%s ", next_node->obj );
			//printf( "child1:%s\n", p );
			initJsonNode( insertee, p, NULL );
			next_node->child = insertee;
			return( 0 );
		}
		
		if( next == NULL )
		{
			return( 0 );
		}
		
		p = next + 1;
	}
#endif
#endif
	
	return( -1 );
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:searchJsonNode
	Input		:struct jnode *root
				 < root of a json tree >
				 char *key_obj
				 < key to search a tree >
	Output		:void
	Return		:struct jnode *
				 < looked up node >
	Description	:search a json tree
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
struct jnode *searchJsonNodes( struct jnode *root, char *key_obj )
{
	struct jnode	*node;
	char			*key_next;
	char			*p;
	char			key[ DEF_JSON_OBJ_NAME_MAX ];
	
	if( root->sibling )
	{
		node = root->sibling;
	}
	else
	{
		if( !root->child )
		{
			return( NULL );
		}
		node = root->child;
	}

	for( int i = 0 ; i < DEF_JSON_OBJ_NAME_MAX ; i++ )
	{
		key[ i ] = key_obj[ i ];
		if( key_obj[ i ] == '\0' )
		{
			break;
		}
	}
	
	p = key;
	p++;

	while( 1 )
	{
		key_next = strstr( ( const char* )p, "/" );
		
		if( key_next != NULL )
		{
			*key_next = '\0';
		}

		if( strcmp( node->obj, p ) == 0 )
		{
			if( node->child )
			{
				if( key_next == NULL )
				{
					return( node );
				}
				node = node->child;
				
				p = key_next + 1;
				continue;
			}
			else
			{
				if( key_next == NULL )
				{
					return( node );
				}
				return( NULL );
			}
		}
		/* search sibling													*/
		while( node )
		{
			if( strcmp( node->obj, p ) == 0 )
			{
				if( node->child )
				{
					if( key_next == NULL )
					{
						return( node );
					}
					node = node->child;
					break;
				}
				else
				{
					if( key_next == NULL )
					{
						return( node );
					}
					return( NULL );
				}
			}
			
			if( node->sibling )
			{
				node = node->sibling;
			}
			else
			{
				return( NULL );
			}
		}

		if( key_next == NULL )
		{
			return( NULL );
		}
		
		p = key_next + 1;
	}
	
	return( NULL );
}

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
void initJsonAnalysisCtx( struct json_ana *ana )
{
	ana->state			= E_JSON_STATE_STRING;
	ana->stackp			= 0;
	ana->obj_level		= 0;
	ana->utf.state		= E_UTF16_NORMAL;
	ana->utf.in_state	= E_JSON_UTF_STATE_FIRST_INPUT;
	ana->utf.count		= 0;
	ana->length			= 0;
	ana->stack[ ana->stackp ] = E_JSON_ANA_OBJ;
	ana->obj_info.index	= 0;
	for( int i = 0 ; i < DEF_JSON_OBJ_NAME_MAX ; i++ )
	{
		ana->obj_info.obj[ i ] = 0x00;
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	Function	:analyzeJson
	Input		:struct ssl_session *session
				 < ssl session >
				 struct http_ctx *hctx
				 < http context >
				 uint8_t *buffer
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
				 int buf_len )
{
	//struct json_obj		obj_info;
	int					length		= 0;
	int					string_esc_u= 0;
	char				c;
	int					current_rs;
	struct jnode		*lookup;

	current_rs		= 0;
	
	length			= 0;
	//obj_info.index	= 0;

	while( 1 )
	{

		if( hctx->content_length <= ana->length )
		{
			return( 0 );
		}

		c = new_tws[ ana->length ];
		//logMessage( "%c", c );
		current_rs++;
#if 1
		if( current_rs == 0 )
		{
			switch( c )
			{
			case	']':
			case	'}':
			case	',':
				//ana->length++;
				//logMessage( "\ncontinue(%c)\n", c );
				//current_rs++;
				continue;
			default:
				break;
			}
		}
#endif

		buffer[ length ] = c;

		//printf( "%c", c );
		//if( buf_len <= ( length - 1 ) )
		if( buf_len <= length )
		{
			logMessage( "-------------------------\n" );
			logMessage( "%s\n", new_tws );
			logMessage( "JSON: recv size is greater than buffer size[%d, %d]\n", length, ana->length );
			logMessage( "buffer:%s\n", buffer );
			logMessage( "c:%c\n", c );
			return( -1 );
		}
		ana->length++;
		length++;

		/* -------------------------------------------------------------------- */
		/* analyze objects or arrays											*/
		/* -------------------------------------------------------------------- */
		if( ( ana->stack[ ana->stackp ] == E_JSON_ANA_OBJ ) ||
			( ana->stack[ ana->stackp ] == E_JSON_ANA_ARRAY ) )
		{
			switch( c )
			{
			case	'[':
				ana->stackp++;
				length--;
				ana->stack[ ana->stackp ] = E_JSON_ANA_ARRAY;
				break;
			case	']':
				ana->stackp--;
				length--;
				break;
			case	'{':
				ana->stackp++;
				length--;
				ana->stack[ ana->stackp ] = E_JSON_ANA_OBJ;
				ana->obj_level++;
				break;
			case	'}':
				ana->stackp--;
				length--;
				ana->obj_level--;
				cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
				//logMessage( "\n<<0:obj:%d>>\n", ana->obj_level );
				if( ana->obj_level <= break_level )
				{
					return( current_rs );
				}
				break;
			case	'\"':
				ana->stackp++;
				length--;
				ana->stack[ ana->stackp ] = E_JSON_ANA_NAME;
				break;
			case	':':
				ana->stackp++;
				length--;
				ana->stack[ ana->stackp ] = E_JSON_ANA_VALUE;
				ana->state = E_JSON_STATE_VALUE_VALUE;
				break;
			case	',':
				length--;
				if( ana->stack[ ana->stackp ] == E_JSON_ANA_ARRAY )
				{
					length = 0;
				}
				else
				{
					cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
				}
				break;
			case	' ': length--; break;
			default:
				break;
			}
		}
		/* -------------------------------------------------------------------- */
		/* analyze values														*/
		/* -------------------------------------------------------------------- */
		else if( ana->stack[ ana->stackp ] == E_JSON_ANA_VALUE )
		{
			if( ana->state == E_JSON_STATE_VALUE_STRING )
			{
				switch( c )
				{
				case	'\\':
					ana->state = E_JSON_STATE_VALUE_STRING_ESC;
					length--;
					break;
				case	'\"':
					if( ana->state == E_JSON_STATE_ESC )
					{
						/* nothing to do	*/
						length--;
					}
					else
					{
						if( length == 1 )
						{
							length = 0;
							ana->stackp--;
							ana->state = E_JSON_STATE_STRING;
							buffer[ length ] = '\0';
							break;
						}
						ana->stackp--;
						length--;
						ana->state = E_JSON_STATE_STRING;
						buffer[ length ] = '\0';
						if( length != 0 )
						{
							//logMessage( "<0>ana->obj_info.obj:%s\n", ana->obj_info.obj );
							lookup = searchJsonNodes( key, ana->obj_info.obj );
							if( lookup )
							{
								if( strncmp( ( const char* )buffer,
											 "null",
											 length ) != 0 )
								{
									lookup->value = malloc( length + 1 );	// +1 for null
									if( !lookup->value )
									{
										logMessage( "JSON: cannot smallc\n" );
										return( -1 );
									}
									strncpy( lookup->value,
											 ( const char* )buffer,
											 length + 1 );
									lookup->length = length;
									//logMessage( "<0>searched!:%s [%s](%d)\n",
									//			lookup->obj, buffer, length );
								}
							}
						}
						length = 0;
						
						if( ana->stack[ ana->stackp ] == E_JSON_ANA_ARRAY )
						{
							if( ana->stackp )
							{
								if( ana->stack[ ana->stackp - 1 ]
									== E_JSON_ANA_OBJ )
								{
									cutLastElement( ana->obj_info.obj,
													&ana->obj_info.index );
								}
							}
							if( ana->obj_level <= break_level )
							{
								return( current_rs );
							}
							ana->stackp++;
							ana->stack[ ana->stackp ] = E_JSON_ANA_VALUE;
						}
						else
						{
							//cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
							if( ana->obj_level <= break_level )
							{
								return( current_rs );
							}
						}

						break;
					}
					break;
				default:
					break;
				}
			}
			else if( ana->state == E_JSON_STATE_VALUE_STRING_ESC )
			{
				switch( c )
				{
				case	'u':
					length--;
					string_esc_u = 0;
					ana->state = E_JSON_STATE_VALUE_STRING_ESC_U;
					break;
				case	'b':
				case	'f':
				case	't':
					ana->state = E_JSON_STATE_VALUE_STRING;
					length--;
					break;
				case	'n':
				case	'r':
					ana->state = E_JSON_STATE_VALUE_STRING;
					buffer[ length - 1 ] = '\n';
					break;
				case	'0':
					ana->state = E_JSON_STATE_VALUE_STRING;
					buffer[ length - 1 ] = '\0';
					break;
				//case	"\"":
				//	break;
				default:
					ana->state = E_JSON_STATE_VALUE_STRING;
					break;
				}
			}
			else if( ana->state == E_JSON_STATE_VALUE_STRING_ESC_U )
			{
				analyzeUtf( ana, buffer, &length, c );
				continue;
			}
			else
			{
				switch( c )
				{
				case	'[':
					ana->stackp++;
					length--;
					ana->stack[ ana->stackp ] = E_JSON_ANA_ARRAY;
					break;
				case	']':
					ana->stackp--;
					length--;
					ana->state = E_JSON_STATE_STRING;
					buffer[ length ] = '\0';
					if( length != 0 )
					{
						//logMessage( "<1>ana->obj_info.obj:%s\n", ana->obj_info.obj );
						lookup = searchJsonNodes( key, ana->obj_info.obj );
						if( lookup )
						{
							if( strncmp( ( const char* )buffer,
										 "null",
										 length ) != 0 )
							{
								//printf( "searched!:%s [%s]\n", lookup->obj, buffer );
								//logMessage( "<1>searched!:%s [%s](%d)\n", lookup->obj, buffer, length );
								lookup->value = malloc( length + 1 );	// +1 for null
								if( !lookup->value )
								{
									logMessage( "JSON: cannot smallc\n" );
									return( -1 );
								}
								strncpy( lookup->value,
										 ( const char* )buffer,
										 length + 1 );
								lookup->length = length;
							}
						}
					}
					length = 0;
					
					//printf( "value[%s]\n", buffer );
					if( ana->stack[ ana->stackp ] == E_JSON_ANA_ARRAY )
					{
						ana->stackp++;
						ana->stack[ ana->stackp ] = E_JSON_ANA_VALUE;
					}
					else
					{
						cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
					}
					if( ana->obj_level <= break_level )
					{
						return( current_rs );
					}
					break;
				case	'{':
					ana->stackp++;
					length--;
					ana->stack[ ana->stackp ] = E_JSON_ANA_OBJ;
					ana->obj_level++;
					break;
				case	'}':
					ana->stackp--;
					length--;
					ana->state = E_JSON_STATE_STRING;
					buffer[ length ] = '\0';
					if( length != 0 )
					{
						//logMessage( "<2>ana->obj_info.obj:%s\n", ana->obj_info.obj );
						lookup = searchJsonNodes( key, ana->obj_info.obj );
						if( lookup )
						{
							if( strncmp( ( const char *)buffer,
										 "null",
										 length ) != 0 )
							{
								//printf( "searched!:%s [%s]\n", lookup->obj, buffer );
								//logMessage( "<2>searched!:%s [%s](%d)\n", lookup->obj, buffer,length );
								lookup->value = malloc( length + 1 );	// +1 for null
								if( !lookup->value )
								{
									logMessage( "JSON: cannot smallc\n" );

									return( -1 );
								}
								strncpy( lookup->value,
										 ( const char* )buffer,
										 length + 1 );
								lookup->length = length;
							}
						}
					}
					length = 0;
					ana->obj_level--;
					//logMessage( "\n<<2:obj:%d %d>>\n", ana->obj_level, break_level );
					//if( ana->obj_level <= break_level )
					//{
					//	return( current_rs );
					//}
					//printf( "value[%s]\n", buffer );
					if( ana->stack[ ana->stackp ] == E_JSON_ANA_ARRAY )
					{
						if( ana->stackp )
						{
							if( ana->stack[ ana->stackp - 1 ] == E_JSON_ANA_OBJ )
							{
								cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
							}
						}
						if( ana->obj_level <= break_level )
						{
							return( current_rs );
						}
						ana->stackp++;
						ana->stack[ ana->stackp ] = E_JSON_ANA_VALUE;
					}
					else
					{
						cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
						ana->stackp--;
						if( ana->obj_level <= break_level )
						{
							return( current_rs );
						}
					}
					break;
				case	'\"':
					length--;
					if( ana->state != E_JSON_STATE_VALUE_STRING )
					{
						ana->state = E_JSON_STATE_VALUE_STRING;
					}
					else
					{
						ana->state = E_JSON_STATE_STRING;
					}
					break;
				case	'\\':
					length--;
					ana->state = E_JSON_STATE_VALUE_STRING_ESC;
					break;
				case	':': break;
				case	' ': break;
				case	',':
					length--;
					ana->stackp--;
					ana->state = E_JSON_STATE_STRING;
					buffer[ length ] = '\0';
					if( length != 0 )
					{
						//logMessage( "<3>ana->obj_info.obj:%s\n", ana->obj_info.obj );
						lookup = searchJsonNodes( key, ana->obj_info.obj );
						if( lookup )
						{
							if( strncmp( ( const char* )buffer,
										 "null",
										 length ) != 0 )
							{
								//printf( "searched!:%s [%s]\n", lookup->obj, buffer );
								//logMessage( "<3>searched!:%s [%s](%d)\n", lookup->obj, buffer,length );
								lookup->value = malloc( length + 1 );
								if( !lookup->value )
								{
									logMessage( "JSON: cannot smallc\n" );
									return( -1 );
								}
								strncpy( lookup->value,
										 ( const char* )buffer,
										 length + 1 );
								lookup->length = length;
							}
						}
					}
					length = 0;
					//printf( "value[%s]\n", buffer );
					if( ana->stack[ ana->stackp ] == E_JSON_ANA_ARRAY )
					{
						if( ana->stackp )
						{
							if( ana->stack[ ana->stackp - 1 ] == E_JSON_ANA_OBJ )
							{
								cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
							}
						}
						ana->stackp++;
						ana->stack[ ana->stackp ] = E_JSON_ANA_VALUE;
					}
					else
					{
						cutLastElement( ana->obj_info.obj, &ana->obj_info.index );
					}
					break;
				default:
					break;
				}
			}
		}
		/* -------------------------------------------------------------------- */
		/* analyze string														*/
		/* -------------------------------------------------------------------- */
		else
		{
			if( ana->state == E_JSON_STATE_ESC_U )
			{
				analyzeUtf( ana, buffer, &length, c );
				continue;
			}
			
			switch( c )
			{
			case	'\\':
				ana->state = E_JSON_STATE_ESC;
				length--;
				break;
			case	'\"':
				length--;
				if( ana->state == E_JSON_STATE_ESC )
				{
					/* nothing to do	*/
				}
				else
				{
					ana->stackp--;
					ana->state = E_JSON_STATE_STRING;
					buffer[ length ] = '\0';
					ana->obj_info.obj[ ana->obj_info.index++ ] = '/';
					strncpy( &ana->obj_info.obj[ ana->obj_info.index ],
							 ( const char* )buffer,
							 length );
					ana->obj_info.index += length;	// except null terminator
					ana->obj_info.obj[ ana->obj_info.index ] = '\0';

					length = 0;
					//logMessage( "name[%s]\n", buffer );
					//logMessage( "\nobj_name(%d)[%s]%d\n", ana->obj_level, ana->obj_info.obj, ana->obj_info.index );
					//logMessage( "obj_stack[%d]\n", ana->stack[ ana->stackp ] );
				}
				break;
#if 1
			case	':':
				length--;
				ana->stackp--;
				ana->state = E_JSON_STATE_STRING;
				break;
#endif
			default:
				if( ana->state == E_JSON_STATE_ESC )
				{
					switch( c )
					{
					case	'u':
						length--;
						string_esc_u = 0;
						ana->state = E_JSON_STATE_ESC_U;
						break;
					case	'b':
					case	'f':
					case	'n':
					case	'r':
					case	't':
						ana->state = E_JSON_STATE_STRING;
						length--;
						break;
					default:
						ana->state = E_JSON_STATE_STRING;
					break;
					}
				}
				break;
			}
		}
	}
}




/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
================================================================================
	Function	:cutLastElement
	Input		:char *elements
				 < path like structure >
				 int length
				 < length of elements >
	Output		:char *elements
				 < cut last element >
	Return		:void
	Description	:cut the last element.
				 < /createdat/id -> /created >
================================================================================
*/
void cutLastElement( char *elements, int *length )
{
	if( *length < 1 )
	{
		elements[ 0 ] = '\0';
		*length = 0;
		return;
	}

	//logMessage( "<?c?>" );

	*length = *length - 1;

	while( *length && elements[ *length ] != '\0' )
	{
		if( elements[ *length ] == '/' )
		{
			elements[ *length ] = '\0';
			return;
		}
		*length = *length - 1;
	}
}

/*
================================================================================
	Function	:analyzeUtf
	Input		:struct json_ana *ana
				 < json analysis context >
				 uint8_t *buffer
				 < json data >
				 int *length
				 < length of buffer >
				 int c
				 < current input of json data >
	Output		:uint8_t *buffer
				 < analyzed utf data >
				 int *length
				 < length of buffer >
	Return		:void
	Description	:analyze utf
================================================================================
*/
void analyzeUtf( struct json_ana *ana, uint8_t *buffer, int *length, int c )
{
	ana->utf.count++;

	//logMessage( "-%c=%d- ", c, buffer[ *length - 1 ] );

	if( ana->utf.in_state == E_JSON_UTF_STATE_FIRST_INPUT )
	{
		if( ( '0' <= c ) && ( c <= '9' ) )
		{
			buffer[ *length - 1 ] = ( uint8_t )( ( c - '0' ) << 4 );
		}
		else if( ( 'a' <= c ) && ( c <= 'f' ) )
		{
			buffer[ *length - 1 ] = ( uint8_t )( ( c - 'a' + 10 ) << 4 );
		}
		else if( ( 'A' <= c ) && ( c <= 'F' ) )
		{
			buffer[ *length - 1 ] = ( uint8_t )( ( c - 'A' + 10 ) << 4 );
		}
		else
		{
			buffer[ *length - 1 ] = 0x00;
		}

		ana->utf.in_state = E_JSON_UTF_STATE_SECOND_INPUT;
	}
	else
	{
		if( ( '0' <= c ) && ( c <= '9' ) )
		{
			buffer[ *length - 2 ] |= ( uint8_t )( c - '0' );
		}
		else if( ( 'a' <= c ) && ( c <= 'f' ) )
		{
			buffer[ *length - 2 ] |= ( uint8_t )( c - 'a' + 10 );
		}
		else if( ( 'A' <= c ) && ( c <= 'F' ) )
		{
			buffer[ *length - 2 ] |= ( uint8_t )( c - 'A' + 10 );
		}
		else
		{
			buffer[ *length - 2 ] |= 0x00;
		}

		*length = *length - 1;
		
		ana->utf.in_state = E_JSON_UTF_STATE_FIRST_INPUT;
	}
	
	if( 4 <= ana->utf.count )
	{
		int		utf_len;

		ana->utf.count = 0;

		switch( whichUtf16CodePoint( &buffer[ *length - 2 ] ) )
		{
		case	E_UTF16_NORMAL:
			ana->utf.in_state	= E_JSON_UTF_STATE_FIRST_INPUT;
			ana->utf.state		= E_UTF16_NORMAL;
			utf_len = convertUtf16ToUtf8( &buffer[ *length - 2 ],
										  &buffer[ *length - 2 ] );
			if( utf_len < 2 )
			{
				*length = *length - 1;
			}
			else if( 2 < utf_len )
			{
				*length = *length + 1;
			}
			break;
		case	E_UTF16_LEAD_SURROGATE:
			ana->utf.in_state	= E_JSON_UTF_STATE_FIRST_INPUT;
			ana->utf.state		= E_UTF16_LEAD_SURROGATE;
			break;
		case	E_UTF16_TRAIL_SURROGATE:
			ana->utf.in_state	= E_JSON_UTF_STATE_FIRST_INPUT;
			ana->utf.state		= E_UTF16_NORMAL;
			utf_len = convertSurrUtf16ToUtf8( &buffer[ *length - 4 ],
											  &buffer[ *length - 2 ],
											  &buffer[ *length - 4 ] );
			break;
		}
		if( ana->state == E_JSON_STATE_VALUE_STRING_ESC_U )
		{
			ana->state = E_JSON_STATE_VALUE_STRING;
		}
		else
		{
			ana->state = E_JSON_STATE_STRING;
		}
	}

}

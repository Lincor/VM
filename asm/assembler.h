#pragma once

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/

/*------------ Parameters -----------------*/
#define MAX_SYMBOL_SIZE 128

/*------------ VM's data types ------------*/
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

/*------------ Token types ----------------*/
#define TK_NONE   (0x0)
#define TK_LABEL  (0x1)
#define TK_SYMBOL (0x2)
#define TK_CMD    (0x3)
#define TK_REG    (0x4)
#define TK_IMM    (0x5)
#define TK_COMMA  (0x6)

/*------- Command argument types ----------*/
#define CA_LABEL   (0x0)
#define CA_REG     (0x1)
#define CA_IMM     (0x2)
#define CA_ADDRESS (0x3)

/*------------------------------------------*
*                  TYPEDEFS                 *
*------------------------------------------*/
typedef struct token_st {
	char *value_s;
	uint32_t value;
	uint8_t type;
	uint32_t code_column;
	struct token_st *next;
} token;

typedef struct token_list_st {
	uint32_t code_line;
	token *first_token;
	struct token_list_st *next;
} token_list;

typedef struct arg_st {
	uint8_t type;
	uint16_t v1, v2, v3, v4;
	char *value_s;
	struct arg_st *next;
} arg;

typedef struct cmd_st {
	uint8_t cmd_i;
	arg *args;
	struct cmd_st *next;
} cmd;

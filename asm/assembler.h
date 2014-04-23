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

#pragma once

/*------------------------------------------*
*                  DEFINES                  *
*------------------------------------------*/

/*------------ Parameters -----------------*/
#define MAX_SYMBOL_SIZE 128
#define FIRST_SEG 1
#define LAST_SEG 4

/*------------ VM's data types ------------*/
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

/*------------ Token types ----------------*/
#define TK_LABEL      (0x0)
#define TK_SYMBOL     (0x1)
#define TK_SYMBOL_ADR (0x2)
#define TK_CMD        (0x3)
#define TK_REG        (0x4)
#define TK_IMM        (0x5)
#define TK_SEG        (0x6)
#define TK_COMMA      (0x7)

/*------- Command argument types ----------*/
#define CA_SYMBOL     (0x0)
#define CA_SYMBOL_ADR (0x1)
#define CA_REG        (0x2)
#define CA_IMM        (0x3)
#define CA_SEG        (0x4)

/*------------ Code line types ------------*/
#define CL_CMD   (0x0)
#define CL_LABEL (0x1)

/*------------- Error codes -------------- */
#define ERR_NO_ERROR               (0x00) /* No error */
#define ERR_INV_TK                 (0x01) /* Invalid token */
#define ERR_INV_REG                (0x02) /* Invalid register name */
#define ERR_INV_DLR                (0x03) /* Invalid token of the form $smth */
#define ERR_INV_SEG_RANGE          (0x04) /* Segment number must be FIRST_SEG through LAST_SEG */
#define ERR_EXP_IMM_AFT_COM        (0x05) /* Integer constant after comma expected */
#define ERR_EXP_REG_AFT_COM        (0x06) /* Register after comma expected */
#define ERR_EXP_LBL_CMD_AT_BEG     (0x07) /* Label or instruction at the beginning of the line expected */
#define ERR_EXP_TK_AFT_COM         (0x08) /* Token after comma expected */
#define ERR_EXP_LBL_IMM_BEF_COL    (0x09) /* Label or number constant before colon expected  */
#define ERR_EXP_OFF_AFT_SEG        (0x0a) /* Offset after segment expected */
#define ERR_UNEXP_TK_AFT_LBL       (0x0b) /* Token after label */
#define ERR_MANY_ARGS              (0x0c) /* Too many arguments */
#define ERR_MANY_COM               (0x0d) /* Several commas in a row */
#define ERR_FEW_ARGS               (0x0e) /* Too few arguments */
#define ERR_LONG_SYM               (0x0f) /* Too long symbol */
#define ERR_ILL_CHR_SYM            (0x10) /* Illegal character(s) in symbol */

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
} cmd;

typedef struct line_st {
	uint8_t type;
	uint32_t code_line;
	cmd *command;
	char *label;
	struct line_st *next;
} line;

#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "asm_error.h"

static char *error_to_str(uint8_t err);

void asm_error(uint8_t err, int line, int column)
{
	fprintf(stderr, "Error: '%s' at %d, %d.\n", error_to_str(err), line ,column);
	 
	exit(err);
}

static char *error_to_str(uint8_t err)
{
	char *str;

	switch (err) {
		case ERR_NO_ERROR:
			str = "no error";
			break;
		case ERR_INV_ADR:
			str = "invalid construction of the form $a(%%b, %%c, $d)";
			break;
		case ERR_INV_TK:
			str = "invalid token";
			break;
		case ERR_INV_IMM_1_2_4:
			str = "constant must be 1, 2 or 4";
			break;
		case ERR_INV_REG:
			str = "invalid register";
			break;
		case ERR_INV_DLR:
			str = "invalid construction of the form $smth";
			break;
		case ERR_EXP_IMM_AFT_COM:
			str = "integer constant after comma expected";
			break;
		case ERR_EXP_REG_AFT_COM:
			str = "register after comma expected";
			break;
		case ERR_EXP_CBRC_COM_AFT_REG:
			str = "closed bracket or comma after register expected";
			break;
		case ERR_EXP_REG_AFT_OBRC:
			str = "register after open bracket expected";
			break;
		case ERR_EXP_LBL_CMD_AT_BEG:
			str = "label or instruction at the beggining of the line expected";
			break;
		case ERR_EXP_TK_AFT_COM:
			str = "token expected after comma";
			break;
		case ERR_EXP_REG_COM_AFT_OBRC:
			str = "register or comma after open bracket expected";
			break;
		case ERR_EXP_LBL_BEF_COL:
			str = "label before colon expected";
			break;
		case ERR_UNEXP_TK_AFT_LBL:
			str = "unexpected token after label";
			break;
		case ERR_MANY_ARGS:
			str = "too many arguments";
			break;
		case ERR_MANY_COM:
			str = "several commas in a row";
			break;
		case ERR_MANY_OBRC:
			str = "too many open brackets";
			break;
		case ERR_MANY_CBRC:
			str = "too many closed brackets";
			break;
		case ERR_FEW_ARGS:
			str = "too few arguments";
			break;
		case ERR_LONG_SYM:
			str = "too long symbol";
			break;
		case ERR_ILL_CHR_SYM:
			str = "illegal characters in symbol";
			break;
	}

	return str;
}
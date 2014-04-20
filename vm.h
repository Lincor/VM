#pragma once

/*
 * Типы данных для ВМ
 */
#define uint16_t unsigned short
#define uint8_t unsigned char

/*
 * Настройки ВМ
 */
#define REG_COUNT   16
#define MEM_SIZE    65536
#define PORTS_CNT   64
// Специальные регистры
#define REG_PC 0xf
#define REG_SP 0xe
#define REG_BP 0xd
#define REG_AC 0xc

typedef struct {
	uint16_t base;
	uint16_t limit;
	uint8_t type; //0 - код, 1 - данные
	uint8_t ro; //только чтение
	uint8_t access; //уровень доступа
} selector;

void vm_cmd_nop(uint8_t args[]);

void vm_cmd_ldw(uint8_t args[]);

void vm_cmd_ldb(uint8_t args[]);

void vm_cmd_llb(uint8_t args[]);

void vm_cmd_lhb(uint8_t args[]);

void vm_cmd_lvar(uint8_t args[]);

void vm_cmd_add(uint8_t args[]);

void vm_cmd_sub(uint8_t args[]);

void vm_cmd_mul(uint8_t args[]);

void vm_cmd_div(uint8_t args[]);

void vm_cmd_mod(uint8_t args[]);

void vm_cmd_shl(uint8_t args[]);

void vm_cmd_shr(uint8_t args[]);

void vm_cmd_jeq(uint8_t args[]);

void vm_cmd_jne(uint8_t args[]);

void vm_cmd_jlt(uint8_t args[]);

void vm_cmd_jgt(uint8_t args[]);

void vm_cmd_jle(uint8_t args[]);

void vm_cmd_jge(uint8_t args[]);

void vm_cmd_in(uint8_t args[]);

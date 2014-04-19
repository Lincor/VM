#include <stdio.h>
#include <stdlib.h>

/* TODO:
 * 1) Реализовать весь набор команд
 * 2) Перывания
 * 3) Оборудование
 * 4) Виртуальная память
 * 5) slb/shb и llb/lhb
 * 6) Проверять переполнение при арфиметических операциях
 */

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
#define REG_FL 0xc

uint16_t vm_reg[REG_COUNT];
uint8_t  vm_mem[MEM_SIZE];
uint16_t vm_pio[PORTS_CNT];

/*
 * Команды ВМ
 */

void vm_cmd_nop(uint8_t args[]) {
    //No oPeration
}

void vm_cmd_ldw(uint8_t args[]) {
    uint8_t  reg;
    uint16_t wrd;
    reg = args[0] & 0xf;
    wrd = (args[1] << 8) | args[2];
    vm_reg[reg] = wrd;
}

void vm_cmd_ldb(uint8_t args[]) {
    uint8_t reg, byt;
    reg = args[0] & 0xf;
    byt = args[1];
    vm_reg[reg] = byt;
}

void vm_cmd_add(uint8_t args[]) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] += vm_reg[rgb];
}

void vm_cmd_sub(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] -= vm_reg[rgb];
}

void vm_cmd_mul(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] *= vm_reg[rgb];
}

void vm_cmd_div(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] /= vm_reg[rgb];
}

void vm_cmd_mod(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] %= vm_reg[rgb];
}

void vm_cmd_in(uint8_t args[]) {
    uint8_t reg, prt;
    reg = args[0];
    prt = args[1];
    switch (prt) {
        case 0: {
            printf("%d", vm_reg[reg]);
        } break;
        case 1: {
            printf("%c", vm_reg[reg]);
        } break;
    }
}

/*
 * Все команды ВМ и кол-во байт-аргументов
 */

#define CMD_COUNT 9

struct {
    void    (*func)();
    uint8_t argc;
} vm_cmd[CMD_COUNT] = {
    {vm_cmd_nop, 0}, //0 пока ассемблера нет, пользуемся этим
    {vm_cmd_ldw, 3}, //1
    {vm_cmd_ldb, 2}, //2
    {vm_cmd_add, 1}, //3
    {vm_cmd_sub, 1}, //4
    {vm_cmd_mul, 1}, //5
    {vm_cmd_div, 1}, //6
    {vm_cmd_mod, 1}, //7
    {vm_cmd_in , 2}  //8
};

void vm_exec_comand() {
    uint8_t cmd;
    cmd = vm_mem[vm_reg[REG_PC]++];

    //Тут надо кидать прерывание!
    if (cmd > CMD_COUNT) {
        printf("Invalid comand!\n");
    }

    uint8_t i, bytes[4];
    for(i = 0; i < vm_cmd[cmd].argc; i++) {
        bytes[i] = vm_mem[vm_reg[REG_PC]++];
    }

    vm_cmd[cmd].func(&bytes);
}

int main()
{
    printf("TINY RISC MACHINE 1975.\n");
    vm_mem[0] = 1;
    vm_mem[1] = 0;
    vm_mem[2] = 255;
    vm_mem[3] = 255;

    vm_mem[4] = 8;
    vm_mem[5] = 0;
    vm_mem[6] = 0;

    vm_exec_comand();
    vm_exec_comand();
    return 0;
}

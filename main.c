#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

/* TODO:
 * 1) Реализовать весь набор команд
 * 2) Перывания
 * 3) Оборудование
 * 4) Виртуальная память
 * 5) slb/shb и llb/lhb
 * 6) Проверять переполнение при арфиметических операциях
 */

uint16_t vm_reg[REG_COUNT];
selector vm_seg_regs[4]; //4 сегмента это мало
uint8_t  vm_mem[MEM_SIZE];
uint16_t vm_pio[PORTS_CNT];
uint8_t access; //текущие привилегии

void segfault() {
	int* ptr = (int*)0;
	*ptr = 1;
}

//внутренняя функция ВМ
uint16_t translate_addr(uint8_t reg, uint16_t vaddr) {
	if (access>vm_seg_regs[reg].access) segfault();
	//физический адрес
	uint16_t paddr;
	paddr=vm_seg_regs[reg].base+vaddr;
	if (paddr>vm_seg_regs[reg].limit) {
		segfault();
		//тут должно быть исключение, сообщающее о пиздеце, но прерываний пока нет
	} else return paddr;
}

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

void vm_cmd_llb(uint8_t args[]) {
    uint8_t reg, byt;
    reg = args[0] & 0xf;
    byt = args[1];
    vm_reg[reg] &= 0xff00;
    vm_reg[reg] |= byt;
}

void vm_cmd_lhb(uint8_t args[]) {
    uint8_t reg, byt;
    reg = args[0] & 0xf0;
    byt = args[1];
    vm_reg[reg] &= 0x00ff;
    vm_reg[reg] |= byt << 8;
}

void vm_cmd_lvar(uint8_t args[]) {
	uint8_t reg,seg;
	reg = args[0];
	seg= args[1];
	if (vm_seg_regs[seg].ro || !vm_seg_regs[seg].type) segfault();
	printf("%d\n",vm_reg[reg]);
	vm_mem[args[2]]=vm_reg[reg];
	printf("%d\n",args[2]);
}

/*
 * Арифметические операции
 */

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

void vm_cmd_shl(uint8_t args[]) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] <<= vm_reg[rgb];
}

void vm_cmd_shr(uint8_t args[]) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    vm_reg[rga] >>= vm_reg[rgb];
}

/*
 * Команды условного перехода
 */

void vm_cmd_jeq(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    uint16_t wrd;
    wrd = (args[1] << 8) | args[2];
    if (vm_reg[rga] == vm_reg[rgb]) {
        vm_reg[REG_PC] = wrd;
    }
}

void vm_cmd_jne(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    uint16_t wrd;
    wrd = (args[1] << 8) | args[2];
    if (vm_reg[rga] != vm_reg[rgb]) {
        vm_reg[REG_PC] = wrd;
    }
}

void vm_cmd_jlt(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    uint16_t wrd;
    wrd = (args[1] << 8) | args[2];
    if (vm_reg[rga] < vm_reg[rgb]) {
        vm_reg[REG_PC] = wrd;
    }
}

void vm_cmd_jgt(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    uint16_t wrd;
    wrd = (args[1] << 8) | args[2];
    if (vm_reg[rga] > vm_reg[rgb]) {
        vm_reg[REG_PC] = wrd;
    }
}

void vm_cmd_jle(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    uint16_t wrd;
    wrd = (args[1] << 8) | args[2];
    if (vm_reg[rga] <= vm_reg[rgb]) {
        vm_reg[REG_PC] = wrd;
    }
}

void vm_cmd_jge(uint8_t *args) {
    uint8_t rga, rgb;
    rga = args[0] >> 4;
    rgb = args[0] & 0xf;
    uint16_t wrd;
    wrd = (args[1] << 8) | args[2];
    if (vm_reg[rga] >= vm_reg[rgb]) {
        vm_reg[REG_PC] = wrd;
    }
}

/*
 * Работа с портами ввода/вывода
 */

void vm_cmd_in(uint8_t args[]) {
    uint8_t reg, prt;
    reg = args[0] & 0xf;
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

#define CMD_COUNT 20

struct {
    void (*func)();
    uint8_t argc;
} vm_cmd[CMD_COUNT] = {
    {vm_cmd_nop,  0}, //0 пока ассемблера нет, пользуемся этим
    //а вот были бы в C ассоциативные массивы...
    {vm_cmd_ldw,  3}, //1
    {vm_cmd_ldb,  2}, //2
    {vm_cmd_llb,  2}, //3
    {vm_cmd_lhb,  2}, //4
    {vm_cmd_add,  1}, //5
    {vm_cmd_sub,  1}, //6
    {vm_cmd_mul,  1}, //7
    {vm_cmd_div,  1}, //8
    {vm_cmd_mod,  1}, //9
    {vm_cmd_jeq,  1}, //10
    {vm_cmd_jne,  1}, //11
    {vm_cmd_jlt,  1}, //12
    {vm_cmd_jgt,  1}, //13
    {vm_cmd_jle,  1}, //14
    {vm_cmd_jge,  1}, //15
    {vm_cmd_in ,  2}, //16
    {vm_cmd_lvar, 3}, //17
	{vm_cmd_shl,  1}, //18
	{vm_cmd_shr,  1}  //19
};

void vm_exec_comand(uint8_t seg) {
	if (vm_seg_regs[seg].type) segfault();
    uint8_t cmd;
    cmd = vm_mem[translate_addr(seg,vm_reg[REG_PC]++)];
    //Тут надо кидать прерывание!
    if (cmd > CMD_COUNT) {
        printf("Invalid comand!\n");
        return;
    }
    uint8_t i, bytes[4];
    for(i = 0; i < vm_cmd[cmd].argc; i++) {
        bytes[i] = vm_mem[translate_addr(seg,vm_reg[REG_PC]++)];
    }
    vm_cmd[cmd].func(&bytes);
}

int main() {
    printf("TINY RISC MACHINE 1975.\n");
    access=0;
    vm_seg_regs[0].base=0;
    vm_seg_regs[0].limit=200;
    vm_seg_regs[0].access=0;
    vm_seg_regs[0].ro=0;
    vm_seg_regs[0].type=0;
    vm_seg_regs[1].base=200;
    vm_seg_regs[1].limit=65535;
    vm_seg_regs[1].access=0;
    vm_seg_regs[1].ro=0;
    vm_seg_regs[1].type=1;
    vm_reg[0] = 'L';
    vm_mem[translate_addr(0,0)] = 16;
    vm_mem[translate_addr(0,1)] = 0;
    vm_mem[translate_addr(0,2)] = 1;
    vm_reg[1] = 'O';
    vm_mem[translate_addr(0,3)] = 16;
    vm_mem[translate_addr(0,4)] = 1;
    vm_mem[translate_addr(0,5)] = 1;
    vm_reg[2] = 'L';
    vm_mem[translate_addr(0,6)] = 16;
    vm_mem[translate_addr(0,7)] = 2;
    vm_mem[translate_addr(0,8)] = 1;
    vm_exec_comand(0);
    vm_exec_comand(0);
    vm_exec_comand(0);
    vm_reg[0] = 15;
    vm_mem[translate_addr(0,9)] = 17;
    vm_mem[translate_addr(0,10)] = 0;
    vm_mem[translate_addr(0,11)] = 1;
    vm_mem[translate_addr(0,12)] = translate_addr(1,0);
    vm_exec_comand(0);
    printf("%d",vm_mem[translate_addr(1,0)]);
    //вызовем сегфолт
    //vm_mem[translate_addr(0,3)] = 1;
    /*
     * Возрадуйся, Илья, ибо прямой доступ к памяти все еще возможен!
     * Эта врезка посвящается тебе. Такая модель памяти (когда все сегменты
     * начинаются с начала и закончиваются в конце памяти) называется "flat"
    access=0;
    vm_seg_regs[0].base=0;
    vm_seg_regs[0].limit=65535;
    vm_seg_regs[0].access=0;
    vm_seg_regs[0].ro=0;
    vm_seg_regs[0].type=0;
    vm_seg_regs[1].base=0;
    vm_seg_regs[1].limit=65535;
    vm_seg_regs[1].access=0;
    vm_seg_regs[1].ro=0;
    vm_seg_regs[1].type=1;
    vm_reg[0] = 'L';
    vm_mem[translate_addr(0,0)] = 16;
    vm_mem[translate_addr(1,100)] = 0;
    vm_mem[translate_addr(1,101)] = 1;
    vm_reg[1] = 'O';
    vm_mem[translate_addr(0,1)] = 16;
    vm_mem[translate_addr(1,102)] = 1;
    vm_mem[translate_addr(1,103)] = 1;
    vm_reg[2] = 'L';
    vm_mem[translate_addr(0,2)] = 16;
    vm_mem[translate_addr(1,104)] = 2;
    vm_mem[translate_addr(1,105)] = 1;
    vm_reg[REG_AC]=100;
    */
    vm_exec_comand(0);
    vm_exec_comand(0);
    vm_exec_comand(0);
    return 0;
}

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

/*
 * Сегментные регистры ВМ
 */
#define SEG_CODE 0
#define SEG_DATA 1
#define SEG_READ_WRITE 0
#define SEG_READ_ONLY  1

struct { //структура жа только в одном месте используется? Тогда можно не засорять глобальное пространство, а просто запилить массив регистров.
	uint16_t base;
	uint16_t limit;
	uint8_t type;	//0 - код, 1 - данные //Не надо так, позязя. Сделал клевенькие дефайны
	uint8_t ro;		//только чтение
	uint8_t access;	//уровень доступа
} vm_seg_regs[4];	//4 сегмента это мало

// Имена спец. регистров
#define REG_PC 0xf
#define REG_SP 0xe
#define REG_BP 0xd
#define REG_AC 0xc

uint16_t vm_reg[REG_COUNT];
uint8_t  vm_mem[MEM_SIZE];
uint16_t vm_pio[PORTS_CNT];
uint8_t  vm_access; //текущие привилегии

void segfault() { //Это жестоко :)
    printf("Surpise!\n");
	int* ptr = (int*)0;
	*ptr = 1;
}

//внутренняя функция ВМ
uint16_t vm_translate_addr(uint8_t reg, uint16_t vaddr) {
	if (vm_access > vm_seg_regs[reg].access) { //Я обернул все сегфолты в скобки и добваил return, на будущее. Не всегда же приложению валится xD
		segfault();
		return;
	}
	//физический адрес
	uint16_t paddr;
	paddr = vm_seg_regs[reg].base + vaddr;
	if (paddr > vm_seg_regs[reg].limit) {
		segfault();
		//тут должно быть исключение, сообщающее о пиздеце, но прерываний пока нет
	} else {
		return paddr;
	}
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

//lvar
//svar (для записи с помощью сегментных регистров)

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

#define CMD_COUNT 19

struct {
    void (*func)();
    uint8_t argc;
} vm_cmd[CMD_COUNT] = {
    {vm_cmd_nop,  0}, //0 пока ассемблера нет, пользуемся этим //а вот были бы в C ассоциативные массивы... //Они именно ЗДЕСЬ, не нужны :) А так, можно же запилить, не сложно.
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
	{vm_cmd_shl,  1}, //17
	{vm_cmd_shr,  1}  //18
};

void vm_exec_comand(uint8_t seg) {
	if (vm_seg_regs[seg].type != SEG_CODE) { //Мы же только код исполнять можем, верно?
		segfault();
		return;
	}
    uint8_t cmd;
    cmd = vm_mem[vm_translate_addr(seg, vm_reg[REG_PC]++)];
    //Тут надо кидать прерывание!
    if (cmd > CMD_COUNT) {
        printf("Invalid comand!\n");
        return;
    }
    uint8_t i, bytes[4];
    for(i = 0; i < vm_cmd[cmd].argc; i++) {
        bytes[i] = vm_mem[vm_translate_addr(seg, vm_reg[REG_PC]++)];
    }
    vm_cmd[cmd].func(&bytes);
}

int main() {
    printf("TINY RISC MACHINE 1975.\n");
    vm_access=0;

    vm_seg_regs[0].base=0;
    vm_seg_regs[0].limit=200;
    vm_seg_regs[0].access=0;
    vm_seg_regs[0].ro=SEG_READ_WRITE;
    vm_seg_regs[0].type=SEG_CODE;

    vm_seg_regs[1].base=200;
    vm_seg_regs[1].limit=65535;
    vm_seg_regs[1].access=0;
    vm_seg_regs[1].ro=SEG_READ_WRITE;
    vm_seg_regs[1].type=SEG_DATA;

    vm_reg[0] = 'L';
    vm_mem[vm_translate_addr(0,0)] = 16;
    vm_mem[vm_translate_addr(0,1)] = 0;
    vm_mem[vm_translate_addr(0,2)] = 1;

    vm_reg[1] = 'O';
    vm_mem[vm_translate_addr(0,3)] = 16;
    vm_mem[vm_translate_addr(0,4)] = 1;
    vm_mem[vm_translate_addr(0,5)] = 1;

    vm_reg[2] = 'L';
    vm_mem[vm_translate_addr(0,6)] = 16;
    vm_mem[vm_translate_addr(0,7)] = 2;
    vm_mem[vm_translate_addr(0,8)] = 1;

    vm_exec_comand(0);
    vm_exec_comand(0);
    vm_exec_comand(0);

    vm_reg[0] = 15;
    vm_mem[vm_translate_addr(0,9)] = 16;
    vm_mem[vm_translate_addr(0,10)] = 0;
    vm_mem[vm_translate_addr(0,11)] = 0;

    vm_mem[vm_translate_addr(0,12)] = vm_translate_addr(1,0);

    vm_exec_comand(0);
    printf("%d",vm_mem[vm_translate_addr(1,0)]);
    //вызовем сегфолт
    //vm_mem[vm_translate_addr(0,3)] = 1;
    /*
     * Возрадуйся, Илья, ибо прямой доступ к памяти все еще возможен!
     * Эта врезка посвящается тебе. Такая модель памяти (когда все сегменты
     * начинаются с начала и закончиваются в конце памяти) называется "flat"
    vm_access=0;
    vm_seg_regs[0].base=0;
    vm_seg_regs[0].limit=65535;
    vm_seg_regs[0].vm_access=0;
    vm_seg_regs[0].ro=0;
    vm_seg_regs[0].type=0;
    vm_seg_regs[1].base=0;
    vm_seg_regs[1].limit=65535;
    vm_seg_regs[1].vm_access=0;
    vm_seg_regs[1].ro=0;
    vm_seg_regs[1].type=1;
    vm_reg[0] = 'L';
    vm_mem[vm_translate_addr(0,0)] = 16;
    vm_mem[vm_translate_addr(1,100)] = 0;
    vm_mem[vm_translate_addr(1,101)] = 1;
    vm_reg[1] = 'O';
    vm_mem[vm_translate_addr(0,1)] = 16;
    vm_mem[vm_translate_addr(1,102)] = 1;
    vm_mem[vm_translate_addr(1,103)] = 1;
    vm_reg[2] = 'L';
    vm_mem[vm_translate_addr(0,2)] = 16;
    vm_mem[vm_translate_addr(1,104)] = 2;
    vm_mem[vm_translate_addr(1,105)] = 1;
    vm_reg[REG_AC]=100;
    */
    //vm_exec_comand(0);
    //vm_exec_comand(0);
    //vm_exec_comand(0);
    return 0;
}

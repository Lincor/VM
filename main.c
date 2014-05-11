#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#ifdef __unix__
	#include "getch.c"
#else
	#include <conio.h>
#endif

/* TODO по проекту:
 * 1) Реализовать весь набор команд ВМ
 * 2) Написать ассемблер, протестировать ВМ
 * 3) Добавить оборудование для ВМ
 * 4) Напистаь загрузчик ОС для ВМ
 * 5) Напистаь компилятор Си
 * 6) Написать ОС (Может MINIX портировать? Красивая маленькая ОС)
 *
 * TODO по ВМ (более приоритетные задачи):
 * 1) Переполнения, прерывания
 * 2) Знаковая арифметика
 */

/*
 * Типы данных для ВМ
 */
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t unsigned char

/*
 * Настройки ВМ
 */
#define REG_COUNT   16
#define MEM_SIZE    65536
#define PORTS_CNT   64

#define CS 0
#define DS 1
#define SS 2

struct {
	uint16_t base;
	uint16_t limit;
	enum {
		SEG_CODE,
		SEG_DATA,
		SEG_STACK
	} type;
	enum {
		SEG_READ_WRITE,
		SEG_READ_ONLY
	} ro;
	uint8_t access;	// Access level
} vm_seg_regs[4];

#define INTERRUPTS_MAX	128		// максимальное кол-во прерываний
#define INTERRUPTS_OFF  100    // потом поменяем

struct {
	uint16_t addr[INTERRUPTS_MAX];
	uint8_t  ptr;
} vm_interrupts;

// Имена спец. регистров
#define REG_PC	0xf
#define REG_SP	0xe
#define REG_BP	0xd
#define REG_INT	0xc

uint16_t vm_reg[REG_COUNT];
uint8_t  vm_mem[MEM_SIZE];
uint16_t vm_pio[PORTS_CNT];
uint8_t  vm_access;

void segfault() {
	printf("Surprise!\n");
	int* ptr = (int*)0;
	*ptr = 1;
}

/*
 * Внутренние функции ВМ
 */
uint16_t vm_translate_addr(uint8_t reg, uint16_t vaddr) {
	if (vm_access > vm_seg_regs[reg].access) {
		segfault();
		return 0;
	}
	uint16_t paddr;
	paddr = vm_seg_regs[reg].base + vaddr;
	if (paddr > vm_seg_regs[reg].limit) {
		segfault();
	} else {
		return paddr;
	}
	return 0;
}

void vm_set(uint8_t seg, uint16_t dest, uint8_t val) {
	if (vm_seg_regs[seg].ro) {
		segfault();
		return;
	}
	vm_mem[vm_translate_addr(seg, dest)] = val;
}

uint8_t vm_get(uint8_t seg, uint16_t addr) {
	return vm_mem[vm_translate_addr(seg, addr)];
}

uint8_t vm_load(char* name) {
	FILE* f = fopen(name, "rb");
	if (f == NULL) return 1;
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fread(vm_mem, 1, size, f);
	return 0;
}

/*
 * Прерывания
 */

/* Таблица прерываний имеет такой формат:
 *    0000: сегмент (1 байт) слово (2 байта)
 *    0003: сегмент (1 байт) слово (2 байта)
 *    0006: сегмент (1 байт) слово (2 байта)
 *    ...
 *
 * Начинается с адреса INTERRUPTS_OFF
 *
 * Таблица прерываний указывает на обработчики прерываний
 */

uint8_t vm_interrupt_add(uint8_t number) {
	if (vm_interrupts.ptr < INTERRUPTS_MAX) {
		vm_interrupts.addr[vm_interrupts.ptr++] = number;
		return 1;
	} else {
		// Печаль беда
		// А если серьезно, то надо с этим что то деалать. Но я хз, что лучше сделать в таком случае
		return 0;
	}
}

uint8_t vm_interrupt_get() {
	uint8_t i, number = vm_interrupts.addr[0];
	for(i = 0; i < vm_interrupts.ptr; i++)
		vm_interrupts.addr[i] = vm_interrupts.addr[i + 1];
	--vm_interrupts.ptr;
}

uint8_t vm_interrupt_exec() {
	if (vm_interrupts.ptr > 0) {
		uint8_t num = vm_interrupt_get() * 2;
		uint16_t adr, ret;
		ret  = vm_reg[REG_PC];
		adr  = vm_mem[INTERRUPTS_OFF + num + 0] << 8;
		adr |= vm_mem[INTERRUPTS_OFF + num + 1];

		vm_set(SS, vm_reg[REG_SP]--, ret >> 8);
		vm_set(SS, vm_reg[REG_SP]--, ret & 0xff);
		vm_reg[REG_PC] = adr;
	}
}

void print_inters() {
	uint8_t i;
	for(i = 0; i < vm_interrupts.ptr; i++)
		printf("%d ", vm_interrupts.addr[i]);
	printf("\n");
}

/*
 * Команды ВМ
 */

void vm_cmd_int(uint8_t args[]) {
	uint8_t num;
	num = args[0] & 0xff; //Максимум - 256 прерываний
	vm_interrupt_add(num);
}

void vm_cmd_nop(uint8_t args[]) {
	//No oPeration
}

void vm_cmd_cpy(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] = vm_reg[src];
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

void vm_cmd_ldmb(uint8_t args[]) {
	uint8_t  reg, seg;
	uint16_t wrd;
	reg = args[0] & 0xf;
	seg = args[1] & 0x3;
	wrd = (args[2] << 8) | args[3];
	vm_reg[reg] = vm_get(seg, wrd);
}

void vm_cmd_ldmw(uint8_t args[]) {
	uint8_t  reg, seg;
	uint16_t wrd;
	reg = args[0] & 0xf;
	seg = args[1] & 0x3;
	wrd = (args[2] << 8) | args[3];
	vm_reg[reg] = (vm_get(seg, wrd) << 8) | vm_get(seg, wrd + 1);
}

void vm_cmd_hlt(uint8_t args[]) {
	exit(0);
}

/*
 * Арифметические операции
 */

void vm_cmd_add(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] += vm_reg[src];
}

void vm_cmd_sub(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] -= vm_reg[src];
}

void vm_cmd_mul(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] *= vm_reg[src];
}

void vm_cmd_div(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] /= vm_reg[src];
}

void vm_cmd_mod(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] %= vm_reg[src];
}

void vm_cmd_shl(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] <<= vm_reg[src];
}

void vm_cmd_shr(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] >>= vm_reg[src];
}

/*
 * Логические операции
 */

void vm_cmd_or(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] |= vm_reg[src];
}

void vm_cmd_and(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] &= vm_reg[src];
}

void vm_cmd_xor(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] ^= vm_reg[src];
}

void vm_cmd_not(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] = ~vm_reg[src];
}

/*
 * Команды условного перехода
 */

void vm_cmd_jeq(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	uint16_t wrd;
	wrd = (args[1] << 8) | args[2];
	if (vm_reg[rga] == vm_reg[rgb]) {
		vm_reg[REG_PC] = wrd;
	}
}

void vm_cmd_jne(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	uint16_t wrd;
	wrd = (args[1] << 8) | args[2];
	if (vm_reg[rga] != vm_reg[rgb]) {
		vm_reg[REG_PC] = wrd;
	}
}

void vm_cmd_jlt(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	uint16_t wrd;
	wrd = (args[1] << 8) | args[2];
	if (vm_reg[rga] < vm_reg[rgb]) {
		vm_reg[REG_PC] = wrd;
	}
}

void vm_cmd_jgt(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	uint16_t wrd;
	wrd = (args[1] << 8) | args[2];
	if (vm_reg[rga] > vm_reg[rgb]) {
		vm_reg[REG_PC] = wrd;
	}
}

void vm_cmd_jle(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	uint16_t wrd;
	wrd = (args[1] << 8) | args[2];
	if (vm_reg[rga] <= vm_reg[rgb]) {
		vm_reg[REG_PC] = wrd;
	}
}

void vm_cmd_jge(uint8_t args[]) {
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
 * Команды безусловного перехода
 */

void vm_cmd_jmp(uint8_t args[]) {
	uint16_t wrd;
	wrd = (args[0] << 8) | args[1];
	vm_reg[REG_PC] = wrd;
}

void vm_cmd_jpr(uint8_t args[]) {
	uint8_t reg;
	reg = args[0] & 0xf;
	vm_reg[REG_PC] = vm_reg[reg];
}

/*
 * Работа с портами ввода/вывода
 */

void vm_cmd_out(uint8_t args[]) {
	uint8_t reg, prt;
	reg = args[0] & 0xf;
	prt = args[1];
	switch (prt) {
		case 0: {
			printf("%d", vm_reg[reg]);
		}
		break;
		case 1: {
			printf("%c", vm_reg[reg]);
		}
		break;
	}
}

void vm_cmd_in(uint8_t args[]) {
	uint8_t reg, prt;
	prt = args[0] & 0xf;
	reg = args[1];
	switch (prt) {
		case 0: {
			scanf("%hu", &vm_reg[reg]);
		}
		break;
		case 1: {
			vm_reg[reg] = getch();
		}
	}
}

/*
 * Работа со стеком
 */

void vm_cmd_pushr(uint8_t args[]) {
	uint8_t reg;
	reg = args[0] & 0xf;
	vm_set(SS, vm_reg[REG_SP]--, vm_reg[reg] >> 8);
	vm_set(SS, vm_reg[REG_SP]--, vm_reg[reg] & 0xff);
}

void vm_cmd_pushv(uint8_t args[]) {
	uint16_t wrd;
	wrd = (args[0] << 8) | args[1];
	vm_set(SS, vm_reg[REG_SP]--, wrd >> 8);
	vm_set(SS, vm_reg[REG_SP]--, wrd & 0xff);
}

void vm_cmd_pop(uint8_t args[]) {
	uint16_t reg;
	reg = args[0] & 0xf;
	uint8_t wh, wl;
	wh = vm_get(SS, ++vm_reg[REG_SP]);
	wl = vm_get(SS, ++vm_reg[REG_SP]);
	vm_reg[reg] = (wl << 8) | wh;
}

void vm_cmd_callr(uint8_t args[]) {
	uint8_t reg;
	reg = vm_reg[args[0] & 0xf];
	vm_set(SS, vm_reg[REG_SP]--, vm_reg[REG_PC] >> 8);
	vm_set(SS, vm_reg[REG_SP]--, vm_reg[REG_PC] & 0xff);
	vm_reg[REG_PC] = reg;
}

void vm_cmd_callv(uint8_t args[]) {
	uint16_t adr;
	adr = (args[0] << 8) | args[1];
	vm_set(SS, vm_reg[REG_SP]--, vm_reg[REG_PC] >> 8);
	vm_set(SS, vm_reg[REG_SP]--, vm_reg[REG_PC] & 0xff);
	vm_reg[REG_PC] = adr;
}

void vm_cmd_ret(uint8_t args[]) {
	uint8_t wh, wl;
	wh = vm_get(SS, ++vm_reg[REG_SP]);
	wl = vm_get(SS, ++vm_reg[REG_SP]);
	vm_reg[REG_PC] = (wl << 8) | wh;
}

/*
 * Все команды ВМ и кол-во байт-аргументов
 */

#define CMD_COUNT 35

struct {
	void (*func)();
	uint8_t argc;
} vm_cmd[CMD_COUNT] = {
	{vm_cmd_nop,  0}, //0
	{vm_cmd_hlt,  0}, //1

	{vm_cmd_int,  1}, //2

	{vm_cmd_cpy,  1}, //3
	{vm_cmd_ldw,  3}, //4
	{vm_cmd_ldb,  2}, //5
	{vm_cmd_llb,  2}, //6
	{vm_cmd_lhb,  2}, //7

	{vm_cmd_add,  1}, //8
	{vm_cmd_sub,  1}, //9
	{vm_cmd_mul,  1}, //10
	{vm_cmd_div,  1}, //11
	{vm_cmd_mod,  1}, //12
	{vm_cmd_shl,  1}, //13
	{vm_cmd_shr,  1}, //14
	{vm_cmd_or,   1}, //15
	{vm_cmd_and,  1}, //16
	{vm_cmd_xor,  1}, //17
	{vm_cmd_not,  1}, //18

	{vm_cmd_jeq,  3}, //19
	{vm_cmd_jne,  3}, //20
	{vm_cmd_jlt,  3}, //21
	{vm_cmd_jgt,  3}, //22
	{vm_cmd_jle,  3}, //23
	{vm_cmd_jge,  3}, //24
	{vm_cmd_jmp,  2}, //25
	{vm_cmd_jpr,  1}, //26

	{vm_cmd_callr,1}, //27
	{vm_cmd_callv,2}, //28
	{vm_cmd_pushr,1}, //29
	{vm_cmd_pushv,2}, //30
	{vm_cmd_pop,  1}, //31
	{vm_cmd_ret,  0}, //32

	{vm_cmd_in,   2}, //33
	{vm_cmd_out,  2}  //34
};

void vm_exec_comand(uint8_t seg) {
	if (vm_seg_regs[seg].type != SEG_CODE) {
		segfault();
		return;
	}
	uint8_t cmd;
	cmd = vm_get(seg, vm_reg[REG_PC]++);
	//Тут надо кидать прерывание!
	if (cmd > CMD_COUNT) {
		printf("Invalid comand: %d\n", cmd);
		return;
	}
	uint8_t i, bytes[4];
	for(i = 0; i < vm_cmd[cmd].argc; i++) {
		bytes[i] = vm_get(seg, vm_reg[REG_PC]++);
	}
	vm_cmd[cmd].func(&bytes);
}

#define IPS 100000

void vm_exec_loop() {
	uint32_t ips = 0;
	double old_time, sleep_time;
	old_time = time(0);

	while (1) {
		if (ips++ < IPS) {
			vm_exec_comand(0);
			vm_interrupt_exec();
		} else {
			//printf("Instructions: %d @ time: %f ms\n", ips, (time(0) - old_time));
			sleep_time = (1000 - (time(0) - old_time)); //отнимаем от секунды время затраченное на выполнение инструкций
			if (sleep_time > 0) {
				usleep(sleep_time);
				old_time = time(0);
				ips = 0;
			}
		}
	}
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
	vm_seg_regs[1].limit=64535;
	vm_seg_regs[1].access=0;
	vm_seg_regs[1].ro=SEG_READ_WRITE;
	vm_seg_regs[1].type=SEG_DATA;

	vm_seg_regs[2].base=64535;
	vm_seg_regs[2].limit=65535;
	vm_seg_regs[2].access=0;
	vm_seg_regs[2].ro=SEG_READ_WRITE;
	vm_seg_regs[2].type=SEG_STACK;
	vm_reg[REG_SP]=65535;

	//vm_load("test");

	vm_reg[0] = 111;
	vm_reg[1] = 222;
	vm_reg[2] = 333;

	//Майн програм
	// out $0, %0
	vm_set(0, 0, 34);
	vm_set(0, 1, 0);
	vm_set(0, 2, 0);

	// int %0
	vm_set(0, 3, 2);
	vm_set(0, 4, 0);

	// out $2, %0
	vm_set(0, 5, 34);
	vm_set(0, 6, 2);
	vm_set(0, 7, 0);

	// hlt
	vm_set(0, 8, 1);

	//Обработчик прерывания

	vm_set(0, 10, 34);
	vm_set(0, 11, 1);
	vm_set(0, 12, 0);

	// ret
	vm_set(0, 13, 32);


	//Таблиаца прерываний. Обработчик прерывания 0 по адресу 10
	vm_set(0, 100, 0);
	vm_set(0, 101, 10);

	vm_exec_loop();

	return 0;
}

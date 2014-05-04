#include <stdio.h>
#include <stdlib.h>

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
#define uint16_t unsigned short
#define uint8_t unsigned char

/*
 * Настройки ВМ
 */
#define REG_COUNT   16
#define MEM_SIZE    65536
#define PORTS_CNT   64

struct {
	uint16_t base;
	uint16_t limit;
	enum {SEG_CODE, SEG_DATA, SEG_STACK} type;
	enum {SEG_READ_WRITE, SEG_READ_ONLY} ro;
	uint8_t access;	// Access level
} vm_seg_regs[4];

#define ITERRUPTS_MAX	128		// максимальное кол-во прерываний
#define ITERRUPTS_SEG	3		// сегмент со смещение по которому хранится таблица прерываний

struct {
	uint16_t addr[ITERRUPTS_MAX];
	uint16_t ptr;
} vm_iterrupts;

// Имена спец. регистров
#define REG_PC 0xf
#define REG_SP 0xe
#define REG_BP 0xd

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
	if (vm_access > vm_seg_regs[reg].access) { //Я обернул все сегфолты в скобки и добваил return, на будущее. Не всегда же приложению валится xD
		segfault();
		return 0;
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
	return 0; //чтоб компилятор не ругался
}

void vm_set(uint8_t seg, uint16_t dest, uint16_t val) {
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
	if (f == NULL) return 1; //ошибка
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	fseek(f, 0, SEEK_SET);
	fread(vm_mem, 1, size, f);
	return 0;
}

uint8_t vm_iterrupt_add(uint8_t number) {
	if (vm_iterrupts.ptr < ITERRUPTS_MAX) {
		vm_iterrupts.addr[vm_iterrupts.ptr++] = number;
	} else {
		// Печаль беда
		// А если серьезно, то надо с этим что то деалать. Но я хз, что лучше сделать в таком случае
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

void vm_cmd_sub(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] -= vm_reg[rgb];
}

void vm_cmd_mul(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] *= vm_reg[rgb];
}

void vm_cmd_div(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] /= vm_reg[rgb];
}

void vm_cmd_mod(uint8_t args[]) {
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
 * Логические операции
 */

void vm_cmd_or(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] |= vm_reg[rgb];
}

void vm_cmd_and(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] &= vm_reg[rgb];
}

void vm_cmd_xor(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] ^= vm_reg[rgb];
}

void vm_cmd_not(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rga] = ~vm_reg[rgb];
}

/*
 * Команды условного перехода
 */

void vm_cmd_jeq(uint8_t args[]) {
	uint8_t seg, rga, rgb;
	seg = args[0] & 0xf;
	rga = args[1] >> 4;
	rgb = args[1] & 0xf;
	uint16_t wrd;
	wrd = (args[2] << 8) | args[3];
	if (vm_reg[rga] == vm_reg[rgb]) {
		vm_reg[REG_PC] = vm_translate_addr(seg, wrd);
	}
}

void vm_cmd_jne(uint8_t args[]) {
	uint8_t rga, rgb, seg;
	seg = args[0] & 0xf;
	rga = args[1] >> 4;
	rgb = args[1] & 0xf;
	uint16_t wrd;
	wrd = (args[2] << 8) | args[3];
	if (vm_reg[rga] != vm_reg[rgb]) {
		vm_reg[REG_PC] = vm_translate_addr(seg,wrd);
	}
}

void vm_cmd_jlt(uint8_t args[]) {
	uint8_t rga, rgb;
	uint8_t seg = args[0] >> 4;
	seg = args[0] & 0xf;
	rga = args[1] >> 4;
	rgb = args[1] & 0xf;
	uint16_t wrd;
	wrd = (args[2] << 8) | args[3];
	if (vm_reg[rga] < vm_reg[rgb]) {
		vm_reg[REG_PC] = vm_translate_addr(seg,wrd);
	}
}

void vm_cmd_jgt(uint8_t args[]) {
	uint8_t rga, rgb, seg;
	seg = args[0] & 0xf;
	rga = args[1] >> 4;
	rgb = args[1] & 0xf;
	uint16_t wrd;
	wrd = (args[2] << 8) | args[3];
	if (vm_reg[rga] > vm_reg[rgb]) {
		vm_reg[REG_PC] = vm_translate_addr(seg,wrd);
	}
}

void vm_cmd_jle(uint8_t args[]) {
	uint8_t rga, rgb, seg;
	seg = args[0] & 0xf;
	rga = args[1] >> 4;
	rgb = args[1] & 0xf;
	uint16_t wrd;
	wrd = (args[2] << 8) | args[3];
	if (vm_reg[rga] <= vm_reg[rgb]) {
		vm_reg[REG_PC] = vm_translate_addr(seg,wrd);
	}
}

void vm_cmd_jge(uint8_t args[]) {
	uint8_t rga, rgb, seg;
	seg = args[0] & 0xf;
	rga = args[1] >> 4;
	rgb = args[1] & 0xf;
	uint16_t wrd;
	wrd = (args[2] << 8) | args[3];
	if (vm_reg[rga] >= vm_reg[rgb]) {
		vm_reg[REG_PC] = vm_translate_addr(seg,wrd);
	}
}

/*
 * Команды безусловного перехода
 */

void vm_cmd_jmp(uint8_t args[]) {
	uint16_t seg,wrd;
	seg = args[0] & 0xf;
	wrd = (args[1] << 8) | args[2];
	vm_reg[REG_PC] = vm_translate_addr(seg,wrd);
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
			scanf("%u", &vm_reg[reg]);
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
	uint8_t seg;
	seg = args[0] & 0xf;
	vm_set(seg,(vm_reg[REG_SP]-=2),vm_reg[args[1]]);
	
}
 
void vm_cmd_pushv(uint8_t args[]) {
	uint16_t seg,wrd;
	seg = args[0] & 0xf;
	wrd = (args[1] << 8) | args[2];
	vm_set(seg,(vm_reg[REG_SP]-=2),wrd);
}

void vm_cmd_pop(uint8_t args[]) {
	uint16_t seg;
	seg = args[0];
	vm_reg[args[1]] = vm_get(seg, vm_reg[REG_SP]);
	vm_reg[REG_SP] += 2;
}

/*
 * Все команды ВМ и кол-во байт-аргументов
 */

#define CMD_COUNT 29

struct {
	void (*func)();
	uint8_t argc;
} vm_cmd[CMD_COUNT] = {
	{vm_cmd_nop,  0}, //0
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
	{vm_cmd_out , 2}, //16
	{vm_cmd_shl,  1}, //17
	{vm_cmd_shr,  1}, //18
	{vm_cmd_or,   1}, //19
	{vm_cmd_and,  1}, //20
	{vm_cmd_xor,  1}, //21
	{vm_cmd_not,  1}, //22
	{vm_cmd_jmp,  3}, //23
	{vm_cmd_jpr,  1}, //24
	{vm_cmd_pushr,2}, //25
	{vm_cmd_pushv,3}, //26
	{vm_cmd_pop,  2}, //27
	{vm_cmd_in,   2}, //28
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
		printf("Invalid comand!\n");
		return;
	}
	uint8_t i, bytes[4];
	for(i = 0; i < vm_cmd[cmd].argc; i++) {
		bytes[i] = vm_get(seg, vm_reg[REG_PC]++);
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

	//vm_reg[0] = 'L';
	//vm_reg[2] = 'L';
	//vm_reg[3] = 15;
	vm_load("test");
	/*
	vm_set(0,0,16);
	vm_set(0,1,0);
	vm_set(0,2,1);
	
	vm_set(0,3,26);
	vm_set(0,4,2);
	vm_set(0,5,0);
	vm_set(0,6,'O');
	
	vm_set(0,7,27);
	vm_set(0,8,2);
	vm_set(0,9,1);

	vm_set(0,10,16);
	vm_set(0,11,1);
	vm_set(0,12,1);

	vm_set(0,13,16);
	vm_set(0,14,2);
	vm_set(0,15,1);

	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);

	vm_reg[0] = 15;
	vm_set(0,16,16);
	vm_set(0,17,3);
	vm_set(0,18,0);
	*/

	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);
	vm_exec_comand(0);

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/queue.h>

#ifdef __unix__
#include "getch.c"
#else
#include <conio.h>
#endif

#define BIT_SET(v, n) (v = ((1 << n) | v))
#define BIT_RST(v, n) (v = ((1 << n) ^ v) & v)
#define BIT_GET(v, n) ((v >> n) & 0x1)

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
 * Виртуальная машина
 */

/*
 * Сегменты
 */

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
	uint8_t access;
} vm_seg_regs[4];

/*
 * Прерывания
 */

#define INTERRUPTS_MAX	128

struct {
	uint16_t tbl;
	uint16_t num[INTERRUPTS_MAX];
	uint8_t  ptr;
} vm_interrupts;

/*
 * Регистры
 */

#define REG_COUNT   16

#define REG_PC	0xf
#define REG_SP	0xe
#define REG_BP	0xd
#define REG_FL	0xc

// Биты регистра флагов
#define REG_FL_BIT_INT 0x0	// Запрет прерываний
#define REG_FL_BIT_OVR 0x1	// Переполнение чисел
#define REG_FL_BIT_CRR 0x2	// Перенос в знаковый разряд

uint16_t vm_reg[REG_COUNT];

/*
 * Память
 */

#define MEM_SIZE    65536

uint8_t  vm_mem[MEM_SIZE];
uint8_t  vm_access;

/*
 * Ввод/вывод
 */

#define PORTS_CNT   64
#define BLOCK_SIZE	512

TAILQ_HEAD(fifo, fifo_entry) dev_hdd_fifo_head;

struct fifo_entry {
	TAILQ_ENTRY(fifo_entry) entries;
	uint16_t sec;
	uint8_t dev;
	uint8_t data[BLOCK_SIZE];
} *n1, *np;

enum {
	BUSY,
	AVAILABLE
} dev_hdd_state;

uint16_t vm_pio[PORTS_CNT];
FILE *dev_hdd[4];
uint8_t cur_dev;
uint8_t cur_sec;

/*
 * Внутренние функции ВМ
 */


void segfault() {
	printf("Surprise!\n");
	int* ptr = (int*)0;
	*ptr = 1;
}


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


uint8_t vm_interrupt_add(uint8_t number) {
	if (vm_interrupts.ptr < INTERRUPTS_MAX) {
		vm_interrupts.num[vm_interrupts.ptr++] = number;
		return 1;
	} else {
		// Печаль беда
		// А если серьезно, то надо с этим что то деалать. Но я хз, что лучше сделать в таком случае
		return 0;
	}
}

uint8_t vm_interrupt_get() {
	uint8_t number = vm_interrupts.num[0];
	if (vm_interrupts.ptr > 0) {
		uint8_t i;
		for(i = 0; i < vm_interrupts.ptr; i++)
			vm_interrupts.num[i] = vm_interrupts.num[i + 1];
		--vm_interrupts.ptr;
	}
	return number;
}

uint8_t vm_interrupt_exec() {
	if (vm_interrupts.ptr > 0 && !BIT_GET(vm_reg[REG_FL], REG_FL_BIT_INT)) {
		uint8_t num = vm_interrupt_get() * 2;
		uint16_t adr, ret;
		ret  = vm_reg[REG_PC];
		adr  = vm_mem[vm_interrupts.tbl + num + 0] << 8;
		adr |= vm_mem[vm_interrupts.tbl + num + 1];

		vm_set(SS, vm_reg[REG_SP]--, ret >> 8);
		vm_set(SS, vm_reg[REG_SP]--, ret & 0xff);
		vm_reg[REG_PC] = adr;
	}
	return 0;
}

void print_inters() {
	uint8_t i;
	for(i = 0; i < vm_interrupts.ptr; i++)
		printf("%d ", vm_interrupts.num[i]);
	printf("\n");
}

/*
 * Команды ВМ
 */

void vm_cmd_nop(uint8_t args[]) {
	//No oPeration
}

void vm_cmd_hlt(uint8_t args[]) {
	exit(0);
}

/*
 * Прерывания
 */

void vm_cmd_int(uint8_t args[]) {
	uint8_t num;
	num = args[0] & 0xff; //Максимум - 256 прерываний
	vm_interrupt_add(num);
}

void vm_cmd_lit(uint8_t args[]) {
	uint16_t wrd;
	wrd = (args[0] << 8) | args[1];
	vm_interrupts.tbl = wrd;
}

void vm_cmd_cli(uint8_t args[]) {
	BIT_SET(vm_reg[REG_FL], REG_FL_BIT_INT);
}

void vm_cmd_sti(uint8_t args[]) {
	BIT_RST(vm_reg[REG_FL], REG_FL_BIT_INT);
}


/*
 * Работа с памятью
 */

void vm_cmd_cpy(uint8_t args[]) {
	uint8_t src, dst;
	src = args[0] >> 4;
	dst = args[0] & 0xf;
	vm_reg[dst] = vm_reg[src];
}

void vm_cmd_ldw(uint8_t args[]) {
	uint8_t  rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	uint8_t wl, wh;
	wl = vm_get(DS, vm_reg[rga]);
	wh = vm_get(DS, vm_reg[rga] + 1);
	vm_reg[rgb] = wl << 8 | wh;
}

void vm_cmd_ldb(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rgb] = vm_get(DS, vm_reg[rga]);
}

void vm_cmd_llb(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rgb] &= 0xff00;
	vm_reg[rgb] |= vm_get(DS, vm_reg[rga]);
}

void vm_cmd_lhb(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_reg[rgb] &= 0x00ff;
	vm_reg[rgb] |= vm_get(DS, vm_reg[rga]) << 8;
}

void vm_cmd_ldwi(uint8_t args[]) {
	uint8_t rgb;
	uint16_t wrd;
	rgb = args[0] & 0xf;
	wrd = args[1] << 8 | args[2];
	uint8_t wl, wh;
	wl = vm_get(DS, wrd);
	wh = vm_get(DS, wrd + 1);
	vm_reg[rgb] = wl << 8 | wh;
}

void vm_cmd_ldbi(uint8_t args[]) {
	uint8_t rgb;
	uint16_t wrd;
	rgb = args[0] & 0xf;
	wrd = args[1] << 8 | args[2];
	vm_reg[rgb] = vm_get(DS, wrd);
}

void vm_cmd_llbi(uint8_t args[]) {
	uint8_t rgb;
	uint16_t wrd;
	rgb = args[0] & 0xf;
	wrd = args[1] << 8 | args[2];
	vm_reg[rgb] &= 0xff00;
	vm_reg[rgb] |= vm_get(DS, wrd);
}

void vm_cmd_lhbi(uint8_t args[]) {
	uint8_t rgb;
	uint16_t wrd;
	rgb = args[0] & 0xf;
	wrd = args[1] << 8 | args[2];
	vm_reg[rgb] &= 0x00ff;
	vm_reg[rgb] |= vm_get(DS, wrd) << 8;
}

void vm_cmd_stw(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_set(DS, vm_reg[rgb], vm_reg[rga] >> 8);
	vm_set(DS, vm_reg[rgb] + 1, vm_reg[rga] & 0xff);
}

void vm_cmd_slb(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_set(DS, vm_reg[rgb], vm_reg[rga] & 0xff);
}

void vm_cmd_shb(uint8_t args[]) {
	uint8_t rga, rgb;
	rga = args[0] >> 4;
	rgb = args[0] & 0xf;
	vm_set(DS, vm_reg[rgb], vm_reg[rga] >> 8);
}

void vm_cmd_stwi(uint8_t args[]) {
	uint8_t rga;
	uint16_t wrd;
	rga = args[0] >> 4;
	wrd = args[1] << 8 | args[2];
	vm_set(DS, wrd, vm_reg[rga] >> 8);
	vm_set(DS, wrd + 1, vm_reg[rga] & 0xff);
}

void vm_cmd_slbi(uint8_t args[]) {
	uint8_t rga;
	uint16_t wrd;
	rga = args[0] >> 4;
	wrd = args[1] << 8 | args[2];
	vm_set(DS, wrd, vm_reg[rga] & 0xff);
}

void vm_cmd_shbi(uint8_t args[]) {
	uint8_t rga;
	uint16_t wrd;
	rga = args[0] >> 4;
	wrd = args[1] << 8 | args[2];
	vm_set(DS, wrd, vm_reg[rga] >> 8);
}

void vm_cmd_ldi(uint8_t args[]) {
	uint8_t rgb;
	uint16_t wrd;
	rgb = args[0] & 0xf;
	wrd = args[1] << 8 | args[2];
	vm_reg[rgb] = wrd;
}

void vm_cmd_lbi(uint8_t args[]) {
	uint8_t byt, rgb;
	rgb = args[0] & 0xf;
	byt = args[1];
	vm_reg[rgb] = byt;
}

void vm_cmd_lli(uint8_t args[]) {
	uint8_t byt, rgb;
	rgb = args[0] & 0xf;
	byt = args[1];
	vm_reg[rgb] &= 0xff00;
	vm_reg[rgb] |= byt;
}

void vm_cmd_lhi(uint8_t args[]) {
	uint8_t byt, rgb;
	rgb = args[0] & 0xf;
	byt = args[1];
	vm_reg[rgb] &= 0x00ff;
	vm_reg[rgb] |= byt << 8;
}

/*
 * Арифметические операции
 */

void vm_cmd_inc(uint8_t args[]) {
	uint8_t reg;
	reg = args[0] & 0xf;
	vm_reg[reg] += 1;
}

void vm_cmd_dec(uint8_t args[]) {
	uint8_t reg;
	reg = args[0] & 0xf;
	vm_reg[reg] -= 1;
}

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
 * Работа с жестким диском
 */

void dev_hdd_read(uint8_t dev, uint16_t num, uint16_t addr) {
	for (np = dev_hdd_fifo_head.tqh_first; np != NULL; np = np->entries.tqe_next) {
		fseek(dev_hdd[np->dev], (np->sec) * BLOCK_SIZE, SEEK_SET);
		fwrite(np->data, BLOCK_SIZE, 1, dev_hdd[dev]);
	}
	while (dev_hdd_fifo_head.tqh_first!=NULL)
		TAILQ_REMOVE(&dev_hdd_fifo_head, dev_hdd_fifo_head.tqh_first, entries);
	fseek(dev_hdd[dev], num * BLOCK_SIZE, SEEK_SET);
	fread(&vm_mem[addr], BLOCK_SIZE, 1, dev_hdd[dev]);
}

void dev_hdd_write(uint8_t dev, uint16_t num, uint16_t addr) {
	n1 = malloc(sizeof(struct fifo_entry));
	TAILQ_INSERT_TAIL(&dev_hdd_fifo_head, n1, entries);
	n1->dev = dev;
	n1->sec = num;
	memcpy(n1->data, &vm_mem[addr], BLOCK_SIZE);
}

void dev_hdd_real_write(uint8_t dev, uint16_t num, uint16_t addr) {
	fseek(dev_hdd[dev], num * BLOCK_SIZE, SEEK_SET);
	fwrite(&vm_mem[addr], BLOCK_SIZE, 1, dev_hdd[dev]);
	//if (ferror(dev_hdd[dev]))  perror("Error");
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
		} break;
		case 1: {
			putchar(vm_reg[reg]);
		} break;
		case 2: {
			cur_dev = vm_reg[reg];
		} break;
		case 3: {
			cur_sec = vm_reg[reg];
		} break;
		case 4: {
			printf("\n%d %d %d\n", cur_dev, cur_sec, vm_mem[vm_reg[reg]]);
			dev_hdd_write(cur_dev, cur_sec, vm_reg[reg]);
		}
		case 5: {
			printf("\n%d %d %d\n", cur_dev, cur_sec, vm_mem[vm_reg[reg]]);
			dev_hdd_real_write(cur_dev, cur_sec, vm_reg[reg]);
		} break;
	}
}

void vm_cmd_in(uint8_t args[]) {
	uint8_t reg, prt;
	prt = args[0] & 0xf;
	reg = args[1];
	switch (prt) {
		case 0: {
			scanf("%hu", &vm_reg[reg]);
		} break;
		case 1: {
			vm_reg[reg] = getchar();
		}
		case 2: {
			vm_reg[reg] = errno;
		} break;
		case 3: {
			dev_hdd_read(cur_dev, cur_sec, vm_reg[reg]);
		} break;
	}
}

/*
 * Работа со стеком
 */

void vm_cmd_pushr(uint8_t args[]) {
	uint8_t reg;
	reg = vm_reg[args[0] & 0xf];
	vm_set(SS, vm_reg[REG_SP]--, reg >> 8);
	vm_set(SS, vm_reg[REG_SP]--, reg & 0xff);

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

#define CMD_COUNT 54

struct {
	void (*func)();
	uint8_t argc;
} vm_cmd[CMD_COUNT] = {
	{vm_cmd_nop,  0}, //0
	{vm_cmd_hlt,  0}, //1
	// Interrupts
	{vm_cmd_int,  1}, //2
	{vm_cmd_cli,  0}, //3
	{vm_cmd_sti,  0}, //4
	{vm_cmd_lit,  2}, //5
	// Copying and moving data
	{vm_cmd_cpy,  1}, //6
	{vm_cmd_ldw,  1}, //7
	{vm_cmd_ldb,  1}, //8
	{vm_cmd_llb,  1}, //9
	{vm_cmd_lhb,  1}, //10
	{vm_cmd_ldwi, 3}, //11
	{vm_cmd_ldbi, 3}, //12
	{vm_cmd_llbi, 3}, //13
	{vm_cmd_lhbi, 3}, //14
	{vm_cmd_stw,  1}, //15
	{vm_cmd_slb,  1}, //16
	{vm_cmd_shb,  1}, //17
	{vm_cmd_stwi, 3}, //18
	{vm_cmd_slbi, 3}, //19
	{vm_cmd_shbi, 3}, //20
	{vm_cmd_ldi,  3}, //21
	{vm_cmd_lbi,  2}, //22
	{vm_cmd_lli,  2}, //23
	{vm_cmd_lhi,  2}, //24
	// Arithmetic operations
	{vm_cmd_inc,  1}, //25
	{vm_cmd_dec,  1}, //26
	{vm_cmd_add,  1}, //27
	{vm_cmd_sub,  1}, //28
	{vm_cmd_mul,  1}, //29
	{vm_cmd_div,  1}, //30
	{vm_cmd_mod,  1}, //31
	{vm_cmd_shl,  1}, //32
	{vm_cmd_shr,  1}, //33
	// Logic operations
	{vm_cmd_or,   1}, //34
	{vm_cmd_and,  1}, //35
	{vm_cmd_xor,  1}, //36
	{vm_cmd_not,  1}, //37
	// branch instructions
	// Conditional
	{vm_cmd_jeq,  3}, //38
	{vm_cmd_jne,  3}, //39
	{vm_cmd_jlt,  3}, //40
	{vm_cmd_jgt,  3}, //41
	{vm_cmd_jle,  3}, //42
	{vm_cmd_jge,  3}, //43
	// Uncoditional
	{vm_cmd_jmp,  2}, //44
	{vm_cmd_jpr,  1}, //45
	// Subroutines
	{vm_cmd_callr,1}, //46
	{vm_cmd_callv,2}, //47
	{vm_cmd_pushr,1}, //48
	{vm_cmd_pushv,2}, //49
	{vm_cmd_pop,  1}, //50
	{vm_cmd_ret,  0}, //51
	// Ports
	{vm_cmd_in,   2}, //52
	{vm_cmd_out,  2}  //53
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
	vm_seg_regs[0].limit=2000;
	vm_seg_regs[0].access=0;
	vm_seg_regs[0].ro=SEG_READ_WRITE;
	vm_seg_regs[0].type=SEG_CODE;

	vm_seg_regs[1].base=2000;
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

	TAILQ_INIT(&dev_hdd_fifo_head);

	dev_hdd[0]=fopen("hdd","rb+");

	//memcpy(vm_mem,dev_hdd_read(0,0),BLOCK_SIZE);

	vm_load("test");

	/*vm_reg[0] = 111;*/
	/*vm_reg[1] = 222;*/
	/*vm_reg[2] = 333;*/
	/*vm_reg[3] = 444;*/

	/*//Майн програм*/
	/*// out %0, $0*/
	/*vm_set(0, 0, 34);*/
	/*vm_set(0, 1, 0);*/
	/*vm_set(0, 2, 0);*/

	/*// int $0*/
	/*vm_set(0, 3, 2);*/
	/*vm_set(0, 4, 0);*/

	/*// int $1*/
	/*//vm_set(0, 5, 2);*/
	/*//vm_set(0, 6, 1);*/

	/*// out %3, $0*/
	/*vm_set(0, 7, 34);*/
	/*vm_set(0, 8, 3);*/
	/*vm_set(0, 9, 0);*/

	/*// hlt*/
	/*vm_set(0,10, 1);*/

	/*// inter 1: out %1, $0*/
	/*vm_set(0, 20, 34);*/
	/*vm_set(0, 21, 1);*/
	/*vm_set(0, 22, 0);*/
	/*vm_set(0, 23, 32);*/

	/*// inter 2: out %2, $0*/
	/*vm_set(0, 30, 34);*/
	/*vm_set(0, 31, 1);*/
	/*vm_set(0, 32, 0);*/
	/*vm_set(0, 33, 32);*/

	/*//Таблиаца прерываний. Обработчик прерывания 0 по адресу 10*/
	/*vm_set(0, 100, 0);*/
	/*vm_set(0, 101, 30);*/

	/*vm_set(0, 100, 0);*/
	/*vm_set(0, 101, 30);*/

	vm_exec_loop();
	fclose(dev_hdd[0]);

	return 0;
}

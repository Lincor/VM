_start:
	ldb %a, $0x14
	ldb %b, $4(%AC,%SP,$4)
	nop
	add %a, %b # %a = 0x14 + 0x45
	add $_start, %2

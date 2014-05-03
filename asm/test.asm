_start:
	ldb %a, $0x14
	ldb %b, $4(%AC,%SP,$4)
	add %a, %b # %a = 0x14 + 0x45

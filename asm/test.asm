_start:
	ldb %a, $0x14
	ldb %b, $0x45
	nop
	add %a, %b # %a = 0x14 + 0x45
	jmp $4:$_start

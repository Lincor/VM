	ldi $0cH, %0
	slbi %0, $0
	ldi $0ce, %0
	slbi %0, $1
	ldi $0cl, %0
	slbi %0, $2
	ldi $0cl, %0
	slbi %0, $3
	ldi $0co, %0
	slbi %0, $4
	ldi $0c\c, %0
	slbi %0, $5
	ldi $0c\s, %0
	slbi %0, $6
	ldi $0cw, %0
	slbi %0, $7
	ldi $0co, %0
	slbi %0, $8
	ldi $0cr, %0
	slbi %0, $9
	ldi $0cl, %0
	slbi %0, $10
	ldi $0cd, %0
	slbi %0, $11
	ldi $0c!, %0
	slbi %0, $12
	ldi $0c\n, %0
	slbi %0, $13
	ldi $0c\0, %0
	slbi %0, $14


	ldi $0, %1
	ldi $0, %a
loop:
	ldb %a, %0
	jeq %0, %1, $end
	
	out %0, $1
	inc %a

	jmp $loop
end:
	hlt

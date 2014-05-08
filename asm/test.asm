	ldb %b, $0c\s
	ldb %0, $0  # first number
	ldb %1, $1  # second number
	ldb %a, $24 # amount of iters

	out %0, $0
	out %b, $1
loop:
	out %1, $0
	out %b, $1

	cpy %0, %3
	cpy %1, %0
	add %3, %1
	
	ldb %3, $1
	sub %3, %a

	ldb %3, $0

	jne %3, %a, $0:$loop

	ldb %3, $0c\n
	out %3, $1

	hlt

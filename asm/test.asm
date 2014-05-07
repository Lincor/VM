	ldw %2, $32 # space
	ldw %0, $1
	ldw %1, $1
	ldw %a, $20 # amount of numbers

loop:
	out %1, $0
	out %2, $1

	ldw %3, $0
	add %0, %3

	ldw %0, $0
	add %1, %0

	add %3, %1
	
	ldw %3, $1
	sub %3, %a

	ldw %3, $0

	jne %3, %a, $0:$loop

	ldw %0, $10

	out %0, $1

	hlt

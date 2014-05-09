	pushv $0
	pushv $0c\n
	pushv $0c!
	pushv $0cd
	pushv $0cl
	pushv $0cr
	pushv $0co
	pushv $0cw
	pushv $0c\s
	pushv $0c\c
	pushv $0co
	pushv $0cl
	pushv $0cl
	pushv $0ce
	pushv $0cH

	ldb %a, $0
	pop %0

loop:
	out %0, $1
	pop %0
	jne %a, %0, $loop

	hlt

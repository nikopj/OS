.data
	str: .ascii "COOL\n"
.text
.global _start
_start:
	movq $1,   %rax	 # 1 -> sys_write
	movq $1,   %rdi  # 1 -> stdout_fileno 
	movq $str, %rsi 
	movq $5,   %rdx  # 5 -> num chars 
	syscall

	movq $60,  %rax	 # 60-> sys_exit
	movq $2,   %rdi  # 2 -> exit with code 2
	syscall

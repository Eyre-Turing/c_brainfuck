.section .data
	SYS_read: .quad 0
	SYS_write: .quad 1
	SYS_mmap: .quad 9
	SYS_munmap: .quad 11
	SYS_exit: .quad 60
	MEM_SIZE_STEP: .quad 4096
	PORT_READ_WRITE: .quad 0x3
	MAP_PRIVATE_ANON: .quad 0x22

.section .bss
	mem_addr: .space 8
	mem_ptr: .space 8
	mem_size: .space 8

.section .text
.global _start

# 把调用mmap封装一下，在需要在rsi传入需要的内存长度，会在rax返回内存地址
# 如果申请失败，自动exit程序
do_mmap:
	mov SYS_mmap, %rax
	mov $0, %rdi
	# mov MEM_SIZE_STEP, %rsi
	mov PORT_READ_WRITE, %rdx
	mov MAP_PRIVATE_ANON, %r10
	mov $-1, %r8
	mov $0, %r9
	syscall

	cmp $-1, %rax
	je .mmap_error

	ret

.mmap_error:
	mov SYS_exit, %rax
	mov $1, %rdi
	syscall

# 把调用munmap封装一下，需要在rdi传入内存地址，在rsi传入内存长度
# 如果失败。什么？失败就失败，管它干锤。
# 如果是内存申请失败我还要处理。内存释放失败，你看我屌不吊它，我屌都不屌它一下
do_munmap:
	mov SYS_munmap, %rax
	# mov $0, %rdi
	# mov MEM_SIZE_STEP, %rsi
	syscall
	ret

# 把内存内容全部设置为0，需要在rdi传入内存地址，在rsi传入内存长度
do_set_mem_zero:
	mov $0, %r14

.set_mem_zero_loop:
	movb $0, (%rdi, %r14, 1)

	inc %r14
	cmp %rsi, %r14
	jl .set_mem_zero_loop

	ret

# 把rsi的内存地址里的内容复制到rdi的内存地址里，长度为rdx
do_memcpy:
	mov $0, %r14

.memcpy_loop:
	movb (%rsi, %r14, 1), %al
	movb %al, (%rdi, %r14, 1)

	inc %r14
	cmp %rdx, %r14
	jl .memcpy_loop

	ret

# 这里初始化bf内存，即mem_addr, mem_ptr, mem_size初始化
# 使用mmap申请内存到mem_addr，并且把内存长度赋值给mem_size，把内存长度除以2赋值给mem_ptr
do_init:
	mov MEM_SIZE_STEP, %rsi
	call do_mmap

	mov %rax, mem_addr
	mov MEM_SIZE_STEP, %rax
	mov %rax, mem_ptr
	sarq $1, mem_ptr
	mov %rax, mem_size

	mov mem_addr, %rdi
	mov mem_size, %rsi
	call do_set_mem_zero

	ret

# 这里销毁bf内存
do_destroy:
	mov mem_addr, %rdi
	mov mem_size, %rsi
	call do_munmap
	ret

# 这里确认mem_ptr是否越界，如果越界则自动扩展mem_addr和mem_size
do_check_expand:
.check_expand_loop:
	# 第一种越界的情况：mem_ptr < 0
	cmpq $0, mem_ptr
	jl .expand

	# 第二种越界的情况：mem_ptr >= mem_size
	mov mem_size, %rax
	cmpq %rax, mem_ptr
	jge .expand

	ret

.expand:
	mov mem_size, %rsi
	sal $1, %rsi
	call do_mmap
	mov %rax, %r12

	mov %r12, %rdi
	call do_set_mem_zero

	mov mem_size, %r13
	sar $1, %r13

	mov %r12, %rdi
	add %r13, %rdi
	mov mem_addr, %rsi
	mov mem_size, %rdx
	call do_memcpy

	mov mem_addr, %rdi
	mov mem_size, %rsi
	call do_munmap

	mov %r12, mem_addr
	addq %r13, mem_ptr
	salq $1, mem_size

	jmp .check_expand_loop

# +
do_bf_add:
	call do_check_expand
	mov mem_addr, %r12
	mov mem_ptr, %r13
	addb $1, (%r12, %r13, 1)
	ret

# -
do_bf_sub:
	call do_check_expand
	mov mem_addr, %r12
	mov mem_ptr, %r13
	subb $1, (%r12, %r13, 1)
	ret

# >
do_bf_move:
	addq $1, mem_ptr
	ret

# <
do_bf_back:
	subq $1, mem_ptr
	ret

# ,
do_bf_input:
	call do_check_expand
	mov SYS_read, %rax
	mov $0, %rdi
	mov mem_addr, %rsi
	add mem_ptr, %rsi
	mov $1, %rdx
	syscall

	cmp $0, %rax
	jle .input_pipe_broke

	ret

.input_pipe_broke:
	movb $0, (%rsi)
	ret

# .
do_bf_output:
	call do_check_expand
	mov SYS_write, %rax
	mov $1, %rdi
	mov mem_addr, %rsi
	add mem_ptr, %rsi
	mov $1, %rdx
	syscall

	ret

_start:
	call do_init

# here insert your code

.exit:
	call do_destroy
	mov SYS_exit, %rax
	mov $0, %rdi
	syscall

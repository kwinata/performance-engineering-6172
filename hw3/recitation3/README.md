# Write-up

## FAQ

### (1) What is `.p2align` in asm?

From [stackoverflow](https://stackoverflow.com/questions/21546946/what-does-p2align-do-in-asm-code):

> The p2 part of the directive name came from gas being possibly the original implementation of the recommendation for Intel P-II CPU to provide conditional alignment of loop body code. As Agner Fog explains, the original purpose was to ensure that the first instruction fetch gets sufficient code to begin decoding.

Can reger to "Agner Fog" explanation [here](https://www.agner.org/optimize/)

### (2) What is `.loc` in asm?

From [stackoverflow](https://stackoverflow.com/questions/24787769/what-are-lfb-lbb-lbe-lvl-loc-in-the-compiler-generated-assembly-code):

> The .loc directive is used to indicate the corresponding line of source code. It indicates the file number, line number and column number of the corresponding source code.

## Recitation

- [x] Write-up 1
- [ ] Write-up 2
- [ ] Write-up 3
- [ ] Write-up 4
- [ ] Write-up 5

### Write-up 1

> Write-up 1: Look at the assembly code above. The compiler has translated the code to set the start index at −2**16 and adds to it for each memory access. Why doesn’t it set the start index to 0 and use small positive offsets?

**Refering to PDF**

This should refer to this part (from the PDF)

``` asm
movq $-65536, %rax
```

`%rax` seems to be the counter.

It's used later as an offset:

``` asm
movdqu 65536(%rsi, %rax), %xmm0
```

This should be because it saves CPU cycle from needing to have a explicit compare. The loop comparison can be combined together with the counter increment, as such:

``` asm
addq $64, %rax
jne .LBB0_3
```

`addq` will set the zero flag while at the same time increments `%rax`. `jne` means jump if not jero, which will repeat the for loop.

**Referring to my code**

However, this is not the case in my compiled code. It does seem to use `%rax` for the loop, but it initially set it as zero, then use compare later:

``` asm
# set eax to zero. eax and rax refers to the same register, just different bit name
xorl	%eax, %eax
...
# use rax as offset to move stuffs. rsi is the base pointer to one of the array
movdqu	(%rsi,%rax), %xmm0
...
# increment rax
addq	$64, %rax
# compare with 2^16
cmpq	$65536, %rax            # imm = 0x10000
# jump if not equal
jne	.LBB0_3
```

The reason I'm not really sure of. Maybe the compiler thinks it's not worth it?

### Write-up 2

> This code is still not aligned when using AVX2 registers. Fix the code to make sure it uses aligned moves for the best performance.

For this we can simply check the offset between each mov instruction:

``` asm
	vmovdqu	(%rdi,%rax), %ymm0
	vmovdqu	32(%rdi,%rax), %ymm1
	vmovdqu	64(%rdi,%rax), %ymm2
	vmovdqu	96(%rdi,%rax), %ymm3
```

In my case it's 32, but the code is still only aligned by 16. After fixing it, we can get it aligned

``` asm
	vmovdqa	(%rdi,%rax), %ymm0
	vmovdqa	32(%rdi,%rax), %ymm1
	vmovdqa	64(%rdi,%rax), %ymm2
	vmovdqa	96(%rdi,%rax), %ymm3
```

The reason for why it's 32, should be because AVX instructions can use 256-bit YMM vector register. 32*8 = 256 so it checks out.

### Understanding example2 asm

``` asm

# empty eax / rax (should be the for loop counter)
xorl %eax, %eax

# skip .LBB0_33, go to .LBB0_1 directly
jmp	.LBB0_1
|                   # indented to add graphings
|                   .LBB0_33:
|                       addq	$16, %rax
|                       cmpq	$65536, %rax
|                       # if rax == 655356: jump to .LBB0_34 (end of loop)
|                       # otherwise continue the loop body
|                       je	.LBB0_34
|                       /
|                      /
V                     V
# TODO: explain .LBB0_1
.LBB0_1:                                # =>This Inner Loop Header: Depth=1
	#DEBUG_VALUE: test:i <- 0
	#DEBUG_VALUE: test:b <- $rsi
	#DEBUG_VALUE: test:a <- $rdi
	.loc	1 17 9                  # example2.c:17:9
	movdqa	(%rsi,%rax), %xmm0
	movdqa	(%rdi,%rax), %xmm1
	.loc	1 17 14 is_stmt 0       # example2.c:17:14
	pminub	%xmm0, %xmm1
	pcmpeqb	%xmm0, %xmm1
	movd	%xmm1, %ecx
	movl	%ecx, %edx
	notb	%dl
.Ltmp3:
	.loc	1 15 26 is_stmt 1       # example2.c:15:26
	testb	$1, %dl
	je	.LBB0_3
```
# Write-up

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

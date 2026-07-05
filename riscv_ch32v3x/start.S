.section  .init,"ax",@progbits
.global  _start
.align   1
.extern start_c
_start:
    j   handle_reset
    .word   0x1234  /* base +4 */
    .word   0x5678  /* base +8 */

.align  1
_vector_base:
.option norvc;
.section .text.handle_reset,"ax",@progbits
.align 1
handle_reset:
.option push
.option norelax
    la gp, __global_pointer$
.option pop
    la sp, _sp
    li t0, 0x1f
    csrw 0xbc0, t0
    j start_c

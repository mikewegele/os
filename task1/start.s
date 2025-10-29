    .section .text
    .global _start
_start:
    ldr sp, =0x00800000
    bl  main
1:  b   1b

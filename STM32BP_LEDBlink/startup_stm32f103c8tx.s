.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Reset_Handler
.extern SystemInit
.extern main

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .rept 14
  .word Default_Handler
  .endr
  .rept 60
  .word Default_Handler
  .endr
.size g_pfnVectors, .-g_pfnVectors

.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler, %function
Reset_Handler:
  ldr r0, =_sidata
  ldr r1, =_sdata
  ldr r2, =_edata
  cmp r1, r2
  bcs 2f
1:
  ldr r3, [r0], #4
  str r3, [r1], #4
  cmp r1, r2
  bcc 1b
2:
  ldr r1, =_sbss
  ldr r2, =_ebss
  movs r3, #0
  cmp r1, r2
  bcs 4f
3:
  str r3, [r1], #4
  cmp r1, r2
  bcc 3b
4:
  bl SystemInit
  bl main
5:
  b 5b
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.weak Default_Handler
.type Default_Handler, %function
Default_Handler:
  b Default_Handler
.size Default_Handler, .-Default_Handler

.section .note.GNU-stack,"",%progbits

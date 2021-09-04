
.global simGlobal
.equ simAbsolute, 11


.section text
ldr r0, simLocal
ldr r1, simGlobal
ldr r2, simAbsolute

ldr r3, %simLocal
ldr r4, %simGlobal
ldr r5, %simAbsolute

.section data
.skip 3
simLocal:
.word 7
simGlobal:
.word 9

.end
.global print
.global A_location
.global B_text
.global B_value
.global another

.extern PRINT_REG
.equ B_value, 5
.equ B_value_l, 4


.section text
.skip 2
ldr r4,  %B_value_l
print_l:
jmp %another
.skip 3
print:
jmp  %PRINT_REG
ldr r4,  %another_l
ldr r4, r4
ldr r4,  [r5]
ldr r4,  %print_l
push r4
pop r2



.section data
.skip 5
another:
.skip 2
another_l:
  A_location: .word 4
  B_text: .word 66, 0x20, 86, 65, 76, 85, 69, 0x3A, 0x20, 0
.end
// clang-format off
  	// 16位代码，务必加上
  	.code16
 	.text
	.global _start
	.extern loader_entry
_start:
	jmp loader_entry

	.code32
	.text
	.global protected_mode_entry
protected_mode_entry:
	jmp .
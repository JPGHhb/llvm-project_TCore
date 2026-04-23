; RUN: llc -mtriple=tcore-unknown-elf -filetype=asm %s -o - | FileCheck %s

target triple = "tcore"

define i32 @callee(i32 %x) {
; CHECK-LABEL: callee:
; CHECK: add r1, 7
; CHECK-NEXT: jmp lr
entry:
  %y = add i32 %x, 7
  ret i32 %y
}

define i32 @caller(i32 %a) {
; CHECK-LABEL: caller:
; CHECK: sub sp, 4
; CHECK-NEXT: str lr, [sp+0]
; CHECK: ldu r14, callee
; CHECK-NEXT: ldl r14, callee
; CHECK-NEXT: jmp r14
; CHECK-NEXT: ldr lr, [sp+0]
; CHECK-NEXT: add sp, 4
; CHECK-NEXT: jmp lr
entry:
  %r = call i32 @callee(i32 %a)
  ret i32 %r
}

define i32 @caller5(i32 %x) {
; CHECK-LABEL: caller5:
; CHECK: sub sp, 8
; CHECK-NEXT: str lr, [sp+4]
; CHECK: mov r0, sp
; CHECK: mov r2, 5
; CHECK-NEXT: str r2, [r0+0]
; CHECK: mov r2, 2
; CHECK-NEXT: mov r3, 3
; CHECK-NEXT: mov r4, 4
; CHECK: ldu r14, callee5
; CHECK-NEXT: ldl r14, callee5
; CHECK-NEXT: jmp r14
entry:
  %r = call i32 @callee5(i32 %x, i32 2, i32 3, i32 4, i32 5)
  ret i32 %r
}

declare i32 @callee5(i32, i32, i32, i32, i32)

define i32 @cmp_imm32_branch(i32 %x) {
; CHECK-LABEL: cmp_imm32_branch:
; CHECK: ldu r0, 4660
; CHECK-NEXT: ldl r0, 22136
; CHECK-NEXT: cmp r1, r0
; CHECK-NEXT: jne
entry:
  %cond = icmp eq i32 %x, 305419896
  br i1 %cond, label %yes, label %no

yes:
  ret i32 1

no:
  ret i32 0
}

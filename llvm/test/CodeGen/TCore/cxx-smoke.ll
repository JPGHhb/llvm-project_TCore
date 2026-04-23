; RUN: llc -mtriple=tcore-unknown-elf -filetype=asm %s -o - | FileCheck %s

target triple = "tcore-unknown-elf"

%struct.Acc = type { i32 }

$_ZN3AccC2Ei = comdat any
$_ZNK3Acc3incEv = comdat any

define i32 @entry() {
; CHECK-LABEL: entry:
; CHECK: sub sp, 12
; CHECK: str r5, [sp+8]
; CHECK: str lr, [sp+4]
; CHECK: mov r5, sp
; CHECK: mov r2, 41
; CHECK: mov r1, r5
; CHECK: ldu r14, _ZN3AccC2Ei
; CHECK: ldl r14, _ZN3AccC2Ei
; CHECK: jmp r14
; CHECK: mov r1, r5
; CHECK: ldu r14, _ZNK3Acc3incEv
; CHECK: ldl r14, _ZNK3Acc3incEv
; CHECK: jmp r14
; CHECK: ldr lr, [sp+4]
; CHECK: ldr r5, [sp+8]
; CHECK: add sp, 12
; CHECK: jmp lr
entry:
  %1 = alloca %struct.Acc, align 4
  call void @_ZN3AccC2Ei(ptr noundef nonnull align 4 dereferenceable(4) %1, i32 noundef 41)
  %2 = call noundef i32 @_ZNK3Acc3incEv(ptr noundef nonnull align 4 dereferenceable(4) %1)
  ret i32 %2
}

define linkonce_odr void @_ZN3AccC2Ei(ptr noundef nonnull align 4 dereferenceable(4) %0, i32 noundef %1) unnamed_addr comdat align 2 {
; CHECK-LABEL: _ZN3AccC2Ei:
; CHECK: sub sp, 8
; CHECK: str r1, [sp+4]
; CHECK: str r2, [sp+0]
; CHECK: str r2, [r1+0]
; CHECK: add sp, 8
; CHECK: jmp lr
entry:
  %3 = alloca ptr, align 4
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %5 = load ptr, ptr %3, align 4
  %6 = getelementptr inbounds %struct.Acc, ptr %5, i32 0, i32 0
  %7 = load i32, ptr %4, align 4
  store i32 %7, ptr %6, align 4
  ret void
}

define linkonce_odr i32 @_ZNK3Acc3incEv(ptr noundef nonnull align 4 dereferenceable(4) %0) comdat align 2 {
; CHECK-LABEL: _ZNK3Acc3incEv:
; CHECK: sub sp, 4
; CHECK: str r1, [sp+0]
; CHECK: ldr r1, [r1+0]
; CHECK: add r1, 1
; CHECK: add sp, 4
; CHECK: jmp lr
entry:
  %2 = alloca ptr, align 4
  store ptr %0, ptr %2, align 4
  %3 = load ptr, ptr %2, align 4
  %4 = getelementptr inbounds %struct.Acc, ptr %3, i32 0, i32 0
  %5 = load i32, ptr %4, align 4
  %6 = add nsw i32 %5, 1
  ret i32 %6
}

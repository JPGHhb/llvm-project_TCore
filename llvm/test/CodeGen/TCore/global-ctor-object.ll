; RUN: llc -mtriple=tcore-unknown-elf -filetype=asm %s -o - | FileCheck %s --check-prefix=ASM
; RUN: llc -mtriple=tcore-unknown-elf -filetype=obj %s -o %t.o
; RUN: llvm-readobj -h -r %t.o | FileCheck %s --check-prefix=OBJ

target triple = "tcore-unknown-elf"

%struct.Counter = type { i32 }

$_ZN7CounterC2Ei = comdat any
$_ZNK7Counter3getEv = comdat any

@g = dso_local global %struct.Counter zeroinitializer, align 4
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_tcore_global_ctor_cpp, ptr null }]

define internal void @__cxx_global_var_init() section ".text.startup" {
; ASM-LABEL: __cxx_global_var_init:
; ASM: ldu r1, g
; ASM-NEXT: ldl r1, g
; ASM: mov r2, 7
; ASM: ldu r14, _ZN7CounterC2Ei
; ASM-NEXT: ldl r14, _ZN7CounterC2Ei
; ASM-NEXT: jmp r14
entry:
  call void @_ZN7CounterC2Ei(ptr noundef nonnull align 4 dereferenceable(4) @g, i32 noundef 7)
  ret void
}

define linkonce_odr void @_ZN7CounterC2Ei(ptr noundef nonnull align 4 dereferenceable(4) %0, i32 noundef %1) unnamed_addr comdat align 2 {
; ASM-LABEL: _ZN7CounterC2Ei:
; ASM: str r2, [r1+0]
entry:
  %3 = alloca ptr, align 4
  %4 = alloca i32, align 4
  store ptr %0, ptr %3, align 4
  store i32 %1, ptr %4, align 4
  %5 = load ptr, ptr %3, align 4
  %6 = getelementptr inbounds %struct.Counter, ptr %5, i32 0, i32 0
  %7 = load i32, ptr %4, align 4
  store i32 %7, ptr %6, align 4
  ret void
}

define dso_local i32 @entry() {
; ASM-LABEL: entry:
; ASM: ldu r1, g
; ASM-NEXT: ldl r1, g
; ASM: ldu r14, _ZNK7Counter3getEv
; ASM-NEXT: ldl r14, _ZNK7Counter3getEv
; ASM-NEXT: jmp r14
entry:
  %1 = call noundef i32 @_ZNK7Counter3getEv(ptr noundef nonnull align 4 dereferenceable(4) @g)
  ret i32 %1
}

define linkonce_odr i32 @_ZNK7Counter3getEv(ptr noundef nonnull align 4 dereferenceable(4) %0) comdat align 2 {
; ASM-LABEL: _ZNK7Counter3getEv:
; ASM: ldr r1, [r1+0]
entry:
  %2 = alloca ptr, align 4
  store ptr %0, ptr %2, align 4
  %3 = load ptr, ptr %2, align 4
  %4 = getelementptr inbounds %struct.Counter, ptr %3, i32 0, i32 0
  %5 = load i32, ptr %4, align 4
  ret i32 %5
}

define internal void @_GLOBAL__sub_I_tcore_global_ctor_cpp() section ".text.startup" {
; ASM-LABEL: _GLOBAL__sub_I_tcore_global_ctor_cpp:
; ASM: ldu r14, __cxx_global_var_init
; ASM-NEXT: ldl r14, __cxx_global_var_init
; ASM-NEXT: jmp r14
entry:
  call void @__cxx_global_var_init()
  ret void
}

; OBJ: Format: elf32-tcore
; OBJ: Arch: tcore
; OBJ: Machine: EM_TCORE (0x5443)
; OBJ: Section ({{[0-9]+}}) .rela.text {
; OBJ: R_TCORE_HI16 g 0x0
; OBJ: R_TCORE_LO16 g 0x0
; OBJ: R_TCORE_HI16 _ZNK7Counter3getEv 0x0
; OBJ: R_TCORE_LO16 _ZNK7Counter3getEv 0x0
; OBJ: Section ({{[0-9]+}}) .rela.text.startup {
; OBJ: R_TCORE_HI16 g 0x0
; OBJ: R_TCORE_LO16 g 0x0
; OBJ: R_TCORE_HI16 _ZN7CounterC2Ei 0x0
; OBJ: R_TCORE_LO16 _ZN7CounterC2Ei 0x0
; OBJ: R_TCORE_HI16 .text.startup 0x0
; OBJ: R_TCORE_LO16 .text.startup 0x0
; OBJ: Section ({{[0-9]+}}) .rela.init_array {
; OBJ: R_TCORE_32 .text.startup

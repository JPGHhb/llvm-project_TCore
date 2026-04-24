// RUN: %clang_cc1 -triple tcore-unknown-elf -emit-llvm -x c++ %s -o - | FileCheck %s

// CHECK: target datalayout = "e-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-a0:0:32-n32"
// CHECK: target triple = "tcore-unknown-unknown-elf"
// CHECK: @g = global %struct.Counter zeroinitializer, align 4
// CHECK: @llvm.global_ctors = appending global
// CHECK: ptr @_GLOBAL__sub_I_tcore_global_ctor.cpp
// CHECK-LABEL: define internal void @__cxx_global_var_init()
// CHECK: call void @_ZN7CounterC1Ei(ptr noundef nonnull align 4 dereferenceable(4) @g, i32 noundef 7)
// CHECK-LABEL: define dso_local i32 @entry()
// CHECK: call noundef i32 @_ZNK7Counter3getEv(ptr noundef nonnull align 4 dereferenceable(4) @g)

struct Counter {
  int x;
  Counter(int v) : x(v) {}
  int get() const { return x; }
};

Counter g(7);

extern "C" int entry() {
  return g.get();
}

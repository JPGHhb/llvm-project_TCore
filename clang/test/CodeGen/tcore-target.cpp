// RUN: %clang_cc1 -triple tcore-unknown-elf -emit-llvm -x c++ %s -o - | FileCheck %s --check-prefix=IR

// IR: target datalayout = "e-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:32:32-f32:32:32-f64:32:32-a0:0:32-n32"
// IR: target triple = "tcore-unknown-unknown-elf"

struct Acc {
  int x;
  Acc(int v) : x(v) {}
  int inc() const { return x + 1; }
};

extern "C" int entry() {
  Acc a(41);
  return a.inc();
}

// IR-LABEL: define{{.*}} i32 @entry()
// IR: call void @_ZN3AccC1Ei
// IR: call noundef i32 @_ZNK3Acc3incEv

// RUN: %clang_cc1 -E -dM -ffreestanding -triple=tcore-unknown-elf < /dev/null | FileCheck %s

// CHECK:#define __TCELE_V1__ 1
// CHECK:#define __TCELE__ 1
// CHECK:#define __TCE_V1__ 1
// CHECK:#define __TCE__ 1
// CHECK:#define __TCORE_V1__ 1
// CHECK:#define __TCORE__ 1

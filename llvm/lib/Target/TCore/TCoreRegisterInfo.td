class TCoreReg<string n, bits<5> num> : Register<n> {
  let Namespace = "TCore";
  let HWEncoding{4-0} = num;
}

def R0  : TCoreReg<"r0",  0>,  DwarfRegNum<[0]>;
def R1  : TCoreReg<"r1",  1>,  DwarfRegNum<[1]>;
def R2  : TCoreReg<"r2",  2>,  DwarfRegNum<[2]>;
def R3  : TCoreReg<"r3",  3>,  DwarfRegNum<[3]>;
def R4  : TCoreReg<"r4",  4>,  DwarfRegNum<[4]>;
def R5  : TCoreReg<"r5",  5>,  DwarfRegNum<[5]>;
def R6  : TCoreReg<"r6",  6>,  DwarfRegNum<[6]>;
def R7  : TCoreReg<"r7",  7>,  DwarfRegNum<[7]>;
def R8  : TCoreReg<"r8",  8>,  DwarfRegNum<[8]>;
def R9  : TCoreReg<"r9",  9>,  DwarfRegNum<[9]>;
def R10 : TCoreReg<"r10", 10>, DwarfRegNum<[10]>;
def R11 : TCoreReg<"r11", 11>, DwarfRegNum<[11]>;
def R12 : TCoreReg<"r12", 12>, DwarfRegNum<[12]>;
def R13 : TCoreReg<"r13", 13>, DwarfRegNum<[13]>;
def R14 : TCoreReg<"r14", 14>, DwarfRegNum<[14]>;
def LR  : TCoreReg<"lr",  15>, DwarfRegNum<[15]>;
def SP  : TCoreReg<"sp",  16>, DwarfRegNum<[16]>;

def GPR : RegisterClass<"TCore", [i32], 32,
  (add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, LR)>;

def GPRSP : RegisterClass<"TCore", [i32], 32,
  (add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, LR, SP)> {
  let isAllocatable = 0;
}

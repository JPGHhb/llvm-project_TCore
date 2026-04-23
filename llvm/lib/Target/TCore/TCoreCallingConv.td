def CSR_TCore : CalleeSavedRegs<(add R5, R6, R7, R8, R9, LR)>;

def CC_TCore : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R1, R2, R3, R4]>>,
  CCAssignToStack<4, 4>
]>;

def RetCC_TCore : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R1]>>
]>;

# NanoVM
PoC lightweight x64 VM implementation

NanoVM is register based turing complete VM with stack memory. The project also includes assembler and debugger with similiar syntax to x86 asm with intel syntax. Note that the project is still in very early development and many thing including the insturction set and format is a subject to change, so bytecode from previous versions might not work. The documentation will be updated when changes happen

## VM architecture

The VM memory are defined as pages which by default are 4096 bytes each. When initialized the VM bytecode will be placed at the bottom of the allocated memory followed by the stack memory base on the next page. While the VM is similiar to x86 the stack grows up unlike in x86. This can be utilized to dynamically increase the stack memory if required with minimal effort.

### Registers
The VM is register based so the instuctions utilize different registers. Registers are encoded with 5 bits so there are 8 registers in total (the names will change in future):

| Register        | Number        | Description                                  |
| -------------   |:-------------:| --------------------------------------------:|
| Reg0            | 0             | General purpose. Used to store return values |
| Reg1            | 1             | General purpose.                             |
| Reg2            | 2             | General purpose.                             |
| Reg3            | 3             | General purpose.                             |
| Reg4            | 4             | General purpose.                             |
| Reg5            | 5             | General purpose.                             |
| Reg6            | 6             | General purpose.                             |
| Esp             | 7             | Stack pointer. Points to the top of the stack|

### Instructions
Instructions have always an opcode and 0-2 operands. Below is the instruction encoding defined from LST to MSB

| 5 bits           | 3 bits                | 1 bit             | 2 bits                      | 1 bits        | 1 bit         | 3 bits        |
| -------------    |:---------------------:|:-----------------:|:---------------------------:|:-------------:|:-------------:|:-------------:|
| Opcode           | Destination register  | Source type       | Source size                 | Is_Dst_pointer| Is_Src_pointer|Source register|
| What instruction | Update this register  | Reg=0, Immediate=1| Byte, short, dword, qword   | True,false    | True, false   | Source register if src type is reg|

So most of the instructions are encoded in 2 bytes + immediate value if used. Instructions that use zero operands effectively being only 1 byte are:
```assembly
Halt ; Stops the execution and exits the VM execution
ret ; Pops value from the top of the stack and performs absolute jump to that address. Updates stack pointer
```
Instructions that use 1 operand do not use either source register or immediate value. They do not use destination register even though it is always defined. Opcodes that use 1 operand:
```assembly
	Jz; Jump if zero flag is set. Example: jz reg0
	Jnz; Jump if zero flag is not set. Example: jnz reg0
	Jg;  Jump if greater flag is set. Example: jg reg0
	Js;  Jump if smaller flag is set. Example: js reg0
	Jmp; Jump ("goto") instruction. Example: jmp reg0
	Not; Flip the bits in value. Example: not reg0
	Inc; Increases the value by one: Example inc reg0
	Dec; Decreases the value by one: Example dec reg0
	Call; Pushes the next instructions absolute memory address to the stack and performs relative jump to the given address. Updates stack pointer Example: call reg0
	Push; Pushes value to the top of the stack. Example: push reg0
	Pop; Pops value from the top of the stack and moves the value to given address. Example: pop reg0
	Printi; prints given integer. Example: printi reg0
	Prints; prints given null terminated string. Example: prints @reg0 | Note that @reg0 uses reg0 as pointer to the string not as an absolute value
	Printc; prints given ASCII char to the console. Example printc reg0
```
Instructions with 2 operands:
```assembly
	Mov; mov reg0, reg0 <=> reg0 = reg0
	Add; add reg0, reg0 <=> reg0 += reg0
	Sub; mov reg0, reg0 <=> reg0 -= reg0
	And; mov reg0, reg0 <=> reg0 &= reg0
	Or;  or reg0, reg0 <=> reg0 |= reg0
	Xor; xor reg0, reg0 <=> reg0 ^= reg0
	Sar; sar reg0, reg0 <=> reg0 >>= reg0
	Sal; sal reg0, reg0 <=> reg0 <<= reg0
	Ror; ror reg0, reg0 <=> performs circular shift to the right on reg0, by reg0 times
	Rol; rol reg0, reg0 <=> performs circular shift to the left on reg0, by reg0 times
	Mul; mul reg0, reg0 <=> reg0 *= reg0
	Div; div reg0, reg0 <=> reg0 /= reg0
	Mod; mod reg0, reg0 <=> reg0 %= reg0
	Cmp; cmp reg0, reg1 | Compares the 2 values and sets flags depending on the comparison.
```

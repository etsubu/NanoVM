package assembler

import (
	"os"
	"regexp"
	"strings"
)

type DataType int
type Size int
type FloatOrInt int
type SignedOrUnsigned int

type Opcode struct {
	opcode          int
	size            int
	RelativeAddress bool
}

const (
	Register  DataType = 0
	Immediate DataType = 1
)

const (
	Bit8  Size = 0
	Bit16 Size = 1
	Bit32 Size = 2
	Bit64 Size = 3
)

const (
	Int   FloatOrInt = 0
	Float FloatOrInt = 1
)

const (
	Unsigned SignedOrUnsigned = 0
	Signed   SignedOrUnsigned = 1
)

type Instruction struct {
	opcode     Opcode
	dstReg     Registry
	sourceType DataType
	sourceReg  Registry
	sourceSize Size
	isInt      FloatOrInt
	isSigned   SignedOrUnsigned
	immediate  Number
	RawLine    string
	LineNumber int
}

func (i *Instruction) Assemble() []byte {
	var bytes = make([]byte, 1, 10)
	bytes[0] = byte(i.opcode.opcode)
	bytes[0] |= byte(i.dstReg.reg) << 5
	if i.opcode.size == 0 {
		return bytes
	}
	operands := byte(i.sourceType)
	operands |= byte(i.sourceSize) << 1
	operands |= byte(i.isInt) << 3
	operands |= byte(i.isSigned) << 4
	operands |= byte(i.sourceReg.reg) << 5
	bytes = append(bytes, operands)
	if i.sourceType == Immediate {
		if i.immediate.isResolved() {
			bytes = append(bytes, i.immediate.bytes...)
		}
	}
	return bytes
}

func (i Instruction) InstructionLength() int {
	if i.opcode.size == 0 {
		return 1
	}
	baseLength := 2
	if i.sourceType == Immediate {
		return baseLength + (1 << i.sourceSize)
	}
	return baseLength
}

type Number struct {
	bytes   []byte
	Sign    SignedOrUnsigned
	NumSize Size
	Type    FloatOrInt
	Label   string
}

func (n Number) isResolved() bool {
	return len(n.bytes) > 0
}

type Registry struct {
	reg     int
	Signed  SignedOrUnsigned
	NumSize Size
	Type    FloatOrInt
}

type Section struct {
	Name    string
	Read    bool
	Write   bool
	Execute bool
}

type InstructionSet struct {
	instructions []Instruction
	labels       map[string]int
	sections     map[string]Section
}

type Assembler struct {
	code        string
	OpcodeMap   map[string]Opcode
	RegisterMap map[string]Registry
}

func NewAssembler() *Assembler {
	opcodeMap := make(map[string]Opcode, 64)
	registerMap := make(map[string]Registry, 16)
	// Hardcoding for explicit documentation of values as these are also subject to change
	opcodeMap["mov"] = Opcode{opcode: 0, size: 2}
	opcodeMap["add"] = Opcode{opcode: 1, size: 2}
	opcodeMap["sub"] = Opcode{opcode: 2, size: 2}
	opcodeMap["and"] = Opcode{opcode: 3, size: 2}
	opcodeMap["or"] = Opcode{opcode: 4, size: 2}
	opcodeMap["xor"] = Opcode{opcode: 5, size: 2}
	opcodeMap["sar"] = Opcode{opcode: 6, size: 2}
	opcodeMap["sal"] = Opcode{opcode: 7, size: 2}
	opcodeMap["ror"] = Opcode{opcode: 8, size: 2}
	opcodeMap["rol"] = Opcode{opcode: 9, size: 2}
	opcodeMap["mul"] = Opcode{opcode: 10, size: 2}
	opcodeMap["div"] = Opcode{opcode: 11, size: 2}
	opcodeMap["mod"] = Opcode{opcode: 12, size: 2}
	opcodeMap["cmp"] = Opcode{opcode: 13, size: 2}

	opcodeMap["jz"] = Opcode{opcode: 14, size: 1, RelativeAddress: true}
	opcodeMap["jnz"] = Opcode{opcode: 15, size: 1, RelativeAddress: true}
	opcodeMap["jg"] = Opcode{opcode: 16, size: 1, RelativeAddress: true}
	opcodeMap["js"] = Opcode{opcode: 17, size: 1, RelativeAddress: true}
	opcodeMap["jmp"] = Opcode{opcode: 18, size: 1, RelativeAddress: true}
	opcodeMap["not"] = Opcode{opcode: 19, size: 1}
	opcodeMap["inc"] = Opcode{opcode: 20, size: 1}
	opcodeMap["dec"] = Opcode{opcode: 21, size: 1}
	opcodeMap["call"] = Opcode{opcode: 22, size: 1, RelativeAddress: true}
	opcodeMap["push"] = Opcode{opcode: 23, size: 1}
	opcodeMap["pop"] = Opcode{opcode: 24, size: 1}
	opcodeMap["ret"] = Opcode{opcode: 25, size: 0}
	opcodeMap["halt"] = Opcode{opcode: 26, size: 0}
	opcodeMap["load"] = Opcode{opcode: 27, size: 2}
	opcodeMap["store"] = Opcode{opcode: 28, size: 2}
	opcodeMap["syscall"] = Opcode{opcode: 29, size: 2}

	registerMap["reg0"] = Registry{reg: 0, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["reg1"] = Registry{reg: 1, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["reg2"] = Registry{reg: 2, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["reg3"] = Registry{reg: 3, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["reg4"] = Registry{reg: 4, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["reg5"] = Registry{reg: 5, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["reg6"] = Registry{reg: 6, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMap["sp"] = Registry{reg: 7, Signed: Unsigned, NumSize: Bit64, Type: Int}
	registerMapTypeExtensions := make(map[string]Registry, 16)
	for key, val := range registerMap {
		registerMapTypeExtensions[key] = val
		registerMapTypeExtensions[key+".8"] = Registry{reg: val.reg, Signed: Unsigned, NumSize: Bit8, Type: Int}
		registerMapTypeExtensions[key+".16"] = Registry{reg: val.reg, Signed: Unsigned, NumSize: Bit16, Type: Int}
		registerMapTypeExtensions[key+".32"] = Registry{reg: val.reg, Signed: Unsigned, NumSize: Bit32, Type: Int}
		registerMapTypeExtensions[key+".64"] = Registry{reg: val.reg, Signed: Unsigned, NumSize: Bit64, Type: Int}
		registerMapTypeExtensions[key+".8s"] = Registry{reg: val.reg, Signed: Signed, NumSize: Bit8, Type: Int}
		registerMapTypeExtensions[key+".16s"] = Registry{reg: val.reg, Signed: Signed, NumSize: Bit16, Type: Int}
		registerMapTypeExtensions[key+".32s"] = Registry{reg: val.reg, Signed: Signed, NumSize: Bit32, Type: Int}
		registerMapTypeExtensions[key+".64s"] = Registry{reg: val.reg, Signed: Signed, NumSize: Bit64, Type: Int}
		registerMapTypeExtensions[key+".32f"] = Registry{reg: val.reg, Signed: Signed, NumSize: Bit32, Type: Float}
		registerMapTypeExtensions[key+".64f"] = Registry{reg: val.reg, Signed: Signed, NumSize: Bit64, Type: Float}
	}

	a := Assembler{code: "", OpcodeMap: opcodeMap, RegisterMap: registerMapTypeExtensions}
	return &a
}

func (a *Assembler) LoadFile(file string) error {
	content, err := os.ReadFile(file)
	if err != nil {
		return err
	}
	a.code = string(content)
	return nil
}

func (a *Assembler) LoadMemory(code string) {
	a.code = code
}

func (a *Assembler) ParseInstructions(code string) (InstructionSet, error) {
	lines := strings.Split(code, "\n")
	var instructions []Instruction
	labels := make(map[string]int)
	sections := make(map[string]Section)
	whitespaces := regexp.MustCompile(`\s+`)
	for i, line := range lines {
		var cleanedLine = line
		if line == "" {
			continue
		}
		commentIdx := strings.Index(line, ";")
		if commentIdx == 0 {
			continue
		}
		if commentIdx != -1 {
			cleanedLine = line[0:commentIdx]
		}
		cleanedLine = strings.Trim(whitespaces.ReplaceAllString(cleanedLine, " "), " ")
		if len(cleanedLine) == 0 {
			continue
		}
		if len(cleanedLine) > 1 && cleanedLine[0] == ':' {
			labels[cleanedLine] = len(instructions)
			continue
		}
		if len(cleanedLine) > 1 && cleanedLine[0] == '.' {
			sec, err := ConvertToSection(cleanedLine)
			if err != nil {
				return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Invalid section definition"}
			}
			sections[sec.Name] = sec
			continue
		}
		var instruction Instruction
		instruction.LineNumber = i + 1
		instruction.RawLine = line
		// parse the cleaned line
		var opcodeStr string
		var dstRegStr string
		var sourceValue string
		// Extract opcode e.g. "mov"
		if opcodeDelimiter := strings.Index(cleanedLine, " "); opcodeDelimiter == -1 {
			// No arg opcode like "halt"
			opcodeStr = strings.Trim(cleanedLine, " ")
			cleanedLine = ""
		} else {
			opcodeStr = cleanedLine[0:opcodeDelimiter]
			cleanedLine = cleanedLine[opcodeDelimiter+1:]
		}
		if op, ok := a.OpcodeMap[opcodeStr]; ok {
			instruction.opcode = op
		} else {
			return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Unrecognized opcode " + opcodeStr}
		}
		// No args
		// Parse remaining parts like "reg0,reg1" or "100"
		parts := strings.Split(cleanedLine, ",")
		if instruction.opcode.size == 0 {
			if cleanedLine != "" {
				return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Opcode takes no operands"}
			}
			instructions = append(instructions, instruction)
			continue
		}
		switch len(parts) {
		case 1:
			if instruction.opcode.size != 1 {
				return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Opcode takes only two operands"}
			}
			dstRegStr = strings.Trim(cleanedLine, " ")
		case 2:
			if instruction.opcode.size != 2 {
				return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Opcode takes only one operand"}
			}
			dstRegStr = strings.Trim(parts[0], " ")
			sourceValue = strings.Trim(parts[1], " ")
		default:
			return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Too many parts " + cleanedLine}
		}
		// destination register
		if reg, ok := a.RegisterMap[dstRegStr]; ok {
			instruction.dstReg = reg
			// source register or immediate value
			if instruction.opcode.size == 2 {
				if reg, ok = a.RegisterMap[sourceValue]; ok {
					instruction.sourceReg = reg
					instruction.sourceType = Register
					instruction.sourceSize = reg.NumSize
					instruction.isInt = reg.Type
					instruction.isSigned = reg.Signed
				} else {
					immediate, err := ConvertToNumber(sourceValue)
					if err != nil {
						return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Value not recognized as number or registry " + sourceValue}
					}
					instruction.sourceType = Immediate
					instruction.immediate = immediate
					instruction.sourceSize = immediate.NumSize
					instruction.isInt = immediate.Type
					instruction.isSigned = immediate.Sign
				}
			}
		} else {
			if instruction.opcode.size == 2 {
				return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "First operand must be a register " + dstRegStr}
			}
			// was not register, this might be for example "jnz -123"
			immediate, err := ConvertToNumber(dstRegStr)
			if err != nil {
				return InstructionSet{}, &AssemblerError{lineNumber: i + 1, line: line, message: "Value not recognized as number or registry " + dstRegStr}
			}
			instruction.sourceType = Immediate
			instruction.immediate = immediate
			instruction.sourceSize = immediate.NumSize
			instruction.isInt = immediate.Type
			instruction.isSigned = immediate.Sign
		}
		instructions = append(instructions, instruction)
	}
	return InstructionSet{instructions: instructions, labels: labels, sections: sections}, nil
}

func (a *Assembler) Assemble() ([]byte, error) {
	instructionSet, err := a.ParseInstructions(a.code)
	if err != nil {
		return []byte{}, err
	}
	// Assemble to bytecode
	labelMemoryLocation := make(map[string]int, len(instructionSet.labels))
	instructionAddress := make([]int, len(instructionSet.instructions)+1)
	var bytecode = make([]byte, 0, len(instructionSet.instructions)*4)
	bytecodeSize := 0
	// First pass to resolve labels
	for i, inst := range instructionSet.instructions {
		instructionAddress[i] = bytecodeSize
		bytecodeSize += inst.InstructionLength()
	}
	instructionAddress[len(instructionSet.instructions)] = bytecodeSize
	for label, index := range instructionSet.labels {
		labelMemoryLocation[label] = instructionAddress[index]
	}
	// Assemble pass
	bytecodeSize = 0
	for _, inst := range instructionSet.instructions {
		if inst.sourceType == Immediate && inst.immediate.Label != "" {
			// Label reference
			if labelAddress, ok := labelMemoryLocation[inst.immediate.Label]; ok {
				deltaAddress := labelAddress
				if inst.opcode.RelativeAddress {
					deltaAddress = labelAddress - bytecodeSize
				}
				num, err := ConvertInt64ToNumber(int64(deltaAddress))
				if err != nil {
					return []byte{}, &AssemblerError{lineNumber: inst.LineNumber, line: inst.RawLine, message: "Failed to resolve label to address " + err.Error()}
				}
				inst.immediate = num
				inst.sourceType = Immediate
				bytecode = append(bytecode, inst.Assemble()...)
				bytecodeSize += inst.InstructionLength()
				continue
			} else {
				return []byte{}, &AssemblerError{lineNumber: inst.LineNumber, line: inst.RawLine, message: "Referenced label that does not exist"}
			}
		}
		bytecode = append(bytecode, inst.Assemble()...)
		bytecodeSize += inst.InstructionLength()
	}
	// Label or section references in immediate values that need to be resolved
	return bytecode, nil
}

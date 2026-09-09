package assembler

import (
	"bytes"
	"strings"
	"testing"
)

func assembleSource(t *testing.T, source string) []byte {
	t.Helper()
	a := NewAssembler()
	a.LoadMemory(source)
	code, err := a.Assemble()
	if err != nil {
		t.Fatalf("assembling %q failed: %v", source, err)
	}
	return code
}

func TestAssembleInstructionEncoding(t *testing.T) {
	tests := []struct {
		source string
		want   []byte
	}{
		{"halt", []byte{0x1a}},
		{"ret", []byte{0x19}},
		{"mov reg0, reg1", []byte{0x00, 0x26}},
		{"mov sp, reg0", []byte{0xe0, 0x06}},
		{"mov reg0, reg1.32s", []byte{0x00, 0x34}},
		{"mov reg0, reg1.64f", []byte{0x00, 0x3e}},
		{"add reg1, 123", []byte{0x21, 0x01, 0x7b}},
		{"mov reg0, -1", []byte{0x00, 0x11, 0xff}},
		{"mov reg2, 1000", []byte{0x40, 0x03, 0xe8, 0x03}},
		{"mov reg0, 100000", []byte{0x00, 0x05, 0xa0, 0x86, 0x01, 0x00}},
		{"mov reg0, 10000000000", []byte{0x00, 0x07, 0x00, 0xe4, 0x0b, 0x54, 0x02, 0x00, 0x00, 0x00}},
		{"mov reg0, 1.5", []byte{0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f}},
		{"store reg0, 123", []byte{0x1c, 0x01, 0x7b}},
		{"inc reg3", []byte{0x74, 0x00}},
		{"jz reg0", []byte{0x0e, 0x00}},
		{"jmp -2", []byte{0x12, 0x11, 0xfe}},
		{"   mov   reg0,   reg1   ; comment", []byte{0x00, 0x26}},
	}
	for _, tc := range tests {
		t.Run(tc.source, func(t *testing.T) {
			if got := assembleSource(t, tc.source); !bytes.Equal(got, tc.want) {
				t.Errorf("assembled %q to % x, want % x", tc.source, got, tc.want)
			}
		})
	}
}

func TestAssembleLabelResolution(t *testing.T) {
	tests := []struct {
		name   string
		source string
		want   []byte
	}{
		{
			"forward relative jump",
			"jz :end\nhalt\n:end\nhalt",
			[]byte{0x0e, 0x17, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a, 0x1a},
		},
		{
			"backward relative jump",
			":top\nhalt\njmp :top",
			[]byte{0x1a, 0x12, 0x17, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
		},
		{
			"label at end of file",
			"jz :end\nhalt\n:end",
			[]byte{0x0e, 0x17, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1a},
		},
		{
			"two labels on the same instruction",
			"jz :a\njnz :b\n:a\n:b\nhalt",
			[]byte{
				0x0e, 0x17, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0f, 0x17, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x1a,
			},
		},
		{
			"absolute label on non relative opcode",
			":target\nhalt\nmov reg0, :target",
			[]byte{0x1a, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			if got := assembleSource(t, tc.source); !bytes.Equal(got, tc.want) {
				t.Errorf("assembled %q to % x, want % x", tc.source, got, tc.want)
			}
		})
	}
}

func TestInstructionLengthMatchesAssembledSize(t *testing.T) {
	sources := []string{
		"halt", "mov reg0, reg1", "mov reg0, 1", "mov reg0, 1000",
		"mov reg0, 100000", "mov reg0, 10000000000", "mov reg0, 1.5", "inc reg0",
	}
	for _, source := range sources {
		t.Run(source, func(t *testing.T) {
			a := NewAssembler()
			set, err := a.ParseInstructions(source)
			if err != nil {
				t.Fatalf("parsing %q failed: %v", source, err)
			}
			inst := set.instructions[0]
			if got, want := inst.InstructionLength(), len(inst.Assemble()); got != want {
				t.Errorf("%q reports length %d but assembles to %d bytes", source, got, want)
			}
		})
	}
}

func TestAssembleRejectsInvalidSource(t *testing.T) {
	tests := []struct {
		name   string
		source string
	}{
		{"unknown opcode", "foo reg0"},
		{"too few operands", "mov reg0"},
		{"too many operands for one operand opcode", "jz reg0, reg1"},
		{"operands on zero operand opcode", "halt reg0"},
		{"more than two operands", "mov reg1, reg2, reg3"},
		{"immediate as destination", "mov 123, reg1"},
		{"unknown register as destination", "mov rax, reg1"},
		{"missing source operand", "mov reg0,"},
		{"unknown source operand", "mov reg0, rax"},
		{"undefined label", "jz :missing"},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			a := NewAssembler()
			a.LoadMemory(tc.source)
			if _, err := a.Assemble(); err == nil {
				t.Errorf("assembling %q succeeded, want an error", tc.source)
			}
		})
	}
}

func TestAssemblerErrorReportsOneBasedLineNumber(t *testing.T) {
	a := NewAssembler()
	a.LoadMemory("halt\n\nmov reg0")
	_, err := a.Assemble()
	if err == nil {
		t.Fatal("assembling invalid source succeeded, want an error")
	}
	if !strings.Contains(err.Error(), "Line number: 3") {
		t.Errorf("error %q does not report line number 3", err)
	}
}

func TestRegisterMapContainsOnlyBaseAndSuffixedNames(t *testing.T) {
	a := NewAssembler()
	base := []string{"reg0", "reg1", "reg2", "reg3", "reg4", "reg5", "reg6", "sp"}
	suffixes := []string{"", ".8", ".16", ".32", ".64", ".8s", ".16s", ".32s", ".64s", ".32f", ".64f"}
	expected := make(map[string]bool, len(base)*len(suffixes))
	for _, name := range base {
		for _, suffix := range suffixes {
			expected[name+suffix] = true
		}
	}
	for name := range a.RegisterMap {
		if !expected[name] {
			t.Errorf("register map contains unexpected name %q", name)
		}
	}
	for name := range expected {
		if _, ok := a.RegisterMap[name]; !ok {
			t.Errorf("register map is missing name %q", name)
		}
	}
}

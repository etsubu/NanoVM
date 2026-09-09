package assembler

import (
	"bytes"
	"testing"
)

func TestNumberToByteArray(t *testing.T) {
	if vals := len(NumberToByteArray(int32(32))); vals != 4 {
		t.Errorf("int32 conversion produced %d bytes instead of 4", vals)
	}
	if vals := len(NumberToByteArray(int8(32))); vals != 1 {
		t.Errorf("int8 conversion produced %d bytes instead of 1", vals)
	}
	if vals := len(NumberToByteArray(int16(32))); vals != 2 {
		t.Errorf("int16 conversion produced %d bytes instead of 2", vals)
	}
	if vals := len(NumberToByteArray(int64(32))); vals != 8 {
		t.Errorf("int64 conversion produced %d bytes instead of 8", vals)
	}
	if vals := len(NumberToByteArray(uint32(32))); vals != 4 {
		t.Errorf("uint32 conversion produced %d bytes instead of 4", vals)
	}
	if vals := len(NumberToByteArray(uint8(32))); vals != 1 {
		t.Errorf("uint8 conversion produced %d bytes instead of 1", vals)
	}
	if vals := len(NumberToByteArray(uint16(32))); vals != 2 {
		t.Errorf("uint16 conversion produced %d bytes instead of 2", vals)
	}
	if vals := len(NumberToByteArray(uint64(32))); vals != 8 {
		t.Errorf("uint64 conversion produced %d bytes instead of 8", vals)
	}
	if vals := len(NumberToByteArray(float32(32))); vals != 4 {
		t.Errorf("float32 conversion produced %d bytes instead of 4", vals)
	}
	if vals := len(NumberToByteArray(float64(32))); vals != 8 {
		t.Errorf("float64 conversion produced %d bytes instead of 8", vals)
	}
}

func TestConvertToValidSection(t *testing.T) {
	sec, err := ConvertToSection(".asd rwe")
	if err != nil {
		t.Errorf("valid section parsing failed")
		return
	}
	if sec.Name != ".asd" {
		t.Errorf("section parsing produced name %s instead of .asd", sec.Name)
		return
	}
	if !sec.Read || !sec.Write || !sec.Execute {
		t.Errorf("section parsing did not parse section flags read=%t, write=%t, execute=%t", sec.Read, sec.Write, sec.Execute)
		return
	}
}

func TestConvertToInValidSection(t *testing.T) {
	if _, err := ConvertToSection(" rwe"); err == nil {
		t.Errorf("invalid section did not error")
	}
	if _, err := ConvertToSection(".asd"); err == nil {
		t.Errorf("invalid section did not error")
	}
	if _, err := ConvertToSection("asfas"); err == nil {
		t.Errorf("invalid section did not error")
	}
}

func TestConvertToNumberWithLabel(t *testing.T) {
	if n, err := ConvertToNumber(":label"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Label != ":label" {
			t.Errorf("number conversion did not parse right label name %s vs %s", n.Label, ":label")
		}
		if n.Sign != Signed || n.Type != Int || n.NumSize != Bit64 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
	}
}

func TestConvertToNumberWithSection(t *testing.T) {
	if n, err := ConvertToNumber(".label"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Label != ".label" {
			t.Errorf("number conversion did not parse right label name %s vs %s", n.Label, ".label")
		}
		if n.Sign != Signed || n.Type != Int || n.NumSize != Bit64 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
	}
}

func TestConvertToNumberInt8(t *testing.T) {
	if n, err := ConvertToNumber("120"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Unsigned || n.Type != Int || n.NumSize != Bit8 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 1 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 1", len(n.bytes))
		}
	}
}

func TestConvertToNumberInt16(t *testing.T) {
	if n, err := ConvertToNumber("260"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Unsigned || n.Type != Int || n.NumSize != Bit16 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 2 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 2", len(n.bytes))
		}
	}
}

func TestConvertToNumberInt32(t *testing.T) {
	if n, err := ConvertToNumber("4294967295"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Unsigned || n.Type != Int || n.NumSize != Bit32 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 4 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 4", len(n.bytes))
		}
	}
}

func TestConvertToNumberInt64(t *testing.T) {
	if n, err := ConvertToNumber("4294967296"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Unsigned || n.Type != Int || n.NumSize != Bit64 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 8 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 8", len(n.bytes))
		}
	}
}

func TestConvertToSignedInt8(t *testing.T) {
	if n, err := ConvertToNumber("-120"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Signed || n.Type != Int || n.NumSize != Bit8 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 1 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 1", len(n.bytes))
		}
	}
}

func TestConvertToSignedInt16(t *testing.T) {
	if n, err := ConvertToNumber("-140"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Signed || n.Type != Int || n.NumSize != Bit16 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 2 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 2", len(n.bytes))
		}
	}
}

func TestConvertToSignedInt32(t *testing.T) {
	if n, err := ConvertToNumber("-32769"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Signed || n.Type != Int || n.NumSize != Bit32 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 4 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 4", len(n.bytes))
		}
	}
}

func TestConvertToSignedInt64(t *testing.T) {
	if n, err := ConvertToNumber("-2147483649"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Signed || n.Type != Int || n.NumSize != Bit64 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 8 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 8", len(n.bytes))
		}
	}
}

func TestConvertToFloat64(t *testing.T) {
	if n, err := ConvertToNumber("-21.1239"); err != nil {
		t.Errorf("invalid section did not error")
	} else {
		if n.Sign != Signed || n.Type != Float || n.NumSize != Bit64 {
			t.Errorf("number conversion did not set right flags %d, %d, %d", n.Sign, n.Type, n.NumSize)
		}
		if len(n.bytes) != 8 {
			t.Errorf("number conversion did not produce right amount of bytes %d vs 8", len(n.bytes))
		}
	}
}

func TestInvalidConversion(t *testing.T) {
	if _, err := ConvertToNumber("-21.12a39"); err == nil {
		t.Errorf("converted invalid number withour errors")
	}
}

func TestNumberToByteArrayIsLittleEndian(t *testing.T) {
	tests := []struct {
		name string
		got  []byte
		want []byte
	}{
		{"int8", NumberToByteArray(int8(-1)), []byte{0xff}},
		{"uint8", NumberToByteArray(uint8(0x7b)), []byte{0x7b}},
		{"int16", NumberToByteArray(int16(-2)), []byte{0xfe, 0xff}},
		{"uint16", NumberToByteArray(uint16(258)), []byte{0x02, 0x01}},
		{"int32", NumberToByteArray(int32(1)), []byte{0x01, 0x00, 0x00, 0x00}},
		{"uint32", NumberToByteArray(uint32(100000)), []byte{0xa0, 0x86, 0x01, 0x00}},
		{"int64", NumberToByteArray(int64(-2)), []byte{0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}},
		{"uint64", NumberToByteArray(uint64(10000000000)), []byte{0x00, 0xe4, 0x0b, 0x54, 0x02, 0x00, 0x00, 0x00}},
		{"float32", NumberToByteArray(float32(1)), []byte{0x00, 0x00, 0x80, 0x3f}},
		{"float64", NumberToByteArray(float64(1.5)), []byte{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x3f}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			if !bytes.Equal(tc.got, tc.want) {
				t.Errorf("%s converted to % x, want % x", tc.name, tc.got, tc.want)
			}
		})
	}
}

func TestConvertToNumberRejectsEmptyValue(t *testing.T) {
	if _, err := ConvertToNumber(""); err == nil {
		t.Errorf("converting an empty value succeeded, want an error")
	}
}

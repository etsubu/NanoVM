package assembler

import (
	"encoding/binary"
	"errors"
	"math"
	"strconv"
	"strings"
)

func NumberToByteArray[T int8 | int16 | int32 | int64 | uint8 | uint16 | uint32 | uint64 | float32 | float64](num T) []byte {
	var bits uint64
	var size int
	switch v := any(num).(type) {
	case int8:
		bits, size = uint64(uint8(v)), 1
	case uint8:
		bits, size = uint64(v), 1
	case int16:
		bits, size = uint64(uint16(v)), 2
	case uint16:
		bits, size = uint64(v), 2
	case int32:
		bits, size = uint64(uint32(v)), 4
	case uint32:
		bits, size = uint64(v), 4
	case int64:
		bits, size = uint64(v), 8
	case uint64:
		bits, size = v, 8
	case float32:
		bits, size = uint64(math.Float32bits(v)), 4
	case float64:
		bits, size = math.Float64bits(v), 8
	}
	arr := make([]byte, 8)
	binary.LittleEndian.PutUint64(arr, bits)
	return arr[:size]
}

func ConvertToNumber(value string) (Number, error) {
	if value == "" {
		return Number{}, errors.New("Empty number")
	}
	switch value[0] {
	case '.':
		return Number{Label: value, Sign: Signed, NumSize: Bit64, Type: Int}, nil
	case ':':
		return Number{Label: value, Sign: Signed, NumSize: Bit64, Type: Int}, nil
	}
	if strings.Contains(value, ".") {
		// Float
		f, err := strconv.ParseFloat(value, 64)
		if err != nil {
			return Number{}, err
		}
		return Number{bytes: NumberToByteArray(f), Sign: Signed, NumSize: Bit64, Type: Float}, nil
	}
	if value[0] == '-' {
		val, err := strconv.ParseInt(value, 10, 64)
		if err != nil {
			return Number{}, err
		}
		if val <= math.MaxInt8 && val >= math.MinInt8 {
			return Number{bytes: NumberToByteArray(int8(val)), Sign: Signed, NumSize: Bit8, Type: Int}, nil
		}
		if val <= math.MaxInt16 && val >= math.MinInt16 {
			return Number{bytes: NumberToByteArray(int16(val)), Sign: Signed, NumSize: Bit16, Type: Int}, nil
		}
		if val <= math.MaxInt32 && val >= math.MinInt32 {
			return Number{bytes: NumberToByteArray(int32(val)), Sign: Signed, NumSize: Bit32, Type: Int}, nil
		}
		return Number{bytes: NumberToByteArray(int64(val)), Sign: Signed, NumSize: Bit64, Type: Int}, nil
	}
	val, err := strconv.ParseUint(value, 10, 64)
	if err != nil {
		return Number{}, err
	}
	if val <= math.MaxUint8 && val >= 0 {
		return Number{bytes: NumberToByteArray(uint8(val)), Sign: Unsigned, NumSize: Bit8, Type: Int}, nil
	}
	if val <= math.MaxUint16 && val >= 0 {
		return Number{bytes: NumberToByteArray(uint16(val)), Sign: Unsigned, NumSize: Bit16, Type: Int}, nil
	}
	if val <= math.MaxUint32 && val >= 0 {
		return Number{bytes: NumberToByteArray(uint32(val)), Sign: Unsigned, NumSize: Bit32, Type: Int}, nil
	}
	return Number{bytes: NumberToByteArray(uint64(val)), Sign: Unsigned, NumSize: Bit64, Type: Int}, nil
}

func ConvertInt64ToNumber(val int64) (Number, error) {
	return Number{bytes: NumberToByteArray(int64(val)), Sign: Signed, NumSize: Bit64, Type: Int}, nil
}

func ConvertToSection(section string) (Section, error) {
	parts := strings.Split(section, " ")
	if len(parts) != 2 {
		return Section{}, errors.New("invalid section definition, needs to be e.g. .section rwe")
	}
	if len(parts[0]) < 2 || parts[0][0] != '.' {
		return Section{}, errors.New("invalid section name, needs to be e.g. .section")
	}
	sec := Section{Name: parts[0]}
	sec.Execute = strings.ContainsRune(parts[1], 'e')
	sec.Read = strings.ContainsRune(parts[1], 'r')
	sec.Write = strings.ContainsRune(parts[1], 'w')
	return sec, nil
}

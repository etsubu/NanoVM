package assembler

import "fmt"

type AssemblerError struct {
	lineNumber int
	line       string
	message    string
}

func (e *AssemblerError) Error() string {
	return fmt.Sprintf("Line number: %d - \"%s\" - %s", e.lineNumber, e.line, e.message)
}

package main

import (
	"github.com/etsubu/NanoVM/GoAssembler/internal/assembler"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"
	"time"
)

func main() {
	args := os.Args
	if len(args) < 2 {
		fmt.Println("Missing target file")
		os.Exit(1)
	}
	sourceFile := args[1]
	asm := assembler.NewAssembler()
	if err := asm.LoadFile(sourceFile); err != nil {
		log.Fatalf("Failed to load input file %s, %v", sourceFile, err)
	}
	start := time.Now()
	bytecode, err := asm.Assemble()
	if err != nil {
		log.Fatalf("Failed to assemble file %v", err)
	}
	// output
	output := strings.TrimSuffix(sourceFile, filepath.Ext(sourceFile)) + ".nanoc"
	if output == sourceFile {
		log.Fatalf("Refusing to overwrite input file %s", sourceFile)
	}
	fileErr := os.WriteFile(output, bytecode, 0644)
	if fileErr != nil {
		log.Fatalf("Failed to write assembled file to output %s, %v", output, fileErr)
	}
	end := time.Now()
	log.Printf("Assembled file %s to %s in %d ms, %d bytes", sourceFile, output, end.Sub(start).Milliseconds(), len(bytecode))
}

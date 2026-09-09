#pragma once

#include "NanoVM.h"
#include "Instructions.h"

struct NanoDebugger {
	NanoVM vm;
	uint64_t* breakpoints;
	size_t breakpointCount;
	size_t breakpointCapacity;
	bool run;
};

typedef struct NanoDebugger NanoDebugger;

void NanoDebuggerInit(NanoDebugger* debugger, const char* file);

void NanoDebuggerInitFromMemory(NanoDebugger* debugger, unsigned char* bytecode, uint64_t size);

void NanoDebuggerDestroy(NanoDebugger* debugger);

bool NanoDebuggerDebug(NanoDebugger* debugger);

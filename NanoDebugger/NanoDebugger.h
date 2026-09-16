#pragma once

#include <stdbool.h>
#include "NanoVM.h"

struct NanoDebugger {
	NanoVM vm;
	uint64_t* breakpoints;
	size_t breakpointCount;
	size_t breakpointCapacity;
	bool run;
};

typedef struct NanoDebugger NanoDebugger;

bool NanoDebuggerInit(NanoDebugger* debugger, const char* file);

bool NanoDebuggerInitFromMemory(NanoDebugger* debugger, unsigned char* bytecode, uint64_t size);

void NanoDebuggerDestroy(NanoDebugger* debugger);

bool NanoDebuggerDebug(NanoDebugger* debugger);

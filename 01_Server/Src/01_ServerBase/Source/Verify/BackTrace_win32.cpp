#include "Verify/BackTrace.h"

#include <DbgHelp.h>
#include <Windows.h>


#pragma comment(lib, "dbghelp.lib")

void PrintBackTrace()
{
    const int kMaxFrame = 100;
    std::array<void*, kMaxFrame> stack{};
    unsigned short frames = 0;
    SYMBOL_INFO* symbol = nullptr;
    HANDLE process = INVALID_HANDLE_VALUE;

    process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

    frames = CaptureStackBackTrace(0, kMaxFrame, stack.data(), NULL);
    // NOLINTBEGIN
    symbol = (SYMBOL_INFO*)malloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for (unsigned int index = 0; index < frames; index++)
    {
        DWORD displacement = 0;
        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

        SymFromAddr(process, (DWORD64)(stack[index]), 0, symbol);
        SymGetLineFromAddr64(process, reinterpret_cast<DWORD64>(stack[index]),
                             &displacement, &line);

        char undec[1024];
        UnDecorateSymbolName(symbol->Name, undec, sizeof(undec), UNDNAME_COMPLETE);

        fmt::println("[{}]: {}()\n{}:{}", index, undec, line.FileName,
                     line.LineNumber);
    }

    free(symbol);
    // NOLINTEND
}

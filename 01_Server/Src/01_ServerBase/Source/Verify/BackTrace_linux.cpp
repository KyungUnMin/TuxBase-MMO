#define UNW_LOCAL_ONLY

#include "Verify/Assert.h"
#include "Verify/BackTrace.h"
#include <execinfo.h>
#include <libunwind-x86_64.h>
#include <sstream>
#include <unistd.h>

namespace
{
    bool ResolveAddr(uintptr_t addr, std::string& outSourcePosition)
    {
        constexpr int kModuleNameLen = 512;
        static std::array<char, kModuleNameLen> moduleName{};

        if (moduleName[0] == '\0')
        {
            const ssize_t kNameLen =
                readlink("/proc/self/exe", moduleName.data(), moduleName.size());
            ASSERT(kNameLen != -1, "readlink failed");
            moduleName.at(kNameLen) = '\0';
        }

        std::stringstream command;
        command << "addr2line -f -C -e " << moduleName.data() << " 0x" << std::hex
                << addr;

        FILE* fileHandler = popen(command.str().c_str(), "r");
        if (!fileHandler)
        {
            return false;
        }

        constexpr int kBufSize = 512;
        std::array<char, kBufSize> buf{};

        // NOLINTNEXTLINE (altera-unroll-loops)
        while (fgets(buf.data(), kBufSize, fileHandler))
        {
            outSourcePosition += buf.data();
        }
        pclose(fileHandler);

        const std::string kNotFoundPrefix = "??";
        const bool kIsFound = (outSourcePosition.compare(0, kNotFoundPrefix.size(),
                                                         kNotFoundPrefix) != 0);

        return kIsFound;
    }
} // namespace

void PrintBackTrace()
{
    constexpr int kNameLen = 256;
    std::array<char, kNameLen> name{};

    unw_word_t ipValue{};
    unw_word_t offsetValue{};
    unw_cursor_t cursor{};
    unw_context_t context{};
    int frameNumber = 0;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    fmt::println("[FrameNo]: (Function Name) (SourceFile:Line)");
    fmt::println("----- BackTrace Start ------");

    // NOLINTNEXTLINE (altera-unroll-loops)
    while (unw_step(&cursor) > 0)
    {
        name[0] = '\0';

        unw_get_proc_name(&cursor, name.data(), kNameLen, &offsetValue);
        unw_get_reg(&cursor, UNW_REG_IP, &ipValue);

        std::string debugInfo;
        if (ResolveAddr(ipValue, debugInfo))
        {
            fmt::print("[{}]: {}", frameNumber, debugInfo);
        }
        else
        {
            fmt::println("[{}]: {} (IP: {:#x})", frameNumber, name.data(),
                         (long)ipValue);
        }

        frameNumber++;
    }

    fmt::println("----- BackTrace End ------");
}

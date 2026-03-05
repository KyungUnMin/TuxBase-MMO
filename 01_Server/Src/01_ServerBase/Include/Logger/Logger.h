#pragma once

#include "Logger/LogSink.h"

enum class eLogLevel : uint8_t
{
    kTrace,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kFatal
};

class Logger
{
private:
    Logger(const Logger& other) = delete;
    Logger(Logger&& other) noexcept = delete;
    Logger& operator=(Logger&& other) noexcept = delete;
    Logger& operator=(const Logger& other) = delete;

public:
    Logger(const char* loggerName, eLogLevel level = eLogLevel::kInfo);
    ~Logger();

    template <eLogLevel Level>
    void Log(const char* message);

    void SetSink(ISink* sink);

private:
    static constexpr int kMaxNameLength = 30;

    std::array<char, kMaxNameLength> _name;
    eLogLevel _level;

    ISink* _sink = nullptr;
};

#include "Logger/Logger.inl"

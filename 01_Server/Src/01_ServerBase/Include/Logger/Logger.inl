constexpr const char* GetLevelString(eLogLevel level)
{
    switch (level)
    {
    case eLogLevel::kTrace:
        return "TRACE";
    case eLogLevel::kDebug:
        return "DEBUG";
    case eLogLevel::kInfo:
        return "INFO";
    case eLogLevel::kWarning:
        return "WARNING";
    case eLogLevel::kError:
        return "ERROR";
    case eLogLevel::kFatal:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

Logger::Logger(const char* loggerName, eLogLevel level)
    : _name(), _level(level)
{
    std::strncpy(_name.data(), loggerName, kMaxNameLength - 1);
    _name[kMaxNameLength - 1] = '\0';
}

Logger::~Logger() {}

template <eLogLevel Level>
void Logger::Log(const char* message)
{
    if (Level >= _level)
    {
        const auto now = std::chrono::system_clock::now();
        const auto timeT = std::chrono::system_clock::to_time_t(now);
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                                now.time_since_epoch()) %
                            1000;

        std::tm dateTime{};
#ifdef _WIN32
        localtime_s(&dateTime, &timeT);
#else
        localtime_r(&timeT, &dateTime);
#endif

        const std::string kFormatMessage =
            fmt::format("[{:%Y-%m-%d %H:%M:%S}.{:03d}][{}][{}] {}", dateTime,
                        static_cast<int>(millis.count()), GetLevelString(Level),
                        _name.data(), message);

        _sink->Log(Level, kFormatMessage);
    }
}

void Logger::SetSink(ISink* sink)
{
    ASSERT(sink != nullptr, "sink is nullptr");
    _sink = sink;
}

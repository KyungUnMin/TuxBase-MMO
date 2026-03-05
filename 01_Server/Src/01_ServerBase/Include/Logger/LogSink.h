#pragma once

#include "Verify/Assert.h"

#include <fstream>
#include <vector>

// TODO: 싱크 인터페이스로 변경 및 팩토리 추가

enum class eLogLevel : uint8_t;

/**
 * @brief 로그 출력 대상을 추상화하는 인터페이스.
 */
class ISink
{
private:
    ISink(const ISink&) = delete;
    ISink(ISink&&) = delete;
    ISink& operator=(ISink&& other) noexcept = delete;
    ISink& operator=(const ISink& other) = delete;

public:
    ISink() = default;
    virtual ~ISink() = default;

    virtual void Log(eLogLevel level, const std::string&) = 0;
};

/**
 * @brief 아무것도 출력하지 않는 Null 싱크.
 */
class NullSink : public ISink
{
private:
    NullSink(const NullSink&) = delete;
    NullSink(NullSink&&) = delete;
    NullSink& operator=(NullSink&& other) noexcept = delete;
    NullSink& operator=(const NullSink& other) = delete;

public:
    NullSink() = default;
    ~NullSink() override = default;

    void Log(eLogLevel level, const std::string& message) override
    {
        (void)level;
        (void)message;
    }
};

/**
 * @brief 콘솔(stdout)에 로그를 출력하는 싱크.
 */
class ConsoleSink : public ISink
{
private:
    ConsoleSink(const ConsoleSink&) = delete;
    ConsoleSink(ConsoleSink&&) = delete;
    ConsoleSink& operator=(ConsoleSink&& other) noexcept = delete;
    ConsoleSink& operator=(const ConsoleSink& other) = delete;

public:
    ConsoleSink() = default;
    ~ConsoleSink() override = default;

    void Log(eLogLevel level, const std::string& message) override
    {
        (void)level;
        fmt::print(fmt::fg(fmt::color::blue), "{}\n", message);
        fflush(stdout);
    }
};

/**
 * @brief 파일에 로그를 기록하는 싱크.
 * std::ofstream 기반으로 크로스 플랫폼 동작합니다.
 */
class FileSink : public ISink
{
private:
    FileSink() = delete;
    FileSink(const FileSink&) = delete;
    FileSink(FileSink&&) = delete;
    FileSink& operator=(FileSink&& other) noexcept = delete;
    FileSink& operator=(const FileSink& other) = delete;

public:
    explicit FileSink(const char* fileName)
        : _fileStream(fileName, std::ios::out | std::ios::app)
    {
        if (!_fileStream.is_open())
        {
            CRASH("Failed to open log file.");
        }
    }

    ~FileSink() override = default;

    void Log(eLogLevel level, const std::string& message) override
    {
        (void)level;

        ASSERT(_fileStream.is_open(), "file stream is not open");

        _fileStream << message;
        _fileStream.flush();
    }

private:
    std::ofstream _fileStream;
};

/**
 * @brief 여러 싱크에 동시에 로그를 전달하는 멀티 싱크.
 */
class MultiSink : public ISink
{
private:
    MultiSink(const MultiSink&) = delete;
    MultiSink(MultiSink&&) = delete;
    MultiSink& operator=(MultiSink&& other) noexcept = delete;
    MultiSink& operator=(const MultiSink& other) = delete;

public:
    explicit MultiSink() = default;
    ~MultiSink() override = default;

    void Log(eLogLevel level, const std::string& message) override
    {
        for (ISink* sink : _sinks)
        {
            sink->Log(level, message);
        }
    }

    void AddSink(ISink* sink) { _sinks.push_back(sink); }

private:
    std::vector<ISink*> _sinks;
};

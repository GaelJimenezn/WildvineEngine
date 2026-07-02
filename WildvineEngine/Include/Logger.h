#pragma once
#include <string>
#include <vector>
#include <mutex>

enum class LogLevel { Info = 0, Warning = 1, Error = 2 };

struct LogEntry {
    LogLevel level;
    std::string message;
};

/**
 * @class Logger
 * @brief Buffer central de logs para la consola del editor (singleton).
 */
class Logger {
public:
    static Logger& get() {
        static Logger instance;
        return instance;
    }

    void add(LogLevel level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_entries.size() > 8000) {
            m_entries.erase(m_entries.begin(), m_entries.begin() + 2000);
        }
        m_entries.push_back({ level, msg });
        m_dirty = true;
    }

    void addW(LogLevel level, const std::wstring& wmsg) {
        add(level, narrow(wmsg));
    }

    std::vector<LogEntry> snapshot() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_entries;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
    }

    bool consumeDirty() {
        std::lock_guard<std::mutex> lock(m_mutex);
        bool d = m_dirty; m_dirty = false; return d;
    }

private:
    Logger() = default;

    static std::string narrow(const std::wstring& w) {
        std::string s;
        s.reserve(w.size());
        for (wchar_t c : w) {
            s.push_back((c >= 32 && c < 127) ? (char)c : (c == L'\n' ? '.' : ' '));
        }
        while (!s.empty() && (s.back() == '\n' || s.back() == ' ')) s.pop_back();
            return s;
    }

    std::vector<LogEntry> m_entries;
    std::mutex m_mutex;
    bool m_dirty = false;
};

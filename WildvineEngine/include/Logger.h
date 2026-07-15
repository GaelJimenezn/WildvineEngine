/**
 * @file Logger.h
 * @brief Declara el sistema de logging centralizado del editor (singleton).
 * @ingroup core
 *
 * Logger es un buffer de entradas de log protegido por mutex que
 * alimenta la ventana de consola del editor. Cualquier subsistema puede
 * escribir en él sin bloquear el render thread gracias al mutex interno.
 */
#pragma once
#include <string>
#include <vector>
#include <mutex>

/**
 * @enum LogLevel
 * @brief Nivel de severidad de una entrada de log.
 */
enum class LogLevel {
    Info    = 0, /**< @brief Mensaje informativo (cyan en la consola). */
    Warning = 1, /**< @brief Advertencia (amarillo en la consola). */
    Error   = 2  /**< @brief Error crítico (rojo en la consola). */
};

/**
 * @struct LogEntry
 * @brief Representa una única entrada en el buffer de log.
 */
struct LogEntry {
    LogLevel    level;   /**< @brief Nivel de severidad de la entrada. */
    std::string message; /**< @brief Texto del mensaje de log. */
};

/**
 * @class Logger
 * @brief Buffer central de logs para la consola del editor (singleton).
 */
class Logger {
public:
    /**
     * @brief Obtiene la instancia singleton del Logger.
     * @return Referencia a la única instancia del Logger.
     */
    static Logger& get() {
        static Logger instance;
        return instance;
    }

    /**
     * @brief Agrega una entrada de texto al buffer de log.
     *
     * Si el buffer supera 8000 entradas, descarta las 2000 más antiguas
     * para evitar consumo excesivo de memoria.
     *
     * @param level Nivel de severidad del mensaje.
     * @param msg   Texto del mensaje (UTF-8 / narrow).
     */
    void add(LogLevel level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_entries.size() > 8000) {
            m_entries.erase(m_entries.begin(), m_entries.begin() + 2000);
        }
        m_entries.push_back({ level, msg });
        m_dirty = true;
    }

    /**
     * @brief Agrega una entrada de texto wide (UTF-16) al buffer de log.
     *
     * Convierte la cadena wide a narrow antes de almacenarla.
     *
     * @param level Nivel de severidad del mensaje.
     * @param wmsg  Texto del mensaje en formato wide (wchar_t).
     */
    void addW(LogLevel level, const std::wstring& wmsg) {
        add(level, narrow(wmsg));
    }

    /**
     * @brief Devuelve una copia del buffer de log en este instante.
     *
     * Hilo seguro: adquiere el mutex antes de copiar.
     *
     * @return Vector de LogEntry con todas las entradas actuales.
     */
    std::vector<LogEntry> snapshot() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_entries;
    }

    /**
     * @brief Vacía completamente el buffer de log.
     */
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_entries.clear();
    }

    /**
     * @brief Consulta y resetea el flag de "hay datos nuevos".
     *
     * Devuelve true la primera vez que se llama tras agregar entradas nuevas,
     * luego false hasta que se agregan más. Útil para refrescar la UI solo
     * cuando hay cambios reales.
     *
     * @return true si el buffer fue modificado desde la última llamada.
     */
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

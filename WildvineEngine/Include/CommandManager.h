#pragma once
#include <vector>
#include <memory>
#include <utility>

/**
 * @class ICommand
 * @brief Interfaz base para acciones reversibles (Command Pattern).
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void undo() = 0;
    virtual void redo() = 0;
    virtual const char* name() const { return "Command"; }
};

/**
 * @class CommandManager
 * @brief Pila de undo/redo. Generico: no depende del motor.
 */
class CommandManager {
public:
    void push(std::unique_ptr<ICommand> cmd) {
        if (!cmd) return;
        m_redo.clear();
        m_undo.push_back(std::move(cmd));
        if (m_undo.size() > m_maxDepth)
            m_undo.erase(m_undo.begin(), m_undo.begin() + (m_undo.size() - m_maxDepth));
    }
    void undo() {
        if (m_undo.empty()) return;
        std::unique_ptr<ICommand> c = std::move(m_undo.back());
        m_undo.pop_back();
        c->undo();
        m_redo.push_back(std::move(c));
    }
    void redo() {
        if (m_redo.empty()) return;
        std::unique_ptr<ICommand> c = std::move(m_redo.back());
        m_redo.pop_back();
        c->redo();
        m_undo.push_back(std::move(c));
    }
    bool canUndo() const { return !m_undo.empty(); }
    bool canRedo() const { return !m_redo.empty(); }
    void clear() { m_undo.clear(); m_redo.clear(); }
    size_t undoCount() const { return m_undo.size(); }

private:
    std::vector<std::unique_ptr<ICommand>> m_undo;
    std::vector<std::unique_ptr<ICommand>> m_redo;
    size_t m_maxDepth = 100;
};

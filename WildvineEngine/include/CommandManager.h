/**
 * @file CommandManager.h
 * @brief Sistema de Undo/Redo basado en el patrón Command.
 * @ingroup core
 *
 * Proporciona la interfaz ICommand y el gestor CommandManager que mantiene
 * dos pilas (undo/redo) con profundidad máxima configurable.
 */
#pragma once
#include <vector>
#include <memory>
#include <utility>

/**
 * @class ICommand
 * @brief Interfaz base para acciones reversibles (Command Pattern).
 *
 * Toda acción que deba poder deshacerse debe implementar esta interfaz.
 * El CommandManager almacena punteros a ICommand en las pilas de undo/redo.
 */
class
ICommand {
public:
  /** @brief Destructor virtual. */
  virtual
  ~ICommand() = default;

  /** @brief Deshace la acción realizada por este comando. */
  virtual void
  undo() = 0;

  /** @brief Re-aplica la acción después de un undo. */
  virtual void
  redo() = 0;

  /**
   * @brief Devuelve el nombre descriptivo del comando.
   * @return Cadena literal con el nombre del comando.
   */
  virtual const char*
  name() const { return "Command"; }
};

/**
 * @class CommandManager
 * @brief Gestor de historial de Undo/Redo genérico e independiente del motor.
 *
 * Mantiene una pila de comandos undo y otra de redo con profundidad máxima
 * configurable (por defecto 100 entradas). Al ejecutar push() la pila
 * de redo se limpia, tal como es convención en editores tipo Unreal.
 */
class
CommandManager {
public:
  /**
   * @brief Añade un comando al historial y limpia la pila de redo.
   *
   * Si la pila supera maxDepth entradas, las más antiguas se eliminan.
   *
   * @param cmd Comando a registrar (ownership transferido).
   */
  void
  push(std::unique_ptr<ICommand> cmd) {
      if (!cmd) return;
      m_redo.clear();
      m_undo.push_back(std::move(cmd));
      if (m_undo.size() > m_maxDepth)
          m_undo.erase(m_undo.begin(), m_undo.begin() + (m_undo.size() - m_maxDepth));
  }
  /**
   * @brief Deshace el último comando ejecutado.
   *
   * El comando se mueve de la pila undo a la pila redo.
   */
  void
  undo() {
      if (m_undo.empty()) return;
      std::unique_ptr<ICommand> c = std::move(m_undo.back());
      m_undo.pop_back();
      c->undo();
      m_redo.push_back(std::move(c));
  }

  /**
   * @brief Re-aplica el último comando deshecho.
   *
   * El comando se mueve de la pila redo a la pila undo.
   */
  void
  redo() {
      if (m_redo.empty()) return;
      std::unique_ptr<ICommand> c = std::move(m_redo.back());
      m_redo.pop_back();
      c->redo();
      m_undo.push_back(std::move(c));
  }
  /** @brief Indica si hay acciones disponibles para deshacer. */
  bool
  canUndo() const { return !m_undo.empty(); }

  /** @brief Indica si hay acciones disponibles para re-aplicar. */
  bool
  canRedo() const { return !m_redo.empty(); }

  /** @brief Vacía ambas pilas (undo y redo). */
  void
  clear() { m_undo.clear(); m_redo.clear(); }

  /**
   * @brief Devuelve el número de comandos en la pila de undo.
   * @return Número de acciones deshacibles.
   */
  size_t
  undoCount() const { return m_undo.size(); }

private:
  /** @brief Pila de comandos deshacibles.. */
  std::vector<std::unique_ptr<ICommand>> m_undo;
  /** @brief Pila de comandos re-aplicables.. */
  std::vector<std::unique_ptr<ICommand>> m_redo;
  size_t m_maxDepth = 100; /**< @brief Profundidad máxima del historial de undo. */
};

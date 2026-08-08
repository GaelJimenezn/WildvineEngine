/**
 * @file Mesh.h
 * @brief Declara la API de Mesh dentro del subsistema de renderizado.
 */
#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

/**
 * @struct Submesh
 * @brief Representa una porción de una malla con su propio conjunto de buffers y
 * material.
 *
 * Un Submesh permite dividir una malla en partes que pueden:
 * - Usar diferentes materiales
 * - Ser renderizadas de manera independiente
 * - Compartir la misma geometría base
 */
struct
Submesh {
  /** @brief Buffer de vértices. */
  Buffer vertexBuffer;

  /** @brief Buffer de índices. */
  Buffer indexBuffer;

  /** @brief Número total de índices. */
  unsigned 
  int indexCount = 0;

  /** @brief Índice inicial dentro del index buffer. */
  unsigned 
  int startIndex = 0;

  /** @brief Slot de material asociado. */
  unsigned
  int materialSlot = 0;

  /** @brief Transform local del submesh relativo al actor. */
  XMFLOAT4X4 localTransform = XMFLOAT4X4(
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f
  );
};

/**
 * @class Mesh
 * @brief Representa una malla compuesta por múltiples submeshes.
 *
 * Permite gestionar geometría compleja dividiéndola en submeshes,
 * cada uno con su propio material y buffers.
 */
class
Mesh {
public:
  /** @brief Configura los límites locales usados por el culling espacial. */
  void
  setLocalBounds(const EU::Vector3& minimum, const EU::Vector3& maximum) {
    m_localBoundsMin = minimum;
    m_localBoundsMax = maximum;
    m_hasLocalBounds = true;
  }

  /** @brief Indica si la malla tiene límites locales válidos. */
  bool
  hasLocalBounds() const { return m_hasLocalBounds; }

  /** @brief Devuelve la esquina mínima de los límites locales. */
  const EU::Vector3&
  getLocalBoundsMin() const { return m_localBoundsMin; }

  /** @brief Devuelve la esquina máxima de los límites locales. */
  const EU::Vector3&
  getLocalBoundsMax() const { return m_localBoundsMax; }

  /**
   * @brief Obtiene la lista de submeshes (mutable).
   * @return Referencia al vector de Submesh.
   */
  std::vector<Submesh>&
  getSubmeshes() { return m_submeshes; }

  /**
   * @brief Obtiene la lista de submeshes (const).
   * @return Referencia constante al vector de Submesh.
   */
  const 
  /** @brief Declara o ejecuta getSubmeshes. */
  std::vector<Submesh>&
  getSubmeshes() const { return m_submeshes; }

  /**
   * @brief Libera los recursos de todos los submeshes.
   *
   * Destruye los buffers de vértices e índices y limpia la lista.
   */
  void
    destroy() {
    for (Submesh& submesh : m_submeshes) {
      submesh.vertexBuffer.destroy();
      submesh.indexBuffer.destroy();
    }
    m_submeshes.clear();
  }

private:
  /** @brief Lista de submeshes que componen la malla. */
  std::vector<Submesh> m_submeshes;
  EU::Vector3 m_localBoundsMin = EU::Vector3(0.0f, 0.0f, 0.0f);
  EU::Vector3 m_localBoundsMax = EU::Vector3(0.0f, 0.0f, 0.0f);
  bool m_hasLocalBounds = false;
};

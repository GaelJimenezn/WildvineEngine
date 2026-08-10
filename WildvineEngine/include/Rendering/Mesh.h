/**
 * @file Mesh.h
 * @brief Declara la API de Mesh dentro del subsistema de renderizado.
 */
#pragma once
#include "Prerequisites.h"
#include "Buffer.h"

/**
 * @struct Submesh
 * @brief Representa una porciÃ³n de una malla con su propio conjunto de buffers y
 * material.
 *
 * Un Submesh permite dividir una malla en partes que pueden:
 * - Usar diferentes materiales
 * - Ser renderizadas de manera independiente
 * - Compartir la misma geometrÃ­a base
 */
struct Submesh {
  /** @brief Buffer de vÃ©rtices. */
  Buffer vertexBuffer;

  /** @brief Buffer de Ã­ndices. */
  Buffer indexBuffer;

  /** @brief NÃºmero total de Ã­ndices. */
  unsigned int indexCount = 0;

  /** @brief Ãndice inicial dentro del index buffer. */
  unsigned int startIndex = 0;

  /** @brief Slot de material asociado. */
  unsigned int materialSlot = 0;

  /** @brief Transform local del submesh relativo al actor. */
  XMFLOAT4X4 localTransform = XMFLOAT4X4(1.0f,
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
                                         1.0f);
};

/**
 * @class Mesh
 * @brief Representa una malla compuesta por mÃºltiples submeshes.
 *
 * Permite gestionar geometrÃ­a compleja dividiÃ©ndola en submeshes,
 * cada uno con su propio material y buffers.
 */
class Mesh {
public:
  /** @brief Configura los lÃ­mites locales usados por el culling espacial. */
  void
  setLocalBounds(const EU::Vector3 &minimum, const EU::Vector3 &maximum) {
    m_localBoundsMin = minimum;
    m_localBoundsMax = maximum;
    m_hasLocalBounds = true;
  }

  /** @brief Indica si la malla tiene lÃ­mites locales vÃ¡lidos. */
  bool
  hasLocalBounds() const {
    return m_hasLocalBounds;
  }

  /** @brief Devuelve la esquina mÃ­nima de los lÃ­mites locales. */
  const EU::Vector3 &
  getLocalBoundsMin() const {
    return m_localBoundsMin;
  }

  /** @brief Devuelve la esquina mÃ¡xima de los lÃ­mites locales. */
  const EU::Vector3 &
  getLocalBoundsMax() const {
    return m_localBoundsMax;
  }

  /**
   * @brief Obtiene la lista de submeshes (mutable).
   * @return Referencia al vector de Submesh.
   */
  std::vector<Submesh> &
  getSubmeshes() {
    return m_submeshes;
  }

  /**
   * @brief Obtiene la lista de submeshes (const).
   * @return Referencia constante al vector de Submesh.
   */
  const
      /** @brief Declara o ejecuta getSubmeshes. */
      std::vector<Submesh> &
      getSubmeshes() const {
    return m_submeshes;
  }

  /**
   * @brief Libera los recursos de todos los submeshes.
   *
   * Destruye los buffers de vÃ©rtices e Ã­ndices y limpia la lista.
   */
  void
  destroy() {
    for (Submesh &submesh : m_submeshes) {
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

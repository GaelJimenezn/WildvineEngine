/**
 * @file Camera.h
 * @brief Declara utilidades de Camera usadas por WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"
#include "EngineUtilities\Vectors\Vector3.h"

/**
 * @class Camera
 * @brief Representa una cÃ¡mara 3D para renderizado en un motor grÃ¡fico.
 *
 * Gestiona la posiciÃ³n, orientaciÃ³n (basis ortonormal),
 * matriz de vista y matriz de proyecciÃ³n.
 * Permite movimiento tipo FPS (walk, strafe) y rotaciones
 * mediante yaw y pitch.
 */
class Camera {
public:
  /**
   * @brief Constructor por defecto.
   */
  Camera();

  /**
   * @brief Destructor por defecto.
   */
  ~Camera() = default;

  /**
   * @brief Configura los parÃ¡metros de la proyecciÃ³n perspectiva.
   *
   * @param fovYRadians Campo de visiÃ³n vertical en radianes.
   * @param aspectRatio RelaciÃ³n de aspecto (ancho/alto).
   * @param nearPlane Distancia del plano cercano.
   * @param farPlane Distancia del plano lejano.
   */
  void setLens(float fovYRadians, float aspectRatio, float nearPlane, float farPlane);

  /**
   * @brief Establece la posiciÃ³n de la cÃ¡mara mediante coordenadas individuales.
   *
   * @param x Coordenada X en mundo.
   * @param y Coordenada Y en mundo.
   * @param z Coordenada Z en mundo.
   */
  void setPosition(float x, float y, float z);

  /**
   * @brief Establece la posiciÃ³n de la cÃ¡mara.
   *
   * @param pos Vector de posiciÃ³n en espacio mundo.
   */
  void setPosition(const EU::Vector3 &pos);

  /**
   * @brief Obtiene la posiciÃ³n actual de la cÃ¡mara.
   *
   * @return EU::Vector3 PosiciÃ³n en mundo.
   */
  EU::Vector3
  getPosition() const {
    return m_position;
  }

  /** @brief Declara o ejecuta getPosition. */
  EU::Vector3 &
  getPosition() {
    return m_position;
  }

  /**
   * @brief Orienta la cÃ¡mara hacia un objetivo.
   *
   * @param pos PosiciÃ³n de la cÃ¡mara.
   * @param target Punto al que la cÃ¡mara mirarÃ¡.
   * @param up Vector arriba (por defecto eje Y positivo).
   */
  void lookAt(const EU::Vector3 &pos,
              const EU::Vector3 &target,
              const EU::Vector3 &up = EU::Vector3(0, 0, 1));

  /**
   * @brief Mueve la cÃ¡mara hacia adelante o atrÃ¡s en su eje forward.
   *
   * @param d Distancia a mover.
   */
  void walk(float d);

  /**
   * @brief Mueve la cÃ¡mara lateralmente en su eje right.
   *
   * @param d Distancia a mover.
   */
  void strafe(float d);

  /**
   * @brief Rota la cÃ¡mara alrededor del eje Y global.
   *
   * @param radians Ãngulo en radianes.
   */
  void yaw(float radians);

  /**
   * @brief Rota la cÃ¡mara alrededor del eje X local.
   *
   * @param radians Ãngulo en radianes.
   */
  void pitch(float radians);

  /**
   * @brief Actualiza la matriz de vista si el estado cambiÃ³.
   */
  void updateViewMatrix();

  /**
   * @brief Obtiene la matriz de vista.
   *
   * @return XMMATRIX Matriz de vista.
   */
  XMMATRIX
  getView() const {
    return XMLoadFloat4x4(&m_view);
  }

  /**
   * @brief Obtiene la matriz de proyecciÃ³n.
   *
   * @return XMMATRIX Matriz de proyecciÃ³n.
   */
  XMMATRIX
  getProj() const {
    return XMLoadFloat4x4(&m_proj);
  }

  /**
   * @brief Obtiene la matriz de vista sin traslaciÃ³n.
   *
   * Ãštil para skyboxes u objetos que no deben trasladarse con la cÃ¡mara.
   *
   * @return XMMATRIX Matriz de vista sin componente de traslaciÃ³n.
   */
  XMMATRIX
  GetViewNoTranslation() const {
    XMMATRIX v = getView();
    // Quitar traslaciÃ³n (fila 4)
    v.r[3] = XMVectorSet(0, 0, 0, 1);
    return v;
  }

  /**
   * @brief Obtiene el campo de visiÃ³n vertical.
   */
  float
  getFovY() const {
    return m_fovY;
  }

  /**
   * @brief Obtiene la relaciÃ³n de aspecto.
   */
  float
  getAspect() const {
    return m_aspectRatio;
  }

  /**
   * @brief Obtiene el plano cercano.
   */
  float
  getNearZ() const {
    return m_nearPlane;
  }

  /**
   * @brief Obtiene el plano lejano.
   */
  float
  getFarZ() const {
    return m_farPlane;
  }

  /**
   * @brief Obtiene el vector Right de la cÃ¡mara.
   */
  EU::Vector3
  GetRight() const {
    return m_right;
  }

  /**
   * @brief Obtiene el vector Up de la cÃ¡mara.
   */
  EU::Vector3
  GetUp() const {
    return m_up;
  }

  /**
   * @brief Obtiene el vector Forward de la cÃ¡mara.
   */
  EU::Vector3
  GetForward() const {
    return m_forward;
  }

  /**
   * @brief Convierte un FXMVECTOR a EU::Vector3.
   *
   * @param v Vector de DirectXMath.
   * @return EU::Vector3 Vector convertido.
   */
  inline EU::Vector3
  FromXM(FXMVECTOR v) {
    XMFLOAT3 t;
    XMStoreFloat3(&t, v);
    return EU::Vector3(t.x, t.y, t.z);
  }

private:
  /**
   * @brief PosiciÃ³n de la cÃ¡mara en espacio mundo.
   */
  EU::Vector3 m_position;

  /**
   * @brief Vector Right (base ortonormal en mundo).
   */
  EU::Vector3 m_right{1.0f, 0.0f, 0.0f};

  /**
   * @brief Vector Up (base ortonormal en mundo).
   */
  EU::Vector3 m_up{0.0f, 0.0f, 1.0f};

  /**
   * @brief Vector Forward (direcciÃ³n de vista).
   */
  EU::Vector3 m_forward{0.0f, 1.0f, 0.0f};

  /**
   * @brief Matriz de vista almacenada.
   */
  XMFLOAT4X4 m_view{};

  /**
   * @brief Matriz de proyecciÃ³n almacenada.
   */
  XMFLOAT4X4 m_proj{};

  /**
   * @brief Campo de visiÃ³n vertical en radianes.
   */
  float m_fovY{XM_PIDIV4};

  /**
   * @brief RelaciÃ³n de aspecto (width / height).
   */
  float m_aspectRatio = 1.0f;

  /**
   * @brief Distancia del plano cercano.
   */
  float m_nearPlane = 0.01f;

  /**
   * @brief Distancia del plano lejano.
   */
  float m_farPlane = 1000.0f;

  /**
   * @brief Indica si la matriz de vista necesita actualizarse.
   */
  bool m_viewDirty = true;
};

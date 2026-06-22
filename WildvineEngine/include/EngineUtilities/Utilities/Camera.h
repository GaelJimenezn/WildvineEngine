#pragma once
#include "Prerequisites.h"
#include "EngineUtilities\Vectors\Vector3.h"

/**
 * @class Camera
 * @brief Representa una cámara 3D para renderizado en un motor gráfico.
 *
 * Gestiona la posición, orientación (basis ortonormal),
 * matriz de vista y matriz de proyección.
 * Permite movimiento tipo FPS (walk, strafe) y rotaciones
 * mediante yaw y pitch.
 */
class
Camera {
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
	 * @brief Configura los parámetros de la proyección perspectiva.
	 *
	 * @param fovYRadians Campo de visión vertical en radianes.
	 * @param aspectRatio Relación de aspecto (ancho/alto).
	 * @param nearPlane Distancia del plano cercano.
	 * @param farPlane Distancia del plano lejano.
	 */
	void
	setLens(float fovYRadians, 
					float aspectRatio, 
					float nearPlane,
					float farPlane);


	/**
	 * @brief Establece la posición de la cámara mediante coordenadas individuales.
	 *
	 * @param x Coordenada X en mundo.
	 * @param y Coordenada Y en mundo.
	 * @param z Coordenada Z en mundo.
	 */
	void
	setPosition(float x, float y, float z);


	/**
	 * @brief Establece la posición de la cámara.
	 *
	 * @param pos Vector de posición en espacio mundo.
	 */
	void
	setPosition(const EU::Vector3& pos);


	/**
	 * @brief Obtiene la posición actual de la cámara.
	 *
	 * @return EU::Vector3 Posición en mundo.
	 */
	EU::Vector3
	getPosition() const { return m_position; }

	EU::Vector3&
	getPosition() { return m_position; }


	/**
	 * @brief Orienta la cámara hacia un objetivo.
	 *
	 * @param pos Posición de la cámara.
	 * @param target Punto al que la cámara mirará.
	 * @param up Vector arriba (por defecto eje Y positivo).
	 */
	void
	lookAt(const EU::Vector3& pos,
				const EU::Vector3& target,
				const EU::Vector3& up = EU::Vector3(0, 1, 0));


	/**
	 * @brief Mueve la cámara hacia adelante o atrás en su eje forward.
	 *
	 * @param d Distancia a mover.
	 */
	void
	walk(float d);


	/**
	 * @brief Mueve la cámara lateralmente en su eje right.
	 *
	 * @param d Distancia a mover.
	 */
	void
	strafe(float d);


	/**
	 * @brief Rota la cámara alrededor del eje Y global.
	 *
	 * @param radians Ángulo en radianes.
	 */
	void
	yaw(float radians);

	/**
	 * @brief Rota la cámara alrededor del eje X local.
	 *
	 * @param radians Ángulo en radianes.
	 */
	void
	pitch(float radians);


	/**
	 * @brief Actualiza la matriz de vista si el estado cambió.
	 */
	void
	updateViewMatrix();


	/**
	 * @brief Obtiene la matriz de vista.
	 *
	 * @return XMMATRIX Matriz de vista.
	 */
	XMMATRIX
	getView() const { return XMLoadFloat4x4(&m_view); }


	/**
	 * @brief Obtiene la matriz de proyección.
	 *
	 * @return XMMATRIX Matriz de proyección.
	 */
	XMMATRIX
	getProj() const { return XMLoadFloat4x4(&m_proj); }


	/**
	 * @brief Obtiene la matriz de vista sin traslación.
	 *
	 * Útil para skyboxes u objetos que no deben trasladarse con la cámara.
	 *
	 * @return XMMATRIX Matriz de vista sin componente de traslación.
	 */
	XMMATRIX
	GetViewNoTranslation() const {
		XMMATRIX v = getView();
		// Quitar traslación (fila 4)
		v.r[3] = XMVectorSet(0, 0, 0, 1);
		return v;
	}


	/**
	 * @brief Obtiene el campo de visión vertical.
	 */
	float 
	getFovY()   const { return m_fovY; }

	/**
	 * @brief Obtiene la relación de aspecto.
	 */
	float 
	getAspect() const { return m_aspectRatio; }

	/**
	 * @brief Obtiene el plano cercano.
	 */
	float 
	getNearZ()  const { return m_nearPlane; }

	/**
	 * @brief Obtiene el plano lejano.
	 */
	float 
	getFarZ()   const { return m_farPlane; }


	/**
	 * @brief Obtiene el vector Right de la cámara.
	 */
	EU::Vector3 GetRight()   const { return m_right; }

	/**
	 * @brief Obtiene el vector Up de la cámara.
	 */
	EU::Vector3 GetUp()      const { return m_up; }

	/**
	 * @brief Obtiene el vector Forward de la cámara.
	 */
	EU::Vector3 GetForward() const { return m_forward; }

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
	 * @brief Posición de la cámara en espacio mundo.
	 */
	EU::Vector3 m_position;

	/**
	 * @brief Vector Right (base ortonormal en mundo).
	 */
	EU::Vector3 m_right{ 1.0f, 0.0f, 0.0f };

	/**
	 * @brief Vector Up (base ortonormal en mundo).
	 */
	EU::Vector3 m_up{ 0.0f, 1.0f, 0.0f };

	/**
	 * @brief Vector Forward (dirección de vista).
	 */
	EU::Vector3 m_forward{ 0.0f, 0.0f, 1.0f };

	/**
	 * @brief Matriz de vista almacenada.
	 */
	XMFLOAT4X4 m_view{};

	/**
	 * @brief Matriz de proyección almacenada.
	 */
	XMFLOAT4X4 m_proj{};

	/**
	 * @brief Campo de visión vertical en radianes.
	 */
	float 
	m_fovY{ XM_PIDIV4 };

	/**
	 * @brief Relación de aspecto (width / height).
	 */
	float
	m_aspectRatio = 1.0f;

	/**
	 * @brief Distancia del plano cercano.
	 */
	float
	m_nearPlane = 0.01f;

	/**
	 * @brief Distancia del plano lejano.
	 */
	float
	m_farPlane = 1000.0f;

	/**
	 * @brief Indica si la matriz de vista necesita actualizarse.
	 */
	bool
	m_viewDirty = true;
};

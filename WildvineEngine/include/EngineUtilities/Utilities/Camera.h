#pragma once
#include "Prerequisites.h"
#include "EngineUtilities\Vectors\Vector3.h"

/**
 * @class Camera
 * @brief Left-handed perspective camera with cached view/projection matrices.
 *
 * Camera stores position, orthonormal basis vectors, projection parameters, and dirty state
 * so movement/rotation can update the view matrix lazily before rendering.
 */
class 
Camera {
public:
	/** @brief Creates a camera with default basis and perspective settings. */
	Camera();
	/** @brief Default destructor. */
	~Camera() = default;

	/**
		 * @brief Configura la proyección en perspectiva (LH).
		 *
		 * **Pasos**
		 * - Calcula la matriz de proyección con XMMatrixPerspectiveFovLH.
		 * - Guarda FOV, aspect, near y far para debug/inspección.
		 *
		 * **Aplicación práctica**
		 * - Llamar al inicializar ventana y al cambiar resolución.
		 */
	void 
	setLens(float fovYRadians, float aspectRatio, float nearPlane, float farPlane);

	/**
	 * @brief Define posición en mundo.
	 */
	void
	setPosition(float x, float y, float z);

	/**
	 * @brief Define posición en mundo.
	 */
	void
	setPosition(const EU::Vector3& pos);

	/**
	 * @brief Obtiene la posición en mundo.
	 */
	EU::Vector3 
	getPosition() const { return m_position; }
	
	/** @brief Obtiene una referencia mutable a la posicion en mundo. */
	EU::Vector3 &
	getPosition() { return m_position; }

	/**
	 * @brief Fuerza la cámara a mirar a un objetivo (LH).
	 *
	 * **Pasos**
	 * - Calcula basis a partir de (target - pos).
	 * - Normaliza Forward, Right y Up.
	 * - Marca dirty para recalcular View.
	 *
	 * **Aplicación práctica**
	 * - Cinemáticas simples o cámaras orbit.
	 */
	void
	lookAt(const EU::Vector3& pos,
				 const EU::Vector3& target, 
				 const EU::Vector3& up = EU::Vector3(0, 1, 0));

	/**
	 * @brief Movimiento relativo a la cámara (adelante/atrás).
	 */
	void 
	walk(float d);

	/**
	 * @brief Movimiento relativo a la cámara (izquierda/derecha).
	 */
	void 
	strafe(float d);

	/**
	 * @brief Rotación sobre el eje Y global (yaw).
	 *
	 * **Aplicación práctica**
	 * - Mouse X para FPS.
	 */
	void 
	yaw(float radians);

	/**
	 * @brief Rotación sobre el eje Right local (pitch).
	 *
	 * **Aplicación práctica**
	 * - Mouse Y para FPS.
	 */
	void 
	pitch(float radians);

	/**
	 * @brief Recalcula la matriz View si es necesario.
	 *
	 * **Pasos**
	 * - Reconstruye basis ortonormal (Right/Up/Forward).
	 * - Calcula View con XMMatrixLookToLH.
	 *
	 * **Aplicación práctica**
	 * - Llamar una vez por frame antes de render.
	 */
	void 
	updateViewMatrix();

	/**
	 * @brief Matriz View (mundo->vista).
	 */
	XMMATRIX 
	getView() const { return XMLoadFloat4x4(&m_view); }

	/**
	 * @brief Matriz Projection (vista->clip).
	 */
	XMMATRIX 
	getProj() const { return XMLoadFloat4x4(&m_proj); }

	/**
	 * @brief View sin traslación (solo rotación). Ideal para Skybox.
	 *
	 * **Aplicación práctica**
	 * - Skybox: ViewNoTranslation * Proj
	 */
	XMMATRIX 
	GetViewNoTranslation() const {
		XMMATRIX v = getView();
		// Quitar traslación (fila 4)
		v.r[3] = XMVectorSet(0, 0, 0, 1);
		return v;
	}

	/**
	 * @brief Devuelve parámetros de proyección (útil para UI/debug).
	 */
	float getFovY()   const { return m_fovY; }
	/** @brief Devuelve el aspect ratio de proyeccion. */
	float getAspect() const { return m_aspectRatio; }
	/** @brief Devuelve la distancia del plano cercano. */
	float getNearZ()  const { return m_nearPlane; }
	/** @brief Devuelve la distancia del plano lejano. */
	float getFarZ()   const { return m_farPlane; }

	/**
	 * @brief Vectores base (mundo) de la cámara.
	 */
	EU::Vector3 GetRight()   const { return m_right; }
	/** @brief Devuelve el vector up de la base de camara. */
	EU::Vector3 GetUp()      const { return m_up; }
	/** @brief Devuelve el vector forward de la base de camara. */
	EU::Vector3 GetForward() const { return m_forward; }

	/**
	 * @brief Converts a DirectX vector into an @c EU::Vector3.
	 * @param v DirectX vector value.
	 * @return Converted engine vector.
	 */
	inline EU::Vector3 
	FromXM(FXMVECTOR v)	{
		XMFLOAT3 t;
		XMStoreFloat3(&t, v);
		return EU::Vector3(t.x, t.y, t.z);
	}


private:
	// Estado espacial
	EU::Vector3 m_position; /**< The position of the camera in world space. */

	// Basis Ortonormal (en mundo)
	EU::Vector3 m_right		{ 1.0f, 0.0f, 0.0f }; /**< The right vector of the camera's orthonormal basis. */
	EU::Vector3 m_up			{ 0.0f, 1.0f, 0.0f }; /**< The up vector of the camera's orthonormal basis. */
	EU::Vector3 m_forward	{ 0.0f, 0.0f, 1.0f }; /**< The forward vector of the camera's orthonormal basis. */

	// Matrices almacenadas
	XMFLOAT4X4 m_view {};
	XMFLOAT4X4 m_proj {};

	// Parametros de la proyeccion
	float m_fovY { XM_PIDIV4 }; /**< The field of view (FOV) angle in radians. */
	float m_aspectRatio = 1.0f; /**< The aspect ratio of the camera's view (width divided by height). */
	float m_nearPlane = 0.01f; /**< The distance to the near clipping plane. */
	float m_farPlane = 1000.0f; /**< The distance to the far clipping plane. */

	bool m_viewDirty = true; /**< Flag indicating whether the view matrix needs to be recalculated. */
};

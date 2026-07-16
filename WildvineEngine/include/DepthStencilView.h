/**
 * @file DepthStencilView.h
 * @brief Declara la API pública de DepthStencilView dentro de WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/** @brief Declara class Device. */
class 
Device;

/** @brief Declara class DeviceContext. */
class 
DeviceContext;

/** @brief Declara class Texture. */
class 
Texture;

/**
 * @brief Encapsula una vista de profundidad y stencil en DirectX.
 *
 * Esta clase administra la creación, actualización, renderizado
 * y destrucción de un recurso DepthStencilView para el pipeline gráfico.
 */
class
DepthStencilView {
public:

  /**
   * @brief Constructor por defecto.
   */
  DepthStencilView() = default;

  /**
   * @brief Destructor por defecto.
   */
  ~DepthStencilView() = default;

  /**
   * @brief Inicializa la vista de profundidad y stencil.
   *
   * @param device Referencia al dispositivo de DirectX.
   * @param depthStencil Textura que servirá como buffer de profundidad/stencil.
   * @param format Formato DXGI usado para la vista.
   * @return HRESULT Código de estado de la operación (S_OK si fue exitosa).
   */
  HRESULT
  init(Device& device, Texture& depthStencil, DXGI_FORMAT format);

  /** @brief Declara o ejecuta init. */
  HRESULT
    init(Device& device,
      Texture& depthStencil,
      DXGI_FORMAT format,
      D3D11_DSV_DIMENSION viewDimension);


  /**
   * @brief Actualiza el estado interno de la vista.
   *
   * Actualmente no realiza ninguna operación.
   */
  void
  update() {};

  /**
   * @brief Renderiza usando la vista de profundidad y stencil.
   *
   * @param deviceContext Contexto del dispositivo de DirectX.
   */
  void
  render(DeviceContext& deviceContext);

  /**
   * @brief Libera los recursos asociados al DepthStencilView.
   */
  void
  destroy();

public:
/** @brief Puntero al recurso de DepthStencilView de DirectX.. */
ID3D11DepthStencilView* m_depthStencilView = nullptr;

};

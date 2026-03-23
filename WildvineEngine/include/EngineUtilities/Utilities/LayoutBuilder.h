/**
 * @file GUI.h
 * @brief Sistema de interfaz de usuario para el editor de Wildvine.
 */

/**
 * @class LayoutBuilder
 * @brief Clase de utilidad para construir y organizar los paneles del editor.
 * * Se encarga de la disposición espacial (Docking) de las herramientas del motor.
 */
#pragma once
#include "Prerequisites.h"

/**
 * @brief (Nota: A pesar de la documentación previa del archivo, esta clase actúa como un Builder
 * para simplificar la creación estructurada de descriptores de elementos de entrada 
 * (D3D11_INPUT_ELEMENT_DESC) utilizados en la creación de los Input Layouts de DirectX 11).
 */
class LayoutBuilder
{
public:
  /**
   * @brief Agrega una definición de un atributo a la estructura de vértices de la GPU.
   * * @param semantic Nombre semántico del HLSL (ej. "POSITION", "TEXCOORD").
   * @param format Formato de datos del atributo (ej. DXGI_FORMAT_R32G32B32_FLOAT).
   * @param semanticIndex Índice adjunto al semántico en HLSL (por defecto 0).
   * @param inputSlot Ranura o buffer desde el cual extraer los datos (por defecto 0).
   * @param alignedByteOffset Desplazamiento en bytes. Utiliza D3D11_APPEND_ALIGNED_ELEMENT para empacar automáticamente.
   * @param slotClass Determina si es información por vértice (por defecto) o por instancia.
   * @param instanceStepRate Tasa a la cual avanza el índice por instancia. Debe ser 0 para datos por vértice.
   * @return Referencia al LayoutBuilder (`*this`) para permitir encadenamiento de llamadas (method chaining).
   */
  LayoutBuilder& Add(
    const char* semantic,
    DXGI_FORMAT format,
    UINT semanticIndex = 0,
    UINT inputSlot = 0,
    UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
    D3D11_INPUT_CLASSIFICATION slotClass = D3D11_INPUT_PER_VERTEX_DATA,
    UINT instanceStepRate = 0)
  {
    D3D11_INPUT_ELEMENT_DESC d{};
    d.SemanticName = semantic;
    d.SemanticIndex = semanticIndex;
    d.Format = format;
    d.InputSlot = inputSlot;
    d.AlignedByteOffset = alignedByteOffset;
    d.InputSlotClass = slotClass;
    d.InstanceDataStepRate = instanceStepRate;
    m_elems.push_back(d);
    return *this;
  }

  /**
   * @brief Atajo conveniente para agregar datos que corresponden a Instancing (datos por instancia).
   * * @param semantic Nombre semántico del HLSL.
   * @param format Formato del atributo.
   * @param semanticIndex Índice del semántico (por defecto 0).
   * @param inputSlot Ranura de los datos (suele ser distinto a 0 en instancing, por defecto asume 1).
   * @param alignedByteOffset Empaquetado automático por defecto.
   * @param instanceStepRate Número de instancias a dibujar con la misma data antes de avanzar (por defecto 1).
   * @return Referencia al LayoutBuilder.
   */
  LayoutBuilder& AddInstance(
    const char* semantic,
    DXGI_FORMAT format,
    UINT semanticIndex = 0,
    UINT inputSlot = 1,
    UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
    UINT instanceStepRate = 1)
  {
    return Add(semantic, format, semanticIndex, inputSlot, alignedByteOffset,
      D3D11_INPUT_PER_INSTANCE_DATA, instanceStepRate);
  }

  /**
   * @brief Obtiene el vector interno con todas las descripciones de los elementos agregados.
   * @return Referencia constante al vector de D3D11_INPUT_ELEMENT_DESC.
   */
  const std::vector<D3D11_INPUT_ELEMENT_DESC>& Get() const { return m_elems; }

  /**
   * @brief Obtiene el número total de atributos agregados al layout.
   * @return Cantidad de elementos (útil para pasar la longitud del arreglo a la API de DX11).
   */
  UINT Count() const { return (UINT)m_elems.size(); }

private:
  std::vector<D3D11_INPUT_ELEMENT_DESC> m_elems; /**< Lista interna que almacena los atributos de los vértices/instancias. */
};
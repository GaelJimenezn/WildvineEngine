/**
 * @file LayoutBuilder.h
 * @brief Declara utilidades de LayoutBuilder usadas por WildvineEngine.
 */
#pragma once
#include "Prerequisites.h"

/**
 * @class LayoutBuilder
 * @brief Facilita la construcciÃ³n de descripciones de layout de entrada para Direct3D.
 *
 * Permite agregar elementos de entrada (input elements) de manera encadenada,
 * soportando tanto datos por vÃ©rtice como por instancia.
 */
class LayoutBuilder {
public:
  // **Add() base** (per-vertex por defecto)
  /**
   * @brief Agrega un elemento de layout de entrada por vÃ©rtice.
   *
   * @param semantic Nombre semÃ¡ntico del elemento (ej. POSITION, NORMAL).
   * @param format Formato de datos (DXGI_FORMAT).
   * @param semanticIndex Ãndice semÃ¡ntico.
   * @param inputSlot Slot de entrada.
   * @param alignedByteOffset Offset en bytes alineado.
   * @param slotClass ClasificaciÃ³n del slot (por vÃ©rtice o por instancia).
   * @param instanceStepRate Frecuencia de avance para instancing.
   * @return LayoutBuilder& Referencia al builder para encadenamiento.
   */
  LayoutBuilder &
  Add(const char *semantic,
      DXGI_FORMAT format,
      UINT semanticIndex = 0,
      UINT inputSlot = 0,
      UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
      D3D11_INPUT_CLASSIFICATION slotClass = D3D11_INPUT_PER_VERTEX_DATA,
      UINT instanceStepRate = 0) {
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

  // Atajo para instancing
  /**
   * @brief Agrega un elemento de layout para datos por instancia.
   *
   * @param semantic Nombre semÃ¡ntico del elemento.
   * @param format Formato de datos.
   * @param semanticIndex Ãndice semÃ¡ntico.
   * @param inputSlot Slot de entrada (por defecto 1).
   * @param alignedByteOffset Offset en bytes alineado.
   * @param instanceStepRate Frecuencia de avance por instancia.
   * @return LayoutBuilder& Referencia al builder para encadenamiento.
   */
  LayoutBuilder &
  AddInstance(const char *semantic,
              DXGI_FORMAT format,
              UINT semanticIndex = 0,
              UINT inputSlot = 1,
              UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
              UINT instanceStepRate = 1) {
    return Add(semantic,
               format,
               semanticIndex,
               inputSlot,
               alignedByteOffset,
               D3D11_INPUT_PER_INSTANCE_DATA,
               instanceStepRate);
  }

  /**
   * @brief Obtiene la lista de elementos del layout.
   *
   * @return Referencia constante al vector de descriptores.
   */
  const std::vector<D3D11_INPUT_ELEMENT_DESC> &
  Get() const {
    return m_elems;
  }

  /**
   * @brief Obtiene la cantidad de elementos en el layout.
   *
   * @return UINT NÃºmero de elementos.
   */
  UINT
  Count() const {
    return (UINT)m_elems.size();
  }

private:
  /**
   * @brief Contenedor de descriptores de elementos de entrada.
   */
  std::vector<D3D11_INPUT_ELEMENT_DESC> m_elems;
};

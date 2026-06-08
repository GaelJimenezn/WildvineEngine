#pragma once
#include "Prerequisites.h"

/**
 * @class LayoutBuilder
 * @brief Fluent helper for constructing D3D11 input-layout element arrays.
 *
 * The builder stores @c D3D11_INPUT_ELEMENT_DESC values in insertion order and returns
 * references to itself so vertex and instance layout declarations can be chained.
 */
class 
LayoutBuilder {
public:
  // **Add() base** (per-vertex por defecto)
  /**
   * @brief Adds one input element descriptor to the layout.
   * @param semantic Semantic name consumed by the shader input signature.
   * @param format DXGI data format for the element.
   * @param semanticIndex Semantic index for repeated semantics.
   * @param inputSlot Vertex-buffer slot that provides the element.
   * @param alignedByteOffset Byte offset or @c D3D11_APPEND_ALIGNED_ELEMENT.
   * @param slotClass Per-vertex or per-instance classification.
   * @param instanceStepRate Instance step rate for instanced data.
   * @return This builder for chained calls.
   */
  LayoutBuilder& 
  Add(const char* semantic,
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

  // **Atajo** para instancing
  /**
   * @brief Adds one per-instance input element descriptor.
   * @return This builder for chained calls.
   */
  LayoutBuilder&
  AddInstance(const char* semantic,
              DXGI_FORMAT format,
              UINT semanticIndex = 0,
              UINT inputSlot = 1,
              UINT alignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
              UINT instanceStepRate = 1) {
    return Add(semantic, format, semanticIndex, inputSlot, alignedByteOffset,
      D3D11_INPUT_PER_INSTANCE_DATA, instanceStepRate);
  }

  /** @brief Returns the accumulated input element descriptors. */
  const std::vector<D3D11_INPUT_ELEMENT_DESC>& Get() const { return m_elems; }
  /** @brief Returns the number of accumulated input element descriptors. */
  UINT Count() const { return (UINT)m_elems.size(); }

private:
  /** @brief Input-layout descriptors in shader declaration order. */
  std::vector<D3D11_INPUT_ELEMENT_DESC> m_elems;
};

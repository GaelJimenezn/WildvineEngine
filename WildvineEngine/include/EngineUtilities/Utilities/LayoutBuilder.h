#pragma once
#include "Prerequisites.h"

class 
LayoutBuilder {
public:
  // **Add() base** (per-vertex por defecto)
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

  const std::vector<D3D11_INPUT_ELEMENT_DESC>& Get() const { return m_elems; }
  UINT Count() const { return (UINT)m_elems.size(); }

private:
  std::vector<D3D11_INPUT_ELEMENT_DESC> m_elems;
};
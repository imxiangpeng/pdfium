// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_barcode.h"

#include "xfa/fgas/font/fgas_gefont.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfx_barcode.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

IFWL_Barcode::IFWL_Barcode(const IFWL_App* app,
                           const CFWL_WidgetImpProperties& properties)
    : IFWL_Edit(app, properties, nullptr), m_dwStatus(0), m_type(BC_UNKNOWN) {}

IFWL_Barcode::~IFWL_Barcode() {}

FWL_Type IFWL_Barcode::GetClassID() const {
  return FWL_Type::Barcode;
}

void IFWL_Barcode::Initialize() {
  if (!m_pDelegate)
    m_pDelegate = new CFWL_BarcodeImpDelegate(this);

  IFWL_Edit::Initialize();
}

void IFWL_Barcode::Finalize() {
  delete m_pDelegate;
  m_pDelegate = nullptr;
  m_pBarcodeEngine.reset();
  IFWL_Edit::Finalize();
}

FWL_Error IFWL_Barcode::Update() {
  if (IsLocked()) {
    return FWL_Error::Indefinite;
  }
  FWL_Error ret = IFWL_Edit::Update();
  GenerateBarcodeImageCache();
  return ret;
}

FWL_Error IFWL_Barcode::DrawWidget(CFX_Graphics* pGraphics,
                                   const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_Error::Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_Error::Indefinite;
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0) {
    GenerateBarcodeImageCache();
    if (!m_pBarcodeEngine || (m_dwStatus & XFA_BCS_EncodeSuccess) == 0) {
      return FWL_Error::Succeeded;
    }
    CFX_Matrix mt;
    mt.e = m_rtClient.left;
    mt.f = m_rtClient.top;
    if (pMatrix) {
      mt.Concat(*pMatrix);
    }
    int32_t errorCode = 0;
    if (!m_pBarcodeEngine->RenderDevice(pGraphics->GetRenderDevice(), pMatrix,
                                        errorCode)) {
      return FWL_Error::Indefinite;
    }
    return FWL_Error::Succeeded;
  }
  return IFWL_Edit::DrawWidget(pGraphics, pMatrix);
}
void IFWL_Barcode::GenerateBarcodeImageCache() {
  if ((m_dwStatus & XFA_BCS_NeedUpdate) == 0)
    return;
  m_dwStatus = 0;
  CreateBarcodeEngine();
  IFWL_BarcodeDP* pData =
      static_cast<IFWL_BarcodeDP*>(m_pProperties->m_pDataProvider);
  if (!pData)
    return;
  if (!m_pBarcodeEngine)
    return;
  CFX_WideString wsText;
  if (GetText(wsText) != FWL_Error::Succeeded)
    return;
  CFWL_ThemePart part;
  part.m_pWidget = this;
  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  CFGAS_GEFont* pFont = static_cast<CFGAS_GEFont*>(
      pTheme->GetCapacity(&part, CFWL_WidgetCapacity::Font));
  CFX_Font* pCXFont = pFont ? pFont->GetDevFont() : nullptr;
  if (pCXFont) {
    m_pBarcodeEngine->SetFont(pCXFont);
  }
  FX_FLOAT* pFontSize = static_cast<FX_FLOAT*>(
      pTheme->GetCapacity(&part, CFWL_WidgetCapacity::FontSize));
  if (pFontSize) {
    m_pBarcodeEngine->SetFontSize(*pFontSize);
  }
  FX_ARGB* pFontColor = static_cast<FX_ARGB*>(
      pTheme->GetCapacity(&part, CFWL_WidgetCapacity::TextColor));
  if (pFontColor) {
    m_pBarcodeEngine->SetFontColor(*pFontColor);
  }
  m_pBarcodeEngine->SetHeight(int32_t(m_rtClient.height));
  m_pBarcodeEngine->SetWidth(int32_t(m_rtClient.width));
  uint32_t dwAttributeMask = pData->GetBarcodeAttributeMask();
  if (dwAttributeMask & FWL_BCDATTRIBUTE_CHARENCODING) {
    m_pBarcodeEngine->SetCharEncoding(pData->GetCharEncoding());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_MODULEHEIGHT) {
    m_pBarcodeEngine->SetModuleHeight(pData->GetModuleHeight());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_MODULEWIDTH) {
    m_pBarcodeEngine->SetModuleWidth(pData->GetModuleWidth());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_DATALENGTH) {
    m_pBarcodeEngine->SetDataLength(pData->GetDataLength());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_CALCHECKSUM) {
    m_pBarcodeEngine->SetCalChecksum(pData->GetCalChecksum());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_PRINTCHECKSUM) {
    m_pBarcodeEngine->SetPrintChecksum(pData->GetPrintChecksum());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_TEXTLOCATION) {
    m_pBarcodeEngine->SetTextLocation(pData->GetTextLocation());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_WIDENARROWRATIO) {
    m_pBarcodeEngine->SetWideNarrowRatio(pData->GetWideNarrowRatio());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_STARTCHAR) {
    m_pBarcodeEngine->SetStartChar(pData->GetStartChar());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_ENDCHAR) {
    m_pBarcodeEngine->SetEndChar(pData->GetEndChar());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_VERSION) {
    m_pBarcodeEngine->SetVersion(pData->GetVersion());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_ECLEVEL) {
    m_pBarcodeEngine->SetErrorCorrectionLevel(pData->GetErrorCorrectionLevel());
  }
  if (dwAttributeMask & FWL_BCDATTRIBUTE_TRUNCATED) {
    m_pBarcodeEngine->SetTruncated(pData->GetTruncated());
  }
  int32_t errorCode = 0;
  m_dwStatus = m_pBarcodeEngine->Encode(wsText.AsStringC(), TRUE, errorCode)
                   ? XFA_BCS_EncodeSuccess
                   : 0;
}

void IFWL_Barcode::CreateBarcodeEngine() {
  if (m_pBarcodeEngine || m_type == BC_UNKNOWN)
    return;

  std::unique_ptr<CFX_Barcode> pBarcode(new CFX_Barcode);
  if (pBarcode->Create(m_type))
    m_pBarcodeEngine = std::move(pBarcode);
}

void IFWL_Barcode::SetType(BC_TYPE type) {
  if (m_type == type)
    return;

  m_pBarcodeEngine.reset();
  m_type = type;
  m_dwStatus = XFA_BCS_NeedUpdate;
}
FWL_Error IFWL_Barcode::SetText(const CFX_WideString& wsText) {
  m_pBarcodeEngine.reset();
  m_dwStatus = XFA_BCS_NeedUpdate;
  return IFWL_Edit::SetText(wsText);
}
FX_BOOL IFWL_Barcode::IsProtectedType() {
  if (!m_pBarcodeEngine) {
    return TRUE;
  }
  BC_TYPE tEngineType = m_pBarcodeEngine->GetType();
  if (tEngineType == BC_QR_CODE || tEngineType == BC_PDF417 ||
      tEngineType == BC_DATAMATRIX) {
    return TRUE;
  }
  return FALSE;
}

CFWL_BarcodeImpDelegate::CFWL_BarcodeImpDelegate(IFWL_Barcode* pOwner)
    : CFWL_EditImpDelegate(pOwner) {}

void CFWL_BarcodeImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetClassID() == CFWL_EventType::TextChanged) {
    IFWL_Barcode* pOwner = static_cast<IFWL_Barcode*>(m_pOwner);
    pOwner->m_pBarcodeEngine.reset();
    pOwner->m_dwStatus = XFA_BCS_NeedUpdate;
  }
  CFWL_EditImpDelegate::OnProcessEvent(pEvent);
}
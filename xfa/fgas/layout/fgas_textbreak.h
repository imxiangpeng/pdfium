// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_FGAS_TEXTBREAK_H_
#define XFA_FGAS_LAYOUT_FGAS_TEXTBREAK_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_char.h"
#include "core/fxge/cfx_renderdevice.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_txtedtpage.h"
#include "xfa/fgas/layout/cfx_break.h"

class CFGAS_GEFont;
struct FDE_TEXTEDITPIECE;

#define FX_TXTCHARSTYLE_ArabicShadda 0x0020
#define FX_TXTCHARSTYLE_OddBidiLevel 0x0040

enum CFX_TxtLineAlignment {
  CFX_TxtLineAlignment_Left = 0,
  CFX_TxtLineAlignment_Center = 1 << 0,
  CFX_TxtLineAlignment_Right = 1 << 1,
  CFX_TxtLineAlignment_Justified = 1 << 2
};

inline bool CFX_BreakTypeNoneOrPiece(CFX_BreakType type) {
  return type == CFX_BreakType::None || type == CFX_BreakType::Piece;
}

struct FX_TXTRUN {
  FX_TXTRUN();
  FX_TXTRUN(const FX_TXTRUN& other);
  ~FX_TXTRUN();

  CFDE_TxtEdtPage* pAccess;
  const FDE_TEXTEDITPIECE* pIdentity;
  CFX_WideString wsStr;
  int32_t* pWidths;
  int32_t iLength;
  CFX_RetainPtr<CFGAS_GEFont> pFont;
  float fFontSize;
  uint32_t dwStyles;
  int32_t iHorizontalScale;
  int32_t iVerticalScale;
  uint32_t dwCharStyles;
  const CFX_RectF* pRect;
  wchar_t wLineBreakChar;
  bool bSkipSpace;
};

class CFX_TxtBreak : public CFX_Break {
 public:
  CFX_TxtBreak();
  ~CFX_TxtBreak() override;

  void SetLineWidth(float fLineWidth);
  void SetAlignment(int32_t iAlignment);
  void SetCombWidth(float fCombWidth);
  CFX_BreakType EndBreak(CFX_BreakType dwStatus);

  int32_t GetDisplayPos(const FX_TXTRUN* pTxtRun,
                        FXTEXT_CHARPOS* pCharPos,
                        bool bCharCode = false,
                        CFX_WideString* pWSForms = nullptr) const;
  std::vector<CFX_RectF> GetCharRects(const FX_TXTRUN* pTxtRun,
                                      bool bCharBBox = false) const;
  CFX_BreakType AppendChar(wchar_t wch);

 private:
  void AppendChar_Combination(CFX_Char* pCurChar);
  void AppendChar_Tab(CFX_Char* pCurChar);
  void AppendChar_PageLoad(CFX_Char* pCurChar, uint32_t dwProps);
  CFX_BreakType AppendChar_Control(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Arabic(CFX_Char* pCurChar);
  CFX_BreakType AppendChar_Others(CFX_Char* pCurChar);

  void ResetContextCharStyles();
  bool EndBreak_SplitLine(CFX_BreakLine* pNextLine, bool bAllChars);
  void EndBreak_BidiLine(std::deque<FX_TPO>* tpos, CFX_BreakType dwStatus);
  void EndBreak_Alignment(const std::deque<FX_TPO>& tpos,
                          bool bAllChars,
                          CFX_BreakType dwStatus);
  int32_t GetBreakPos(std::vector<CFX_Char>& ca,
                      int32_t& iEndPos,
                      bool bAllChars = false,
                      bool bOnlyBrk = false);
  void SplitTextLine(CFX_BreakLine* pCurLine,
                     CFX_BreakLine* pNextLine,
                     bool bAllChars = false);

  int32_t m_iAlignment;
  int32_t m_iCombWidth;
};

#endif  // XFA_FGAS_LAYOUT_FGAS_TEXTBREAK_H_

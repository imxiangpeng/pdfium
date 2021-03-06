// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_COMBOBOX_H_
#define XFA_FWL_CFWL_COMBOBOX_H_

#include <memory>

#include "xfa/fwl/cfwl_comboboxproxy.h"
#include "xfa/fwl/cfwl_comboedit.h"
#include "xfa/fwl/cfwl_combolist.h"
#include "xfa/fwl/cfwl_form.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFWL_WidgetProperties;
class CFWL_ComboBox;
class CFWL_ComboBoxProxy;
class CFWL_FormProxy;
class CFWL_ListBox;
class CFWL_Widget;

#define FWL_STYLEEXT_CMB_DropDown (1L << 0)
#define FWL_STYLEEXT_CMB_Sort (1L << 1)
#define FWL_STYLEEXT_CMB_OwnerDraw (1L << 3)
#define FWL_STYLEEXT_CMB_EditHNear 0
#define FWL_STYLEEXT_CMB_EditHCenter (1L << 4)
#define FWL_STYLEEXT_CMB_EditVNear 0
#define FWL_STYLEEXT_CMB_EditVCenter (1L << 6)
#define FWL_STYLEEXT_CMB_EditVFar (2L << 6)
#define FWL_STYLEEXT_CMB_EditJustified (1L << 8)
#define FWL_STYLEEXT_CMB_EditHAlignMask (3L << 4)
#define FWL_STYLEEXT_CMB_EditVAlignMask (3L << 6)
#define FWL_STYLEEXT_CMB_ListItemLeftAlign 0
#define FWL_STYLEEXT_CMB_ListItemCenterAlign (1L << 10)
#define FWL_STYLEEXT_CMB_ListItemAlignMask (3L << 10)
#define FWL_STYLEEXT_CMB_ReadOnly (1L << 13)

class CFWL_ComboBox : public CFWL_Widget {
 public:
  explicit CFWL_ComboBox(const CFWL_App* pApp);
  ~CFWL_ComboBox() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void ModifyStylesEx(uint32_t dwStylesExAdded,
                      uint32_t dwStylesExRemoved) override;
  void SetStates(uint32_t dwStates) override;
  void RemoveStates(uint32_t dwStates) override;
  void Update() override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) override;
  void SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix) override;

  CFX_WideString GetTextByIndex(int32_t iIndex) const;
  int32_t GetCurSel() const { return m_iCurSel; }
  void SetCurSel(int32_t iSel);

  void AddString(const CFX_WideStringC& wsText);
  void RemoveAt(int32_t iIndex);
  void RemoveAll();

  void SetEditText(const CFX_WideString& wsText);
  CFX_WideString GetEditText() const;

  void OpenDropDownList(bool bActivate);

  bool EditCanUndo() const { return m_pEdit->CanUndo(); }
  bool EditCanRedo() const { return m_pEdit->CanRedo(); }
  bool EditUndo() { return m_pEdit->Undo(); }
  bool EditRedo() { return m_pEdit->Redo(); }
  bool EditCanCopy() const { return m_pEdit->CountSelRanges() > 0; }
  bool EditCanCut() const {
    if (m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly)
      return false;
    return EditCanCopy();
  }
  bool EditCanSelectAll() const { return m_pEdit->GetTextLength() > 0; }
  bool EditCopy(CFX_WideString& wsCopy) const { return m_pEdit->Copy(wsCopy); }
  bool EditCut(CFX_WideString& wsCut) { return m_pEdit->Cut(wsCut); }
  bool EditPaste(const CFX_WideString& wsPaste) {
    return m_pEdit->Paste(wsPaste);
  }
  void EditSelectAll() { m_pEdit->AddSelRange(0); }
  void EditDelete() { m_pEdit->ClearText(); }
  void EditDeSelect() { m_pEdit->ClearSelections(); }

  CFX_RectF GetBBox() const;
  void EditModifyStylesEx(uint32_t dwStylesExAdded, uint32_t dwStylesExRemoved);

  void DrawStretchHandler(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix);
  bool IsDropListVisible() const {
    return m_pComboBoxProxy &&
           !(m_pComboBoxProxy->GetStates() & FWL_WGTSTATE_Invisible);
  }
  void ShowDropList(bool bActivate);

  CFWL_ComboEdit* GetComboEdit() const { return m_pEdit.get(); }

  void ProcessSelChanged(bool bLButtonUp);
  int32_t GetCurrentSelection() const { return m_iCurSel; }

 private:
  bool IsDropDownStyle() const {
    return !!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_DropDown);
  }
  void MatchEditText();
  void SyncEditText(int32_t iListItem);
  void Layout();
  void ResetTheme();
  void ResetEditAlignment();
  void ResetListItemAlignment();
  void InitProxyForm();
  void OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  void OnMouseMove(CFWL_MessageMouse* pMsg);
  void OnMouseLeave(CFWL_MessageMouse* pMsg);
  void OnKey(CFWL_MessageKey* pMsg);
  void DoSubCtrlKey(CFWL_MessageKey* pMsg);

  void DisForm_InitComboList();
  void DisForm_InitComboEdit();
  void DisForm_ShowDropList(bool bActivate);
  bool DisForm_IsDropListVisible() const {
    return !(m_pListBox->GetStates() & FWL_WGTSTATE_Invisible);
  }
  void DisForm_ModifyStylesEx(uint32_t dwStylesExAdded,
                              uint32_t dwStylesExRemoved);
  void DisForm_Update();
  FWL_WidgetHit DisForm_HitTest(const CFX_PointF& point);
  void DisForm_DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix);
  CFX_RectF DisForm_GetBBox() const;
  void DisForm_Layout();
  void DisForm_OnProcessMessage(CFWL_Message* pMessage);
  void DisForm_OnLButtonDown(CFWL_MessageMouse* pMsg);
  void DisForm_OnFocusChanged(CFWL_Message* pMsg, bool bSet);
  void DisForm_OnKey(CFWL_MessageKey* pMsg);

  CFX_RectF m_rtClient;
  CFX_RectF m_rtContent;
  CFX_RectF m_rtBtn;
  CFX_RectF m_rtList;
  CFX_RectF m_rtProxy;
  CFX_RectF m_rtHandler;
  std::unique_ptr<CFWL_ComboEdit> m_pEdit;
  std::unique_ptr<CFWL_ComboList> m_pListBox;
  CFWL_ComboBoxProxy* m_pComboBoxProxy;  // Can this be a unique_ptr?
  bool m_bLButtonDown;
  int32_t m_iCurSel;
  int32_t m_iBtnState;
};

#endif  // XFA_FWL_CFWL_COMBOBOX_H_

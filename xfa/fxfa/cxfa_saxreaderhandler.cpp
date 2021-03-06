// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_saxreaderhandler.h"

#include "xfa/fxfa/cxfa_checksumcontext.h"

CXFA_SAXReaderHandler::CXFA_SAXReaderHandler(CXFA_ChecksumContext* pContext)
    : m_pContext(pContext) {
  ASSERT(m_pContext);
}

CXFA_SAXReaderHandler::~CXFA_SAXReaderHandler() {}

CXFA_SAXContext* CXFA_SAXReaderHandler::OnTagEnter(
    const CFX_ByteStringC& bsTagName,
    CFX_SAXItem::Type eType,
    uint32_t dwStartPos) {
  UpdateChecksum(true);
  if (eType != CFX_SAXItem::Type::Tag &&
      eType != CFX_SAXItem::Type::Instruction) {
    return nullptr;
  }

  m_SAXContext.m_eNode = eType;
  CFX_ByteTextBuf& textBuf = m_SAXContext.m_TextBuf;
  textBuf << "<";
  if (eType == CFX_SAXItem::Type::Instruction)
    textBuf << "?";

  textBuf << bsTagName;
  m_SAXContext.m_bsTagName = bsTagName;
  return &m_SAXContext;
}

void CXFA_SAXReaderHandler::OnTagAttribute(CXFA_SAXContext* pTag,
                                           const CFX_ByteStringC& bsAttri,
                                           const CFX_ByteStringC& bsValue) {
  if (!pTag)
    return;
  pTag->m_TextBuf << " " << bsAttri << "=\"" << bsValue << "\"";
}

void CXFA_SAXReaderHandler::OnTagBreak(CXFA_SAXContext* pTag) {
  if (!pTag)
    return;

  pTag->m_TextBuf << ">";
  UpdateChecksum(false);
}

void CXFA_SAXReaderHandler::OnTagData(CXFA_SAXContext* pTag,
                                      CFX_SAXItem::Type eType,
                                      const CFX_ByteStringC& bsData,
                                      uint32_t dwStartPos) {
  if (!pTag)
    return;

  CFX_ByteTextBuf& textBuf = pTag->m_TextBuf;
  if (eType == CFX_SAXItem::Type::CharData)
    textBuf << "<![CDATA[";

  textBuf << bsData;
  if (eType == CFX_SAXItem::Type::CharData)
    textBuf << "]]>";
}

void CXFA_SAXReaderHandler::OnTagClose(CXFA_SAXContext* pTag,
                                       uint32_t dwEndPos) {
  if (!pTag)
    return;

  CFX_ByteTextBuf& textBuf = pTag->m_TextBuf;
  if (pTag->m_eNode == CFX_SAXItem::Type::Instruction)
    textBuf << "?>";
  else if (pTag->m_eNode == CFX_SAXItem::Type::Tag)
    textBuf << "></" << pTag->m_bsTagName.AsStringC() << ">";

  UpdateChecksum(false);
}

void CXFA_SAXReaderHandler::OnTagEnd(CXFA_SAXContext* pTag,
                                     const CFX_ByteStringC& bsTagName,
                                     uint32_t dwEndPos) {
  if (!pTag)
    return;

  pTag->m_TextBuf << "</" << bsTagName << ">";
  UpdateChecksum(false);
}

void CXFA_SAXReaderHandler::OnTargetData(CXFA_SAXContext* pTag,
                                         CFX_SAXItem::Type eType,
                                         const CFX_ByteStringC& bsData,
                                         uint32_t dwStartPos) {
  if (!pTag && eType != CFX_SAXItem::Type::Comment)
    return;

  if (eType == CFX_SAXItem::Type::Comment) {
    m_SAXContext.m_TextBuf << "<!--" << bsData << "-->";
    UpdateChecksum(false);
  } else {
    pTag->m_TextBuf << " " << bsData;
  }
}

void CXFA_SAXReaderHandler::UpdateChecksum(bool bCheckSpace) {
  int32_t iLength = m_SAXContext.m_TextBuf.GetLength();
  if (iLength < 1)
    return;

  uint8_t* pBuffer = m_SAXContext.m_TextBuf.GetBuffer();
  bool bUpdata = true;
  if (bCheckSpace) {
    bUpdata = false;
    for (int32_t i = 0; i < iLength; i++) {
      bUpdata = (pBuffer[i] > 0x20);
      if (bUpdata)
        break;
    }
  }
  if (bUpdata)
    m_pContext->Update(CFX_ByteStringC(pBuffer, iLength));

  m_SAXContext.m_TextBuf.Clear();
}

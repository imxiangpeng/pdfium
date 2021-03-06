// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_STRING_H_
#define CORE_FXCRT_FX_STRING_H_

#include <stdint.h>  // For intptr_t.

#include <algorithm>
#include <functional>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_string_c_template.h"
#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"

class CFX_ByteString;
class CFX_WideString;

using CFX_ByteStringC = CFX_StringCTemplate<char>;
using CFX_WideStringC = CFX_StringCTemplate<wchar_t>;

#define FXBSTR_ID(c1, c2, c3, c4)                                      \
  (((uint32_t)c1 << 24) | ((uint32_t)c2 << 16) | ((uint32_t)c3 << 8) | \
   ((uint32_t)c4))

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_ByteString {
 public:
  using CharType = char;

  CFX_ByteString();
  CFX_ByteString(const CFX_ByteString& other);
  CFX_ByteString(CFX_ByteString&& other);

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_ByteString(char ch);
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_ByteString(const char* ptr);

  CFX_ByteString(const char* ptr, FX_STRSIZE len);
  CFX_ByteString(const uint8_t* ptr, FX_STRSIZE len);

  explicit CFX_ByteString(const CFX_ByteStringC& bstrc);
  CFX_ByteString(const CFX_ByteStringC& bstrc1, const CFX_ByteStringC& bstrc2);

  ~CFX_ByteString();

  void clear() { m_pData.Reset(); }

  static CFX_ByteString FromUnicode(const wchar_t* ptr, FX_STRSIZE len = -1);
  static CFX_ByteString FromUnicode(const CFX_WideString& str);

  // Explicit conversion to C-style string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const char* c_str() const { return m_pData ? m_pData->m_String : ""; }

  // Explicit conversion to uint8_t*.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const uint8_t* raw_str() const {
    return m_pData ? reinterpret_cast<const uint8_t*>(m_pData->m_String)
                   : nullptr;
  }

  // Explicit conversion to CFX_ByteStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_ByteStringC AsStringC() const {
    return CFX_ByteStringC(raw_str(), GetLength());
  }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  bool IsEmpty() const { return !GetLength(); }

  int Compare(const CFX_ByteStringC& str) const;
  bool EqualNoCase(const CFX_ByteStringC& str) const;

  bool operator==(const char* ptr) const;
  bool operator==(const CFX_ByteStringC& str) const;
  bool operator==(const CFX_ByteString& other) const;

  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_ByteStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_ByteString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_ByteString& str) const;

  const CFX_ByteString& operator=(const char* str);
  const CFX_ByteString& operator=(const CFX_ByteStringC& bstrc);
  const CFX_ByteString& operator=(const CFX_ByteString& stringSrc);

  const CFX_ByteString& operator+=(char ch);
  const CFX_ByteString& operator+=(const char* str);
  const CFX_ByteString& operator+=(const CFX_ByteString& str);
  const CFX_ByteString& operator+=(const CFX_ByteStringC& bstrc);

  uint8_t GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  uint8_t operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, char ch);
  FX_STRSIZE Insert(FX_STRSIZE index, char ch);
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const char* lpszFormat, ...);
  void FormatV(const char* lpszFormat, va_list argList);

  void Reserve(FX_STRSIZE len);
  char* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len = -1);

  CFX_ByteString Mid(FX_STRSIZE first) const;
  CFX_ByteString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  CFX_ByteString Left(FX_STRSIZE count) const;
  CFX_ByteString Right(FX_STRSIZE count) const;

  FX_STRSIZE Find(const CFX_ByteStringC& lpszSub, FX_STRSIZE start = 0) const;
  FX_STRSIZE Find(char ch, FX_STRSIZE start = 0) const;
  FX_STRSIZE ReverseFind(char ch) const;

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(char chTarget);
  void TrimRight(const CFX_ByteStringC& lpszTargets);

  void TrimLeft();
  void TrimLeft(char chTarget);
  void TrimLeft(const CFX_ByteStringC& lpszTargets);

  FX_STRSIZE Replace(const CFX_ByteStringC& lpszOld,
                     const CFX_ByteStringC& lpszNew);

  FX_STRSIZE Remove(char ch);

  CFX_WideString UTF8Decode() const;

  uint32_t GetID(FX_STRSIZE start_pos = 0) const;

#define FXFORMAT_SIGNED 1
#define FXFORMAT_HEX 2
#define FXFORMAT_CAPITAL 4

  static CFX_ByteString FormatInteger(int i, uint32_t flags = 0);
  static CFX_ByteString FormatFloat(float f, int precision = 0);

 protected:
  using StringData = CFX_StringDataTemplate<char>;

  void ReallocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocCopy(CFX_ByteString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const char* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const char* lpszSrcData, FX_STRSIZE nSrcLen);

  CFX_RetainPtr<StringData> m_pData;

  friend class fxcrt_ByteStringConcat_Test;
  friend class fxcrt_ByteStringPool_Test;
};

inline bool operator==(const char* lhs, const CFX_ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_ByteStringC& lhs, const CFX_ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const char* lhs, const CFX_ByteString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_ByteStringC& lhs, const CFX_ByteString& rhs) {
  return rhs != lhs;
}

inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1, const char* str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const char* str1, const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1, char ch) {
  return CFX_ByteString(str1, CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(char ch, const CFX_ByteStringC& str2) {
  return CFX_ByteString(ch, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1, char ch) {
  return CFX_ByteString(str1.AsStringC(), CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(char ch, const CFX_ByteString& str2) {
  return CFX_ByteString(ch, str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1, const char* str2) {
  return CFX_ByteString(str1.AsStringC(), str2);
}
inline CFX_ByteString operator+(const char* str1, const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1.AsStringC(), str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2.AsStringC());
}

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_WideString {
 public:
  using CharType = wchar_t;

  CFX_WideString();
  CFX_WideString(const CFX_WideString& other);
  CFX_WideString(CFX_WideString&& other);

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(wchar_t ch);
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_WideString(const wchar_t* ptr);

  CFX_WideString(const wchar_t* ptr, FX_STRSIZE len);

  explicit CFX_WideString(const CFX_WideStringC& str);
  CFX_WideString(const CFX_WideStringC& str1, const CFX_WideStringC& str2);

  ~CFX_WideString();

  static CFX_WideString FromLocal(const CFX_ByteStringC& str);
  static CFX_WideString FromCodePage(const CFX_ByteStringC& str,
                                     uint16_t codepage);

  static CFX_WideString FromUTF8(const CFX_ByteStringC& str);
  static CFX_WideString FromUTF16LE(const unsigned short* str, FX_STRSIZE len);

  static FX_STRSIZE WStringLength(const unsigned short* str);

  // Explicit conversion to C-style wide string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const wchar_t* c_str() const { return m_pData ? m_pData->m_String : L""; }

  // Explicit conversion to CFX_WideStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(c_str(), GetLength());
  }

  void clear() { m_pData.Reset(); }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  bool IsEmpty() const { return !GetLength(); }

  const CFX_WideString& operator=(const wchar_t* str);
  const CFX_WideString& operator=(const CFX_WideString& stringSrc);
  const CFX_WideString& operator=(const CFX_WideStringC& stringSrc);

  const CFX_WideString& operator+=(const wchar_t* str);
  const CFX_WideString& operator+=(wchar_t ch);
  const CFX_WideString& operator+=(const CFX_WideString& str);
  const CFX_WideString& operator+=(const CFX_WideStringC& str);

  bool operator==(const wchar_t* ptr) const;
  bool operator==(const CFX_WideStringC& str) const;
  bool operator==(const CFX_WideString& other) const;

  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_WideString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_WideString& str) const;

  wchar_t GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  wchar_t operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, wchar_t ch);

  int Compare(const wchar_t* str) const;
  int Compare(const CFX_WideString& str) const;
  int CompareNoCase(const wchar_t* str) const;

  CFX_WideString Mid(FX_STRSIZE first) const;
  CFX_WideString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  CFX_WideString Left(FX_STRSIZE count) const;
  CFX_WideString Right(FX_STRSIZE count) const;

  FX_STRSIZE Insert(FX_STRSIZE index, wchar_t ch);
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const wchar_t* lpszFormat, ...);
  void FormatV(const wchar_t* lpszFormat, va_list argList);

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(wchar_t chTarget);
  void TrimRight(const CFX_WideStringC& pTargets);

  void TrimLeft();
  void TrimLeft(wchar_t chTarget);
  void TrimLeft(const CFX_WideStringC& pTargets);

  void Reserve(FX_STRSIZE len);
  wchar_t* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len = -1);

  int GetInteger() const;
  float GetFloat() const;

  FX_STRSIZE Find(const CFX_WideStringC& pSub, FX_STRSIZE start = 0) const;
  FX_STRSIZE Find(wchar_t ch, FX_STRSIZE start = 0) const;
  FX_STRSIZE Replace(const CFX_WideStringC& pOld, const CFX_WideStringC& pNew);
  FX_STRSIZE Remove(wchar_t ch);

  CFX_ByteString UTF8Encode() const;
  CFX_ByteString UTF16LE_Encode() const;

 protected:
  using StringData = CFX_StringDataTemplate<wchar_t>;

  void ReallocBeforeWrite(FX_STRSIZE nLen);
  void AllocBeforeWrite(FX_STRSIZE nLen);
  void AllocCopy(CFX_WideString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const wchar_t* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const wchar_t* lpszSrcData, FX_STRSIZE nSrcLen);

  CFX_RetainPtr<StringData> m_pData;

  friend class fxcrt_WideStringConcatInPlace_Test;
  friend class fxcrt_WideStringPool_Test;
};

inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const wchar_t* str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const wchar_t* str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1, wchar_t ch) {
  return CFX_WideString(str1, CFX_WideStringC(ch));
}
inline CFX_WideString operator+(wchar_t ch, const CFX_WideStringC& str2) {
  return CFX_WideString(ch, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1, wchar_t ch) {
  return CFX_WideString(str1.AsStringC(), CFX_WideStringC(ch));
}
inline CFX_WideString operator+(wchar_t ch, const CFX_WideString& str2) {
  return CFX_WideString(ch, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const wchar_t* str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const wchar_t* str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline bool operator==(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}

CFX_ByteString FX_UTF8Encode(const CFX_WideStringC& wsStr);
float FX_atof(const CFX_ByteStringC& str);
inline float FX_atof(const CFX_WideStringC& wsStr) {
  return FX_atof(FX_UTF8Encode(wsStr).c_str());
}
bool FX_atonum(const CFX_ByteStringC& str, void* pData);
FX_STRSIZE FX_ftoa(float f, char* buf);

uint32_t FX_HashCode_GetA(const CFX_ByteStringC& str, bool bIgnoreCase);
uint32_t FX_HashCode_GetW(const CFX_WideStringC& str, bool bIgnoreCase);

namespace std {

template <>
struct hash<CFX_ByteString> {
  std::size_t operator()(const CFX_ByteString& str) const {
    return FX_HashCode_GetA(str.AsStringC(), false);
  }
};

template <>
struct hash<CFX_WideString> {
  std::size_t operator()(const CFX_WideString& str) const {
    return FX_HashCode_GetW(str.AsStringC(), false);
  }
};

}  // namespace std

extern template struct std::hash<CFX_ByteString>;
extern template struct std::hash<CFX_WideString>;

#endif  // CORE_FXCRT_FX_STRING_H_

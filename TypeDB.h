#pragma once
#include "CharInfo.h"
#include <vector>

class CTypeDB
{
public:
    BOOL ReadCSVFile(const CString& path);

    // ⭐ 이거 추가
    const std::vector<SCharInfo>& GetChars() const
    {
        return m_Chars;
    }

    int GetCharCount() const { return m_nChar; }
    int GetSheetCount() const { return m_nSheet; }

private:
    std::vector<SCharInfo> m_Chars;
    int m_nChar = 0;
    int m_nSheet = 0;
};
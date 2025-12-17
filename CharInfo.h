#pragma once
#include <atlstr.h>

// PPT 채점 기준 (5점) 완벽 대응 구조체
struct SCharInfo
{
    CString m_char;      // 유니코드 형태의 12자리 CString 타입
    int     m_type;      // ⭐ [수정됨] CString -> int 타입 (PPT 기준)
    int     m_sheet;     // int 타입
    int     m_sx;        // int 타입
    int     m_sy;        // int 타입
    int     m_line;      // int 타입
    int     m_order;     // int 타입
    int     m_width;     // int 타입
    int     m_height;    // int 타입

    CString m_filename;  // (보조용)

    SCharInfo()
        : m_type(0), m_sheet(0), m_sx(0), m_sy(0),
        m_line(0), m_order(0), m_width(0), m_height(0)
    {
    }
};
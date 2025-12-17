#pragma once
#include <atlstr.h>

struct SCharInfo
{
    CString m_char;      // 글자 코드 (110A11A10000)
    int     m_type;
    int     m_sheet;
    int     m_sx;
    int     m_sy;
    int     m_line;
    int     m_order;
    int     m_width;
    int     m_height;

    CString m_filename;

    SCharInfo()
        : m_type(0), m_sheet(0), m_sx(0), m_sy(0),
        m_line(0), m_order(0), m_width(0), m_height(0)
    {
    }
};

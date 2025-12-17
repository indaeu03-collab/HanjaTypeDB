#include "pch.h"
#include "TypeDB.h"
#include <fstream>
#include <sstream>

BOOL CTypeDB::ReadCSVFile(const CString& path)
{
    m_Chars.clear();
    m_nSheet = 0;

    std::string filePath = CT2A(path);
    std::ifstream file(filePath);
    if (!file.is_open())
        return FALSE;

    std::string line;
    std::getline(file, line); // 헤더 스킵

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        SCharInfo c;

        // 1. 글자 (CString)
        std::getline(ss, token, ','); c.m_char = token.c_str();

        // 2. 자료번호 (⭐ int로 변환)
        std::getline(ss, token, ',');
        c.m_type = atoi(token.c_str()); // PPT 기준에 맞춰 int로 변환

        // 3. 나머지 정보들 (int)
        std::getline(ss, token, ','); c.m_sheet = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_sx = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_sy = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_line = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_order = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_width = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_height = atoi(token.c_str());

        // 4. 파일명
        std::getline(ss, token, ','); c.m_filename = token.c_str();

        m_Chars.push_back(c);

        if (c.m_sheet > m_nSheet)
            m_nSheet = c.m_sheet;
    }

    m_nChar = (int)m_Chars.size();
    return TRUE;
}
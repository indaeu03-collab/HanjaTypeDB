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
    std::getline(file, line); // Çì´õ ½ºÅµ

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        SCharInfo c;

        std::getline(ss, token, ','); c.m_char = token.c_str();
        std::getline(ss, token, ','); c.m_type = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_sheet = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_sx = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_sy = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_line = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_order = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_width = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_height = atoi(token.c_str());
        std::getline(ss, token, ','); c.m_filename = token.c_str();

        m_Chars.push_back(c);

        if (c.m_sheet > m_nSheet)
            m_nSheet = c.m_sheet;
    }

    m_nChar = (int)m_Chars.size();
    return TRUE;
}

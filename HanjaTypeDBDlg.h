#pragma once

#include "TypeDB.h"
#include <atlimage.h>

class CHanjaTypeDBDlg : public CDialogEx
{
public:
    CHanjaTypeDBDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_HANJATYPEDB_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedButtonOpen();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()

private:
    // 아이콘
    HICON m_hIcon = nullptr;

    // DB
    CTypeDB m_db;

    // 경로 / 상태
    CString m_bookPath;
    int     m_curSheet = 1;
    int     m_selectedIndex = -1;

    // 스캔 장 이미지
    CImage   m_sheetImage;
    CStatic* m_pSheetCtrl = nullptr;

    // 선택 글자 이미지
    CImage   m_charImage;
    CStatic* m_pCharCtrl = nullptr;

    // 정보 표시
    CStatic* m_pInfoCtrl = nullptr;

    // 구성 글자 리스트
    CListCtrl m_listCompose;

private:
    // 내부 함수
    CString GetBookRoot() const;
    void LoadSheetImage();
    void UpdateSelectedInfo();
    void ShowComposeList(const CString& targetChar);
    void ShowSelectedCharImage();
};

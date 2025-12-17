#pragma once
#include "TypeDB.h"
#include <atlimage.h>
#include <set>

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
    afx_msg void OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult);

    // ⭐ [추가] 리스트 컨트롤 클릭 이벤트
    afx_msg void OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    HICON m_hIcon = nullptr;
    CTypeDB m_db;

    CString m_bookPath;
    int     m_curSheet = 1;
    int     m_selectedIndex = -1;

    // 메인 스캔 이미지
    CImage   m_sheetImage;
    CStatic* m_pSheetCtrl = nullptr;

    // 오른쪽 위 글자 이미지 (2D)
    CImage   m_charImage;
    CStatic* m_pCharCtrl = nullptr;

    // ⭐ [추가] 오른쪽 아래 선택 글자 이미지 컨트롤
    CStatic* m_pSelCharCtrl = nullptr;

    CStatic* m_pInfoCtrl = nullptr;
    CListCtrl m_listCompose;
    CSpinButtonCtrl m_spinSheet;
    CSpinButtonCtrl m_spinType;
    // ⭐ [추가] 책 이름 표시할 콤보박스
    CComboBox m_comboBook;

private:
    CString GetBookRoot() const;
    void LoadSheetImage();
    void UpdateSelectedInfo();
    void ShowComposeList(const CString& targetChar);
    void ShowSelectedCharImage();
    bool FindCharImageRecursive(const CString& root, CString& outPath);
    void UpdateStatistics();
    afx_msg void OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult);
};
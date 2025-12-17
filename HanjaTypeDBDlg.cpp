// HanjaTypeDBDlg.cpp

#include "pch.h"
#include "framework.h"
#include "HanjaTypeDB.h"
#include "HanjaTypeDBDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// =======================
// About Dialog
// =======================
class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

protected:
    virtual void DoDataExchange(CDataExchange* pDX)
    {
        CDialogEx::DoDataExchange(pDX);
    }
    DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// =======================
// Main Dialog
// =======================
CHanjaTypeDBDlg::CHanjaTypeDBDlg(CWnd* pParent)
    : CDialogEx(IDD_HANJATYPEDB_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHanjaTypeDBDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_CHARS, m_listCompose);
}

BEGIN_MESSAGE_MAP(CHanjaTypeDBDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_OPEN, &CHanjaTypeDBDlg::OnBnClickedButtonOpen)
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

BOOL CHanjaTypeDBDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    m_pSheetCtrl = (CStatic*)GetDlgItem(IDC_STATIC_SHEET);
    m_pCharCtrl = (CStatic*)GetDlgItem(IDC_STATIC_CHARIMG); // 🔥 네 ID
    m_pInfoCtrl = (CStatic*)GetDlgItem(IDC_STATIC_INFO);

    // 구성 글자 리스트
    m_listCompose.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listCompose.InsertColumn(0, _T("장"), LVCFMT_CENTER, 50);
    m_listCompose.InsertColumn(1, _T("행"), LVCFMT_CENTER, 50);
    m_listCompose.InsertColumn(2, _T("번"), LVCFMT_CENTER, 50);

    return TRUE;
}

void CHanjaTypeDBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlg;
        dlg.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// =======================
// CSV 로드 후 장 이미지
// =======================
void CHanjaTypeDBDlg::OnBnClickedButtonOpen()
{
    CFileDialog dlg(TRUE, _T("csv"), nullptr,
        OFN_FILEMUSTEXIST, _T("CSV 파일 (*.csv)|*.csv||"));

    if (dlg.DoModal() != IDOK)
        return;

    m_bookPath = dlg.GetPathName();

    if (!m_db.ReadCSVFile(m_bookPath))
    {
        AfxMessageBox(_T("CSV 로딩 실패"));
        return;
    }

    m_curSheet = 1;
    LoadSheetImage();
}

// =======================
// 장 이미지 + 박스
// =======================
void CHanjaTypeDBDlg::LoadSheetImage()
{
    if (m_bookPath.IsEmpty() || !m_pSheetCtrl)
        return;

    CString root = GetBookRoot();

    CString imgPath;
    imgPath.Format(
        _T("%s\\01_scan\\sheet_%03d.jpg"),
        root.GetString(),
        m_curSheet
    );

    if (!m_sheetImage.IsNull())
        m_sheetImage.Destroy();

    if (FAILED(m_sheetImage.Load(imgPath)))
    {
        AfxMessageBox(_T("스캔 장 이미지 로드 실패:\n") + imgPath);
        return;
    }

    CDC* pDC = m_pSheetCtrl->GetDC();
    CRect rc;
    m_pSheetCtrl->GetClientRect(&rc);

    CDC memDC;
    memDC.CreateCompatibleDC(pDC);

    CBitmap bmp;
    bmp.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());
    CBitmap* oldBmp = memDC.SelectObject(&bmp);

    memDC.FillSolidRect(rc, RGB(255, 255, 255));

    double sx = (double)rc.Width() / m_sheetImage.GetWidth();
    double sy = (double)rc.Height() / m_sheetImage.GetHeight();

    m_sheetImage.StretchBlt(
        memDC.m_hDC,
        0, 0,
        rc.Width(), rc.Height(),
        SRCCOPY
    );

    CPen redPen(PS_SOLID, 2, RGB(255, 0, 0));
    CPen bluePen(PS_SOLID, 3, RGB(0, 0, 255));

    CBrush* oldBrush =
        memDC.SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

    const auto& chars = m_db.GetChars();

    for (int i = 0; i < (int)chars.size(); ++i)
    {
        const auto& c = chars[i];
        if (c.m_sheet != m_curSheet) continue;

        int x = (int)(c.m_sx * sx);
        int y = (int)(c.m_sy * sy);
        int w = (int)(c.m_width * sx);
        int h = (int)(c.m_height * sy);

        CPen* oldPen =
            (i == m_selectedIndex)
            ? memDC.SelectObject(&bluePen)
            : memDC.SelectObject(&redPen);

        memDC.Rectangle(x, y, x + w, y + h);
        memDC.SelectObject(oldPen);
    }

    memDC.SelectObject(oldBrush);

    pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldBmp);
    m_pSheetCtrl->ReleaseDC(pDC);
}


// =======================
// 마우스 클릭
// =======================
void CHanjaTypeDBDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (!m_pSheetCtrl) return;

    CRect rc;
    m_pSheetCtrl->GetWindowRect(&rc);
    ScreenToClient(&rc);

    if (!rc.PtInRect(point)) return;

    point.x -= rc.left;
    point.y -= rc.top;

    double sx = (double)rc.Width() / m_sheetImage.GetWidth();
    double sy = (double)rc.Height() / m_sheetImage.GetHeight();

    m_selectedIndex = -1;

    const auto& chars = m_db.GetChars();
    for (int i = 0; i < (int)chars.size(); ++i)
    {
        const auto& c = chars[i];
        if (c.m_sheet != m_curSheet) continue;

        CRect box(
            (int)(c.m_sx * sx),
            (int)(c.m_sy * sy),
            (int)((c.m_sx + c.m_width) * sx),
            (int)((c.m_sy + c.m_height) * sy));

        if (box.PtInRect(point))
        {
            m_selectedIndex = i;
            break;
        }
    }

    LoadSheetImage();
    UpdateSelectedInfo();
    ShowSelectedCharImage();
}

// =======================
// 선택 글자 정보
// =======================
void CHanjaTypeDBDlg::UpdateSelectedInfo()
{
    if (!m_pInfoCtrl || m_selectedIndex < 0)
        return;

    const auto& c = m_db.GetChars()[m_selectedIndex];

    CString text;
    text.Format(_T("%s\n\n%d장 %d행 %d번"),
        c.m_char.GetString(),
        c.m_sheet,
        c.m_line,
        c.m_order);

    m_pInfoCtrl->SetWindowText(text);
    ShowComposeList(c.m_char);
}

// =======================
// 구성 글자 리스트
// =======================
void CHanjaTypeDBDlg::ShowComposeList(const CString& targetChar)
{
    m_listCompose.DeleteAllItems();

    int row = 0;
    for (const auto& c : m_db.GetChars())
    {
        if (c.m_char == targetChar)
        {
            CString sSheet, sLine, sOrder;
            sSheet.Format(_T("%d"), c.m_sheet);
            sLine.Format(_T("%d"), c.m_line);
            sOrder.Format(_T("%d"), c.m_order);

            m_listCompose.InsertItem(row, sSheet);
            m_listCompose.SetItemText(row, 1, sLine);
            m_listCompose.SetItemText(row, 2, sOrder);
            row++;
        }
    }
}

// =======================
// 선택 글자 이미지 (03_type)
// =======================
void CHanjaTypeDBDlg::ShowSelectedCharImage()
{
    if (!m_pCharCtrl || m_selectedIndex < 0)
        return;

    const auto& c = m_db.GetChars()[m_selectedIndex];

    CString root = GetBookRoot();

    CString imgPath;
    imgPath.Format(
        _T("%s\\03_type\\%s\\%d\\%s.png"),
        root.GetString(),
        c.m_char.GetString(),   // 110A11A10000
        c.m_line,               // 8 (폴더)
        c.m_filename.GetString()// 2_6190_3705
    );

    if (!m_charImage.IsNull())
        m_charImage.Destroy();

    if (FAILED(m_charImage.Load(imgPath)))
    {
        AfxMessageBox(_T("글자 이미지 없음:\n") + imgPath);
        return;
    }

    CDC* pDC = m_pCharCtrl->GetDC();
    CRect rc;
    m_pCharCtrl->GetClientRect(&rc);

    m_charImage.StretchBlt(
        pDC->m_hDC,
        0, 0,
        rc.Width(), rc.Height(),
        SRCCOPY
    );

    m_pCharCtrl->ReleaseDC(pDC);
}



CString CHanjaTypeDBDlg::GetBookRoot() const
{
    // bookDB.csv가 있는 폴더
    return m_bookPath.Left(m_bookPath.ReverseFind('\\'));
}




void CHanjaTypeDBDlg::OnPaint()
{
    CDialogEx::OnPaint();
}

HCURSOR CHanjaTypeDBDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

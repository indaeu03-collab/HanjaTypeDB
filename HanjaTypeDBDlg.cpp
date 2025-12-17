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
    DDX_Control(pDX, IDC_SPIN_SHEET, m_spinSheet);

    // ⭐ [수정됨] ID를 IDC_EDIT_BOOKNAME으로 변경했습니다!
    // (변수 이름 m_comboBook은 그대로 써도 상관없습니다)
    DDX_Control(pDX, IDC_EDIT_BOOKNAME, m_comboBook);
}

BEGIN_MESSAGE_MAP(CHanjaTypeDBDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_OPEN, &CHanjaTypeDBDlg::OnBnClickedButtonOpen)
    ON_WM_LBUTTONDOWN()
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHEET, &CHanjaTypeDBDlg::OnDeltaposSpinSheet)

    // ⭐ [추가] 리스트 클릭하면 함수 실행 연결
    ON_NOTIFY(NM_CLICK, IDC_LIST_CHARS, &CHanjaTypeDBDlg::OnNMClickListChars)
END_MESSAGE_MAP()

BOOL CHanjaTypeDBDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    // 컨트롤 포인터 가져오기 (기존 ID들)
    m_pSheetCtrl = (CStatic*)GetDlgItem(IDC_STATIC_SHEET);
    m_pCharCtrl = (CStatic*)GetDlgItem(IDC_STATIC_CHARIMG);
    m_pInfoCtrl = (CStatic*)GetDlgItem(IDC_STATIC_INFO);

    // ⭐ [수정됨] 요청하신 ID(IDC_STATIC_SELECTCHAR)로 연결했습니다!
    m_pSelCharCtrl = (CStatic*)GetDlgItem(IDC_STATIC_SELECTCHAR);

    // 리스트 설정
    m_listCompose.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_listCompose.InsertColumn(0, _T("장"), LVCFMT_CENTER, 40);
    m_listCompose.InsertColumn(1, _T("행"), LVCFMT_CENTER, 40);
    m_listCompose.InsertColumn(2, _T("번"), LVCFMT_CENTER, 40);

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

    // ==========================================
    // ⭐ 폴더 이름(책 이름) 추출해서 콤보박스에 넣기
    // ==========================================
    // 1. 파일 경로에서 폴더 경로만 떼어내기
    CString folderPath = m_bookPath.Left(m_bookPath.ReverseFind('\\'));

    // 2. 맨 마지막 폴더 이름 가져오기 (예: 월인천강지곡 권상)
    int nPos = folderPath.ReverseFind('\\');
    CString bookName = folderPath.Mid(nPos + 1);

    // 3. 콤보박스(IDC_EDIT_BOOKNAME)에 넣고 선택하기
    m_comboBook.ResetContent();      // 기존 내용 지우기
    m_comboBook.AddString(bookName); // 책 이름 추가
    m_comboBook.SetCurSel(0);        // 첫 번째 항목 선택

    // ==========================================

    m_curSheet = 1;
    LoadSheetImage();

    // 스핀 컨트롤 설정
    m_spinSheet.SetBuddy(GetDlgItem(IDC_EDIT_SHEET));
    m_spinSheet.SetRange(1, m_db.GetSheetCount());
    m_spinSheet.SetPos(1);

    SetDlgItemInt(IDC_EDIT_SHEET, 1);
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

    CPen redPen(PS_SOLID, 3, RGB(255, 0, 0));
    CPen greenPen(PS_SOLID, 2, RGB(0, 255, 0));

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
            ? memDC.SelectObject(&redPen)
            : memDC.SelectObject(&greenPen);

        memDC.Rectangle(x, y, x + w, y + h);
        memDC.SelectObject(oldPen);
    }

    memDC.SelectObject(oldBrush);

    pDC->BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);

    memDC.SelectObject(oldBmp);
    m_pSheetCtrl->ReleaseDC(pDC);
    UpdateStatistics();
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
    if (!m_pCharCtrl || m_selectedIndex < 0) return;

    const auto& c = m_db.GetChars()[m_selectedIndex];
    CString bookRoot = m_bookPath.Left(m_bookPath.ReverseFind('\\'));

    // 이미지 경로 찾기 (m_char 기준)
    CString charRoot;
    charRoot.Format(_T("%s\\03_type\\%s"), bookRoot.GetString(), c.m_char.GetString());

    CString foundImage;
    if (!FindCharImageRecursive(charRoot, foundImage))
    {
        m_pCharCtrl->SetWindowText(_T("이미지 없음"));
        return;
    }

    if (!m_charImage.IsNull()) m_charImage.Destroy();
    if (FAILED(m_charImage.Load(foundImage))) return;

    // (1) 우측 상단 표시에 그리기
    CDC* pDC = m_pCharCtrl->GetDC();
    CRect rc; m_pCharCtrl->GetClientRect(&rc);
    pDC->FillSolidRect(rc, RGB(255, 255, 255)); // 흰 배경
    m_charImage.StretchBlt(pDC->m_hDC, 0, 0, rc.Width(), rc.Height(), SRCCOPY);
    m_pCharCtrl->ReleaseDC(pDC);

    // (2) ⭐ [추가] 우측 하단 [선택 글자] 에도 똑같이 그리기
    if (m_pSelCharCtrl) {
        CDC* pSelDC = m_pSelCharCtrl->GetDC();
        CRect rcSel;
        m_pSelCharCtrl->GetClientRect(&rcSel);
        pSelDC->FillSolidRect(rcSel, RGB(255, 255, 255)); // 흰 배경
        m_charImage.StretchBlt(pSelDC->m_hDC, 0, 0, rcSel.Width(), rcSel.Height(), SRCCOPY);
        m_pSelCharCtrl->ReleaseDC(pSelDC);
    }
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

bool CHanjaTypeDBDlg::FindCharImageRecursive(
    const CString& root,
    CString& outPath
)
{
    CFileFind finder;
    BOOL bWorking = finder.FindFile(root + _T("\\*.*"));

    while (bWorking)
    {
        bWorking = finder.FindNextFile();

        if (finder.IsDots())
            continue;

        if (finder.IsDirectory())
        {
            // 🔁 하위 폴더 재귀 탐색
            if (FindCharImageRecursive(finder.GetFilePath(), outPath))
            {
                finder.Close();
                return true;
            }
        }
        else
        {
            CString name = finder.GetFileName();
            name.MakeLower();

            if (name.Right(4) == _T(".png"))
            {
                outPath = finder.GetFilePath();
                finder.Close();
                return true;
            }
        }
    }

    finder.Close();
    return false;
}

// =======================
// 스핀 컨트롤(화살표) 눌렀을 때 페이지 이동
// =======================
void CHanjaTypeDBDlg::OnDeltaposSpinSheet(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    if (m_db.GetSheetCount() <= 0)
    {
        *pResult = 0;
        return;
    }

    // 1. 페이지 계산
    int change = 0;
    if (pNMUpDown->iDelta < 0) change = 1;
    if (pNMUpDown->iDelta > 0) change = -1;

    int nextSheet = m_curSheet + change;

    if (nextSheet < 1) nextSheet = 1;
    if (nextSheet > m_db.GetSheetCount()) nextSheet = m_db.GetSheetCount();

    // 2. 페이지가 바뀌었을 때
    if (nextSheet != m_curSheet)
    {
        m_curSheet = nextSheet;

        // ⭐ [추가됨] 기존 선택(파란색) 정보 싹 지우기 (초기화)
        m_selectedIndex = -1;                 // 선택된 번호 없음으로 변경
        m_pInfoCtrl->SetWindowText(_T(""));   // 글자 정보 텍스트 지우기
        m_listCompose.DeleteAllItems();       // 리스트 목록 지우기

        // 오른쪽 글자 이미지도 흰색으로 덮어서 지우기
        if (m_pCharCtrl) {
            CDC* pDC = m_pCharCtrl->GetDC();
            CRect rc; m_pCharCtrl->GetClientRect(&rc);
            pDC->FillSolidRect(rc, RGB(255, 255, 255));
            m_pCharCtrl->ReleaseDC(pDC);
        }

        // 3. 화면 갱신
        LoadSheetImage();
        SetDlgItemInt(IDC_EDIT_SHEET, m_curSheet);
    }

    *pResult = 0;
}

// =======================
// 통계 계산 및 표시 함수
// =======================
void CHanjaTypeDBDlg::UpdateStatistics()
{
    // DB가 비어있으면 아무것도 안 함
    if (m_db.GetChars().empty()) return;

    const auto& chars = m_db.GetChars();

    // 1. 책 전체 통계용 (중복 제거를 위해 set 사용)
    std::set<CString> totalCharKinds; // 글자 종류 (가, 나, 다...)
    std::set<CString> totalTypeKinds; // 활자 종류 (자료번호 기준)

    // 2. 장내(현재 페이지) 통계용
    int sheetCount = 0;
    std::set<CString> sheetCharKinds;
    std::set<CString> sheetTypeKinds;

    for (const auto& c : chars)
    {
        // [책 전체] 무조건 넣기
        totalCharKinds.insert(c.m_char);
        totalTypeKinds.insert(c.m_type);

        // [장내] 현재 페이지랑 같으면 넣기
        if (c.m_sheet == m_curSheet)
        {
            sheetCount++;
            sheetCharKinds.insert(c.m_char);
            sheetTypeKinds.insert(c.m_type);
        }
    }

    // 화면에 글자 예쁘게 출력하기
    CString str;

    // [책 전체] 출력
    str.Format(_T("%d"), (int)chars.size());
    SetDlgItemText(IDC_STATIC_T_COUNT, str);

    str.Format(_T("%d"), (int)totalCharKinds.size());
    SetDlgItemText(IDC_STATIC_T_KIND, str);

    str.Format(_T("%d"), (int)totalTypeKinds.size());
    SetDlgItemText(IDC_STATIC_T_TYPE, str);

    // [장내] 출력
    str.Format(_T("%d"), sheetCount);
    SetDlgItemText(IDC_STATIC_S_COUNT, str);

    str.Format(_T("%d"), (int)sheetCharKinds.size());
    SetDlgItemText(IDC_STATIC_S_KIND, str);

    str.Format(_T("%d"), (int)sheetTypeKinds.size());
    SetDlgItemText(IDC_STATIC_S_TYPE, str);
}

// =======================
// 리스트 클릭 시 해당 글자로 이동
// =======================
void CHanjaTypeDBDlg::OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    // 선택된 줄이 없으면 무시
    int nItem = pNMItemActivate->iItem;
    if (nItem == -1) {
        *pResult = 0;
        return;
    }

    // 리스트에 적힌 장, 행, 번호 가져오기
    CString sSheet = m_listCompose.GetItemText(nItem, 0);
    CString sLine = m_listCompose.GetItemText(nItem, 1);
    CString sOrder = m_listCompose.GetItemText(nItem, 2);

    int targetSheet = _ttoi(sSheet);
    int targetLine = _ttoi(sLine);
    int targetOrder = _ttoi(sOrder);

    // 1. 다른 페이지면 페이지 이동
    if (targetSheet != m_curSheet)
    {
        m_curSheet = targetSheet;
        SetDlgItemInt(IDC_EDIT_SHEET, m_curSheet); // 스핀 컨트롤 옆 숫자 갱신
    }

    // 2. 해당 글자 찾아서 선택하기 (DB 검색)
    const auto& chars = m_db.GetChars();
    for (int i = 0; i < (int)chars.size(); ++i)
    {
        const auto& c = chars[i];
        if (c.m_sheet == targetSheet && c.m_line == targetLine && c.m_order == targetOrder)
        {
            m_selectedIndex = i; // 찾았다! 선택 인덱스 업데이트
            break;
        }
    }

    // 3. 화면 갱신
    LoadSheetImage();
    UpdateSelectedInfo();
    ShowSelectedCharImage();

    *pResult = 0;
}
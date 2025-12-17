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
    DDX_Control(pDX, IDC_EDIT_BOOKNAME, m_comboBook);
    DDX_Control(pDX, IDC_SPIN_TYPE, m_spinType);
}

BEGIN_MESSAGE_MAP(CHanjaTypeDBDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_OPEN, &CHanjaTypeDBDlg::OnBnClickedButtonOpen)
    ON_WM_LBUTTONDOWN()
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_SHEET, &CHanjaTypeDBDlg::OnDeltaposSpinSheet)
    ON_NOTIFY(NM_CLICK, IDC_LIST_CHARS, &CHanjaTypeDBDlg::OnNMClickListChars)

    // ⭐ [추가] 활자 스핀 컨트롤 클릭 연결
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TYPE, &CHanjaTypeDBDlg::OnDeltaposSpinType)
END_MESSAGE_MAP()

BOOL CHanjaTypeDBDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    // 컨트롤 포인터 가져오기
    m_pSheetCtrl = (CStatic*)GetDlgItem(IDC_STATIC_SHEET);
    m_pCharCtrl = (CStatic*)GetDlgItem(IDC_STATIC_CHARIMG);
    m_pInfoCtrl = (CStatic*)GetDlgItem(IDC_STATIC_INFO);
    m_pSelCharCtrl = (CStatic*)GetDlgItem(IDC_STATIC_SELECTCHAR);

    // ⭐ [수정됨] "구분" 칸 삭제 -> "장, 행, 번"만 표시
    m_listCompose.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    while (m_listCompose.DeleteColumn(0)); // 기존 컬럼 싹 지우기

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

    // ⭐ [핵심] 딱 이 한 줄이면 됩니다!
    // 이 변수가 있는 동안 마우스가 '모래시계(대기)' 모양으로 고정됩니다.
    CWaitCursor wait;
    m_bookPath = dlg.GetPathName();

    if (!m_db.ReadCSVFile(m_bookPath))
    {
        AfxMessageBox(_T("CSV 로딩 실패"));
        return;
    }

    // 1. 책 이름 콤보박스 설정
    CString folderPath = m_bookPath.Left(m_bookPath.ReverseFind('\\'));
    int nPos = folderPath.ReverseFind('\\');
    CString bookName = folderPath.Mid(nPos + 1);

    m_comboBook.ResetContent();
    m_comboBook.AddString(bookName);
    m_comboBook.SetCurSel(0);

    // 2. 첫 번째 글자(1행 1번) 자동 선택
    if (m_db.GetChars().size() > 0)
    {
        m_selectedIndex = 0;
        m_curSheet = m_db.GetChars()[0].m_sheet;
    }
    else
    {
        m_selectedIndex = -1;
        m_curSheet = 1;
    }

    // 3. 화면 갱신
    LoadSheetImage();
    UpdateSelectedInfo();
    ShowSelectedCharImage();

    // 4. 스핀 컨트롤 설정
    m_spinSheet.SetBuddy(GetDlgItem(IDC_EDIT_SHEET));
    m_spinSheet.SetRange(1, m_db.GetSheetCount());
    m_spinSheet.SetPos(m_curSheet);

    SetDlgItemInt(IDC_EDIT_SHEET, m_curSheet);

    // (함수가 끝나면 커서는 자동으로 원래 화살표로 돌아옵니다)
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

    // 1. 기본 정보 표시
    CString text;
    text.Format(_T("%s\n\n%d장 %d행 %d번"),
        c.m_char.GetString(), c.m_sheet, c.m_line, c.m_order);
    m_pInfoCtrl->SetWindowText(text);

    // 2. 같은 '글자(m_char)'가 몇 개인지 세기
    int totalCount = 0;
    int currentRank = 0;
    const auto& allChars = m_db.GetChars();

    for (int i = 0; i < (int)allChars.size(); ++i)
    {
        if (allChars[i].m_char == c.m_char)
        {
            totalCount++;
            if (i == m_selectedIndex) currentRank = totalCount;
        }
    }

    // 3. 화면 표시
    SetDlgItemInt(IDC_EDIT_TYPE, currentRank);

    CString sTotal;
    sTotal.Format(_T("/ %d개"), totalCount);
    SetDlgItemText(IDC_STATIC_TYPES, sTotal);

    ShowComposeList(c.m_char);
}
// =======================
// 구성 글자 리스트 (구분 없이 위치만 표시)
// =======================
void CHanjaTypeDBDlg::ShowComposeList(const CString& targetChar)
{
    m_listCompose.DeleteAllItems();
    if (targetChar.GetLength() < 12) return;

    // 1. 검색할 코드 만들기 (나머지는 0으로 채움)
    CString codeCho = targetChar.Left(4) + _T("00000000");
    CString codeJung = _T("0000") + targetChar.Mid(4, 4) + _T("0000");
    CString codeJong = _T("00000000") + targetChar.Right(4);

    // 검색 대상 (0:초성, 1:중성, 2:종성)
    std::vector<CString> targetCodes;
    targetCodes.push_back(codeCho);
    targetCodes.push_back(codeJung);

    if (targetChar.Right(4) != _T("0000"))
        targetCodes.push_back(codeJong);
    else
        targetCodes.push_back(_T("")); // 받침 없으면 빈 거

    // 2. DB에서 위치 찾기
    const auto& chars = m_db.GetChars();
    int row = 0;

    for (const auto& code : targetCodes)
    {
        // 받침 없는 경우 처리 ("-" 표시)
        if (code.IsEmpty())
        {
            int nItem = m_listCompose.InsertItem(row, _T("-"));
            m_listCompose.SetItemText(nItem, 1, _T("-"));
            m_listCompose.SetItemText(nItem, 2, _T("-"));
            row++;
            continue;
        }

        bool bFound = false;

        // DB 검색
        for (const auto& c : chars)
        {
            if (c.m_char == code) // 찾았다!
            {
                CString sSheet, sLine, sOrder;
                sSheet.Format(_T("%d"), c.m_sheet);
                sLine.Format(_T("%d"), c.m_line);
                sOrder.Format(_T("%d"), c.m_order);

                // ⭐ [수정됨] 첫 번째 칸(0번)에 바로 '장' 번호를 넣습니다.
                int nItem = m_listCompose.InsertItem(row, sSheet);
                m_listCompose.SetItemText(nItem, 1, sLine);
                m_listCompose.SetItemText(nItem, 2, sOrder);

                bFound = true;
                break;
            }
        }

        if (!bFound) // 못 찾았으면 "-"
        {
            int nItem = m_listCompose.InsertItem(row, _T("-"));
            m_listCompose.SetItemText(nItem, 1, _T("-"));
            m_listCompose.SetItemText(nItem, 2, _T("-"));
        }
        row++;
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
    if (m_db.GetChars().empty()) return;

    const auto& chars = m_db.GetChars();

    // 1. 책 전체 통계용
    std::set<CString> totalCharKinds;
    std::set<int> totalTypeKinds;      // ⭐ [수정됨] CString -> int

    // 2. 장내(현재 페이지) 통계용
    int sheetCount = 0;
    std::set<CString> sheetCharKinds;
    std::set<int> sheetTypeKinds;      // ⭐ [수정됨] CString -> int

    for (const auto& c : chars)
    {
        // [책 전체]
        totalCharKinds.insert(c.m_char);
        totalTypeKinds.insert(c.m_type);

        // [장내]
        if (c.m_sheet == m_curSheet)
        {
            sheetCount++;
            sheetCharKinds.insert(c.m_char);
            sheetTypeKinds.insert(c.m_type);
        }
    }

    CString str;

    // [책 전체] 출력
    str.Format(_T("%d 개"), (int)chars.size());
    SetDlgItemText(IDC_STATIC_T_COUNT, str);

    str.Format(_T("%d 종"), (int)totalCharKinds.size());
    SetDlgItemText(IDC_STATIC_T_KIND, str);

    str.Format(_T("%d 개"), (int)totalTypeKinds.size());
    SetDlgItemText(IDC_STATIC_T_TYPE, str);

    // [장내] 출력
    str.Format(_T("%d 개"), sheetCount);
    SetDlgItemText(IDC_STATIC_S_COUNT, str);

    str.Format(_T("%d 종"), (int)sheetCharKinds.size());
    SetDlgItemText(IDC_STATIC_S_KIND, str);

    str.Format(_T("%d 개"), (int)sheetTypeKinds.size());
    SetDlgItemText(IDC_STATIC_S_TYPE, str);
}

// =======================
// 리스트 클릭 시 해당 위치로 이동
// =======================
// =======================
// 리스트 클릭 시 해당 위치로 이동
// =======================
void CHanjaTypeDBDlg::OnNMClickListChars(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    int nItem = pNMItemActivate->iItem;
    if (nItem == -1) {
        *pResult = 0;
        return;
    }

    // ⭐ [수정됨] 칸 번호가 당겨졌습니다. (1,2,3 -> 0,1,2)
    CString sSheet = m_listCompose.GetItemText(nItem, 0); // 장
    CString sLine = m_listCompose.GetItemText(nItem, 1); // 행
    CString sOrder = m_listCompose.GetItemText(nItem, 2); // 번

    // "-" (없음) 이면 무시
    if (sSheet == _T("-") || sSheet.IsEmpty()) {
        *pResult = 0;
        return;
    }

    int targetSheet = _ttoi(sSheet);
    int targetLine = _ttoi(sLine);
    int targetOrder = _ttoi(sOrder);

    // 1. 페이지 이동
    if (targetSheet != m_curSheet)
    {
        m_curSheet = targetSheet;
        SetDlgItemInt(IDC_EDIT_SHEET, m_curSheet);
    }

    // 2. 해당 위치의 글자 선택
    const auto& chars = m_db.GetChars();
    for (int i = 0; i < (int)chars.size(); ++i)
    {
        const auto& c = chars[i];
        if (c.m_sheet == targetSheet && c.m_line == targetLine && c.m_order == targetOrder)
        {
            m_selectedIndex = i;
            break;
        }
    }

    // 3. 화면 갱신
    LoadSheetImage();

    if (m_pInfoCtrl && m_selectedIndex >= 0)
    {
        const auto& c = chars[m_selectedIndex];
        CString text;
        text.Format(_T("%s\n\n%d장 %d행 %d번"), c.m_char.GetString(), c.m_sheet, c.m_line, c.m_order);
        m_pInfoCtrl->SetWindowText(text);
    }
    ShowSelectedCharImage();

    *pResult = 0;
}

// =======================
// 활자 정보 스핀 컨트롤 (같은 활자 찾아서 이동)
// =======================
void CHanjaTypeDBDlg::OnDeltaposSpinType(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    if (m_selectedIndex < 0) {
        *pResult = 0;
        return;
    }

    std::vector<int> sameCharIndices;
    const auto& allChars = m_db.GetChars();
    CString currentChar = allChars[m_selectedIndex].m_char;

    int currentIndexInList = -1;
    for (int i = 0; i < (int)allChars.size(); ++i)
    {
        if (allChars[i].m_char == currentChar)
        {
            if (i == m_selectedIndex) currentIndexInList = (int)sameCharIndices.size();
            sameCharIndices.push_back(i);
        }
    }

    if (sameCharIndices.empty()) {
        *pResult = 0;
        return;
    }

    int change = 0;
    if (pNMUpDown->iDelta < 0) change = 1;
    if (pNMUpDown->iDelta > 0) change = -1;

    int nextIndexInList = currentIndexInList + change;

    if (nextIndexInList < 0) nextIndexInList = 0;
    if (nextIndexInList >= (int)sameCharIndices.size()) nextIndexInList = (int)sameCharIndices.size() - 1;

    if (nextIndexInList != currentIndexInList)
    {
        int newGlobalIndex = sameCharIndices[nextIndexInList];
        const auto& target = allChars[newGlobalIndex];

        if (target.m_sheet != m_curSheet)
        {
            m_curSheet = target.m_sheet;
            SetDlgItemInt(IDC_EDIT_SHEET, m_curSheet);
        }

        m_selectedIndex = newGlobalIndex;

        LoadSheetImage();
        UpdateSelectedInfo();
        ShowSelectedCharImage();
    }

    *pResult = 0;
}
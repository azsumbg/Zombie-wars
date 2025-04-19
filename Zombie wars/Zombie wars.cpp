#include "framework.h"
#include "Zombie wars.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "ErrH.h"
#include "FCheck.h"
#include "BaseServ.h"
#include "gifresizer.h"
#include "D2BMPLOADER.h"
#include <chrono>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "baseserv.lib")
#pragma comment(lib, "gifresizer.lib")
#pragma comment(lib, "d2bmploader.lib")


constexpr wchar_t bWinClassName[]{ L"AZombieGame" };

constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\sound.wav" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int no_record{ 2001 };
constexpr int first_record{ 2001 };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HICON bIcon{ nullptr };
HCURSOR bCur{ nullptr };
HCURSOR outCur{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
PAINTSTRUCT bPaint{};
MSG bMsg{};
BOOL bRet{ 0 };
HWND bHwnd{ nullptr };
POINT cur_pos{};
HDC PaintDC{ nullptr };

dll::RANDIT RandMachine{};

bool pause{ false };
bool in_client{ true };
bool show_help{ false };
bool sound{ true };
bool b1Hglt{ false };
bool b2Hglt{ false };
bool b3Hglt{ false };
bool name_set = false;

wchar_t current_player[16]{ L"ONE WARRIOR" };

D2D1_RECT_F b1Rect{ 20.0f, 0, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 20.0f, 0, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 20.0f, 0, scr_width - 20.0f, 50.0f };

D2D1_RECT_F b1Txt1Rect{ 40.0f, 15.0f, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Txt2Rect{ scr_width / 3 + 40.0f, 15.0f, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2 / 3 + 40.0f, 15.0f, scr_width - 20.0f, 50.0f };

int level = 1;
int score = 0;

int intro_frame = 0;
int intro_frame_delay = 8;

int field_frame = 0;
int field_frame_delay = 4;

// ********************************************

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

ID2D1SolidColorBrush* statusBckgBrush{ nullptr };
ID2D1SolidColorBrush* TxtBrush{ nullptr };
ID2D1SolidColorBrush* HgltBrush{ nullptr };
ID2D1SolidColorBrush* InactBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpHouse1{ nullptr };
ID2D1Bitmap* bmpHouse2{ nullptr };
ID2D1Bitmap* bmpHouse3{ nullptr };
ID2D1Bitmap* bmpTomb{ nullptr };
ID2D1Bitmap* bmpRip{ nullptr };
ID2D1Bitmap* bmpPotion{ nullptr };

ID2D1Bitmap* bmpField[18]{ nullptr };
ID2D1Bitmap* bmpIntro[10]{ nullptr };

ID2D1Bitmap* bmpFighterKick1L[48]{ nullptr };
ID2D1Bitmap* bmpFighterKick1R[48]{ nullptr };

ID2D1Bitmap* bmpFighterKick2L[8]{ nullptr };
ID2D1Bitmap* bmpFighterKick2R[8]{ nullptr };

ID2D1Bitmap* bmpFighterPunchL[23]{ nullptr };
ID2D1Bitmap* bmpFighterPunchR[23]{ nullptr };

ID2D1Bitmap* bmpFighterStandL[6]{ nullptr };
ID2D1Bitmap* bmpFighterStandR[6]{ nullptr };

ID2D1Bitmap* bmpSoul[120]{ nullptr };

ID2D1Bitmap* bmpZombie1[10]{ nullptr };
ID2D1Bitmap* bmpZombie2[11]{ nullptr };
ID2D1Bitmap* bmpZombie3[4]{ nullptr };
ID2D1Bitmap* bmpZombie4[37]{ nullptr };

//////////////////////////////////////////////

template<typename U>concept HasRelease = requires (U check)
{
    check.Release();
};
template<HasRelease T> bool ClearMem(T** var)
{
    if (*var)
    {
        (*var)->Release();
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << what << L" Error occurred at: " << std::chrono::system_clock::now() << std::endl << std::endl;
    err.close();
}
void ReleaseResources()
{
    if (!ClearMem(&iFactory))LogError(L"Error releasing iFactory !");
    if (!ClearMem(&Draw))LogError(L"Error releasing Draw !");
    if (!ClearMem(&b1BckgBrush))LogError(L"Error releasing b1BckgBrush !");
    if (!ClearMem(&b2BckgBrush))LogError(L"Error releasing b2BckgBrush !");
    if (!ClearMem(&b3BckgBrush))LogError(L"Error releasing b3BckgBrush !");
    if (!ClearMem(&statusBckgBrush))LogError(L"Error releasing StatusBckgBrush !");
    if (!ClearMem(&TxtBrush))LogError(L"Error releasing TxtBrush !");
    if (!ClearMem(&HgltBrush))LogError(L"Error releasing HgltBrush !");
    if (!ClearMem(&InactBrush))LogError(L"Error releasing InactBrush !");
    if (!ClearMem(&iWriteFactory))LogError(L"Error releasing iWriteFactory !");
    if (!ClearMem(&nrmFormat))LogError(L"Error releasing nrmFormat !");
    if (!ClearMem(&midFormat))LogError(L"Error releasing midFormat !");
    if (!ClearMem(&bigFormat))LogError(L"Error releasing bigFormat !");
    
    if (!ClearMem(&bmpHouse1))LogError(L"Error releasing bmpHouse1 !");
    if (!ClearMem(&bmpHouse2))LogError(L"Error releasing bmpHouse2 !");
    if (!ClearMem(&bmpHouse3))LogError(L"Error releasing bmpHouse3 !");
    if (!ClearMem(&bmpTomb))LogError(L"Error releasing bmpTomb !");
    if (!ClearMem(&bmpRip))LogError(L"Error releasing bmpRip !");
    if (!ClearMem(&bmpPotion))LogError(L"Error releasing bmpPotion !");

    for (int i = 0; i < 18; ++i)if (!ClearMem(&bmpField[i]))LogError(L"Error releasing bmpField !");
    for (int i = 0; i < 10; ++i)if (!ClearMem(&bmpIntro[i]))LogError(L"Error releasing bmpIntro !");

    for (int i = 0; i < 48; ++i)if (!ClearMem(&bmpFighterKick1L[i]))LogError(L"Error releasing bmpFighterKick1L !");
    for (int i = 0; i < 48; ++i)if (!ClearMem(&bmpFighterKick1R[i]))LogError(L"Error releasing bmpFighterKick1R !");
    for (int i = 0; i < 8; ++i)if (!ClearMem(&bmpFighterKick2L[i]))LogError(L"Error releasing bmpFighterKick2L !");
    for (int i = 0; i < 8; ++i)if (!ClearMem(&bmpFighterKick2R[i]))LogError(L"Error releasing bmpFighterKick2R !");
    for (int i = 0; i < 23; ++i)if (!ClearMem(&bmpFighterPunchL[i]))LogError(L"Error releasing bmpFighterPunchL !");
    for (int i = 0; i < 23; ++i)if (!ClearMem(&bmpFighterPunchR[i]))LogError(L"Error releasing bmpFighterPunchR !");
    for (int i = 0; i < 6; ++i)if (!ClearMem(&bmpFighterStandL[i]))LogError(L"Error releasing bmpFighterStandL !");
    for (int i = 0; i < 6; ++i)if (!ClearMem(&bmpFighterStandR[i]))LogError(L"Error releasing bmpFighterStandR !");
    for (int i = 0; i < 120; ++i)if (!ClearMem(&bmpSoul[i]))LogError(L"Error releasing bmpSoul !");
    for (int i = 0; i < 10; ++i)if (!ClearMem(&bmpZombie1[i]))LogError(L"Error releasing bmpZombie1 !");
    for (int i = 0; i < 11; ++i)if (!ClearMem(&bmpZombie2[i]))LogError(L"Error releasing bmpZombie2 !");
    for (int i = 0; i < 4; ++i)if (!ClearMem(&bmpZombie3[i]))LogError(L"Error releasing bmpZombie3 !");
    for (int i = 0; i < 37; ++i)if (!ClearMem(&bmpZombie4[i]))LogError(L"Error releasing bmpZombie4 !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИЧНА ГРЕШКА !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    ReleaseResources();
    std::remove(tmp_file);
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitGame()
{
    wcscpy_s(current_player, L"ONE WARRIOR");
    score = 0;
    level = 1;
    name_set = false;

    /////////////////////////

    
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(bIcon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
            {
                wcscpy_s(current_player, L"ONE WARRIOR");
                if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
                MessageBox(bHwnd, L"Името ли си забрави ?", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        if (bIns)
        {
            bBar = CreateMenu();
            bMain = CreateMenu();
            bStore = CreateMenu();

            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
            AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

            AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
            AppendMenu(bMain, MF_STRING, mLvl, L"Турбо скорост");
            AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bMain, MF_STRING, mExit, L"Изход от играта");

            AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
            AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
            AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
            AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

            SetMenu(hwnd, bBar);

            InitGame();
        }
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, губиш прогреса по тази игра !\n\nНаистина ли излизаш ?",
            L"Изход", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }

            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = true;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = false;
                        b2Hglt = false;
                        b3Hglt = true;
                    }
                }
            
                SetCursor(outCur);
                return true;
            }
            else
            {
                if (b1Hglt || b2Hglt || b3Hglt)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                    b1Hglt = false;
                    b2Hglt = false;
                    b3Hglt = false;
                }
                SetCursor(bCur);
                return true;
            }
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }
            
            if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
        
            SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        return true;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, губиш прогреса по тази игра !\n\nНаистина ли рестартираш ?",
                L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mLvl:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако минеш на следващо ниво, губиш прогреса по това !\n\nНаистина ли минаваш на следващо ниво ?",
                L"Следващо ниво", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            GameOver();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;






    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    int result = 0;
    CheckFile(Ltmp_file, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream mytmp(tmp_file);
        mytmp << L"Game started at: " << std::chrono::system_clock::now();
        mytmp.close();
    }

    int win_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2);
    int win_y = 10;

    if (GetSystemMetrics(SM_CXSCREEN) < win_x + (int)(scr_width) || GetSystemMetrics(SM_CYSCREEN) < win_y + (int)(scr_height))
        ErrExit(eScreen);

    bIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
    if (!bIcon)ErrExit(eIcon);
    bCur = LoadCursorFromFile(L".\\res\\main.ani");
    outCur = LoadCursorFromFile(L".\\res\\out.ani");
    if (!bCur || !outCur)ErrExit(eCursor);

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &WinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
    bWinClass.hCursor = bCur;
    bWinClass.hIcon = bIcon;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"ЗОМБИТА В АТАКА !", WS_CAPTION | WS_SYSMENU, win_x, win_y, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);

    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating D2D1Factory !");
            ErrExit(eD2D);
        }

        if (iFactory)
        {
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1HwndRenderTarget !");
                ErrExit(eD2D);
            }

            if (Draw)
            {
                D2D1_GRADIENT_STOP gStops[2]{};
                ID2D1GradientStopCollection* gColl{ nullptr };

                gStops[0].position = 0;
                gStops[0].color = D2D1::ColorF(D2D1::ColorF::DarkOliveGreen);
                gStops[1].position = 1.0f;
                gStops[1].color = D2D1::ColorF(D2D1::ColorF::Crimson);

                hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
                if (hr != S_OK)
                {
                    LogError(L"Error creating D2D1GradientStopCollection !");
                    ErrExit(eD2D);
                }

                if (gColl)
                {
                    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
                        (b1Rect.right - b1Rect.left) / 2, b1Rect.top), D2D1::Point2F(0, 0), (b1Rect.right - b1Rect.left) / 2, 25.0f),
                        gColl, &b1BckgBrush);
                    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
                        (b2Rect.right - b2Rect.left) / 2, b2Rect.top), D2D1::Point2F(0, 0), (b2Rect.right - b2Rect.left) / 2, 25.0f),
                        gColl, &b2BckgBrush);
                    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
                        (b3Rect.right - b3Rect.left) / 2, b3Rect.top), D2D1::Point2F(0, 0), (b3Rect.right - b3Rect.left) / 2, 25.0f),
                        gColl, &b3BckgBrush);
                
                    if (hr != S_OK)
                    {
                        LogError(L"Error creating D2D1RadialGradientBrushes for buttons background !");
                        ErrExit(eD2D);
                    }

                    ClearMem(&gColl);
                
                }

                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkSlateBlue), &statusBckgBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue), &TxtBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::MistyRose), &HgltBrush);
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &InactBrush);

                if (hr != S_OK)
                {
                    LogError(L"Error creating D2D1SolidColorBrushes for text and status background !");
                    ErrExit(eD2D);
                }

                bmpHouse1 = Load(L".\\res\\img\\assets\\house1.png", Draw);
                if (!bmpHouse1)
                {
                    LogError(L"Error loading bmpHouse1 !");
                    ErrExit(eD2D);
                }
                bmpHouse2 = Load(L".\\res\\img\\assets\\house2.png", Draw);
                if (!bmpHouse2)
                {
                    LogError(L"Error loading bmpHouse2 !");
                    ErrExit(eD2D);
                }
                bmpHouse3 = Load(L".\\res\\img\\assets\\house3.png", Draw);
                if (!bmpHouse3)
                {
                    LogError(L"Error loading bmpHouse3 !");
                    ErrExit(eD2D);
                }
                bmpTomb = Load(L".\\res\\img\\assets\\tomb.png", Draw);
                if (!bmpTomb)
                {
                    LogError(L"Error loading bmpTomb !");
                    ErrExit(eD2D);
                }
                bmpRip = Load(L".\\res\\img\\assets\\Rip.png", Draw);
                if (!bmpRip)
                {
                    LogError(L"Error loading bmpRip !");
                    ErrExit(eD2D);
                }
                bmpPotion = Load(L".\\res\\img\\assets\\potion.png", Draw);
                if (!bmpPotion)
                {
                    LogError(L"Error loading bmpPotion !");
                    ErrExit(eD2D);
                }

                for (int i = 0; i < 18; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\assets\\field\\";
                    wchar_t add[6] = L"\0";
                    
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpField[i] = Load(name, Draw);
                    if (!bmpField[i])
                    {
                        LogError(L"Error loading bmpField !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 10; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\assets\\intro\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpIntro[i] = Load(name, Draw);
                    if (!bmpIntro[i])
                    {
                        LogError(L"Error loading bmpIntro !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 48; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\kick1\\l\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterKick1L[i] = Load(name, Draw);
                    if (!bmpFighterKick1L[i])
                    {
                        LogError(L"Error loading bmpFighterKick1L !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 48; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\kick1\\r\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterKick1R[i] = Load(name, Draw);
                    if (!bmpFighterKick1R[i])
                    {
                        LogError(L"Error loading bmpFighterKick1R !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 8; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\kick2\\l\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterKick2L[i] = Load(name, Draw);
                    if (!bmpFighterKick2L[i])
                    {
                        LogError(L"Error loading bmpFighterKick2L !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 8; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\kick2\\r\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterKick2R[i] = Load(name, Draw);
                    if (!bmpFighterKick2R[i])
                    {
                        LogError(L"Error loading bmpFighterKick2R !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 23; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\punch\\l\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterPunchL[i] = Load(name, Draw);
                    if (!bmpFighterPunchL[i])
                    {
                        LogError(L"Error loading bmpFighterPunchL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 23; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\punch\\r\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterPunchR[i] = Load(name, Draw);
                    if (!bmpFighterPunchR[i])
                    {
                        LogError(L"Error loading bmpFighterPunchR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 6; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\stand\\l\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterStandL[i] = Load(name, Draw);
                    if (!bmpFighterStandL[i])
                    {
                        LogError(L"Error loading bmpFighterStandhL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 6; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\fighter\\stand\\r\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpFighterStandR[i] = Load(name, Draw);
                    if (!bmpFighterStandR[i])
                    {
                        LogError(L"Error loading bmpFighterStandR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 10; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\zombie1\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpZombie1[i] = Load(name, Draw);
                    if (!bmpZombie1[i])
                    {
                        LogError(L"Error loading bmpZombie1 !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 11; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\zombie2\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpZombie2[i] = Load(name, Draw);
                    if (!bmpZombie2[i])
                    {
                        LogError(L"Error loading bmpZombie2 !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 4; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\zombie3\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpZombie3[i] = Load(name, Draw);
                    if (!bmpZombie3[i])
                    {
                        LogError(L"Error loading bmpZombie3 !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 37; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\zombie4\\";
                    wchar_t add[6] = L"\0";

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpZombie4[i] = Load(name, Draw);
                    if (!bmpZombie4[i])
                    {
                        LogError(L"Error loading bmpZombie4 !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 120; ++i)
                {
                    wchar_t name[200] = L".\\res\\img\\soul\\0";
                    wchar_t add[6] = L"\0";

                    if (i < 10)wcscat_s(name, L"00");
                    else if (i < 100)wcscat_s(name, L"0");

                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");

                    bmpSoul[i] = Load(name, Draw);
                    if (!bmpSoul[i])
                    {
                        LogError(L"Error loading bmpSoul !");
                        ErrExit(eD2D);
                    }
                }

            }

            hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
            if (hr != S_OK)
            {
                LogError(L"Error creating D2D1WriteFactory !");
                ErrExit(eD2D);
            }

            if (iWriteFactory)
            {
                hr = iWriteFactory->CreateTextFormat(L"SEGOE SCRIPT", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                    DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &nrmFormat);
                hr = iWriteFactory->CreateTextFormat(L"SEGOE SCRIPT", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                    DWRITE_FONT_STRETCH_NORMAL, 28.0f, L"", &midFormat);
                hr = iWriteFactory->CreateTextFormat(L"SEGOE SCRIPT", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK, DWRITE_FONT_STYLE_OBLIQUE,
                    DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"", &bigFormat);
                if (hr != S_OK)
                {
                    LogError(L"Error creating D2D1WriteFactory Text Formats !");
                    ErrExit(eD2D);
                }
            }

        }
    }

    if (Draw && bigFormat && TxtBrush)
    {
        result = 0;
        PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_ASYNC);
        
        while (result < 300)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
            --intro_frame_delay;
            if (intro_frame_delay < 0)
            {
                intro_frame_delay = 8;
                ++intro_frame;
                if (intro_frame > 9)intro_frame = 0;
            }
            Draw->DrawText(L"ЗОМБИ АПОКАЛИПСИС\n\n\n   dev. Daniel !", 37, bigFormat, D2D1::RectF(20.0f, 100.0f,
                scr_width, scr_height), TxtBrush);
            Draw->EndDraw();
        
            ++result;
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)
    {
        LogError(L"Error in Windows hInstance parameter !");
        ErrExit(eWindow);
    }

    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);

            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }

        if (pause)
        {
            if (show_help)continue;

            if (Draw && bigFormat && TxtBrush)
            {
                Draw->BeginDraw();
                Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
                --intro_frame_delay;
                if (intro_frame_delay < 0)
                {
                    intro_frame_delay = 8;
                    ++intro_frame;
                    if (intro_frame > 9)intro_frame = 0;
                }
                Draw->DrawText(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f,
                    scr_width, scr_height), TxtBrush);
                Draw->EndDraw();
                continue;
            }
        }

        /////////////////////////////////////////////////////////////////

















        // DRAW THINGS ***************************************

        Draw->BeginDraw();

        if (statusBckgBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && TxtBrush && HgltBrush && InactBrush && nrmFormat)
        {
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statusBckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 10.0f, 8.0f), b1BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 10.0f, 8.0f), b2BckgBrush);
            Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 10.0f, 8.0f), b3BckgBrush);

            if (name_set)Draw->DrawTextW(L"ИМЕ НА ГЕРОЙ", 13, nrmFormat, b1Txt1Rect, InactBrush);
            else
            {
                if (b1Hglt)Draw->DrawTextW(L"ИМЕ НА ГЕРОЙ", 13, nrmFormat, b1Txt1Rect, HgltBrush);
                else Draw->DrawTextW(L"ИМЕ НА ГЕРОЙ", 13, nrmFormat, b1Txt1Rect, TxtBrush);


            }

            if (b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2Txt2Rect, HgltBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2Txt2Rect, TxtBrush);

            if (b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, HgltBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, TxtBrush);

        }

        --field_frame_delay;
        if (field_frame_delay < 0)
        {
            field_frame_delay = 4;
            ++field_frame;
            if (field_frame > 17)field_frame = 0;
        }

        Draw->DrawBitmap(bmpField[field_frame], D2D1::RectF(0, 50, scr_width, scr_height));




        ///////////////////////////

        Draw->EndDraw();
    }

    ReleaseResources();
    std::remove(tmp_file);

    return (int) bMsg.wParam;
}
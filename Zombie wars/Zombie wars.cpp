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
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int no_record{ 2002 };
constexpr int first_record{ 2003 };

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

bool hero_killed = false;
float RIP_X = 0;
float RIP_Y = 0;

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

int saved_souls = 0;
int killed_souls = 0;

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

dll::Creature Hero{ nullptr };
float hero_mx{ 0 };
float hero_my{ 0 };

struct TOMBS
{
    unsigned char type;
    dll::PROTON tomb;
};
std::vector<TOMBS> vTombs;

std::vector<dll::Creature> vEvils;
std::vector<dll::Creature> vSouls;

std::vector<dll::PROTON> vPotions;

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
        (*var) = nullptr;
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
BOOL CheckRecord()
{
    if (score < 1)return no_record;

    int result{ 0 };
    CheckFile(record_file, &result);

    if (result == FILE_NOT_EXIST)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return first_record;
    }
    else
    {
        std::wifstream check(record_file);
        check >> result;
        check.close();
    }

    if (result < score)
    {
        std::wofstream rec(record_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return record;
    }

    return no_record;
}
void GameOver()
{
    PlaySound(NULL, NULL, NULL);

    switch (CheckRecord())
    {
    case no_record:
        if (HgltBrush && bigFormat)
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
            Draw->DrawTextW(L"ЗОМБИТАТА ПОБЕДИХА !", 21, bigFormat, D2D1::RectF(20.0f, 300.0f, scr_width, scr_height), HgltBrush);
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
            else Sleep(3000);
            break;
        }
        break;

    case first_record:
        if (HgltBrush && bigFormat)
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
            Draw->DrawTextW(L"ПЪРВИ РЕКОРД НА ИГРАТА !", 25, bigFormat, D2D1::RectF(20.0f, 300.0f, scr_width, scr_height), HgltBrush);
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
            else Sleep(3000);
            break;
        }
        break;

    case record:
        if (HgltBrush && bigFormat)
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
            Draw->DrawTextW(L"СВЕТОВЕН РЕКОРД НА ИГРАТА !", 28, bigFormat, D2D1::RectF(5.0f, 300.0f, scr_width, scr_height), HgltBrush);
            Draw->EndDraw();
            if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
            else Sleep(3000);
            break;
        }
        break;
    }

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

    saved_souls = 0;
    killed_souls = 0;

    ClearMem(&Hero);
    Hero = dll::Factory(hero, (float)(RandMachine(10, 800)), ground - 80.0f);
    
    vTombs.clear();

    while (vTombs.size() < 3)
    {
        switch (RandMachine(0, 3))
        {
        case 0:
            {
                TOMBS aTomb(tomb, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                    150.0f, 147.0f));

                if (!vTombs.empty())
                {
                    bool is_ok = true;
                    for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                    {
                        if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                            && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                        {
                            is_ok = false;
                            break;
                        }
                    }

                    if (is_ok)vTombs.push_back(aTomb);
                }
                else vTombs.push_back(aTomb);
            }
            break;

        case 1:
        {
            TOMBS aTomb(house1, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                189.0f, 200.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;
        
        case 2:
        {
            TOMBS aTomb(house2, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                182.0f, 180.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;

        case 3:
        {
            TOMBS aTomb(house3, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                186.0f, 180.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;
        }
    }

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearMem(&vEvils[i]);
    vEvils.clear();

    if (!vSouls.empty())
        for (int i = 0; i < vSouls.size(); ++i)ClearMem(&vSouls[i]);
    vSouls.clear();

    vPotions.clear();
}
void LevelUp()
{
    int bonus = 0;

    if (HgltBrush && bigFormat && saved_souls > 0)
    {
        while (bonus <= saved_souls * level)
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
            wchar_t txt[50] = L"БОНУС: ";
            wchar_t add[5] = L"\0";
            int txt_size = 0;

            wsprintf(add, L"%d", bonus);
            wcscat_s(txt, add);

            for (int i = 0; i < 50; ++i)
            {
                if (txt[i] != '\0')txt_size++;
                else break;
            }

            Draw->DrawTextW(txt, txt_size, bigFormat, D2D1::RectF(300.0f, 350.0f, scr_width, scr_height), HgltBrush);

            Draw->EndDraw();
            if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
            Sleep(80);

            bonus++;
        }
    }

    if (HgltBrush && bigFormat)
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
        Draw->DrawText(L"НИВОТО ИЗЧИСТЕНО !", 19, bigFormat, D2D1::RectF(100.0f, scr_height / 2 - 50.0f,
            scr_width, scr_height), TxtBrush);
        Draw->EndDraw();
    }

    saved_souls = 0;
    killed_souls = 0;

    ClearMem(&Hero);
    Hero = dll::Factory(hero, (float)(RandMachine(10, 800)), ground - 80.0f);

    vTombs.clear();

    while (vTombs.size() < 3)
    {
        switch (RandMachine(0, 3))
        {
        case 0:
        {
            TOMBS aTomb(tomb, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                150.0f, 147.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;

        case 1:
        {
            TOMBS aTomb(house1, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                189.0f, 200.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;

        case 2:
        {
            TOMBS aTomb(house2, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                182.0f, 180.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;

        case 3:
        {
            TOMBS aTomb(house3, dll::PROTON((float)(RandMachine(0, 800)), (float)(RandMachine((int)(up_ground_boundary), 500)),
                186.0f, 180.0f));

            if (!vTombs.empty())
            {
                bool is_ok = true;
                for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
                {
                    if (abs(aTomb.tomb.center.x - it->tomb.center.x) <= aTomb.tomb.x_radius + it->tomb.x_radius
                        && abs(aTomb.tomb.center.y - it->tomb.center.y) <= aTomb.tomb.y_radius + it->tomb.y_radius)
                    {
                        is_ok = false;
                        break;
                    }
                }

                if (is_ok)vTombs.push_back(aTomb);
            }
            else vTombs.push_back(aTomb);
        }
        break;
        }
    }

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearMem(&vEvils[i]);
    vEvils.clear();

    if (!vSouls.empty())
        for (int i = 0; i < vSouls.size(); ++i)ClearMem(&vSouls[i]);
    vSouls.clear();

    vPotions.clear();

    level++;

    if (sound)
    {
        PlaySound(NULL, NULL, NULL);
        PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
        PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
    }
    else Sleep(3000);
}
void HallOfFame()
{
    int result{ 0 };
    CheckFile(record_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Все още липсва рекорд на играта !\n\nПостарай се повече !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    wchar_t hof_txt[150]{ L"НАЙ-ВЕЛИК ГЕРОЙ: " };
    wchar_t saved_player[16] = L"\0";
    wchar_t add[5] = L"\0";

    std::wifstream check(record_file);

    check >> result;
    for (int i = 0; i < 16; ++i)
    {
        int letter = 0;
        check >> letter;
        saved_player[i] = static_cast<wchar_t>(letter);
    }

    check.close();

    wsprintf(add, L"%d", result);
    wcscat_s(hof_txt, saved_player);
    wcscat_s(hof_txt, L"\n\nСВЕТОВЕН РЕКОРД: ");
    wcscat_s(hof_txt, add);

    result = 0;

    for (int i = 0; i < 150; ++i)
    {
        if (hof_txt[i] != '\0')++result;
        else break;
    }

    if (sound)mciSendString(L"play .\\res\\snd\\showrec.wav", NULL, NULL, NULL);

    if (HgltBrush && bigFormat)
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
        Draw->DrawText(hof_txt, result, bigFormat, D2D1::RectF(10.0f, 200.0f,
            scr_width, scr_height), TxtBrush);
        Draw->EndDraw();
    }

    Sleep(3500);
}
void SaveGame()
{
    int result{ 0 };
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(bHwnd, L"Има предишна записана игра, която ще бъде загубена !\n\nНаистина ли я презаписваш ?",
            L"Презапис", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }

    std::wofstream save(save_file);

    save << hero_killed << std::endl;
    save << score << std::endl;
    save << level << std::endl;

    save << saved_souls << std::endl;
    save << killed_souls << std::endl;

    for (int i = 0; i < 16; ++i)save << static_cast<int>(current_player[i]) << std::endl;
    save << name_set << std::endl;

    save << Hero->start.x << std::endl;
    save << Hero->start.y << std::endl;
    save << Hero->lifes << std::endl;

    save << vTombs.size() << std::endl;
    if (!vTombs.empty())
    {
        for (int i = 0; i < vTombs.size(); ++i)
        {
            save << vTombs[i].type << std::endl;
            save << vTombs[i].tomb.start.x << std::endl;
            save << vTombs[i].tomb.start.y << std::endl;
        }
    }

    save << vEvils.size() << std::endl;
    if (!vEvils.empty())
    {
        for (int i = 0; i < vEvils.size(); ++i)
        {
            save << vEvils[i]->type << std::endl;
            save << vEvils[i]->start.x << std::endl;
            save << vEvils[i]->start.y << std::endl;
            save << vEvils[i]->lifes << std::endl;
        }
    }

    save << vSouls.size() << std::endl;
    if (!vSouls.empty())
    {
        for (int i = 0; i < vSouls.size(); ++i)
        {
            save << vSouls[i]->start.x << std::endl;
            save << vSouls[i]->start.y << std::endl;
        }
    }

    save << vPotions.size() << std::endl;
    if (!vPotions.empty())
    {
        for (int i = 0; i < vPotions.size(); ++i)
        {
            save << vPotions[i].start.x << std::endl;
            save << vPotions[i].start.y << std::endl;
        }
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);

    MessageBox(bHwnd, L"Играта е запазена !", L"Запис !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
    int result{ 0 };
    CheckFile(save_file, &result);

    if (result == FILE_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(bHwnd, L"Ако продължиш, губиш прогреса по тази игра !\n\nНаистина ли зареждаш записаната игра ?",
            L"Зареждане", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
    }
    else
    {
        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Все още липсва записана игра !\n\nПостарай се повече !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    ClearMem(&Hero);
    Hero = dll::Factory(hero, (float)(RandMachine(10, 800)), ground - 80.0f);

    vTombs.clear();

    if (!vEvils.empty())
        for (int i = 0; i < vEvils.size(); ++i)ClearMem(&vEvils[i]);
    vEvils.clear();

    if (!vSouls.empty())
        for (int i = 0; i < vSouls.size(); ++i)ClearMem(&vSouls[i]);
    vSouls.clear();

    vPotions.clear();

    std::wifstream save(save_file);

    save >> hero_killed;
    if (hero_killed)GameOver();

    save >> score;
    save >> level;

    save >> saved_souls;
    save >> killed_souls;

    for (int i = 0; i < 16; ++i)
    {
        int letter = 0;
        save >> letter;
        current_player[i] = static_cast<wchar_t>(letter);
    }
    save >> name_set;

    float tx{ 0 };
    float ty{ 0 };
    save >> tx;
    save >> ty;
    save >> result;

    Hero = dll::Factory(hero, tx, ty);
    Hero->lifes = result;

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; ++i)
        {
            int atype{ 0 };
            float ax{ 0 };
            float ay{ 0 };

            save >> atype;
            save >> ax;
            save >> ay;
            
            switch ((unsigned char)(atype))
            {
            case tomb:
                vTombs.push_back(TOMBS(tomb, dll::PROTON(ax, ay, 150.0f, 147.0f)));
                break;
            
            case house1:
                vTombs.push_back(TOMBS(house1, dll::PROTON(ax, ay, 189.0f, 200.0f)));
                break;

            case house2:
                vTombs.push_back(TOMBS(house2, dll::PROTON(ax,ay,182.0f, 180.0f)));
                break;
                
            case house3:
                vTombs.push_back(TOMBS(house2, dll::PROTON(ax, ay, 186.0f, 180.0f)));
                break;
            }
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; ++i)
        {
            int atype{ 0 };
            float ax{ 0 };
            float ay{ 0 };
            int alife{ 0 };
            
            save >> atype;
            save >> ax;
            save >> ay;
            save >> alife;

            vEvils.push_back(dll::Factory((unsigned char)(atype), ax, ay));
            vEvils.back()->lifes = alife;
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; ++i)
        {
            float ax = 0;
            float ay = 0;

            save >> ax;
            save >> ay;
            vSouls.push_back(dll::Factory(soul, ax, ay));
        }
    }

    save >> result;
    if (result > 0)
    {
        for (int i = 0; i < result; ++i)
        {
            float ax = 0;
            float ay = 0;

            save >> ax;
            save >> ay;
            vPotions.push_back(dll::PROTON(ax, ay, 22.0f, 26.0f));
        }
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);

    MessageBox(bHwnd, L"Играта е заредена !", L"Зареждане !", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void ShowHelp()
{
    int result = 0;
    CheckFile(help_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Липсва помощна информация за играта !\n\nСвържете се с разработчика !", L"Липсва файл !",
            MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    std::wifstream help(help_file);

    wchar_t help_txt[1000]{ L"\0" };

    help >> result;

    for (int i = 0; i < result; ++i)
    {
        int letter = 0;
        help >> letter;
        help_txt[i] = static_cast<wchar_t>(letter);
    }

    help.close();

    Draw->BeginDraw();

    Draw->DrawBitmap(bmpIntro[intro_frame], D2D1::RectF(0, 0, scr_width, scr_height));
    --intro_frame_delay;
    if (intro_frame_delay < 0)
    {
        intro_frame_delay = 8;
        ++intro_frame;
        if (intro_frame > 9)intro_frame = 0;
    }

    if (statusBckgBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && TxtBrush && HgltBrush && InactBrush && nrmFormat && midFormat)
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

        Draw->DrawTextW(help_txt, result, midFormat, D2D1::RectF(50.0f, 120.0f, scr_width, scr_height), HgltBrush);
    }

    Draw->EndDraw();

    if (sound)mciSendString(L"play .\\res\\snd\\showhelp.wav", NULL, NULL, NULL);
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
            saved_souls = 0;
            LevelUp();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        case mSave:
            pause = true;
            SaveGame();
            pause = false;
            break;

        case mLoad:
            pause = true;
            LoadGame();
            pause = false;
            break;

        case mHoF:
            pause = true;
            HallOfFame();
            pause = false;
            break;
        }
        break;

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= 50)
        {
            if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
            {
                if (name_set)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                    break;
                }

                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && LOWORD(lParam) <= b2Rect.right)
            {
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }
            }
            if (LOWORD(lParam) >= b3Rect.left && LOWORD(lParam) <= b3Rect.right)
            {
                if (!show_help)
                {
                    show_help = true;
                    pause = true;
                    ShowHelp();
                    break;
                }
                else
                {
                    show_help = false;
                    pause = false;
                    break;
                }
            }
        }
        else
        {
            hero_mx = (float)(LOWORD(lParam));
            hero_my = (float)(HIWORD(lParam));
            if (Hero)Hero->Move(hero_mx, hero_my, (float)(level));
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
                    DWRITE_FONT_STRETCH_NORMAL, 24.0f, L"", &midFormat);
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
        
        while (result < 250)
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

    PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);

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

        if (Hero)
        {
            if (Hero->in_battle)
            {
                if (RandMachine(0, 50) == 6)Hero->ChangeState(states::stand);
                
                if (Hero->GetState() == states::stand || Hero->GetState() == states::run)
                {
                    switch (RandMachine(0, 2))
                    {
                    case 0:
                        Hero->ChangeState(states::punch);
                        break;

                    case 1:
                        Hero->ChangeState(states::kick1);
                        break;

                    case 2:
                        Hero->ChangeState(states::kick2);
                        break;
                    }
                }
            }
            else if (Hero->GetState() == states::run) Hero->Move(hero_mx, hero_my, (float)(level));   
        }

        if (vEvils.size() < level + 2 && !vTombs.empty() && RandMachine(0, 300) == 66)
        {
            switch (RandMachine(0, 3))
            {
            case 0:
                {
                    int which_tomb = RandMachine(0, (int)(vTombs.size() - 1));
                    vEvils.push_back(dll::Factory(zombie1, vTombs[which_tomb].tomb.center.x, vTombs[which_tomb].tomb.center.y));
                }
                break;

            case 1:
            {
                int which_tomb = RandMachine(0, (int)(vTombs.size() - 1));
                vEvils.push_back(dll::Factory(zombie2, vTombs[which_tomb].tomb.center.x, vTombs[which_tomb].tomb.center.y));
            }
            break;

            case 2:
            {
                int which_tomb = RandMachine(0, (int)(vTombs.size() - 1));
                vEvils.push_back(dll::Factory(zombie3, vTombs[which_tomb].tomb.center.x, vTombs[which_tomb].tomb.center.y));
            }
            break;

            case 3:
            {
                int which_tomb = RandMachine(0, (int)(vTombs.size() - 1));
                vEvils.push_back(dll::Factory(zombie4, vTombs[which_tomb].tomb.center.x, vTombs[which_tomb].tomb.center.y));
            }
            break;

            }
        }

        if (vSouls.size() < 3 + level && RandMachine(0, 400) == 33)
            vSouls.push_back(dll::Factory(soul, (float)(RandMachine(50, 800)), 
                (float)(RandMachine((int)(up_ground_boundary), 600))));

        if (!vEvils.empty() && Hero)
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                dll::BAG<FPOINT> targets(vSouls.size() + 1);

                targets.push_back(Hero->center);

                if (!vSouls.empty())
                    for (std::vector<dll::Creature>::iterator soul = vSouls.begin(); soul < vSouls.end(); ++soul)
                        targets.push_back((*soul)->center);

                (*evil)->NextMove(targets, (float)(level));
            }
        }

        if (!vEvils.empty() && !vSouls.empty())
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                bool killed = false;

                for (std::vector<dll::Creature>::iterator soul = vSouls.begin(); soul < vSouls.end(); ++soul)
                {
                    if (!((*evil)->start.x >= (*soul)->end.x || (*evil)->end.x <= (*soul)->start.x
                        || (*evil)->start.y >= (*soul)->end.y || (*evil)->end.y <= (*soul)->start.y))
                    {
                        (*soul)->Release();
                        vSouls.erase(soul);
                        killed = true;
                        ++killed_souls;
                        if (sound)mciSendString(L"play .\\res\\snd\\soulkilled.wav", NULL, NULL, NULL);
                        break;
                    }
                }
                if (killed)break;
            }
        }

        if (Hero && !vSouls.empty())
        {
            for (std::vector<dll::Creature>::iterator soul = vSouls.begin(); soul < vSouls.end(); ++soul)
            {
                if (!(Hero->start.x >= (*soul)->end.x || Hero->end.x <= (*soul)->start.x
                    || Hero->start.y >= (*soul)->end.y || Hero->end.y <= (*soul)->start.y))
                {
                    (*soul)->Release();
                    vSouls.erase(soul);
                    ++saved_souls;
                    if (Hero->lifes + 10 <= 100)Hero->lifes += 10;
                    if (sound)mciSendString(L"play .\\res\\snd\\soulkilled.wav", NULL, NULL, NULL);
                    break;
                }
            }
        }

        if(Hero)Hero->in_battle = false;
        
        if (Hero && !vEvils.empty())
        {
            for (std::vector<dll::Creature>::iterator evil = vEvils.begin(); evil < vEvils.end(); ++evil)
            {
                if (!(Hero->start.x >= (*evil)->end.x || Hero->end.x <= (*evil)->start.x
                    || Hero->start.y >= (*evil)->end.y || Hero->end.y <= (*evil)->start.y))
                {
                    Hero->in_battle = true;

                    if (sound)mciSendString(L"play .\\res\\snd\\hurt.wav", NULL, NULL, NULL);

                    (*evil)->lifes -= Hero->Attack();
                    if ((*evil)->lifes <= 0)
                    {
                        if (RandMachine(0, 3) == 2)
                            vPotions.push_back(dll::PROTON((*evil)->center.x, (*evil)->center.y, 22.0f, 26.0f));
                        (*evil)->Release();
                        vEvils.erase(evil);
                        score += level;
                        Hero->in_battle = false;
                        if (sound)mciSendString(L"play .\\res\\snd\\evilkilled.wav", NULL, NULL, NULL);
                        break;
                    }

                    Hero->lifes -= (*evil)->Attack();
                    if (Hero->lifes <= 0)
                    {
                        hero_killed = true;
                        RIP_X = Hero->center.x;
                        RIP_Y = Hero->center.y;
                        ClearMem(&Hero);
                        break;
                    }
                    break;
                }
            }
        }

        if (Hero && !vPotions.empty())
        {
            for (std::vector<dll::PROTON>::iterator pot = vPotions.begin(); pot < vPotions.end(); ++pot)
            {
                if (!(Hero->start.x >= pot->end.x || Hero->end.x <= pot->start.x
                    || Hero->start.y >= pot->end.y || Hero->end.y <= pot->start.y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\life.wav", NULL, NULL, NULL);
                    Hero->lifes = 100;
                    vPotions.erase(pot);
                    break;
                }
            }
        }

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
        //////////////////////////////////////////////////////

        if (Hero)
        {
            switch (Hero->GetState())
            {
            case states::stand:
                if (Hero->dir == dirs::right || Hero->dir == dirs::stop)
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterStandR[aframe], Resizer(bmpFighterStandR[aframe], Hero->start.x, Hero->start.y));
                }
                else 
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterStandL[aframe], Resizer(bmpFighterStandL[aframe], Hero->start.x, Hero->start.y));
                }
                break;

            case states::run:
                if (Hero->dir == dirs::right || Hero->dir == dirs::stop)
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterStandR[aframe], Resizer(bmpFighterStandR[aframe], Hero->start.x, Hero->start.y));
                }
                else
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterStandL[aframe], Resizer(bmpFighterStandL[aframe], Hero->start.x, Hero->start.y));
                }
                break;

            case states::kick1:
                if (Hero->dir == dirs::right || Hero->dir == dirs::stop)
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterKick1R[aframe], Resizer(bmpFighterKick1R[aframe], Hero->start.x, Hero->start.y));
                }
                else
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterKick1L[aframe], Resizer(bmpFighterKick1L[aframe], Hero->start.x, Hero->start.y));
                }
                break;

            case states::kick2:
                if (Hero->dir == dirs::right || Hero->dir == dirs::stop)
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterKick2R[aframe], Resizer(bmpFighterKick2R[aframe], Hero->start.x, Hero->start.y));
                }
                else
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterKick2L[aframe], Resizer(bmpFighterKick2L[aframe], Hero->start.x, Hero->start.y));
                }
                break;

            case states::punch:
                if (Hero->dir == dirs::right || Hero->dir == dirs::stop)
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterPunchR[aframe], Resizer(bmpFighterPunchR[aframe], Hero->start.x, Hero->start.y));
                }
                else
                {
                    int aframe = Hero->GetFrame();
                    Draw->DrawBitmap(bmpFighterPunchL[aframe], Resizer(bmpFighterPunchL[aframe], Hero->start.x, Hero->start.y));
                }
                break;

            }

            Draw->DrawLine(D2D1::Point2F(Hero->start.x, Hero->end.y), D2D1::Point2F(Hero->start.x + (float)(Hero->lifes - 20),
                Hero->end.y), HgltBrush, 10.0f);
        }

        if (hero_killed)
        {
            Draw->DrawBitmap(bmpRip, D2D1::RectF(RIP_X, RIP_Y, RIP_X + 80.0f, RIP_Y + 94.0f));
            if (sound)
            {
                PlaySound(NULL, NULL, NULL);
                Draw->EndDraw();
                PlaySound(L".\\res\\snd\\killed.wav", NULL, SND_SYNC);
                GameOver();
            }
            else
            {
                Draw->EndDraw();
                Sleep(2000);
                GameOver();
            }
        }

        if (!vTombs.empty())
        {
            for (std::vector<TOMBS>::iterator it = vTombs.begin(); it < vTombs.end(); ++it)
            {
                switch (it->type)
                {
                case tomb:
                    Draw->DrawBitmap(bmpTomb, D2D1::RectF(it->tomb.start.x, it->tomb.start.y, it->tomb.end.x, it->tomb.end.y));
                    break;

                case house1:
                    Draw->DrawBitmap(bmpHouse1, D2D1::RectF(it->tomb.start.x, it->tomb.start.y, it->tomb.end.x, it->tomb.end.y));
                    break;

                case house2:
                    Draw->DrawBitmap(bmpHouse2, D2D1::RectF(it->tomb.start.x, it->tomb.start.y, it->tomb.end.x, it->tomb.end.y));
                    break;

                case house3:
                    Draw->DrawBitmap(bmpHouse3, D2D1::RectF(it->tomb.start.x, it->tomb.start.y, it->tomb.end.x, it->tomb.end.y));
                    break;
                }
            }
        }

        if (!vEvils.empty())
        {
            for (std::vector<dll::Creature>::iterator it = vEvils.begin(); it < vEvils.end(); ++it)
            {
                switch ((*it)->type)
                {
                case zombie1:
                    {
                        int aframe = (*it)->GetFrame();
                        Draw->DrawBitmap(bmpZombie1[aframe], Resizer(bmpZombie1[aframe], (*it)->start.x, (*it)->start.y));
                    }
                    break;

                case zombie2:
                {
                    int aframe = (*it)->GetFrame();
                    Draw->DrawBitmap(bmpZombie2[aframe], Resizer(bmpZombie2[aframe], (*it)->start.x, (*it)->start.y));
                }
                break;

                case zombie3:
                {
                    int aframe = (*it)->GetFrame();
                    Draw->DrawBitmap(bmpZombie3[aframe], Resizer(bmpZombie3[aframe], (*it)->start.x, (*it)->start.y));
                }
                break;

                case zombie4:
                {
                    int aframe = (*it)->GetFrame();
                    Draw->DrawBitmap(bmpZombie4[aframe], Resizer(bmpZombie4[aframe], (*it)->start.x, (*it)->start.y));
                }
                break;
                }

                Draw->DrawLine(D2D1::Point2F((*it)->start.x, (*it)->end.y), D2D1::Point2F((*it)->start.x + (float)((*it)->lifes / 3),
                    (*it)->end.y), TxtBrush, 10.0f);
            }
        }

        if (!vSouls.empty())
        {
            for (std::vector<dll::Creature>::iterator it = vSouls.begin(); it < vSouls.end(); ++it)
            {
                int aframe = (*it)->GetFrame();
                Draw->DrawBitmap(bmpSoul[aframe], Resizer(bmpSoul[aframe], (*it)->start.x, (*it)->start.y));
            }
        }

        if (!vPotions.empty())
        {
            for (int i = 0; i < vPotions.size(); ++i)
                Draw->DrawBitmap(bmpPotion, D2D1::RectF(vPotions[i].start.x, vPotions[i].start.y,
                    vPotions[i].end.x, vPotions[i].end.y));
        }


        // STATUS ********************

        if (midFormat && HgltBrush)
        {
            wchar_t txt[150] = L"герой: ";
            wchar_t add[5] = L"\0";
            int txt_size = 0;

            wcscat_s(txt, current_player);
            wcscat_s(txt, L", спасени души: ");
            wsprintf(add, L"%d", saved_souls);
            wcscat_s(txt, add);
            wcscat_s(txt, L", изгубени души: ");
            wsprintf(add, L"%d", killed_souls);
            wcscat_s(txt, add);
            wcscat_s(txt, L", ниво: ");
            wsprintf(add, L"%d", level);
            wcscat_s(txt, add);

            for (int i = 0; i < 150; ++i)
            {
                if (txt[i] != '\0')txt_size++;
                else break;
            }

            Draw->DrawTextW(txt, txt_size, midFormat, D2D1::RectF(10.0f, ground + 5.0f, scr_width, scr_height), HgltBrush);
        }

        ///////////////////////////

        Draw->EndDraw();
        
        /////////////////////////////

        if (saved_souls > 5 + level)LevelUp();
        if (killed_souls > 5 + level)GameOver();
    }

    ReleaseResources();
    std::remove(tmp_file);

    return (int) bMsg.wParam;
}
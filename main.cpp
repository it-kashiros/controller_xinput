/*****************************************************************//**
 * \file   main.cpp
 * \brief  コントローラー入力デバッグ用（XInput版）
 *********************************************************************/
#include <cstdio>
#include <cstring>
#include <conio.h>
#include <windows.h>
#include "game_controller.h"

 // カーソルを左上に戻す
void ClearScreen() {
    COORD coord = { 0, 0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// 固定幅で1行出力（79文字+改行）
void PrintLine(const char* pStr) {
    printf("%-79s\n", pStr);
}

// スティック用バー文字列生成
void GetStickBar(char* pBuf, size_t bufSize, float value) {
    int pos = static_cast<int>((value + 1.0f) * 6.0f);
    if (pos < 0) pos = 0;
    if (pos > 12) pos = 12;
    pBuf[0] = '[';
    for (int i = 0; i < 13; i++) {
        if (i == 6) pBuf[i + 1] = '|';
        else if (i == pos) pBuf[i + 1] = '*';
        else pBuf[i + 1] = '-';
    }
    pBuf[14] = ']';
    pBuf[15] = '\0';
}

// トリガー用バー文字列生成
void GetTriggerBar(char* pBuf, size_t bufSize, float value) {
    int filled = static_cast<int>(value * 10);
    pBuf[0] = '[';
    for (int i = 0; i < 10; i++) {
        pBuf[i + 1] = (i < filled) ? '=' : ' ';
    }
    pBuf[11] = ']';
    pBuf[12] = '\0';
}

// イベント文字列に追加
void AppendEvent(char* pEvent, size_t eventSize, const char* pText) {
    strcat_s(pEvent, eventSize, pText);
}

int main() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    // ウィンドウサイズ設定
    SMALL_RECT rect = { 0, 0, 79, 24 };
    SetConsoleWindowInfo(hConsole, TRUE, &rect);
    COORD bufSize = { 80, 25 };
    SetConsoleScreenBufferSize(hConsole, bufSize);

    // カーソル非表示
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    GameController::Initialize();

    char line[128];
    char barLX[16], barLY[16], barRX[16], barRY[16];
    char barLT[16], barRT[16];
    bool isRunning = true;

    while (isRunning) {
        // キー入力処理
        if (_kbhit()) {
            int key = _getch();
            switch (key) {
            case 27:  // ESC
                isRunning = false;
                break;
            case 'v':
            case 'V':
                GameController::StartVibration(1.0f, 0.5f);
                break;
            case 'b':
            case 'B':
                GameController::StartVibration(0.3f, 0.3f);
                break;
            }
        }

        GameController::Update();
        ClearScreen();

        if (!GameController::IsConnected()) {
            PrintLine("===============================================================================");
            PrintLine("                    XINPUT CONTROLLER DEBUG MONITOR                            ");
            PrintLine("===============================================================================");
            PrintLine("");
            PrintLine(" Controller not connected...");
            PrintLine("");
            PrintLine(" Waiting for XInput compatible controller (Xbox, etc.)");
            PrintLine("");
            for (int i = 0; i < 13; i++) {
                PrintLine("");
            }
            PrintLine("-------------------------------------------------------------------------------");
            PrintLine(" ESC: Exit");
            Sleep(100);
            continue;
        }

        const GamepadState& state = GameController::GetCurrentState();

        GetStickBar(barLX, sizeof(barLX), state.leftStickX);
        GetStickBar(barLY, sizeof(barLY), state.leftStickY);
        GetStickBar(barRX, sizeof(barRX), state.rightStickX);
        GetStickBar(barRY, sizeof(barRY), state.rightStickY);
        GetTriggerBar(barLT, sizeof(barLT), state.leftTrigger);
        GetTriggerBar(barRT, sizeof(barRT), state.rightTrigger);

        // ボタン表示用
        const char* pDpadUp = state.dpadUp ? "[U]" : " U ";
        const char* pDpadDown = state.dpadDown ? "[D]" : " D ";
        const char* pDpadLeft = state.dpadLeft ? "[L]" : " L ";
        const char* pDpadRight = state.dpadRight ? "[R]" : " R ";

        const char* pMainUp = state.buttonUp ? "[△]" : " △ ";
        const char* pMainDown = state.buttonDown ? "[×]" : " × ";
        const char* pMainLeft = state.buttonLeft ? "[□]" : " □ ";
        const char* pMainRight = state.buttonRight ? "[○]" : " ○ ";

        const char* pBtnL1 = state.buttonL1 ? "[LB]" : " LB ";
        const char* pBtnR1 = state.buttonR1 ? "[RB]" : " RB ";
        const char* pBtnL2 = state.buttonL2 ? "[LT]" : " LT ";
        const char* pBtnR2 = state.buttonR2 ? "[RT]" : " RT ";
        const char* pBtnL3 = state.buttonL3 ? "[LS]" : " LS ";
        const char* pBtnR3 = state.buttonR3 ? "[RS]" : " RS ";

        const char* pBtnSelect = state.buttonSelect ? "[BACK]" : " BACK  ";
        const char* pBtnStart = state.buttonStart ? "[START]" : " START ";

        const char* pVibration = GameController::IsVibrating() ? "[VIBRATING]" : "           ";

        PrintLine("===============================================================================");
        PrintLine("                    XINPUT CONTROLLER DEBUG MONITOR                            ");
        PrintLine("===============================================================================");

        sprintf_s(line, sizeof(line), " Status: Connected                                           %s", pVibration);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        sprintf_s(line, sizeof(line), " L Stick | X:%6.2f %s   Y:%6.2f %s",
            state.leftStickX, barLX, state.leftStickY, barLY);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " R Stick | X:%6.2f %s   Y:%6.2f %s",
            state.rightStickX, barRX, state.rightStickY, barRY);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " Trigger | LT:%5.2f %s    RT:%5.2f %s",
            state.leftTrigger, barLT, state.rightTrigger, barRT);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        sprintf_s(line, sizeof(line), "  D-PAD        %s                MAIN             %s", pDpadUp, pMainUp);
        PrintLine(line);

        sprintf_s(line, sizeof(line), "            %s   %s                           %s  %s",
            pDpadLeft, pDpadRight, pMainLeft, pMainRight);
        PrintLine(line);

        sprintf_s(line, sizeof(line), "               %s                                 %s", pDpadDown, pMainDown);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        sprintf_s(line, sizeof(line), " Shoulder: %s %s                                     %s %s",
            pBtnL1, pBtnL2, pBtnR2, pBtnR1);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " Stick   : %s                                             %s",
            pBtnL3, pBtnR3);
        PrintLine(line);

        sprintf_s(line, sizeof(line), " System  : %s                                      %s",
            pBtnSelect, pBtnStart);
        PrintLine(line);

        PrintLine("-------------------------------------------------------------------------------");

        char event[128] = " Event:";
        if (GameController::IsTrigger_ButtonDown())  AppendEvent(event, sizeof(event), " A+");
        if (GameController::IsTrigger_ButtonRight()) AppendEvent(event, sizeof(event), " B+");
        if (GameController::IsTrigger_ButtonLeft())  AppendEvent(event, sizeof(event), " X+");
        if (GameController::IsTrigger_ButtonUp())    AppendEvent(event, sizeof(event), " Y+");
        if (GameController::IsTrigger_L1())          AppendEvent(event, sizeof(event), " LB+");
        if (GameController::IsTrigger_R1())          AppendEvent(event, sizeof(event), " RB+");
        if (GameController::IsTrigger_L2())          AppendEvent(event, sizeof(event), " LT+");
        if (GameController::IsTrigger_R2())          AppendEvent(event, sizeof(event), " RT+");
        if (GameController::IsTrigger_L3())          AppendEvent(event, sizeof(event), " LS+");
        if (GameController::IsTrigger_R3())          AppendEvent(event, sizeof(event), " RS+");
        if (GameController::IsTrigger_Start())       AppendEvent(event, sizeof(event), " START+");
        if (GameController::IsTrigger_Select())      AppendEvent(event, sizeof(event), " BACK+");
        if (GameController::IsTrigger_DpadUp())      AppendEvent(event, sizeof(event), " U+");
        if (GameController::IsTrigger_DpadDown())    AppendEvent(event, sizeof(event), " D+");
        if (GameController::IsTrigger_DpadLeft())    AppendEvent(event, sizeof(event), " L+");
        if (GameController::IsTrigger_DpadRight())   AppendEvent(event, sizeof(event), " R+");
        if (GameController::IsRelease_ButtonDown())  AppendEvent(event, sizeof(event), " A-");
        if (GameController::IsRelease_ButtonRight()) AppendEvent(event, sizeof(event), " B-");
        if (GameController::IsRelease_ButtonLeft())  AppendEvent(event, sizeof(event), " X-");
        if (GameController::IsRelease_ButtonUp())    AppendEvent(event, sizeof(event), " Y-");
        if (GameController::IsRelease_L1())          AppendEvent(event, sizeof(event), " LB-");
        if (GameController::IsRelease_R1())          AppendEvent(event, sizeof(event), " RB-");
        if (GameController::IsRelease_L2())          AppendEvent(event, sizeof(event), " LT-");
        if (GameController::IsRelease_R2())          AppendEvent(event, sizeof(event), " RT-");
        if (GameController::IsRelease_L3())          AppendEvent(event, sizeof(event), " LS-");
        if (GameController::IsRelease_R3())          AppendEvent(event, sizeof(event), " RS-");
        if (GameController::IsRelease_Start())       AppendEvent(event, sizeof(event), " START-");
        if (GameController::IsRelease_Select())      AppendEvent(event, sizeof(event), " BACK-");
        if (GameController::IsRelease_DpadUp())      AppendEvent(event, sizeof(event), " U-");
        if (GameController::IsRelease_DpadDown())    AppendEvent(event, sizeof(event), " D-");
        if (GameController::IsRelease_DpadLeft())    AppendEvent(event, sizeof(event), " L-");
        if (GameController::IsRelease_DpadRight())   AppendEvent(event, sizeof(event), " R-");
        PrintLine(event);

        PrintLine("===============================================================================");
        PrintLine(" ESC: Exit  |  V: Vibration Strong  |  B: Vibration Weak");

        Sleep(16);
    }

    GameController::Finalize();

    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    return 0;
}

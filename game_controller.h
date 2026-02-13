/*****************************************************************//**
 * \file   game_controller.h
 * \brief  ゲームコントローラー入力管理（XInput版）
 *
 * \date   2026/1/5
 *********************************************************************/
#pragma once
#include <windows.h>
#include <Xinput.h>
#include <cmath>

#pragma comment(lib, "xinput.lib")

 //==============================================================================
 // ゲームパッド状態構造体
 //==============================================================================
struct GamepadState {
    // 左スティック（-1.0 ~ 1.0）
    float leftStickX = 0.0f;
    float leftStickY = 0.0f;

    // 右スティック（-1.0 ~ 1.0）
    float rightStickX = 0.0f;
    float rightStickY = 0.0f;

    // トリガー（0.0 ~ 1.0）
    float leftTrigger = 0.0f;
    float rightTrigger = 0.0f;

    // 十字キー
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;

    // メインボタン（位置ベース）
    bool buttonDown = false;   // A
    bool buttonRight = false;  // B
    bool buttonLeft = false;   // X
    bool buttonUp = false;     // Y

    // ショルダーボタン（LB/RB）
    bool buttonL1 = false;
    bool buttonR1 = false;

    // トリガーボタン（LT/RT）- デジタル判定
    bool buttonL2 = false;
    bool buttonR2 = false;

    // スティック押し込み（LS/RS）
    bool buttonL3 = false;
    bool buttonR3 = false;

    // システムボタン
    bool buttonStart = false;
    bool buttonSelect = false;

    // 接続状態
    bool connected = false;

    // いずれかのボタンが押されているか
    bool IsAnyButtonPressed() const {
        return buttonDown || buttonRight || buttonLeft || buttonUp ||
            buttonL1 || buttonR1 || buttonL2 || buttonR2 ||
            buttonL3 || buttonR3 ||
            buttonStart || buttonSelect ||
            dpadUp || dpadDown || dpadLeft || dpadRight;
    }

    // デッドゾーン適用
    static float ApplyDeadzone(float value, float deadzone = 0.15f) {
        if (std::fabs(value) < deadzone) return 0.0f;
        float sign = (value > 0) ? 1.0f : -1.0f;
        return sign * (std::fabs(value) - deadzone) / (1.0f - deadzone);
    }
};

//==============================================================================
// バイブレーション設定構造体
//==============================================================================
struct VibrationSettings {
    float leftMotor = 0.0f;   // 左モーター（低周波・重い振動）0.0~1.0
    float rightMotor = 0.0f;  // 右モーター（高周波・軽い振動）0.0~1.0
    float duration = 0.0f;    // 持続時間（秒）
};

//==============================================================================
// バッテリー情報構造体
//==============================================================================
struct BatteryInfo {
    bool isWired = false;           // 有線接続か
    bool hasBatteryInfo = false;    // バッテリー情報が有効か
    int level = 0;                  // バッテリーレベル（0:空、1:低、2:中、3:満）
    const char* levelText = "";     // レベルのテキスト表示
};

//==============================================================================
// コントローラー能力情報構造体
//==============================================================================
struct ControllerCapabilities {
    bool isValid = false;
    bool isGamepad = false;         // ゲームパッドか
    bool hasVoiceSupport = false;   // ボイスサポートあり
    bool hasFFB = false;            // フォースフィードバック対応
    bool isWireless = false;        // ワイヤレスか
    WORD buttons = 0;               // 対応ボタンのビットフラグ
    BYTE leftTrigger = 0;           // 左トリガーの最大値
    BYTE rightTrigger = 0;          // 右トリガーの最大値
    SHORT thumbLX = 0;              // 左スティックXの最大値
    SHORT thumbLY = 0;              // 左スティックYの最大値
    SHORT thumbRX = 0;              // 右スティックXの最大値
    SHORT thumbRY = 0;              // 右スティックYの最大値
};

//==============================================================================
// ゲームコントローラークラス（XInput版）
//==============================================================================
class GameController {
public:
    //==========================================================================
    // 初期化・終了・更新
    //==========================================================================
    static bool Initialize();
    static void Finalize();
    static void Update();

    //==========================================================================
    // 状態取得
    //==========================================================================
    static const GamepadState& GetCurrentState() { return s_currentState; }
    static const GamepadState& GetPrevState() { return s_prevState; }
    static DWORD GetControllerIndex() { return s_controllerIndex; }

    //==========================================================================
    // バイブレーション制御
    //==========================================================================
    // 両モーター同じ強度
    static void StartVibration(float intensity, float duration);
    // 左右モーター個別設定
    static void StartVibrationEx(float leftMotor, float rightMotor, float duration);
    // 設定構造体で指定
    static void StartVibrationEx(const VibrationSettings& settings);
    static void StopVibration();
    static bool IsVibrating() { return s_isVibrating; }

    //==========================================================================
    // バッテリー情報
    //==========================================================================
    static BatteryInfo GetBatteryInfo();

    //==========================================================================
    // コントローラー能力
    //==========================================================================
    static ControllerCapabilities GetCapabilities();

    //==========================================================================
    // キーストローク取得（テキスト入力用）
    //==========================================================================
    static bool GetKeystroke(XINPUT_KEYSTROKE* pKeystroke);

    //==========================================================================
    // オーディオデバイスID取得（ヘッドセット用）
    //==========================================================================
    static bool GetAudioDeviceIds(LPWSTR pRenderDeviceId, UINT* pRenderCount,
        LPWSTR pCaptureDeviceId, UINT* pCaptureCount);

    //==========================================================================
    // Press判定（押している間ずっとtrue）
    //==========================================================================
    static bool IsPressed_ButtonDown() { return s_currentState.buttonDown; }
    static bool IsPressed_ButtonRight() { return s_currentState.buttonRight; }
    static bool IsPressed_ButtonLeft() { return s_currentState.buttonLeft; }
    static bool IsPressed_ButtonUp() { return s_currentState.buttonUp; }
    static bool IsPressed_L1() { return s_currentState.buttonL1; }
    static bool IsPressed_R1() { return s_currentState.buttonR1; }
    static bool IsPressed_L2() { return s_currentState.buttonL2; }
    static bool IsPressed_R2() { return s_currentState.buttonR2; }
    static bool IsPressed_L3() { return s_currentState.buttonL3; }
    static bool IsPressed_R3() { return s_currentState.buttonR3; }
    static bool IsPressed_Start() { return s_currentState.buttonStart; }
    static bool IsPressed_Select() { return s_currentState.buttonSelect; }
    static bool IsPressed_DpadUp() { return s_currentState.dpadUp; }
    static bool IsPressed_DpadDown() { return s_currentState.dpadDown; }
    static bool IsPressed_DpadLeft() { return s_currentState.dpadLeft; }
    static bool IsPressed_DpadRight() { return s_currentState.dpadRight; }

    //==========================================================================
    // Trigger判定（押した瞬間だけtrue）
    //==========================================================================
    static bool IsTrigger_ButtonDown() { return s_currentState.buttonDown && !s_prevState.buttonDown; }
    static bool IsTrigger_ButtonRight() { return s_currentState.buttonRight && !s_prevState.buttonRight; }
    static bool IsTrigger_ButtonLeft() { return s_currentState.buttonLeft && !s_prevState.buttonLeft; }
    static bool IsTrigger_ButtonUp() { return s_currentState.buttonUp && !s_prevState.buttonUp; }
    static bool IsTrigger_L1() { return s_currentState.buttonL1 && !s_prevState.buttonL1; }
    static bool IsTrigger_R1() { return s_currentState.buttonR1 && !s_prevState.buttonR1; }
    static bool IsTrigger_L2() { return s_currentState.buttonL2 && !s_prevState.buttonL2; }
    static bool IsTrigger_R2() { return s_currentState.buttonR2 && !s_prevState.buttonR2; }
    static bool IsTrigger_L3() { return s_currentState.buttonL3 && !s_prevState.buttonL3; }
    static bool IsTrigger_R3() { return s_currentState.buttonR3 && !s_prevState.buttonR3; }
    static bool IsTrigger_Start() { return s_currentState.buttonStart && !s_prevState.buttonStart; }
    static bool IsTrigger_Select() { return s_currentState.buttonSelect && !s_prevState.buttonSelect; }
    static bool IsTrigger_DpadUp() { return s_currentState.dpadUp && !s_prevState.dpadUp; }
    static bool IsTrigger_DpadDown() { return s_currentState.dpadDown && !s_prevState.dpadDown; }
    static bool IsTrigger_DpadLeft() { return s_currentState.dpadLeft && !s_prevState.dpadLeft; }
    static bool IsTrigger_DpadRight() { return s_currentState.dpadRight && !s_prevState.dpadRight; }

    //==========================================================================
    // Release判定（離した瞬間だけtrue）
    //==========================================================================
    static bool IsRelease_ButtonDown() { return !s_currentState.buttonDown && s_prevState.buttonDown; }
    static bool IsRelease_ButtonRight() { return !s_currentState.buttonRight && s_prevState.buttonRight; }
    static bool IsRelease_ButtonLeft() { return !s_currentState.buttonLeft && s_prevState.buttonLeft; }
    static bool IsRelease_ButtonUp() { return !s_currentState.buttonUp && s_prevState.buttonUp; }
    static bool IsRelease_L1() { return !s_currentState.buttonL1 && s_prevState.buttonL1; }
    static bool IsRelease_R1() { return !s_currentState.buttonR1 && s_prevState.buttonR1; }
    static bool IsRelease_L2() { return !s_currentState.buttonL2 && s_prevState.buttonL2; }
    static bool IsRelease_R2() { return !s_currentState.buttonR2 && s_prevState.buttonR2; }
    static bool IsRelease_L3() { return !s_currentState.buttonL3 && s_prevState.buttonL3; }
    static bool IsRelease_R3() { return !s_currentState.buttonR3 && s_prevState.buttonR3; }
    static bool IsRelease_Start() { return !s_currentState.buttonStart && s_prevState.buttonStart; }
    static bool IsRelease_Select() { return !s_currentState.buttonSelect && s_prevState.buttonSelect; }
    static bool IsRelease_DpadUp() { return !s_currentState.dpadUp && s_prevState.dpadUp; }
    static bool IsRelease_DpadDown() { return !s_currentState.dpadDown && s_prevState.dpadDown; }
    static bool IsRelease_DpadLeft() { return !s_currentState.dpadLeft && s_prevState.dpadLeft; }
    static bool IsRelease_DpadRight() { return !s_currentState.dpadRight && s_prevState.dpadRight; }

    //==========================================================================
    // スティック・トリガー値取得
    //==========================================================================
    static float GetLeftStickX() { return s_currentState.leftStickX; }
    static float GetLeftStickY() { return s_currentState.leftStickY; }
    static float GetRightStickX() { return s_currentState.rightStickX; }
    static float GetRightStickY() { return s_currentState.rightStickY; }
    static float GetLeftTrigger() { return s_currentState.leftTrigger; }
    static float GetRightTrigger() { return s_currentState.rightTrigger; }

    //==========================================================================
    // 接続状態
    //==========================================================================
    static bool IsConnected() { return s_currentState.connected; }

private:
    static bool UpdateState();
    static float NormalizeStickValue(SHORT value, SHORT deadzone);
    static float NormalizeTriggerValue(BYTE value, BYTE threshold);

    // コントローラーインデックス（0-3）
    static DWORD s_controllerIndex;

    // 現在フレームと前フレームの状態
    static GamepadState s_currentState;
    static GamepadState s_prevState;

    // バイブレーション関連
    static bool s_isVibrating;
    static DWORD s_vibrationEndTime;
    static float s_leftMotorSpeed;
    static float s_rightMotorSpeed;
};

/*****************************************************************//**
 * \file   game_controller.cpp
 * \brief  ゲームコントローラー入力管理（XInput版）
 *
 * \date   2026/1/5
 *********************************************************************/

 // Windows.hのmin/maxマクロを無効化
#define NOMINMAX
#include "game_controller.h"
#include <algorithm>

//==============================================================================
// 静的メンバ変数の定義
//==============================================================================
DWORD GameController::s_controllerIndex = 0;
GamepadState GameController::s_currentState = {};
GamepadState GameController::s_prevState = {};
bool GameController::s_isVibrating = false;
DWORD GameController::s_vibrationEndTime = 0;
float GameController::s_leftMotorSpeed = 0.0f;
float GameController::s_rightMotorSpeed = 0.0f;

//==============================================================================
// 定数定義
//==============================================================================
namespace {
    // デッドゾーン
    constexpr SHORT STICK_DEADZONE_LEFT = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    constexpr SHORT STICK_DEADZONE_RIGHT = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
    constexpr BYTE TRIGGER_THRESHOLD = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;

    // トリガーをデジタルボタンとして判定する閾値（50%）
    constexpr BYTE TRIGGER_DIGITAL_THRESHOLD = 128;

    // クランプ関数（自前実装）
    template<typename T>
    T Clamp(T value, T minVal, T maxVal) {
        if (value < minVal) return minVal;
        if (value > maxVal) return maxVal;
        return value;
    }

    // 最小値（自前実装）
    template<typename T>
    T Min(T a, T b) {
        return (a < b) ? a : b;
    }

    // 最大値（自前実装）
    template<typename T>
    T Max(T a, T b) {
        return (a > b) ? a : b;
    }
}

//==============================================================================
// 初期化
//==============================================================================
bool GameController::Initialize() {
    s_controllerIndex = 0;
    s_currentState = {};
    s_prevState = {};
    s_isVibrating = false;
    s_vibrationEndTime = 0;
    s_leftMotorSpeed = 0.0f;
    s_rightMotorSpeed = 0.0f;

    // 接続されているコントローラーを探す
    for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
        XINPUT_STATE state;
        if (XInputGetState(i, &state) == ERROR_SUCCESS) {
            s_controllerIndex = i;
            return true;
        }
    }
    return false;
}

//==============================================================================
// 終了処理
//==============================================================================
void GameController::Finalize() {
    StopVibration();
    s_currentState = {};
    s_prevState = {};
}

//==============================================================================
// 更新（毎フレーム呼び出し）
//==============================================================================
void GameController::Update() {
    UpdateState();

    // バイブレーションの時間管理
    if (s_isVibrating && GetTickCount64() >= s_vibrationEndTime) {
        StopVibration();
    }
}

//==============================================================================
// 状態更新
//==============================================================================
bool GameController::UpdateState() {
    s_prevState = s_currentState;

    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    DWORD result = XInputGetState(s_controllerIndex, &state);

    // 接続されていない場合、他のコントローラーを探す
    if (result != ERROR_SUCCESS) {
        for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
            if (XInputGetState(i, &state) == ERROR_SUCCESS) {
                s_controllerIndex = i;
                result = ERROR_SUCCESS;
                break;
            }
        }
        if (result != ERROR_SUCCESS) {
            s_currentState.connected = false;
            return false;
        }
    }

    s_currentState.connected = true;
    WORD buttons = state.Gamepad.wButtons;

    // 十字キー
    s_currentState.dpadUp = (buttons & XINPUT_GAMEPAD_DPAD_UP) != 0;
    s_currentState.dpadDown = (buttons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
    s_currentState.dpadLeft = (buttons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
    s_currentState.dpadRight = (buttons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

    // メインボタン
    s_currentState.buttonDown = (buttons & XINPUT_GAMEPAD_A) != 0;
    s_currentState.buttonRight = (buttons & XINPUT_GAMEPAD_B) != 0;
    s_currentState.buttonLeft = (buttons & XINPUT_GAMEPAD_X) != 0;
    s_currentState.buttonUp = (buttons & XINPUT_GAMEPAD_Y) != 0;

    // ショルダーボタン
    s_currentState.buttonL1 = (buttons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
    s_currentState.buttonR1 = (buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;

    // スティック押し込み
    s_currentState.buttonL3 = (buttons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
    s_currentState.buttonR3 = (buttons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;

    // システムボタン
    s_currentState.buttonStart = (buttons & XINPUT_GAMEPAD_START) != 0;
    s_currentState.buttonSelect = (buttons & XINPUT_GAMEPAD_BACK) != 0;

    // トリガー
    s_currentState.leftTrigger = NormalizeTriggerValue(state.Gamepad.bLeftTrigger, TRIGGER_THRESHOLD);
    s_currentState.rightTrigger = NormalizeTriggerValue(state.Gamepad.bRightTrigger, TRIGGER_THRESHOLD);
    s_currentState.buttonL2 = (state.Gamepad.bLeftTrigger > TRIGGER_DIGITAL_THRESHOLD);
    s_currentState.buttonR2 = (state.Gamepad.bRightTrigger > TRIGGER_DIGITAL_THRESHOLD);

    // スティック
    float rawLeftX = NormalizeStickValue(state.Gamepad.sThumbLX, STICK_DEADZONE_LEFT);
    float rawLeftY = NormalizeStickValue(state.Gamepad.sThumbLY, STICK_DEADZONE_LEFT);
    float rawRightX = NormalizeStickValue(state.Gamepad.sThumbRX, STICK_DEADZONE_RIGHT);
    float rawRightY = NormalizeStickValue(state.Gamepad.sThumbRY, STICK_DEADZONE_RIGHT);

    s_currentState.leftStickX = GamepadState::ApplyDeadzone(rawLeftX);
    s_currentState.leftStickY = GamepadState::ApplyDeadzone(-rawLeftY);
    s_currentState.rightStickX = GamepadState::ApplyDeadzone(rawRightX);
    s_currentState.rightStickY = GamepadState::ApplyDeadzone(-rawRightY);

    return true;
}

//==============================================================================
// スティック値の正規化
//==============================================================================
float GameController::NormalizeStickValue(SHORT value, SHORT deadzone) {
    if (value < 0) {
        if (value > -deadzone) return 0.0f;
    } else {
        if (value < deadzone) return 0.0f;
    }

    constexpr float MAX_VALUE = 32767.0f;
    float normalizedValue;

    if (value > 0) {
        normalizedValue = static_cast<float>(value - deadzone) / (MAX_VALUE - deadzone);
    } else {
        normalizedValue = static_cast<float>(value + deadzone) / (MAX_VALUE - deadzone);
    }

    return Clamp(normalizedValue, -1.0f, 1.0f);
}

//==============================================================================
// トリガー値の正規化
//==============================================================================
float GameController::NormalizeTriggerValue(BYTE value, BYTE threshold) {
    if (value < threshold) {
        return 0.0f;
    }

    constexpr float MAX_VALUE = 255.0f;
    float normalizedValue = static_cast<float>(value - threshold) / (MAX_VALUE - threshold);
    return Min(1.0f, normalizedValue);
}

//==============================================================================
// バイブレーション開始（両モーター同じ強度）
//==============================================================================
void GameController::StartVibration(float intensity, float duration) {
    StartVibrationEx(intensity, intensity, duration);
}

//==============================================================================
// バイブレーション開始（左右個別）
//==============================================================================
void GameController::StartVibrationEx(float leftMotor, float rightMotor, float duration) {
    s_leftMotorSpeed = Clamp(leftMotor, 0.0f, 1.0f);
    s_rightMotorSpeed = Clamp(rightMotor, 0.0f, 1.0f);

    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    vibration.wLeftMotorSpeed = static_cast<WORD>(s_leftMotorSpeed * 65535.0f);
    vibration.wRightMotorSpeed = static_cast<WORD>(s_rightMotorSpeed * 65535.0f);

    XInputSetState(s_controllerIndex, &vibration);

    s_isVibrating = true;
    s_vibrationEndTime = GetTickCount64() + static_cast<DWORD>(duration * 1000.0f);
}

//==============================================================================
// バイブレーション開始（設定構造体）
//==============================================================================
void GameController::StartVibrationEx(const VibrationSettings& settings) {
    StartVibrationEx(settings.leftMotor, settings.rightMotor, settings.duration);
}

//==============================================================================
// バイブレーション停止
//==============================================================================
void GameController::StopVibration() {
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    XInputSetState(s_controllerIndex, &vibration);

    s_isVibrating = false;
    s_leftMotorSpeed = 0.0f;
    s_rightMotorSpeed = 0.0f;
}

//==============================================================================
// バッテリー情報取得
//==============================================================================
BatteryInfo GameController::GetBatteryInfo() {
    BatteryInfo info = {};

    XINPUT_BATTERY_INFORMATION batteryInfo;
    DWORD result = XInputGetBatteryInformation(s_controllerIndex, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo);

    if (result != ERROR_SUCCESS) {
        return info;
    }

    info.hasBatteryInfo = true;

    switch (batteryInfo.BatteryType) {
    case BATTERY_TYPE_WIRED:
        info.isWired = true;
        info.levelText = "Wired";
        info.level = 3;
        break;
    case BATTERY_TYPE_ALKALINE:
    case BATTERY_TYPE_NIMH:
        info.isWired = false;
        switch (batteryInfo.BatteryLevel) {
        case BATTERY_LEVEL_EMPTY:
            info.level = 0;
            info.levelText = "Empty";
            break;
        case BATTERY_LEVEL_LOW:
            info.level = 1;
            info.levelText = "Low";
            break;
        case BATTERY_LEVEL_MEDIUM:
            info.level = 2;
            info.levelText = "Medium";
            break;
        case BATTERY_LEVEL_FULL:
            info.level = 3;
            info.levelText = "Full";
            break;
        default:
            info.levelText = "Unknown";
            break;
        }
        break;
    case BATTERY_TYPE_UNKNOWN:
    default:
        info.levelText = "Unknown";
        break;
    }

    return info;
}

//==============================================================================
// コントローラー能力取得
//==============================================================================
ControllerCapabilities GameController::GetCapabilities() {
    ControllerCapabilities caps = {};

    XINPUT_CAPABILITIES xinputCaps;
    DWORD result = XInputGetCapabilities(s_controllerIndex, XINPUT_FLAG_GAMEPAD, &xinputCaps);

    if (result != ERROR_SUCCESS) {
        return caps;
    }

    caps.isValid = true;
    caps.isGamepad = (xinputCaps.Type == XINPUT_DEVTYPE_GAMEPAD);
    caps.hasVoiceSupport = (xinputCaps.Flags & XINPUT_CAPS_VOICE_SUPPORTED) != 0;
    caps.hasFFB = (xinputCaps.Flags & XINPUT_CAPS_FFB_SUPPORTED) != 0;
    caps.isWireless = (xinputCaps.Flags & XINPUT_CAPS_WIRELESS) != 0;
    caps.buttons = xinputCaps.Gamepad.wButtons;
    caps.leftTrigger = xinputCaps.Gamepad.bLeftTrigger;
    caps.rightTrigger = xinputCaps.Gamepad.bRightTrigger;
    caps.thumbLX = xinputCaps.Gamepad.sThumbLX;
    caps.thumbLY = xinputCaps.Gamepad.sThumbLY;
    caps.thumbRX = xinputCaps.Gamepad.sThumbRX;
    caps.thumbRY = xinputCaps.Gamepad.sThumbRY;

    return caps;
}

//==============================================================================
// キーストローク取得
//==============================================================================
bool GameController::GetKeystroke(XINPUT_KEYSTROKE* pKeystroke) {
    if (pKeystroke == nullptr) {
        return false;
    }

    DWORD result = XInputGetKeystroke(s_controllerIndex, 0, pKeystroke);
    return (result == ERROR_SUCCESS);
}

//==============================================================================
// オーディオデバイスID取得
//==============================================================================
bool GameController::GetAudioDeviceIds(LPWSTR pRenderDeviceId, UINT* pRenderCount,
    LPWSTR pCaptureDeviceId, UINT* pCaptureCount) {
    DWORD result = XInputGetAudioDeviceIds(s_controllerIndex,
        pRenderDeviceId, pRenderCount,
        pCaptureDeviceId, pCaptureCount);
    return (result == ERROR_SUCCESS);
}

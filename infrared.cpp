#include "infrared.h"
#include "bsp.h"
#include <IRremote.h>

const String INFRARED_PRESET_PREFIX = "ir_preset_";

infrared_code_t *ir_preset[] = {NULL, NULL, NULL, NULL};
constexpr int IR_PRESET_NUM = sizeof(ir_preset) / sizeof(ir_preset[0]);

bool ir_is_capturing = false;

bool Infrared_IsCapturing() { return ir_is_capturing; }

#define IS_VALID_PRESET_NUM(N) ((N) >= 0 && (N) < IR_PRESET_NUM)

#define ASSERT_PRESET_NUM(N)                   \
    do {                                       \
        if (!IS_VALID_PRESET_NUM(n)) {         \
            log_e("invalid preset num %d", n); \
            return false;                      \
        }                                      \
    } while (0)

bool Infrared_UpdatePreset(int n, infrared_code_t ir_code) {
    ASSERT_PRESET_NUM(n);

    if (ir_preset[n] == NULL) {
        ir_preset[n] = (infrared_code_t *)malloc(sizeof(infrared_code_t));
        if (ir_preset[n] == NULL) {
            log_e("out of memory");
            return false;
        }
    }
    memset(ir_preset[n], 0, sizeof(infrared_code_t));
    memcpy(ir_preset[n], &ir_code, sizeof(infrared_code_t));
    log_i("preset %d updated", n);
    return true;
}

bool Infrared_GetPreset(int n, infrared_code_t &ir_code) {
    ASSERT_PRESET_NUM(n);

    if (ir_preset[n] == NULL)
        return false;
    memcpy(&ir_code, ir_preset[n], sizeof(infrared_code_t));
    return true;
}

bool Infrared_SendPreset(int n) {
    ASSERT_PRESET_NUM(n);

    if (ir_preset[n] == NULL) {
        log_e("preset %d does not exist", n);
        return false;
    }
    IrSender.begin(INFRARED_OUT_PIN, true);
    IrSender.sendRaw(ir_preset[n]->code, ir_preset[n]->len, 38);
    log_i("infrared preset %d sent", n);
    return true;
}

void Infrared_StartCapture() {
    log_i("infrared capture start");
    ir_is_capturing = true;
    IrReceiver.begin(INFRARED_IN_PIN);
}

bool Infrared_EndCapture(int n) {
    ASSERT_PRESET_NUM(n);

    ir_is_capturing = false;
    int len = IrReceiver.decodedIRData.rawDataPtr->rawlen - 1;
    if (len <= 5) { // 红外跳变少于5次视为捕获失败 
        log_e("capture failed, len:%d", len);
        IrReceiver.end();
        return false;
    }
    infrared_code_t ir_code;
    IrReceiver.compensateAndStoreIRResultInArray(ir_code.code);
    ir_code.len = len;
    IrReceiver.end();
    Infrared_UpdatePreset(n, ir_code);
    log_i("capture end, stored in preset %d", n);
    log_i("code[%d] captured", len);
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_INFO
    for (auto &&i : ir_code.code)
        Serial.printf("%u, ", i);
    Serial.println();
#endif
    return true;
}

void Infrared_RestorePreset(Preferences &pref) {
    infrared_code_t ir_code;
    for (int i = 0; i < IR_PRESET_NUM; i++) {
        int len = pref.getBytes((INFRARED_PRESET_PREFIX + i).c_str(),
                                &ir_code,
                                sizeof(infrared_code_t));
        if (len == sizeof(infrared_code_t)) {
            Infrared_UpdatePreset(i, ir_code);
            log_i("preset %d restored", i);
        } else {
            log_i("preset %d invalid", i);
        }
    }
}

void Infrared_StorePreset(Preferences &pref) {
    infrared_code_t ir_code;
    for (int i = 0; i < IR_PRESET_NUM; i++) {
        if (Infrared_GetPreset(i, ir_code)) {
            pref.putBytes((INFRARED_PRESET_PREFIX + i).c_str(),
                          &ir_code,
                          sizeof(infrared_code_t));
            log_i("preset %d stored", i);
        }
    }
}

bool Infrared_RemovePreset(int n, Preferences &pref) {
    ASSERT_PRESET_NUM(n);
    if (ir_preset[n] != NULL) {
        free(ir_preset[n]);
        ir_preset[n] = NULL;
    }
    bool ret = pref.remove((INFRARED_PRESET_PREFIX + n).c_str());
    log_i("preset %d removed:%d", n, ret);
    return ret;
}

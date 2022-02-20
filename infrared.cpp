#include "infrared.h"
#include "bsp.h"
#include <IRremote.h>
#include <vector>

typedef struct {
    uint16_t code[INFRARED_MAX_CODE_LENGTH];
    uint16_t len;
} infrared_code_t;

const String INFRARED_PRESET_PREFIX = "ir_preset_";

infrared_code_t *ir_preset[] = {NULL, NULL, NULL, NULL};
constexpr int IR_PRESET_NUM = sizeof(ir_preset) / sizeof(ir_preset[0]);

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

void dump_ir_code(infrared_code_t& ir_code) {
    if (ir_code.len > 1) {
        Serial.printf("Dump: len:%d\n", ir_code.len);
        for (int i = 0; i < ir_code.len; i++) {
            Serial.printf("%u, ", ir_code.code[i]);
        }
        Serial.println();
    } else {
        log_e("Nothing to dump");
    }
}

infrared_code_t ir_capture(int timeout) {
    infrared_code_t ir_code;
    memset(&ir_code, 0, sizeof(infrared_code_t));

    unsigned long t = millis();
    int current, last = digitalRead(INFRARED_IN_PIN);
    int64_t current_t, last_t = esp_timer_get_time();

    do {
        do {
            current = digitalRead(INFRARED_IN_PIN);
        } while (current == last && t + timeout > millis());
        current_t = esp_timer_get_time();
        ir_code.code[ir_code.len++] = (uint16_t)(current_t - last_t);
        last = current;
        last_t = current_t;
    } while (t + timeout > millis());

    if (ir_code.len > 5) { // first and last data is useless;
        for (int i = 1; i < ir_code.len - 1; i++) {
            ir_code.code[i - 1] = ir_code.code[i];
        }
        ir_code.len -= 2;
    } else {
        ir_code.len = 0;
    }
    return ir_code;
}

bool Infrared_Capture(int n) {
    ASSERT_PRESET_NUM(n);

    log_i("infrared capture %d start", n);
    OLED_FullScreenMsg("Infrared Capturing...");    
    infrared_code_t ir_captured = ir_capture(INFRARED_CAPTURE_TIMEOUT);
    bool res = ir_captured.len >= 5; // 红外跳变少于5次视为捕获失败
    OLED_FullScreenMsg(res ? "SUCCESS!" : "FAIL!");
    if (res) { 
        log_i("capture end, stored in preset %d", n);
        dump_ir_code(ir_captured);
        Infrared_UpdatePreset(n, ir_captured);
        Infrared_StorePreset(n, Preferences_Get());
    } else {
        log_e("capture failed, len:%d", ir_captured.len);
    }
    delay(500);
    return res;
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
            log_w("preset %d invalid", i);
        }
    }
}

bool Infrared_StorePreset(int n, Preferences &pref) {
    ASSERT_PRESET_NUM(n);
    infrared_code_t ir_code;
    if (Infrared_GetPreset(n, ir_code)) {
        pref.putBytes((INFRARED_PRESET_PREFIX + n).c_str(),
                      &ir_code,
                      sizeof(infrared_code_t));
        log_i("preset %d stored", n);
        return true;
    } else {
        return false;
    }
}

void Infrared_StorePreset(Preferences &pref) {
    for (int i = 0; i < IR_PRESET_NUM; i++) {
        Infrared_StorePreset(i, pref);
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

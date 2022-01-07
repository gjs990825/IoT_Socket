#include "infrared.h"
#include "bsp.h"
#include <IRremote.h>

infrared_code_t *ir_preset[] = {NULL, NULL, NULL, NULL};
constexpr int IR_PRESET_NUM = sizeof(ir_preset) / sizeof(ir_preset[0]);

#define IS_VALID_PRESET_NUM(N) ((N) >= 0 && (N) < IR_PRESET_NUM)

#define ASSERT_PRESET_NUM(N)                   \
    do                                         \
    {                                          \
        if (!IS_VALID_PRESET_NUM(n))           \
        {                                      \
            log_e("invalid preset num %d", n); \
            return;                            \
        }                                      \
    } while (0)

void Infrared_UpdatePreset(int n, infrared_code_t ir_code)
{
    ASSERT_PRESET_NUM(n);
    if (ir_preset[n] == NULL)
    {
        ir_preset[n] = (infrared_code_t *)malloc(sizeof(infrared_code_t));
        if (ir_preset[n] == NULL)
        {
            log_e("out of memory");
            return;
        }
    }
    memset(ir_preset[n], 0, sizeof(infrared_code_t));
    memcpy(ir_preset[n], &ir_code, sizeof(infrared_code_t));
    log_i("preset %d updated", n);
}

void Infrared_GetPreset(int n, infrared_code_t &ir_code)
{
    ASSERT_PRESET_NUM(n);
    if (ir_preset[n] != NULL)
        memcpy(&ir_code, ir_preset[n], sizeof(infrared_code_t));
}

void Infrared_SendPreset(int n)
{
    ASSERT_PRESET_NUM(n);
    if (ir_preset[n] == NULL)
    {
        log_e("%d infrared preset does not exist", n);
        return;
    }

    IrSender.begin(IR_OUT_PIN, true);
    IrSender.sendRaw(ir_preset[n]->code, ir_preset[n]->len, 38);
    log_i("infrared preset %d sent", 0);
}

void Infrared_StartCapture()
{
    log_i("infrared capture start");
    IrReceiver.begin(IR_IN_PIN);
}

void Infrared_EndCapture(int n)
{
    ASSERT_PRESET_NUM(n);

    int len = IrReceiver.decodedIRData.rawDataPtr->rawlen - 1;
    if (len <= 5) // 红外跳变少于5次视为捕获失败 
    {
        log_e("capture failed, len:%d", len);
        return;
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
}

void Infrared_RestorePreset(Preferences &pref)
{
    infrared_code_t ir_code;
    String prefix = "ir_preset_";
    for (int i = 0; i < IR_PRESET_NUM; i++)
    {
        int len = pref.getBytes((prefix + i).c_str(),
                                &ir_code,
                                sizeof(infrared_code_t));
        if (len == sizeof(infrared_code_t))
        {
            Infrared_UpdatePreset(i, ir_code);
            log_i("preset %d restored", i);
        }
        else
        {
            log_i("preset %d invalid", i);
        }
    }
}

void Infrared_StorePreset(Preferences &pref)
{
    infrared_code_t ir_code;
    String prefix = "ir_preset_";
    for (int i = 0; i < IR_PRESET_NUM; i++)
    {
        ir_code.len = 0;
        Infrared_GetPreset(i, ir_code);
        if (ir_code.len != 0)
        {
            pref.putBytes((prefix + i).c_str(),
                          &ir_code,
                          sizeof(infrared_code_t));
            log_i("preset %d stored", i);
        }
    }
}

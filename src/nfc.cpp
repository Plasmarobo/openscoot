#include "nfc.h"

#if defined(ARDUINO)
#include <Base64.h>
#include <ST25DVSensor.h>

#include "scheduler.h"
#include "trace.h"

// #define GPO_PIN
// #define LPD_PIN
// #define SDA_PIN
// #define SCL_PIN
// #define WireNFC i2cDev

#define NFC_INT_PIN (12)
#define DEV_I2C Wire
#define MAX_UNENCODED_LEN ((NFC_MESSAGE_SIZE * 3) / 4)
#define NFC_READ_DELAY_MS (250)
namespace {
void nfc_update_cb(void* ctx);

Scheduler* sched_ref;
ST25DV st32dv(12, -1, &DEV_I2C);
char b64_workspace[NFC_MESSAGE_SIZE + 1];
NFCMessageBuffer msg_buffer;

void nfc_update_cb(void* ctx) {
    ENTER;
    // Copy data from tag into msg buffer
    NDEF* protocol = st32dv.getNDEF();
    if (NULL != protocol) {
        sRecordInfo_t sri;
        NDEF_Text_info_t txt;
        protocol->NDEF_ReadText(&sri, &txt);
        // We don't care about encoding since were processing b64 data
        Base64.decode((char*)&msg_buffer, txt.text, strlen(txt.text));
    }
    EXIT;
}

void nfc_isr() {
    // ISR for RF activity on the NFC
    // Give it a delay, then read
    call_deferred(sched_ref, SCHED_MILLISECONDS(NFC_READ_DELAY_MS),
                  nfc_update_cb, __PRETTY_FUNCTION__);
}
}  // namespace

void nfc_init(Scheduler* sched) {
    ENTER;
    sched_ref = sched;
    // Set an interrupt on the GPO pin
    pinMode(NFC_INT_PIN, INPUT_PULLUP);
    attachInterrupt(NFC_INT_PIN, nfc_isr, FALLING);
    EXIT;
}

NFCMessageBuffer get_nfc_data() { return msg_buffer; }

void set_nfc_data(NFCMessageBuffer msg) {
    NDEF* protocol = st32dv.getNDEF();
    if (NULL != protocol) {
        memset(b64_workspace, '\0', NFC_MESSAGE_SIZE + 1);
        Base64.encode(b64_workspace, (char*)&msg, sizeof(msg));
        protocol->NDEF_WriteText(b64_workspace);
    }
}
#else
void nfc_init() {}
NFCMessageBuffer get_nfc_data() { return {}; }
void set_nfc_data(NFCMessageBuffer data) {}
#endif

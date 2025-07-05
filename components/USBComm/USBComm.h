#ifndef USB_COMM_H
#define USB_COMM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void usb_comm_init();
void usb_send_data(const char* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif // USB_COMM_H
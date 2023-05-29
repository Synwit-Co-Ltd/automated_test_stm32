/*-
 * BSD 2-Clause License
 *
 * Copyright (c) 2012-2018, Jan Breuer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scpi/scpi.h"
#include "scpi-def.h"
#include "tusb.h"

extern size_t usb_buffer_len;
extern char usb_buffer[225];

//#define SCPI_USE_SERIAL
#define SCPI_USE_USBTMC

size_t SCPI_Write(scpi_t *context, const char *data, size_t len)
{
    (void)context;
#ifdef SCPI_USE_SERIAL
    for(uint32_t i = 0; i < len; i++)
    {
     printf("%c",data[i]);
    }
#endif
#ifdef SCPI_USE_USBTMC
    if (len + usb_buffer_len < sizeof(usb_buffer))
    {
        memcpy(&(usb_buffer[usb_buffer_len]), data, len);
        usb_buffer_len += len;
    }
    else
    {
        return SCPI_RES_ERR; // buffer overflow!
    }
#endif
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Flush(scpi_t *context)
{
    (void)context;
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t *context, int_fast16_t err)
{
    (void)context;
#ifdef SCPI_USE_SERIAL
    printf("**ERROR: %d, \"%s\"\r\n", (int16_t) err, SCPI_ErrorTranslate(err));
#endif
#ifdef SCPI_USE_USBTMC
    char _err[64];
    uint8_t _err_len = 0;
    memset(_err, 0, sizeof(_err));
    sprintf(_err, "**ERROR: %d, \"%s\"\r\n", (int16_t)err, SCPI_ErrorTranslate(err));
    _err_len = strlen(_err);
    if (_err_len + usb_buffer_len < sizeof(usb_buffer))
    {
        memcpy(&(usb_buffer[usb_buffer_len]), _err, _err_len);
        usb_buffer_len += _err_len;
    }
    else
    {
        return SCPI_RES_ERR; // buffer overflow!
    }
#endif
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
    (void)context;
#ifdef SCPI_USE_SERIAL
    if (SCPI_CTRL_SRQ == ctrl) {
        printf("**SRQ: 0x%X (%d)\r\n", val, val);
    } else {
        printf("**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }
#endif
#ifdef SCPI_USE_USBTMC
    char _err[64];
    uint8_t _err_len = 0;
    memset(_err, 0, sizeof(_err));
    if (SCPI_CTRL_SRQ == ctrl)
    {
        sprintf(usb_buffer, "**SRQ: 0x%X (%d)\r\n", val, val);
    }
    else
    {
        sprintf(usb_buffer, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }
    _err_len = strlen(_err);
    if (_err_len + usb_buffer_len < sizeof(usb_buffer))
    {
        memcpy(&(usb_buffer[usb_buffer_len]), _err, _err_len);
        usb_buffer_len += _err_len;
    }
    else
    {
        return SCPI_RES_ERR; // buffer overflow!
    }
#endif
    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context)
{
    (void)context;
#ifdef SCPI_USE_SERIAL
    printf("**Reset\r\n");
#endif
#ifdef SCPI_USE_USBTMC
    char _err[64];
    uint8_t _err_len = 0;
    memset(_err, 0, sizeof(_err));
    sprintf(usb_buffer, "**Reset\r\n");
    _err_len = strlen(_err);
    if (_err_len + usb_buffer_len < sizeof(usb_buffer))
    {
        memcpy(&(usb_buffer[usb_buffer_len]), _err, _err_len);
        usb_buffer_len += _err_len;
    }
    else
    {
        return SCPI_RES_ERR; // buffer overflow!
    }
#endif
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t *context)
{
    (void)context;

    return SCPI_RES_ERR;
}

/* END OF LICENSE */

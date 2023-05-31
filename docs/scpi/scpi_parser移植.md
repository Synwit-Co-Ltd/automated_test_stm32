# SCPI Parser移植

SCPI(Standard Commands for Programmable Instruments)，也称为可编程仪器标准命令，定义了总线控制器与仪器的通讯方式。是一种基于ASCII的仪器命令语言，供测试和测量仪器使用。

需要注意的是，SCPI并未定义物理层的传输信道的实现方法。虽然上文指出它最开始是和IEEE 488.2(即GPIB)面世的，但SCPI控制命令也可用于串口(RS-232)，以太网，USB接口，VXIbus等若干硬件总线。本项目实现了串口和USB接口。

## 为什么使用SCPI

- 标准化的命令和语法意味着具有一种SCPI兼容仪器经验的技术人员和程序员可以立即掌握任何其他SCPI仪器
- 为控制一个SCPI仪器而编写的程序/脚本可以更轻松地移植以控制不同的SCPI仪器
- 无需为控制语言的命令和语法设计自己的专有方案，因此可以做到更少的时间，更低的成本

## SCPI parser介绍

我们选择开源的[SCPI parser](https://github.com/j123b567/scpi-parser)进行移植。

以下内容节选自[SCPI parser官方文档](https://www.jaybee.cz/scpi-parser/)。

### 源码结构

源代码被分成几个文件夹，以提供更好的可移植性到其他系统。

- libscpi/src/ - 主源代码目录
    - error.c - 提供基本的错误处理（仪器的错误队列）
    - expression.c - 提供表达式处理的基本实现
    - fifo.c - 提供错误队列FIFO的基本实现
    - ieee488.c - 提供IEEE488.2强制命令的基本实现
    - lexer.c - 提供关键字和数据类型的识别
    - minimal.c - 提供SCPI强制命令的基本实现
    - parser.c - 提供核心解析器库
    - units.c - 提供特殊数字（DEF，MIN，MAX，...）和单位的处理
    - utils.c - 提供字符串处理例程和转换例程
- libscpi/inc/ - 主包含目录
    - scpi.h - 主包含文件 - 只应包含此文件
    - config.h - 配置文件
- libscpi/port/ - 移植文件目录
    - scpi_port.c - scpi_parser的移植实现
    - scpi-def.c - 命令的通用实现

### 输出回调

然后我们需要初始化回调结构体接口。如果不想提供某些回调，只需将它们初始化为NULL。`write`回调是必需的，用于从库中输出数据。这些回调函数分别使用串口和USB进行了实现，默认使用USB。

```c title="scpi-def.c" linenums="1"
scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};
```

### 处理输入

在我们的代码中，应该调用`SCPI_Input`函数。输入命令不需要在此函数的一次调用中完成。它旨在缓冲完整命令，完成后调用解析器。

您可以通过以零长度调用此函数来强制解析缓冲命令。

如果您有另一个缓冲函数，或者缓冲区中始终有完整命令，则无需使用中间缓冲区，可以直接使用特定于应用程序的缓冲区调用`SCPI_Parse`。

如果需要使用内置缓冲，则需要指定输入缓冲区。最大大小由您决定，它应该大于任何可能的最大命令。

```c title="scpi-def.c" linenums="1"
#define SCPI_INPUT_BUFFER_LENGTH 256
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
```

还需要分配错误队列。这可以通过以下方式完成

```c title="scpi-def.c" linenums="1"
#define SCPI_ERROR_QUEUE_SIZE 17
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];
```

### SCPI解析上下文

最后一个结构体是解析器库中使用的scpi上下文。因此，通过初始化函数将所有内容组合在一起。在使用库之前，必须由`SCPI_Init`执行初始化。

```c title="scpi-def.c" linenums="1"
scpi_t scpi_context;
SCPI_Init(&scpi_context,
    scpi_commands,
    &scpi_interface,
    scpi_units_def,
    SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
    scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
    scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);
```

所有这些结构体都应该是c文件的全局变量，或者由malloc等函数分配。在函数内创建这些结构作为该函数的局部变量是常见的错误。这是行不通的。如果你不知道为什么，你应该阅读一些关于函数栈的东西。

可以使用更多的SCPI上下文并共享一些配置（命令列表，寄存器，单元列表，错误回调...），但请注意，该库不是线程安全的。

## 串口移植实现

### 串口使用IDLE中断加DMA接收实现接收不定长数据

使用IDLE中断加DMA接收是最方便的接收不定长数据的方式。DMA的作用是无需CPU干预下自动将串口接收到的数据转移到缓存数组中。然而若是直接使用DMA接收串口数据，则必须得等长接收，当DMA存不满时CPU就无法判断一帧是否接收完毕，因此需要使用IDLE中断判断串口线是否空闲，当串口空闲时则触发IDLE中断，此时进行取数据帧的操作。

HAL提供的`HAL_UARTEx_ReceiveToIdle_DMA`函数可以完成此功能，在开启串口DMA接收的同时也开启了IDLE中断，当中断发生时会进入`HAL_UARTEx_RxEventCallback`回调函数，在回调函数中调用`SCPI_Input`实现数据输入。因此可以做到在不修改CubeMX生成的代码的前提下完成了该功能，方便了后续在其他STM32芯片上的实现。

```c title="usart.c" linenums="1"
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  SCPI_Input(&scpi_context, (const char *)uart_rxbuf, Size);
  if(HAL_UARTEx_ReceiveToIdle_DMA(&huart1,uart_rxbuf,UART_RXBUF_SIZE) != HAL_OK)
  {
    Error_Handler();
  }
}
```

我们需要给DMA接收设置一个不可能达到长度（根据上位机最长的命令来确定），保证只会进入IDLE中断。

## USB移植实现

[USBTMC](https://www.usb.org/document-library/test-measurement-class-specification)是用于测试设备和仪器仪表设备（如示波器、数字万用表和函数发生器）的USB设备类规范。

[TinyUSB](https://github.com/hathach/tinyusb)是用于嵌入式系统的开源跨平台USB主机/设备堆栈，设计为内存安全，无需动态分配，线程安全，所有中断事件都被延迟，然后在非ISR任务功能中处理。`TinyUSB`库支持`USBTMC`类，只需要简单的移植就可以实现`USBTMC`功能。

### 源码结构

源代码被分成几个文件夹，以提供更好的可移植性到其他系统。

- tinyusb/class - USB类
    - hid - HID类
        - hid.h
    - usbtmc - USBTMC类
        - usbtmc_device.c
        - usbtmc_device.h
        - usbtmc.h
- tinyusb/common - 公用文件
    - tusb_common.h
    - tusb_compiler.h
    - tusb_debug.h
    - tusb_fifo.c
    - tusb_fifo.h
    - tusb_mcu.h
    - tusb_private.h
    - tusb_timeout.h
    - tusb_types.h
    - tusb_verify.h
- tinyusb/device - USB设备公用文件
    - dcd.h
    - usbd_control.c
    - usbd_pvt.h
    - usbd.c
    - usbd.h
- tinyusb/osal - OS抽象层
    - osal_none.h
    - osal.h
- tinyusb/port - 移植文件
    - tusb.config.h
    - usb_descriptors.c
    - usbtmc_app.c
    - usbtmc_app.h
- tinyusb/portable - 厂商移植文件
    - st/synopsys
        - dcd_synopsys.c
        - synopsys_common.h
- tusb_option.h
- tusb.c
- tusb.h

`tinyusb/port`中的代码移植自[tinyusb/examples/device/usbtmc/src](https://github.com/hathach/tinyusb/tree/master/examples/device/usbtmc/src)。

修改`tud_usbtmc_msg_data_cb`回调函数，在此函数中调用`SCPI_Input`即可实现数据输入。

```c title="usbtmc_app.c" linenums="1"
bool tud_usbtmc_msg_data_cb(void *data, size_t len, bool transfer_complete)
{
    // If transfer isn't finished, we just ignore it (for now)

    SCPI_Input(&scpi_context, (const char *)data, len);
    queryState = transfer_complete;

    tud_usbtmc_start_bus_read();
    return true;
}
```
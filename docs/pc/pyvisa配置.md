# pyvisa配置

## 软件安装

软件环境搭建方面，需要

- 安装python，自行到[官网](https://www.python.org/)下载。

- 安装pyvisa，这个很简单，在第一步的基础上，`pip install pyvisa`即可。

- 安装NI-VISA，这一点需要解释一下，pyvisa仅仅只提供了一种中介，让使用者可以利用python以简洁的方式对仪器进行操作，而内核依然是VISA，因此需要安装驱动，VISA驱动有很多种，常用的为NI（labview母公司）提供的NI-VISA。直接到官网下载即可，参考[ni-visa配置](./ni-visa%E9%85%8D%E7%BD%AE.md)。

## pyvisa连接开发板

使用测试脚本`code/automated_test/testscripts/control.py`即可测试pyvisa是否连接上了开发板。

![pyvisa](../images/pc/pyvisa.png)
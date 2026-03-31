# macOS 下 T5 Board / T5AI Board 驱动安装与排坑

这份文档记录的是本仓库在一台较新的 macOS 机器上，为 `T5 Board / T5AI Board` 烧录 `LCD + Camera` 固件时实际踩到的坑。

适用范围：

- 开发板：`TUYA_T5AI_BOARD`
- USB 转串口芯片：`CH34x / CH342F`
- 场景：`tos.py flash`、`tos.py monitor`

## 1. 需要安装什么驱动

需要安装 `WCH CH34xVCPDriver`。

作用：

- 让 macOS 正常识别板子上的 `CH34x` 串口
- 安装后串口名通常会从系统默认的 `usbmodem*` 变成 `wchusbserial*`

## 2. 安装后最容易漏掉的一步

不是“装完 pkg/app 就结束了”，还要让系统扩展真正生效。

在这次环境里，安装后驱动状态一度是：

```text
cn.wch.CH34xVCPDriver [activated waiting for user]
```

这表示驱动包已经装上，但还没有被系统授权。

这台机器上的处理路径是：

1. 打开 `系统设置`
2. 进入 `通用`
3. 打开 `登录项与扩展`
4. 找到 `WCH / CH34xVCPDriver`
5. 点“允许”或“启用”
6. 按提示重新插拔设备，必要时重启 Mac

如果这一步没做，驱动通常不会真正接管串口。

## 3. 识别成功后应该看到什么

驱动生效后，可以用：

```bash
ls /dev/cu.wchusbserial*
```

在这次环境里，板子识别为两条口：

```bash
/dev/cu.wchusbserial56D70424971
/dev/cu.wchusbserial56D70424973
```

通常可按下面理解：

- 一条用于烧录
- 一条用于日志

本机上最终使用的是：

- 烧录口：`/dev/cu.wchusbserial56D70424971`
- 日志口：`/dev/cu.wchusbserial56D70424973`

## 4. 这次实际踩到的硬件层坑

### 4.1 扩展坞会干扰识别

这次一开始就是因为走了扩展坞，系统没有稳定识别出开发板串口。

建议：

- 直接插 Mac 机身
- 不要先走 Hub / 扩展坞

### 4.2 线材必须是数据线

很多 Type-C 线只能供电不能传数据。现象通常是：

- 板子亮灯
- 但系统里没有新的 `/dev/cu.*`

### 4.3 `TF card` 不是烧录失败的根因

这块板的关键不是“有没有插 TF 卡”，而是板上 `S1` 拨码是否切到了下载串口。

对 `T5AI Board`：

- `1/2` 控制 `DL_UART / TF Card`
- `3/4` 控制 `LOG_UART / IO-LED`

要烧录和看日志时，建议：

- `1 2 3 4` 全部拨到 `ON`

如果拨到了 `TF Card / IO-LED` 一侧，烧录会异常，即使 USB 已经连上。

## 5. 哪些提示看起来像错误，其实不是驱动问题

### 5.1 `Ignoring windows-curses`

这个不是编译选项配错，也不是刷错平台。

仓库的 [requirements.txt](/Users/auric/aevatar-tuya-open/requirements.txt#L16) 里有一条：

```text
windows-curses==2.4.1; sys_platform == "win32"
```

所以在 macOS 上 `pip` 会打印：

```text
Ignoring windows-curses: markers 'sys_platform == "win32"' don't match your environment
```

这是正常行为，可以忽略。

### 5.2 `complete: command not found` 或 `invalid option name`

这也不是驱动坏了。

这是 [export.sh](/Users/auric/aevatar-tuya-open/export.sh#L182) 里尝试启用 shell completion，而当前 shell 兼容性不一致导致的噪声。

相关位置：

- [export.sh](/Users/auric/aevatar-tuya-open/export.sh#L182)
- [tos.sh](/Users/auric/aevatar-tuya-open/tools/completion/tos.sh#L26)
- [tos.sh](/Users/auric/aevatar-tuya-open/tools/completion/tos.sh#L60)

如果遇到这类提示，但虚拟环境已经激活、`OPEN_SDK_PYTHON` 已设置，它通常不影响真正的烧录。

## 6. 这次和驱动一起出现的工具链坑

除了驱动，这次还碰到了两个容易误判成“板子有问题”的软件问题：

### 6.1 上游脚本里裸用了 `python` 和 `pip`

如果环境里没有 `python` 命令，或者 `pip` 装进了别的解释器，会导致：

- `python: command not found`
- 依赖明明装过，但运行时还是找不到模块

这次已经在仓库里修过，改成统一走 `OPEN_SDK_PYTHON` / `python -m pip`。

### 6.2 新驱动接管后，自动复位握手会更敏感

安装 WCH 驱动后，串口从 `usbmodem*` 变成了 `wchusbserial*`，但早期一度出现过：

```text
Waiting Reset ...
```

也就是串口能打开，但下载握手时序不稳定。

后来通过工具链修正和串口握手重试，`tos.py flash` 已可稳定工作。

如果以后再次遇到：

- 先确认 `S1` 拨码正确
- 再确认是不是接了扩展坞
- 最后再看是否需要手动按一下 `RST`

## 7. 当前这台机器上的确认结果

以下链路已经在本机实际跑通：

- `WCH CH34xVCPDriver` 安装并启用
- `T5AI Board` 枚举为 `wchusbserial*`
- `tos.py flash` 成功
- `tos.py monitor` 成功
- `examples/graphics/lvgl_camera` 成功刷入并运行

## 8. 常用命令

激活环境：

```bash
cd /Users/auric/aevatar-tuya-open
. ./.venv/bin/activate
export OPEN_SDK_ROOT=/Users/auric/aevatar-tuya-open
export OPEN_SDK_PYTHON=/Users/auric/aevatar-tuya-open/.venv/bin/python
export OPEN_SDK_PIP=/Users/auric/aevatar-tuya-open/.venv/bin/pip
```

刷 `lvgl_camera`：

```bash
cd /Users/auric/aevatar-tuya-open/examples/graphics/lvgl_camera
"$OPEN_SDK_PYTHON" /Users/auric/aevatar-tuya-open/tos.py flash -p /dev/cu.wchusbserial56D70424971
```

看日志：

```bash
/Users/auric/aevatar-tuya-open/.venv/bin/python /Users/auric/aevatar-tuya-open/tos.py monitor -p /dev/cu.wchusbserial56D70424973
```

## 9. 最简判断顺序

如果后面有人再遇到“刷不进去”，建议按这个顺序排：

1. 是否直连 Mac，而不是扩展坞
2. USB 线是否是数据线
3. 系统扩展是否已允许
4. 是否出现 `/dev/cu.wchusbserial*`
5. `S1` 是否拨到 `DL_UART / LOG_UART`
6. `tos.py flash` 是否卡在 `Waiting Reset`
7. 再判断是不是工具链或固件问题


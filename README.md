# DxgiDesktopDuplicationTest

用于实际测试 Windows DXGI Desktop Duplication 是否能够在指定 GPU 与显示器之间正常建立。

与[单纯枚举 DXGI Adapter](https://github.com/ckx000/DxgiDisplayEnum) 不同，本工具会进一步创建 D3D11 Device，并调用 `DuplicateOutput()` 进行实际测试。

## 功能

- 枚举系统 DXGI Adapter
- 显示 GPU 名称、Vendor ID、Device ID 和 Adapter LUID
- 指定目标 DISPLAY 进行测试
- 查找该 Adapter 是否提供目标 DXGI Output
- 创建 D3D11 Device
- 获取 `IDXGIOutput1`
- 调用 `DuplicateOutput()`
- 输出 Desktop Duplication 的分辨率、格式、刷新率和旋转信息
- 分别测试多个 GPU 与目标 DISPLAY 的兼容性

## 使用场景

适用于排查：

- DXGI Desktop Duplication 无法初始化
- 混合显卡 / Optimus 架构下的采集问题
- 录屏、远程桌面、游戏串流软件无法使用 DXGI 的问题
- 某个 GPU 是否实际拥有指定 DISPLAY 的 DXGI Output
- 区分“GPU 不支持 DXGI”与“GPU 与 DISPLAY 不匹配”

## 示例

成功建立 Desktop Duplication 时：

```text
[1] Find DXGI Output
    SUCCESS

[2] D3D11CreateDevice
    SUCCESS

[3] QueryInterface IDXGIOutput1
    SUCCESS

[4] DuplicateOutput
    SUCCESS
    HRESULT: 0x00000000

*** DXGI DESKTOP DUPLICATION IS WORKING ***
```

如果 GPU 不提供目标 DISPLAY，则会出现：

```text
[1] Find DXGI Output
    FAILED

This adapter does NOT expose the target DISPLAY as a DXGI Output.
```

## 环境
- Windows
- C++
- Direct3D 11/DXGI
- Visual Studio

本项目仅执行 DXGI/D3D11 检测，不修改系统设置，也不会改变 GPU 配置。
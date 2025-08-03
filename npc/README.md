# NPC 项目

## 📁 文件结构

```
npc/
├── Makefile              # 主构建文件
├── config.mk             # 项目配置
├── source.mk             # 源文件管理
├── verilator.mk          # Verilator配置
├── csrc/
│   ├── filelist.mk       # 源文件列表
│   ├── tools/
│   │   └── tools.mk      # 工具模块配置
│   └── monitor/
│       └── monitor.mk    # Monitor模块配置
└── build/
    └── ysyx_25060173_core  # 成功构建的可执行文件
```

## 🚀 可用构建目标

```bash
make              # 构建项目
make all          # 同上
make run          # 构建并运行仿真
make clean        # 清理构建文件
make wave         # 打开波形查看器
make info         # 显示项目信息
make help         # 显示帮助信息
```
## 📊 技术细节

### 编译统计
- **Verilog 源文件**: 5个
- **C/C++ 源文件**: 7个
- **构建时间**: ~14秒
- **输出大小**: 458KB

### 模块组成
- **主程序**: `npc-main.cpp`
- **CPU模块**: `cpu/cpu.cpp`
- **源文件**: `src/` 目录下的 `dpi-c.cpp`, `expr.cpp`
- **工具模块**: `tools/Log/Log.cpp`, `tools/Tools/tools.cpp`
- **监控模块**: `monitor/sdb.cpp`

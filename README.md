# 声明

本项目用于Qt企业实训

# 智能家居监控平台（Qt）

基于 Qt Widgets 的桌面端智能家居管理示例项目，包含登录、设备控制、场景管理、历史记录、报警管理与系统设置等模块。

## 功能概览

- 用户登录与退出
- 首页总览（环境数据/状态展示）
- 设备控制（增删改查、状态更新）
- 场景模式（场景与设备动作关联）
- 历史记录（操作日志查询）
- 报警管理（报警记录新增、查看、清理）
- 系统设置（键值配置）

## 技术栈

- Qt 5（`core` `gui` `widgets` `sql` `network` `multimedia`）
- C++14
- qmake（`.pro` 项目）
- SQLite（默认数据库文件：`smart_home.db`）

## 项目结构

```text
smart_home/
├─ main.cpp
├─ mainwindow.cpp/.h
├─ databasemanager.cpp/.h
├─ loginwidget.cpp/.h
├─ homewidget.cpp/.h
├─ devicecontrolwidget.cpp/.h
├─ scenewidget.cpp/.h
├─ historywidget.cpp/.h
├─ alarmwidget.cpp/.h
├─ settingswidget.cpp/.h
├─ smart_home.pro
├─ bin/
└─ build/
```

## 环境要求

- Windows（当前工程已在 Windows + MinGW 目录结构下构建）
- Qt 5.15.x（建议使用 Qt Creator）
- MinGW 64-bit（或与你 Qt 套件匹配的编译器）

## 快速开始

### 方式一：Qt Creator

1. 用 Qt Creator 打开 `smart_home.pro`
2. 选择 Qt 5.15.x 对应 Kit（如 `Desktop Qt 5.15.19 MinGW 64-bit`）
3. 点击构建并运行

### 方式二：命令行（qmake + mingw32-make）

在 Qt 命令行环境中执行：

```bash
qmake smart_home.pro
mingw32-make
```

编译产物默认输出到 `bin/`（由 `smart_home.pro` 中 `DESTDIR = $$PWD/bin` 指定）。

## 默认数据

程序首次启动会初始化数据库，并创建默认管理员账号：

- 用户名：`admin`
- 密码：`admin123`

建议首次登录后立即修改密码。

## 数据与持久化

- 默认数据库文件：`smart_home.db`
- 该文件用于保存用户、设备、场景、日志、报警和系统设置等数据

## 注意事项

- 如果更换了 Qt Kit 或编译器，建议清理旧的 `build/` 目录后重新构建
- 提交代码时请忽略编译产物与本地数据库文件（已在 `.gitignore` 配置）

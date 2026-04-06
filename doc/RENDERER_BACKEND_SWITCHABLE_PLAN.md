# MonsterWar 可切换渲染后端计划

> 目标：让游戏在启动时可以选择使用 `SDL` 默认渲染驱动或 `OpenGL` / `Vulkan` 驱动，并且保持现有 `Scene`、`ECS`、`UI`、`ResourceManager` 的调用方式不变。  
> 设计原则：**不做运行时热切换，只做启动时选择**。这样实现成本最低，也最适合当前 demo 结构。

---

## 1. 选择策略

建议把渲染后端的选择做成“启动时配置项”，优先级如下：

1. 命令行参数
2. `assets/config.json` 中的配置字段
3. 默认值（`SDL`）

推荐新增配置字段：

- `renderer_backend = "sdl"`
- `renderer_backend = "opengl"`

如果命令行参数和配置文件同时存在，则以命令行参数为准。

### 建议命令行形式

- `--renderer=sdl`
- `--renderer=opengl`

### 推荐默认值

- 默认继续使用 `SDL`
- `OpenGL` 作为可选增强后端

---

## 2. 现有架构中的落点

### 2.1 `GameApp`

`GameApp` 负责：

- 读取配置
- 创建窗口
- 根据选择初始化对应后端
- 初始化 `Renderer`、`TextRenderer`、`ImGui`

这里最适合新增一个“后端选择入口”。

### 2.2 `Renderer`

当前 `Renderer` 直接封装了 SDL 渲染。  
未来建议把它变成稳定的上层接口：

- 上层继续调用 `drawSprite()`、`drawUIImage()`、`present()`、`clearScreen()`
- 底层按选择使用 SDL 或 OpenGL 实现

### 2.3 `TextRenderer`

字体渲染要和后端选择同步：

- SDL 后端：沿用当前实现
- OpenGL 后端：改为生成纹理后提交给 OpenGL 绘制

### 2.4 `UI` 与 `Scene`

`UI` 和 `Scene` 不应该知道底层用的是 SDL 还是 OpenGL。  
它们仍然只依赖 `Renderer` 的统一接口。

---

## 3. 推荐实现路线

### 第 1 步：增加后端枚举

建议新增一个渲染后端枚举，例如：

- `RendererBackend::SDL`
- `RendererBackend::OpenGL`

这样配置读取、命令行解析和内部创建逻辑都更清晰。

### 第 2 步：拆分后端实现

建议把渲染实现分成两层：

- **上层接口**：`Renderer`
- **底层后端**：`SdlRendererBackend` / `OpenGlRendererBackend`

上层只管 API，不管实现细节。

### 第 3 步：接入 OpenGL 后端

先让 OpenGL 后端支持最小功能：

- 清屏
- 纹理绘制
- UI 图片绘制
- 矩形绘制
- 圆形绘制
- 帧提交

先让它能正常跑完整个 demo，再逐步补齐细节。

### 第 4 步：保留 SDL 后端

SDL 后端不要立刻删除。  
应该保留为：

- 兼容后端
- 回退后端
- 对照验证后端

这样出问题时可以快速切回 SDL。

---

## 4. 配置与初始化建议

### 4.1 配置文件

建议在 `assets/config.json` 中增加：

- `renderer_backend`

这样可以让 demo 不改代码就切换渲染后端。

### 4.2 启动参数

建议支持命令行覆盖配置：

- 用于调试
- 用于性能对比
- 用于 CI 或测试环境

### 4.3 初始化顺序

推荐顺序：

1. 读取配置
2. 解析渲染后端选择
3. 初始化 SDL 窗口
4. 根据后端创建 SDL Renderer 或 OpenGL Context
5. 初始化 `ResourceManager`
6. 初始化 `Renderer` / `TextRenderer` / `ImGui`

---

## 5. 风险点

### 5.1 文本系统回归

`TextRenderer` 依赖字体和纹理路径，后端切换后最容易出问题。

### 5.2 ImGui 适配

ImGui 后端初始化要和选择的渲染后端匹配。

### 5.3 资源生命周期差异

SDL 纹理、OpenGL 纹理和字体对象的销毁顺序不同，必须统一管理。

### 5.4 调试复杂度增加

两套后端并存时，需要在日志里明确打印当前使用的是哪一个后端。

---

## 6. 验证点

每次切换后端后，至少验证：

- 启动页正常显示
- 进入关卡正常
- 地图、单位、UI、技能特效正常渲染
- 暂停、音效、保存不受影响
- 关卡切换和退出不崩溃

---

## 7. 推荐落地顺序

1. **先做后端选择配置**
2. **再抽离渲染后端接口**
3. **接入 OpenGL 后端**
4. **保留 SDL 回退后端**
5. **补齐文本和 ImGui 适配**

---

## 8. 结论

这套方案的目标不是一次性把图形系统重做，而是让项目做到：

- 启动时可选 `SDL` 或 `OpenGL`
- 上层代码不感知后端变化
- 出问题时可快速切回 `SDL`

对当前 demo 来说，这是最稳妥的切换方式。

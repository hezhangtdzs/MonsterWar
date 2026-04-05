# MonsterWar 资源驱动功能需求说明

> **目标**：根据 `assets/` 下现有资源，梳理当前项目应该支持的运行时功能与渲染需求。  
> **依据**：`assets/maps/*.tmj`、`assets/maps/tileset/*.tsj`、`assets/data/*.json`、`assets/textures/*`、`assets/fonts/*`。

---

## 1. 资源扫描结果

### 1.1 地图与 tileset

| 文件 | 类型 | 资源含义 |
|------|------|----------|
| `assets/maps/level1.tmj` | 地图 | 第 1 关地图，包含地形层和装饰对象层 |
| `assets/maps/level2.tmj` | 地图 | 第 2 关地图，结构与第 1 关一致，支持不同资源摆放 |
| `assets/maps/tileset/Tilemap.tsj` | tileset | 主地形图块，含大图块集与动画地形帧 |
| `assets/maps/tileset/buildings.tsj` | tileset | 建筑、岩石、魔法圈与建造占位图块 |
| `assets/maps/tileset/Warrior.tsj` | tileset | 战士单位动画图块 |
| `assets/maps/tileset/Archer.tsj` | tileset | 弓箭手单位动画图块 |
| `assets/maps/tileset/Lancer.tsj` | tileset | 枪兵单位动画图块 |
| `assets/maps/tileset/Tree.tsj` | tileset | 树木装饰动画图块 |
| `assets/maps/tileset/Tree2.tsj` | tileset | 第二组树木装饰动画图块 |
| `assets/maps/tileset/Bush.tsj` | tileset | 灌木装饰动画图块 |
| `assets/maps/tileset/Sheep.tsj` | tileset | 羊群装饰动画图块 |

### 1.2 数据文件

| 文件 | 内容 | 资源驱动含义 |
|------|------|--------------|
| `assets/data/projectile_data.json` | 投射物配置 | 箭矢、魔法球的贴图、偏移、飞行时间、命中音效 |
| `assets/data/effect_data.json` | 特效配置 | 治疗、升级、技能激活、技能就绪图标/动画 |
| `assets/data/level_config.json` | 关卡配置 | 关卡地图、准备时间、敌人等级、波次、刷怪节奏 |
| `assets/data/default_session_data.json` | 默认存档 | 默认角色阵容、初始等级、稀有度、关卡进度 |

### 1.3 纹理与字体

| 目录 | 资源含义 |
|------|----------|
| `assets/textures/Units` | 单位立绘与角色贴图 |
| `assets/textures/Enemy` | 敌人立绘、法术效果贴图 |
| `assets/textures/FX` | 战斗特效贴图 |
| `assets/textures/UI` | 界面框、标题、图标、提示贴图 |
| `assets/textures/Decorations` | 场景装饰、建造位、地图装饰贴图 |
| `assets/textures/Terrain` | 地形底图 |
| `assets/fonts/VonwaonBitmap-16px.ttf` | UI 文本字体 |

---

## 2. 由资源推导出的功能需求

### FR-01 地图加载与渲染

- 系统必须能够加载 `level1.tmj` 与 `level2.tmj`。
- 系统必须支持 `tilelayer`、`objectgroup` 等 TMJ 图层类型。
- 系统必须能够渲染 `Tilemap.tsj` 对应的主地形图块。
- 系统必须支持地图对象层中的装饰物、建筑占位物和地形附加物。

### FR-02 单位动画渲染

- 系统必须能够渲染 `Warrior.tsj`、`Archer.tsj`、`Lancer.tsj` 的角色动画帧。
- 系统必须支持角色动画的循环播放与非循环播放。
- 系统必须支持角色朝向和动作状态切换时的贴图帧更新。

### FR-03 远程投射物渲染

- 系统必须能够读取 `projectile_data.json` 并创建投射物表现。
- `arrow` 必须使用 `assets/textures/Units/Arrow.png` 进行渲染。
- `magic_ball` 必须使用 `assets/textures/Enemy/effects.png` 进行渲染。
- 投射物必须支持偏移、飞行时长和弧线高度参数。
- 弓箭手远程攻击必须显示箭矢，而不是只播放近战攻击动作。

### FR-04 治疗与技能特效渲染

- 系统必须能够读取 `effect_data.json` 并创建治疗、升级和技能特效。
- `heal` 必须显示治疗特效。
- `level_up` 必须显示升级特效。
- `skill_active` 与 `skill_ready` 必须支持状态图标展示。
- 治疗动作必须有可见特效，而不是只有数值变化。

### FR-05 建造点与占位预览

- 系统必须能够读取 `buildings.tsj` 中的建筑与占位资源。
- 系统必须显示 `melee_place` 与 `range_place` 建造占位提示。
- 系统必须支持城堡、房屋、塔、岩石和魔法圈等场景建筑的渲染。
- 建造模式必须能区分近战位与远程位。

### FR-06 UI 与提示渲染

- 系统必须能够使用 `VonwaonBitmap-16px.ttf` 渲染 HUD、按钮和提示文本。
- 系统必须能够使用 `assets/textures/UI` 下的资源渲染标题、框体与图标。
- 系统必须支持波次提示、暂停面板、状态文本和错误提示。
- 系统应支持将 `assets/textures/UI` 资源与 `ImGui` 调试层并存，用于运行时 HUD 与开发调试信息展示。

### FR-07 关卡与波次数据驱动

- 系统必须能够从 `level_config.json` 读取关卡名、地图路径、准备时间和波次表。
- 系统必须能够按关卡配置生成敌人波次。
- 系统必须支持不同关卡的敌人等级与稀有度差异。

### FR-08 默认阵容与初始存档

- 系统必须能够从 `default_session_data.json` 读取默认单位阵容。
- 系统必须支持默认关卡进度与默认点数数据。
- 系统必须能够根据默认阵容初始化战役或测试场景。

### FR-09 场景装饰渲染

- 系统必须能够渲染 `Tree.tsj`、`Tree2.tsj`、`Bush.tsj`、`Sheep.tsj` 等装饰动画。
- 场景装饰物应作为独立图块资源，不应与单位战斗贴图混用。

### FR-10 音效资源关联

- 系统必须能够根据资源 JSON 中的音效 key 触发对应音效。
- 投射物命中、治疗、升级和攻击触发都应与音效配置联动。

---

## 3. 当前最优先补齐项

1. 远程投射物真实渲染：箭矢、魔法球、命中特效。
2. 治疗动作视觉反馈：治疗特效与状态变化同步。
3. 地图对象层资源接入：树木、岩石、建筑、建造位可见。
4. UI 字体与图标统一：HUD、暂停、波次提示、结算界面。

---

## 4. 约束与约定

- 资源路径应继续沿用 `assets/` 目录结构。
- `tsj` 中的 tileset 资源应作为地图与单位动画的配置来源。
- `json` 中的数值、路径和 key 必须保持数据驱动，不应硬编码到渲染层。
- 若新增单位、投射物或特效，应优先补充对应资源 JSON，再接入系统。

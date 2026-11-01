# Agent 场景编辑：配置与验收

Agent 是 OpenGL 编辑器右侧的一个辅助页。场景仍由本地 OpenGL 渲染器实时绘制；模型只生成一组受限的场景操作，程序逐条验证并执行。直接编辑、OBJ 导入和场景文件功能无需模型服务。

## 启动与模型配置

在启动编辑器的同一个 PowerShell 会话中设置环境变量：

```powershell
$env:OPENAI_API_KEY = '你的 API key'
$env:OPENAI_MODEL = '你可用且支持结构化输出的模型名'
# 仅在网络需要代理时设置：
$env:HTTPS_PROXY = 'http://127.0.0.1:7897'
& '.\build-vs\Debug\scene_editor.exe'
```

在 CLion 中，可以把 `OPENAI_API_KEY`、`OPENAI_MODEL` 和可选的 `HTTPS_PROXY` 填入 `scene_editor` 的 Run/Debug Configuration 的 Environment variables，然后运行目标。不要把密钥写入源码、场景文件或提交到 Git。程序只从进程环境读取密钥，不把对话写入场景文件。模型请求使用 OpenAI [Responses API](https://developers.openai.com/api/docs/guides/text) 和 [Structured Outputs](https://developers.openai.com/api/docs/guides/structured-outputs)；当前客户端面向 Windows。

进入右侧 **Agent** 页，在输入框描述场景，点击 **生成并执行**。界面会显示计划摘要、每一步的执行结果和失败原因。可点击 **清空会话** 删除当前窗口的对话上下文。关闭应用也会丢弃对话上下文。

## 离线验收

即使没有配置模型，也可以在 Agent 输入框粘贴 JSON 计划并点击 **生成并执行**。例如：

```json
{
  "summary": "增加一棵简化的树并用暖光照亮",
  "operations": [
    {"operation_id": "trunk", "tool": "add_object", "arguments": {"type": "cube", "id": "tree-trunk", "name": "树干", "position": [0, 1, 0], "scale": [0.3, 2, 0.3]}},
    {"operation_id": "crown", "tool": "add_object", "arguments": {"type": "sphere", "id": "tree-crown", "name": "树冠", "position": [0, 2.3, 0], "scale": [1, 1, 1]}},
    {"operation_id": "green", "tool": "set_material", "arguments": {"object_id": "tree-crown", "color": [0.15, 0.55, 0.2], "roughness": 0.8}},
    {"operation_id": "lamp", "tool": "add_light", "arguments": {"type": "point", "id": "agent-warm-light", "position": [2, 3, 1], "color": [1, 0.72, 0.42], "intensity": 2}},
    {"operation_id": "look", "tool": "move_camera", "arguments": {"position": [4, 3, 6], "target": [0, 1.5, 0]}},
    {"operation_id": "render", "tool": "render_scene", "arguments": {}}
  ]
}
```

验收时确认视口出现树干和树冠，右侧灯光页新增暖色点光源，相机朝向树。再输入 `{"summary":"检查场景","operations":[{"operation_id":"state","tool":"get_scene_state","arguments":{}}]}`，展开操作进度中的 `state` 查看当前对象、灯光和相机数据。

## 可用操作

每个操作包含唯一 `operation_id`、`tool` 和对象类型的 `arguments`。支持：

| 工具 | 主要参数 |
| --- | --- |
| `add_object` | `type` 为 `cube`、`sphere`、`plane` 或 `imported_model`；可设置 `id`、`name`、`position`、`rotation`、`scale`，导入 OBJ 时另需本地 `path` |
| `update_object`、`delete_object` | `object_id`；更新时可设置名称、变换和可见性 |
| `set_material` | `object_id`；可设置 `color`、`roughness`、`shininess`、其他光照颜色或本地 `texture_path` |
| `add_light`、`update_light`、`delete_light` | 点光源或平行光；更新和删除使用 `light_id` |
| `move_camera` | `position`、`target` 或 `yaw`/`pitch`，以及可选投影参数 |
| `get_scene_state`、`render_scene` | 无参数；渲染在下一帧显示 |
| `save_scene`、`load_scene` | `filename`，限简单文件名，存放于启动目录的 `scenes` 文件夹 |

要自行选择保存位置，请使用顶部 **场景文件** 的系统文件选择器。Agent 的 `save_scene`/`load_scene` 仅使用启动目录下的 `scenes` 文件夹，以避免模型生成任意写入路径。

计划最多 32 步，逐帧执行。对象目标可以是稳定 ID 或唯一名称。若对象或灯光不存在，执行器会停止后续操作，自动查询场景状态并在进度中显示；更正描述后可重试。无效参数会返回错误，当前操作不会部分修改对象。点光源最多 4 个、平行光最多 2 个；超限会显示错误。网络错误、服务拒绝和无效模型响应会显示在 Agent 页，不影响直接编辑。

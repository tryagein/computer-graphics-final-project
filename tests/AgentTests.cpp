#include "agent/AgentProvider.hpp"
#include "agent/ConversationContext.hpp"
#include "agent/OperationCodec.hpp"
#include "agent/PlanExecutor.hpp"
#include "agent/SceneStateQuery.hpp"
#include "persistence/SceneSerializer.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <stdexcept>

using namespace scene_editor;
using nlohmann::json;

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void run() {
    SceneOperation decoded;
    require(decodeOperation({{"operation_id", "1"}, {"tool", "add_object"},
                             {"arguments", {{"type", "cube"}}}}, decoded).ok,
            "valid operation rejected");
    require(!decodeOperation({{"operation_id", "1"}, {"tool", "run_code"},
                              {"arguments", json::object()}}, decoded).ok,
            "unknown operation accepted");
    require(!decodeOperation({{"operation_id", "1"}, {"tool", "render_scene"},
                              {"arguments", json::object()}, {"extra", 1}}, decoded).ok,
            "extra envelope field accepted");

    Scene scene;
    scene.makeDefault();
    Camera camera;
    PlanExecutor executor;
    const json plan = {
        {"summary", "Create and illuminate a cube"},
        {"operations", json::array({
            {{"operation_id", "add"}, {"tool", "add_object"},
             {"arguments", {{"type", "cube"}, {"id", "agent-cube"},
                            {"position", {2, 1, 0}}}}},
            {{"operation_id", "mat"}, {"tool", "set_material"},
             {"arguments", {{"object_id", "agent-cube"}, {"color", {0.2, 0.4, 0.8}}}}},
            {{"operation_id", "light"}, {"tool", "add_light"},
             {"arguments", {{"type", "point"}, {"id", "agent-lamp"},
                            {"position", {2, 3, 1}}, {"intensity", 2}}}},
            {{"operation_id", "camera"}, {"tool", "move_camera"},
             {"arguments", {{"position", {0, 2, 6}}, {"target", {2, 1, 0}}}}},
            {{"operation_id", "render"}, {"tool", "render_scene"},
             {"arguments", json::object()}}
        })}
    };
    require(executor.load(plan).ok, "valid plan rejected");
    while (!executor.finished()) executor.step(scene, camera);
    require(!executor.stoppedOnError() && executor.completed() == 5,
            "valid plan did not complete");
    require(scene.find("agent-cube") && scene.find("agent-cube")->material.color.z == 0.8F,
            "object or material change missing");
    require(scene.lights.size() == 3 && camera.yaw() != -90,
            "light or camera change missing");

    const auto before = scene.objects.size();
    const json badPlan = {
        {"summary", "Modify a missing object"},
        {"operations", json::array({
            {{"operation_id", "missing"}, {"tool", "update_object"},
             {"arguments", {{"object_id", "absent"}, {"position", {1, 2, 3}}}}},
            {{"operation_id", "later"}, {"tool", "add_object"},
             {"arguments", {{"type", "sphere"}, {"id", "should-not-exist"}}}}
        })}
    };
    require(executor.load(badPlan).ok, "missing-target plan rejected too early");
    executor.step(scene, camera);
    require(executor.stoppedOnError() && scene.objects.size() == before &&
            !scene.find("should-not-exist") && executor.results().size() == 2 &&
            executor.results().back().operation.tool == "get_scene_state",
            "missing target did not stop and query scene");

    SceneOperation invalid{"bad-scale", "update_object",
        {{"object_id", "agent-cube"}, {"scale", {0, 1, 1}}}};
    const auto originalScale = scene.find("agent-cube")->transform.scale;
    const auto invalidResult = dispatchSceneOperation(invalid, scene, camera);
    require(!invalidResult.ok && scene.find("agent-cube")->transform.scale == originalScale,
            "invalid operation partially changed scene");
    require(!executor.load({{"summary", "duplicate"}, {"operations", json::array({
        {{"operation_id", "x"}, {"tool", "render_scene"}, {"arguments", json::object()}},
        {{"operation_id", "x"}, {"tool", "render_scene"}, {"arguments", json::object()}}
    })}}).ok, "duplicate plan IDs accepted");

    const json modelResponse = {{"status", "completed"}, {"output", json::array({
        {{"type", "reasoning"}},
        {{"type", "message"}, {"content", json::array({
            {{"type", "output_text"}, {"text", R"({"summary":"Add sphere","operations":[{"operation_id":"s1","tool":"add_object","arguments_json":"{\"type\":\"sphere\"}"}]})"}}
        })}}
    })}};
    const auto parsed = parseDeepSeekPlanResponse(modelResponse);
    require(parsed.status.ok && executor.load(parsed.plan).ok,
            "model response parsing failed");
    require(!parseDeepSeekPlanResponse({{"status", "incomplete"}, {"output", json::array()}}).status.ok,
            "incomplete model response accepted");
    ConversationContext context;
    context.add("user", "Add a cube");
    require(context.promptText().find("Add a cube") != std::string::npos,
            "conversation context missing turn");
    context.clear();
    require(context.turns().empty(), "conversation context did not clear");
    SceneDocument document;
    document.scene = scene;
    document.camera = camera;
    require(!serializeSceneDocument(document).contains("conversation"),
            "conversation was persisted with scene");
    require(!DeepSeekProvider({}).plan("Add a cube", context, compactSceneState(scene, camera)).status.ok,
            "unconfigured provider did not fail cleanly");
}
}

int main() {
    try { run(); std::cout << "agent tests passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

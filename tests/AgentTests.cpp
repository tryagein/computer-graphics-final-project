#include "agent/AgentProvider.hpp"
#include "agent/ConversationContext.hpp"
#include "agent/OperationCodec.hpp"
#include "agent/PlanExecutor.hpp"
#include "agent/SceneStateQuery.hpp"
#include "assets/ObjImporter.hpp"
#include "persistence/SceneFileService.hpp"
#include "persistence/SceneSerializer.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

using namespace scene_editor;
using nlohmann::json;

namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

bool liveRequested(const char* name) {
#ifdef _WIN32
    char* value = nullptr;
    size_t size = 0;
    _dupenv_s(&value, &size, name);
    const bool enabled = value && *value;
    std::free(value);
    return enabled;
#else
    return std::getenv(name) != nullptr;
#endif
}

void requireMemoryOnlyPlan(const json& plan) {
    for (const auto& operation : plan.at("operations")) {
        const auto tool = operation.at("tool").get<std::string>();
        require(tool != "save_scene" && tool != "load_scene",
                "Live smoke plan unexpectedly requested file access");
        if (tool == "add_object") {
            const auto arguments = operation.contains("arguments_json") ?
                json::parse(operation.at("arguments_json").get<std::string>()) : operation.at("arguments");
            require(arguments.value("type", "") != "imported_model",
                    "Live smoke plan unexpectedly requested an external OBJ");
        }
    }
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
    const auto fixture = std::filesystem::path(SCENE_EDITOR_ASSET_DIR) /
                         "examples" / "course-demo";
    Scene demo;
    Camera demoCamera;
    require(loadSceneFile(fixture / "course-demo.json", demo, demoCamera).ok &&
            demo.objects.size() == 10 && demo.lights.size() == 2,
            "portable course demo did not load");
    const auto* crate = demo.find("textured-crate");
    require(crate && crate->kind == ObjectKind::ImportedModel &&
            crate->material.texturePath.empty(),
            "demo OBJ reference or MTL override is unexpected");
    const auto imported = importObj(crate->geometryPath);
    const bool hasUvAndNormal = imported.valid() && std::any_of(
        imported.submeshes.front().vertices.begin(), imported.submeshes.front().vertices.end(),
        [](const Vertex& vertex) {
            return (vertex.uv.x != 0 || vertex.uv.y != 0) && glm::length(vertex.normal) > 0.9F;
        });
    require(imported.valid() && !imported.submeshes.front().vertices.empty() &&
            hasUvAndNormal &&
            !imported.submeshes.front().material.texturePath.empty() &&
            std::filesystem::is_regular_file(imported.submeshes.front().material.texturePath),
            "demo OBJ geometry, MTL, UV, or texture did not import");
    Scene sourceScene;
    Camera sourceCamera;
    const auto addedFixture = dispatchSceneOperation({"fixture", "add_object",
        {{"type", "imported_model"}, {"id", "source-crate"},
         {"path", (fixture / "textured-crate.obj").string()}}}, sourceScene, sourceCamera);
    require(addedFixture.ok && sourceScene.find("source-crate"),
            "direct OBJ import operation failed");
    require(dispatchSceneOperation({"scale", "update_object",
        {{"object_id", "source-crate"}, {"scale", {1.5, 1.5, 1.5}}}},
        sourceScene, sourceCamera).ok && sourceScene.find("source-crate")->transform.scale.x == 1.5F,
        "imported OBJ scale did not update");
    require(dispatchSceneOperation({"relight", "update_light",
        {{"light_id", "warm-lamp"}, {"color", {0.9, 0.5, 0.2}}, {"intensity", 1.8}}},
        demo, demoCamera).ok && demo.lights.at(1).intensity == 1.8F,
        "demo point light edit failed");
    if (liveRequested("DEEPSEEK_LIVE_TEST")) {
        const auto config = ProviderConfig::fromEnvironment();
        require(config.ready(), "DeepSeek key is not configured in the project .env");
        const bool scenario = liveRequested("DEEPSEEK_LIVE_SCENARIO");
        Scene liveScene;
        liveScene.makeDefault();
        Camera liveCamera;
        const auto beforeObjects = liveScene.objects.size();
        const auto beforeLights = liveScene.lights.size();
        const auto live = DeepSeekProvider(config).plan(
            scenario ? "Create a snowy night scene with three trees and a warm point-light street lamp; "
                       "face the camera toward the lamp. Approximate trees and lamp from primitives."
                     : "Plan adding one blue cube at [2, 1, 0] and rendering the scene.",
            context, compactSceneState(liveScene, liveCamera));
        if (!live.status.ok) throw std::runtime_error(live.status.message);
        require(executor.load(live.plan).ok && executor.total() > 0,
                "DeepSeek returned an unusable or empty scene plan");
        const size_t initialOperations = executor.total();
        requireMemoryOnlyPlan(live.plan);
        while (!executor.finished()) executor.step(liveScene, liveCamera);
        require(!executor.stoppedOnError(), "DeepSeek plan failed during scene execution");
        if (scenario) {
            std::cout << "New objects: " << liveScene.objects.size() - beforeObjects
                      << ", new lights: " << liveScene.lights.size() - beforeLights << '\n';
            const bool lightOperation = std::any_of(executor.results().begin(), executor.results().end(),
                [](const PlanStepResult& step) {
                    return step.operation.tool == "add_light" || step.operation.tool == "update_light";
                });
            const bool warmPointLight = std::any_of(liveScene.lights.begin(), liveScene.lights.end(),
                [](const Light& light) {
                    return light.type == LightType::Point && light.enabled &&
                           light.intensity > 0 && light.color.r > light.color.b;
                });
            require(liveScene.objects.size() >= beforeObjects + 6 &&
                    lightOperation && warmPointLight,
                    "Snowy-night plan omitted the three trees or street lamp light");
            const std::string targetId = liveScene.objects.at(beforeObjects).id;
            ConversationContext liveContext;
            liveContext.add("user", "Create a snowy night scene with three trees and a warm street lamp.");
            liveContext.add("assistant", live.plan.at("summary").get<std::string>());
            SceneDocument snapshot;
            snapshot.scene = liveScene;
            snapshot.camera = liveCamera;
            const auto beforeFollowup = serializeSceneDocument(snapshot);
            const auto followup = DeepSeekProvider(config).plan(
                "Change only object ID " + targetId +
                " to blue base color [0.1, 0.2, 0.9]. Leave all other objects, lights and camera unchanged.",
                liveContext, compactSceneState(liveScene, liveCamera));
            if (!followup.status.ok) throw std::runtime_error("Follow-up: " + followup.status.message);
            require(executor.load(followup.plan).ok && executor.total() > 0,
                    "DeepSeek follow-up returned no usable operation");
            requireMemoryOnlyPlan(followup.plan);
            while (!executor.finished()) executor.step(liveScene, liveCamera);
            require(!executor.stoppedOnError(), "DeepSeek follow-up failed during execution");
            snapshot.scene = liveScene;
            snapshot.camera = liveCamera;
            const auto afterFollowup = serializeSceneDocument(snapshot);
            require(afterFollowup.at("objects").size() == beforeFollowup.at("objects").size() &&
                    afterFollowup.at("lights") == beforeFollowup.at("lights") &&
                    afterFollowup.at("camera") == beforeFollowup.at("camera"),
                    "Follow-up changed scene structure, lights, or camera");
            for (size_t i = 0; i < beforeObjects; ++i)
                require(afterFollowup.at("objects").at(i) == beforeFollowup.at("objects").at(i),
                        "Follow-up changed an unrelated original object");
            for (size_t i = beforeObjects + 1; i < liveScene.objects.size(); ++i)
                require(afterFollowup.at("objects").at(i) == beforeFollowup.at("objects").at(i),
                        "Follow-up changed an unrelated created object");
            require(liveScene.find(targetId) && liveScene.find(targetId)->material.color.z > 0.7F,
                    "Follow-up did not make the target object blue");
            const auto beforeMissing = afterFollowup;
            liveContext.add("user", "Change " + targetId + " to blue.");
            liveContext.add("assistant", followup.plan.at("summary").get<std::string>());
            const auto missing = DeepSeekProvider(config).plan(
                "Move object ID missing-phase6-target to [4, 1, 0]. If it does not exist, explain that and leave the scene alone.",
                liveContext, compactSceneState(liveScene, liveCamera));
            if (!missing.status.ok) throw std::runtime_error("Missing target: " + missing.status.message);
            require(executor.load(missing.plan).ok, "Missing-target response was not a valid plan");
            requireMemoryOnlyPlan(missing.plan);
            while (!executor.finished()) executor.step(liveScene, liveCamera);
            snapshot.scene = liveScene;
            snapshot.camera = liveCamera;
            require(serializeSceneDocument(snapshot) == beforeMissing,
                    "Missing-target request changed the scene");
            std::cout << "Targeted follow-up and missing-target requests preserved unrelated scene state.\n";
        }
        std::cout << "DeepSeek live plan received: " << initialOperations << " operations\n";
        std::cout << "Resulting scene: " << liveScene.objects.size() << " objects, "
                  << liveScene.lights.size() << " lights\n";
    }
}
}

int main() {
    try { run(); std::cout << "agent tests passed\n"; return 0; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

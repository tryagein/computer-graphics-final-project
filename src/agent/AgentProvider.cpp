#include "agent/AgentProvider.hpp"
#include "agent/PlanExecutor.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>

#ifdef _WIN32
#include <Windows.h>
#include <winhttp.h>
#endif

namespace scene_editor {
namespace {
std::string env(const char* name) {
#ifdef _WIN32
    char* value = nullptr;
    size_t size = 0;
    if (_dupenv_s(&value, &size, name) != 0 || !value) return {};
    std::string result(value);
    std::free(value);
    return result;
#else
    const char* value = std::getenv(name);
    return value ? value : "";
#endif
}

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::map<std::string, std::string> dotenv() {
#ifdef SCENE_EDITOR_PROJECT_DIR
    const auto file = std::filesystem::path(SCENE_EDITOR_PROJECT_DIR) / ".env";
#else
    const auto file = std::filesystem::current_path() / ".env";
#endif
    std::ifstream input(file, std::ios::binary);
    std::map<std::string, std::string> values;
    if (!input) return values;
    std::string line;
    size_t bytes = 0;
    while (std::getline(input, line) && bytes < 16384) {
        bytes += line.size() + 1;
        if (line.size() > 4096) continue;
        line = trim(std::move(line));
        if (line.empty() || line.front() == '#') continue;
        if (line.rfind("export ", 0) == 0) line.erase(0, 7);
        const auto equal = line.find('=');
        if (equal == std::string::npos) continue;
        auto key = trim(line.substr(0, equal));
        auto value = trim(line.substr(equal + 1));
        if (value.size() >= 2 && ((value.front() == '"' && value.back() == '"') ||
                                  (value.front() == '\'' && value.back() == '\'')))
            value = value.substr(1, value.size() - 2);
        values[std::move(key)] = std::move(value);
    }
    return values;
}

std::string setting(const std::map<std::string, std::string>& file, const char* name) {
    const auto processValue = env(name);
    if (!processValue.empty()) return processValue;
    const auto it = file.find(name);
    return it == file.end() ? std::string{} : it->second;
}

nlohmann::json planSchema() {
    const auto stringType = nlohmann::json{{"type", "string"}};
    return {
        {"type", "object"}, {"additionalProperties", false},
        {"required", {"summary", "operations"}},
        {"properties", {
            {"summary", stringType},
            {"operations", {
                {"type", "array"},
                {"items", {
                    {"type", "object"}, {"additionalProperties", false},
                    {"required", {"operation_id", "tool", "arguments_json"}},
                    {"properties", {
                        {"operation_id", stringType},
                        {"tool", {{"type", "string"}, {"enum", {
                            "add_object", "update_object", "delete_object", "set_material",
                            "add_light", "update_light", "delete_light", "move_camera",
                            "get_scene_state", "render_scene", "save_scene", "load_scene"
                        }}}},
                        {"arguments_json", stringType}
                    }}
                }}
            }}
        }}
    };
}

const char* instructions = R"(You plan edits for a native OpenGL scene editor. Return JSON matching the supplied schema.
Each operation's arguments_json is a JSON object encoded as a string. Use only supported tools and arguments.
Tools: add_object {type:cube|sphere|plane|imported_model, id?, name?, position?, rotation?, scale?, path?}; update_object {object_id,name?,position?,rotation?,scale?,visible?}; delete_object {object_id}; set_material {object_id,color?,ambient?,diffuse?,specular?,shininess?,roughness?,texture_path?}; add_light {id?,type:point|directional,position?,direction?,color?,intensity?,enabled?}; update_light {light_id,position?,direction?,color?,intensity?,enabled?}; delete_light {light_id}; move_camera {position?,target?,yaw?,pitch?,field_of_view?,near_plane?,far_plane?}; get_scene_state {}; render_scene {}; save_scene/load_scene {filename}.
Vectors are [x,y,z], colors are [r,g,b] in [0,1]. Y is up. Light limits: four point and two directional. The renderer supports cube, sphere, plane, and a local OBJ path; approximate unsupported forms (such as trees) from primitives. Existing targets must use an exact ID or unique name from scene state. For newly created objects, assign explicit unique IDs and use them in later operations. Never invent a path for an unavailable asset. For missing or ambiguous targets, return a summary with zero operations. Use no more than 24 operations. End visual edits with render_scene. No OpenGL code or arbitrary commands.)";

#ifdef _WIN32
struct HttpHandle {
    HINTERNET value{};
    ~HttpHandle() { if (value) WinHttpCloseHandle(value); }
};

std::wstring wideAscii(const std::string& input) {
    return {input.begin(), input.end()};
}

std::string normalizeProxy(std::string proxy) {
    const auto scheme = proxy.find("://");
    if (scheme != std::string::npos) proxy.erase(0, scheme + 3);
    const auto slash = proxy.find('/');
    if (slash != std::string::npos) proxy.resize(slash);
    return proxy;
}

Result postResponse(const ProviderConfig& config, const std::string& body, std::string& response) {
    if (body.size() > 128 * 1024) return Result::failure("Agent request is too large.");
    const auto proxy = normalizeProxy(config.proxy);
    HttpHandle session{WinHttpOpen(L"OpenGLSceneEditor/1.0", proxy.empty() ?
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY : WINHTTP_ACCESS_TYPE_NAMED_PROXY,
        proxy.empty() ? WINHTTP_NO_PROXY_NAME : wideAscii(proxy).c_str(),
        WINHTTP_NO_PROXY_BYPASS, 0)};
    if (!session.value) return Result::failure("Could not initialize HTTPS client.");
    WinHttpSetTimeouts(session.value, 5000, 10000, 10000, 30000);
    HttpHandle connection{WinHttpConnect(session.value, L"api.deepseek.com", INTERNET_DEFAULT_HTTPS_PORT, 0)};
    if (!connection.value) return Result::failure("Could not connect to model service.");
    HttpHandle request{WinHttpOpenRequest(connection.value, L"POST", L"/responses", nullptr,
        WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE)};
    if (!request.value) return Result::failure("Could not create model request.");
    const auto headers = std::wstring(L"Content-Type: application/json\r\nAuthorization: Bearer ") +
                         wideAscii(config.apiKey);
    if (!WinHttpAddRequestHeaders(request.value, headers.c_str(), DWORD(headers.size()),
                                  WINHTTP_ADDREQ_FLAG_ADD))
        return Result::failure("Could not set model request headers.");
    if (!WinHttpSendRequest(request.value, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            const_cast<char*>(body.data()), DWORD(body.size()), DWORD(body.size()), 0) ||
        !WinHttpReceiveResponse(request.value, nullptr))
        return Result::failure("Model request failed or timed out (WinHTTP " +
                               std::to_string(GetLastError()) + ").");
    DWORD status = 0, statusSize = sizeof(status);
    if (!WinHttpQueryHeaders(request.value, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX))
        return Result::failure("Could not read model response status.");
    response.clear();
    for (;;) {
        char buffer[4096];
        DWORD read = 0;
        if (!WinHttpReadData(request.value, buffer, sizeof(buffer), &read))
            return Result::failure("Could not read model response.");
        if (!read) break;
        if (response.size() + read > 1024 * 1024)
            return Result::failure("Model response exceeds 1 MB.");
        response.append(buffer, buffer + read);
    }
    if (status != 200) {
        std::string detail;
        try {
            const auto error = nlohmann::json::parse(response);
            detail = error.at("error").at("message").get<std::string>();
            if (detail.size() > 300) detail.resize(300);
        } catch (...) {}
        return Result::failure("Model service HTTP " + std::to_string(status) +
                               (detail.empty() ? "" : ": " + detail));
    }
    return {};
}
#endif
}

ProviderConfig ProviderConfig::fromEnvironment() {
    const auto file = dotenv();
    auto proxy = setting(file, "HTTPS_PROXY");
    if (proxy.empty()) proxy = env("https_proxy");
    auto model = setting(file, "DEEPSEEK_MODEL");
    if (model.empty()) model = "deepseek-flash";
    return {setting(file, "DEEPSEEK_API_KEY"), model, proxy};
}

ProviderPlan parseDeepSeekPlanResponse(const nlohmann::json& response) {
    try {
        if (response.at("status") != "completed")
            return {Result::failure("Model response was incomplete."), {}};
        std::string text;
        for (const auto& item : response.at("output")) {
            if (item.value("type", "") != "message") continue;
            for (const auto& content : item.at("content")) {
                if (content.value("type", "") == "refusal")
                    return {Result::failure("Model declined the request."), {}};
                if (content.value("type", "") == "output_text")
                    text += content.at("text").get<std::string>();
            }
        }
        if (text.empty() || text.size() > 64 * 1024)
            return {Result::failure("Model returned no usable plan or an oversized plan."), {}};
        const auto plan = nlohmann::json::parse(text);
        PlanExecutor check;
        const auto valid = check.load(plan);
        if (!valid.ok) return {valid, {}};
        return {{true, "Plan received."}, plan};
    } catch (const std::exception& error) {
        return {Result::failure(std::string("Invalid model response: ") + error.what()), {}};
    }
}

ProviderPlan DeepSeekProvider::plan(const std::string& request,
                                  const ConversationContext& context,
                                  const nlohmann::json& sceneState) {
    if (!config_.ready())
        return {Result::failure("Set DEEPSEEK_API_KEY in the project .env file before using natural language."), {}};
    if (config_.apiKey.size() > 512 || config_.apiKey.find_first_of("\r\n") != std::string::npos)
        return {Result::failure("Invalid API key format."), {}};
    if (config_.model.size() > 100 || config_.model.find_first_of("\r\n") != std::string::npos)
        return {Result::failure("Invalid model name."), {}};
    if (request.empty() || request.size() > 4000)
        return {Result::failure("Request must contain 1 to 4000 bytes."), {}};
    const auto history = context.promptText();
    const auto snapshot = sceneState.dump();
    if (history.size() > 16000 || snapshot.size() > 12000)
        return {Result::failure("Conversation or scene snapshot is too large."), {}};
    const nlohmann::json body = {
        {"model", config_.model}, {"instructions", instructions},
        {"input", "Recent conversation:\n" + history + "\nCurrent scene state:\n" +
                  snapshot + "\nLatest user request:\n" + request},
        {"max_output_tokens", 4096},
        {"text", {{"format", {{"type", "json_schema"}, {"name", "scene_plan"},
            {"strict", true}, {"schema", planSchema()}}}}}
    };
#ifdef _WIN32
    std::string raw;
    auto posted = postResponse(config_, body.dump(), raw);
    if (!posted.ok) return {posted, {}};
    try { return parseDeepSeekPlanResponse(nlohmann::json::parse(raw)); }
    catch (const std::exception& error) {
        return {Result::failure(std::string("Invalid model JSON: ") + error.what()), {}};
    }
#else
    (void)body;
    return {Result::failure("DeepSeek provider is currently available on Windows only."), {}};
#endif
}
}

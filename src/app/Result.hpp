#pragma once
#include <string>
#include <utility>
namespace scene_editor { struct Result { bool ok{true}; std::string message; static Result failure(std::string m){return {false,std::move(m)};} }; }

#pragma once
#include <string>
#include "app/Result.hpp"
namespace scene_editor { class Notifications {public:void set(std::string message){message_=std::move(message);}void report(const Result& result){message_=result.message;}void draw()const;private:std::string message_;}; }

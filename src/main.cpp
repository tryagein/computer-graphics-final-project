#include "app/Application.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    int benchmarkSeconds = 0;
    std::string capturePath;
    if (argc > 1) {
        if (std::string(argv[1]) == "--capture-demo" && argc == 3) {
            capturePath = argv[2];
        } else if (std::string(argv[1]) == "--benchmark" && argc <= 3) {
            benchmarkSeconds = 60;
            if (argc == 3) {
                try { benchmarkSeconds = std::stoi(argv[2]); }
                catch (...) { benchmarkSeconds = 0; }
            }
            if (benchmarkSeconds < 1 || benchmarkSeconds > 600) {
                std::cerr << "Benchmark duration must be 1 to 600 seconds.\n";
                return 1;
            }
        } else {
            std::cerr << "Usage: scene_editor [--benchmark [seconds] | --capture-demo output.png]\n";
            return 1;
        }
    }
    scene_editor::Application app;
    return app.run(benchmarkSeconds, capturePath);
}

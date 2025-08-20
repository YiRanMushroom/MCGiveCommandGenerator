export module ProgramConfig;

import Util.Json;
import std.compat;

export struct ProgramConfig {
    std::string language;

    static ProgramConfig LoadOrDefault() {
        try {
            return Load();
        } catch (...) {
            return ProgramConfig{
                .language{"en_us"}
            };
        }
    }

    static ProgramConfig Load() {
        auto ifs = std::ifstream(std::filesystem::current_path() / "config.json");
        auto json = nlohmann::json::parse(ifs);
        ProgramConfig config{};
        config.language = json["value"].get<std::string>();
        return config;
    }

    static void Save(const ProgramConfig &config) {
        nlohmann::json json;
        json["language"] = config.language;
        std::ofstream ofs(std::filesystem::current_path() / "config.json");
        ofs << json.dump(4); // Pretty print with 4 spaces
    }
};
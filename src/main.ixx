export module Main;

import EasyGui;
import EasyGui.Utils.WindowsApi;
import Layers.BackGround;
import TranslationLibarary;
import Util.Json;
import std.compat;
import ProgramConfig;

const ImWchar *GetGlyphRangesChineseFull() {
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0xFFFD, 0xFFFD, // Invalid
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

void AddAllTranslations(TranslationLibrary &translationLibrary);

export int main(int argc, char *argv[]) {
    TranslationLibrary translationLibrary;

    ProgramConfig config = ProgramConfig::LoadOrDefault();

    AddAllTranslations(translationLibrary);

    TranslatableEntry titleEntry{
        &translationLibrary,
        config.language,
        "program.name"
    };

    EasyGui::GlobalContext::Init();

    auto window = EasyGui::CreateWindow(EasyGui::WindowSpec{
        .title = titleEntry.GetResult(),
    });

    auto &io = ImGui::GetIO();

    ImFontConfig font_cfg;
    font_cfg.SizePixels = 32.0f; // Set your desired font size, all languages
    io.Fonts->AddFontFromFileTTF("assets/fonts/OpenSans.ttf", font_cfg.SizePixels, &font_cfg,
                                 io.Fonts->GetGlyphRangesDefault());
    font_cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansSC.ttf", font_cfg.SizePixels, &font_cfg,
                                 GetGlyphRangesChineseFull());

    window->GetGraphicsContext().SetClearColor(vk::ClearColorValue(0.f, 0.f, 0.f, 1.0f));

    window->EmplaceLayer<BackGroundLayer>(&translationLibrary, config, [&] (TranslationLibrary *translationLibrary, ProgramConfig &config) {
        titleEntry = TranslatableEntry{
            translationLibrary,
            config.language,
            "program.name"
        };

        SDL_SetWindowTitle(window->GetWindow(), titleEntry.GetResult().c_str());
    });
    window->MainLoop();
    window.reset();

    EasyGui::GlobalContext::Shutdown();

    ProgramConfig::Save(config);

    return 0; // Return 0 to indicate successful execution.
}

void AddAllTranslations(TranslationLibrary &translationLibrary) {
    std::filesystem::path translations = std::filesystem::current_path() / "assets" / "lang";
    for (auto entry: std::filesystem::directory_iterator(translations)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            try {
                auto lang = entry.path().stem().string();
                auto jsonContent = nlohmann::json::parse(std::ifstream(entry.path()));
                std::unordered_map<std::string, std::string> translationMap;
                for (const auto &[k, v]: jsonContent.items()) {
                    translationMap[k] = std::move(v.get<std::string>());
                }
                translationLibrary.AddTranslation(lang, std::move(translationMap));
                std::cout << "Loaded translation for language: " << lang << std::endl;
            } catch (const std::exception &e) {
                std::cerr << "Error parsing translation file " << entry.path() << ": " << e.what() << std::endl;
            }
        }
    }
}

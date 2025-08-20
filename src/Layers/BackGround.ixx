export module Layers.BackGround;

import EasyGui;
import TranslationLibarary;
import ProgramConfig;
import EasyGui.Utils.AsyncProvider;
import std.compat;

export class BackGroundLayer : public EasyGui::IUpdatableLayer {
private:
    ProgramConfig *m_ProgramConfig{nullptr};
    TranslationLibrary *m_TranslationLibrary{nullptr};

    TranslatableLabel m_LanguageLabel{};
    std::unordered_map<std::string, TranslatableEntry> m_Languages{};

    std::function<void(TranslationLibrary *translationLibrary, ProgramConfig &config)> m_RefreshCallback;

    TranslatableLabel m_ApplicationLabel{};

    struct EnchantmentInfo {
        TranslatableLabel SliderLabel;
        std::string CheckBoxLabel;
        bool Enabled{false};
        int Level{1};
    };

    TranslatableLabel m_EnchantmentsLabel{};
    std::unordered_map<std::string, EnchantmentInfo> m_Enchantments{};

    bool DemoEnabled = false;

    std::string m_EnchantmentSearchText;

    std::mutex m_EnchantmentSearchMutex;
    std::string m_SynchedEnchantmentSearchText;

    TranslatableLabel m_EnchantmentSearchLabel;

    EasyGui::AsyncProvider<std::vector<std::string>> m_FilteredEnchantments{EasyGui::DefaultConstructed{}};

public:
    void UpdateEnchantmentList() {
        m_FilteredEnchantments.SetFuture([&] {
            std::vector<std::string> filteredEnchantments;
            std::lock_guard lock(m_EnchantmentSearchMutex);
            for (const auto &[key, enchantment]: m_Enchantments) {
                if (enchantment.SliderLabel.GetResult().contains(m_SynchedEnchantmentSearchText)) {
                    filteredEnchantments.push_back(key);
                }
            }
            return filteredEnchantments;
        });
    }

    void Refresh(TranslationLibrary *translationLibrary, ProgramConfig &config) {
        m_ProgramConfig = &config;
        m_TranslationLibrary = translationLibrary;
        m_LanguageLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.language"
        };

        m_ApplicationLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.name"
        };

        m_EnchantmentsLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.enchantments"
        };

        m_EnchantmentSearchLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.enchantment_search"
        };

        m_FilteredEnchantments.Wait();

        for (const auto &key: translationLibrary->GetAllTranslations().at(config.language) | std::views::keys) {
            if (key.starts_with("enchantment.minecraft")) {
                m_Enchantments[key].SliderLabel = TranslatableLabel{
                    translationLibrary,
                    config.language,
                    key
                };
                m_Enchantments[key].CheckBoxLabel = std::string("##") + key;
            }
        }

        UpdateEnchantmentList();

        for (const auto &[lang, translation]: translationLibrary->GetAllTranslations()) {
            if (translation.contains("program.current_language")) {
                m_Languages[lang] = TranslatableEntry{
                    translationLibrary,
                    lang,
                    "program.current_language"
                };
            }
        }

        m_RefreshCallback(translationLibrary, config);
    }

    BackGroundLayer(TranslationLibrary *translationLibrary, ProgramConfig &config,
                    std::function<void(TranslationLibrary *translationLibrary, ProgramConfig &config)> refreshCallback =
                            [](TranslationLibrary *, ProgramConfig &) {}) {
        m_RefreshCallback = std::move(refreshCallback);
        Refresh(translationLibrary, config);
    }

    void OnUpdate() override {
        EasyGui::UI::RenderBackgroundSpace([this] {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu(m_LanguageLabel.GetResult().data())) {
                    for (const auto &[lang, entry]: m_Languages) {
                        if (ImGui::MenuItem(entry.GetResult().data())) {
                            m_ProgramConfig->language = lang;
                            Refresh(m_TranslationLibrary, *m_ProgramConfig);
                        }
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Debug")) {
                    ImGui::MenuItem("Demo Window", nullptr, &DemoEnabled);
                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }
        }, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar);

        ImGui::Begin("Application", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse);

        ImVec2 childSize = ImVec2(500, 800);

        // set maximum height to available height - 100

        ImVec2 availableSize = ImGui::GetContentRegionAvail();

        ImGui::SetNextWindowSizeConstraints(
            ImVec2(0, 0),
            ImVec2(availableSize.x, availableSize.y - 100)
        );


        ImGui::BeginChild(m_EnchantmentsLabel.GetResult().data(), childSize,
                          ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_NoBackground);

        if (ImGui::InputText(m_EnchantmentSearchLabel.GetResult().data(), &m_EnchantmentSearchText)) {
            // Filter enchantments based on search text
            std::lock_guard lock(m_EnchantmentSearchMutex);
            m_SynchedEnchantmentSearchText = m_EnchantmentSearchText;
            UpdateEnchantmentList();
        }

        ImGui::BeginChild("EnchantmentsList");

        for (const auto &key: m_FilteredEnchantments.Get()) {
            auto &enchantment = m_Enchantments[key];
            ImGui::Checkbox(enchantment.CheckBoxLabel.data(), &enchantment.Enabled);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(400);
            ImGui::SliderInt(enchantment.SliderLabel.GetResult().data(), &enchantment.Level, 1, 255);
        }

        ImGui::EndChild();

        ImGui::EndChild();

        ImGui::End();

        if (DemoEnabled)
            ImGui::ShowDemoWindow(&DemoEnabled);
    }
};

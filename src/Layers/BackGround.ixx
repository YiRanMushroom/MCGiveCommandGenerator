export module Layers.BackGround;

import EasyGui;
import TranslationLibarary;
import ProgramConfig;
import EasyGui.Utils.AsyncProvider;
import std.compat;
import EasyGui.Utils.Atomic;

export class BackGroundLayer : public EasyGui::IUpdatableLayer {
private:
    ProgramConfig *m_ProgramConfig{nullptr};
    TranslationLibrary *m_TranslationLibrary{nullptr};

    TranslatableLabel m_LanguageLabel{};
    std::unordered_map<std::string, TranslatableEntry> m_Languages{};

    std::function<void(TranslationLibrary *translationLibrary, ProgramConfig &config)> m_RefreshCallback;

    TranslatableLabel m_ApplicationLabel{};

    bool DemoEnabled = false;

    struct EnchantmentInfo {
        TranslatableLabel SliderLabel{};
        std::string CheckBoxLabel{};
        bool Enabled{false};
        int Level{1};
    };

    TranslatableLabel m_EnchantmentsLabel{};
    std::unordered_map<std::string, EnchantmentInfo> m_Enchantments{};

    std::string m_EnchantmentSearchText;
    std::mutex m_EnchantmentSearchMutex;
    std::string m_SyncedEnchantmentSearchText;
    TranslatableLabel m_EnchantmentSearchLabel;
    EasyGui::AsyncProvider<std::vector<std::string>> m_FilteredEnchantments{EasyGui::DefaultConstructed{}};

    TranslatableLabel m_ItemsLabel{};
    std::unordered_map<std::string, TranslatableLabel> m_Items{};

    std::string m_ItemSearchText;
    std::mutex m_ItemSearchMutex;
    std::string m_SyncedItemSearchText;
    TranslatableLabel m_ItemSearchLabel;
    EasyGui::AsyncProvider<std::vector<std::string>> m_FilteredItems{EasyGui::DefaultConstructed{}};

    std::mutex m_SelectedItemMutex;
    std::string m_SelectedItem{};

    TranslatableLabel m_GenerateButtonLabel{};
    TranslatableLabel m_CopyButtonLabel{};

    EasyGui::ARCMutex<std::string> m_GeneratedCommandBuffer{EasyGui::DefaultConstructed{}};
    std::future<void> m_GeneratedCommandFuture;

    TranslatableEntry m_ErrorTitleEntry{};
    TranslatableEntry m_ErrorNoItemSelectedEntry{};

    TranslatableEntry m_SelectedItemEntry{};
    TranslatableEntry m_NoItemEntry{};

public:
    void UpdateEnchantmentList() {
        m_FilteredEnchantments.SetFuture([&] {
            std::vector<std::string> filteredEnchantments;
            std::lock_guard lock(m_EnchantmentSearchMutex);
            for (const auto &[key, enchantment]: m_Enchantments) {
                if (enchantment.SliderLabel.GetResult().contains(m_SyncedEnchantmentSearchText)) {
                    filteredEnchantments.push_back(key);
                }
            }
            return filteredEnchantments;
        });
    }

    void UpdateItemList() {
        m_FilteredItems.SetFuture([&] {
            std::vector<std::string> filteredItems;
            std::lock_guard lock(m_ItemSearchMutex);
            for (const auto &[key, item]: m_Items) {
                if (item.GetResult().contains(m_SyncedItemSearchText)) {
                    filteredItems.push_back(key);
                }
            }
            return filteredItems;
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
        m_FilteredItems.Wait();
        if (m_GeneratedCommandFuture.valid()) {
            m_GeneratedCommandFuture.wait();
        }

        m_Enchantments.clear();
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

        m_ItemsLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.items"
        };

        m_ItemSearchLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.item_search"
        };


        m_Items.clear();
        for (const auto &key: translationLibrary->GetAllTranslations().at(config.language) | std::views::keys) {
            if (key.starts_with("item.minecraft") && !key.substr(15).contains('.')) {
                m_Items[key] = TranslatableLabel{
                    translationLibrary,
                    config.language,
                    key
                };
            }
        }

        UpdateItemList();

        for (const auto &[lang, translation]: translationLibrary->GetAllTranslations()) {
            if (translation.contains("program.current_language")) {
                m_Languages[lang] = TranslatableEntry{
                    translationLibrary,
                    lang,
                    "program.current_language"
                };
            }
        }

        m_GenerateButtonLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.generate"
        };

        m_CopyButtonLabel = TranslatableLabel{
            translationLibrary,
            config.language,
            "program.copy"
        };

        m_ErrorTitleEntry = TranslatableEntry{
            translationLibrary,
            config.language,
            "program.error"
        };

        m_ErrorNoItemSelectedEntry = TranslatableEntry{
            translationLibrary,
            config.language,
            "program.error.no_item_selected"
        };

        m_SelectedItemEntry = TranslatableEntry{
            translationLibrary,
            config.language,
            "program.selected_item"
        };

        m_NoItemEntry = TranslatableEntry{
            translationLibrary,
            config.language,
            "program.no_item"
        };

        m_RefreshCallback(translationLibrary, config);
    }

    BackGroundLayer(TranslationLibrary *translationLibrary, ProgramConfig &config,
                    std::function<void(TranslationLibrary *translationLibrary, ProgramConfig &config)> refreshCallback =
                            [](TranslationLibrary *, ProgramConfig &) {}) {
        m_RefreshCallback = std::move(refreshCallback);
        Refresh(translationLibrary, config);
    }

    void RenderGeneration() {
        ImGui::TextFmt("{}: {}",
                       m_SelectedItemEntry.GetResult().data(),
                       m_SelectedItem.empty()
                            ? m_NoItemEntry.GetResult().data()
                            : m_Items.at(m_SelectedItem).GetPure().data()
        );
        if (ImGui::Button(m_GenerateButtonLabel.GetResult().data())) {
            GenerateCommand();
        }
        ImGui::SameLine();
        if (ImGui::Button(m_CopyButtonLabel.GetResult().data())) {
            CopyCommand();
        }

        m_GeneratedCommandBuffer.Let([&](std::string &command) {
            ImGui::InputTextMultiline("##GeneratedCommand", &command,
                                      ImVec2(ImGui::GetContentRegionAvail().x, 100),
                                      ImGuiInputTextFlags_AutoSelectAll);
        });
    }

    void GenerateCommand() {
        m_GeneratedCommandBuffer.DeferredSet([&] {
            std::lock_guard lock{m_SelectedItemMutex};
            if (m_SelectedItem.empty()) {
                EasyGui::Windows::ShowErrorMessage(
                    EasyGui::Windows::Utf8ToUtf16(m_ErrorNoItemSelectedEntry.GetResult()),
                    EasyGui::Windows::Utf8ToUtf16(m_ErrorTitleEntry.GetResult()));
                return std::string("");
            }

            std::string command = "/give @p " + m_SelectedItem.substr(15) + "[minecraft:enchantments={";
            bool isInitial = true;
            for (const auto &[key, enchant]: m_Enchantments) {
                if (enchant.Enabled) {
                    if (!isInitial) {
                        command += ", ";
                    } else {
                        isInitial = false;
                    }
                    command += std::format(R"("{}": {})", key.substr(22), enchant.Level);
                }
            }
            command += "}]";
            return command;
        });
    }

    void CopyCommand() {
        auto command = m_GeneratedCommandBuffer.Read();

        if (command->empty()) {
            return;
        }

        std::wstring wstr = EasyGui::Windows::Utf8ToUtf16(*command);

        EasyGui::Windows::SetClipboardContent(wstr);
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
            m_SyncedEnchantmentSearchText = m_EnchantmentSearchText;
            UpdateEnchantmentList();
        }

        ImGui::BeginChild("EnchantmentsList");

        for (const auto &key: m_FilteredEnchantments.Get()) {
            auto &enchantment = m_Enchantments[key];
            ImGui::Checkbox(enchantment.CheckBoxLabel.data(), &enchantment.Enabled);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(400);
            if (ImGui::SliderInt(enchantment.SliderLabel.GetResult().data(), &enchantment.Level, 1, 255))
                enchantment.Enabled = true;
        }

        ImGui::EndChild();

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild(m_ItemsLabel.GetResult().data(), childSize,
                          ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders,
                          ImGuiWindowFlags_NoBackground);

        if (ImGui::InputText(m_ItemSearchLabel.GetResult().data(), &m_ItemSearchText)) {
            // Filter items based on search text
            std::lock_guard lock(m_ItemSearchMutex);
            m_SyncedItemSearchText = m_ItemSearchText;
            UpdateItemList();
        }

        ImGui::BeginChild();

        for (const auto &key: m_FilteredItems.Get()) {
            auto &item = m_Items[key];
            if (ImGui::Selectable(item.GetResult().data())) {
                std::lock_guard lock(m_SelectedItemMutex);
                m_SelectedItem = key;
            }
        }

        ImGui::EndChild();

        ImGui::EndChild();

        RenderGeneration();

        ImGui::End();

        if (DemoEnabled)
            ImGui::ShowDemoWindow(&DemoEnabled);
    }
};

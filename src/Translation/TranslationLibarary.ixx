export module TranslationLibarary;

import std.compat;
import Util.Json;

export class TranslationLibrary;
export class TranslatableEntry;
export class TranslatableLabel;

class TranslationLibrary {
    friend class TranslatableEntry;
    friend class TranslatableLabel;

public:
    TranslationLibrary() = default;

    TranslationLibrary(const TranslationLibrary &) = delete;

    TranslationLibrary(TranslationLibrary &&) = delete;

    TranslationLibrary &operator=(const TranslationLibrary &) = delete;

    TranslationLibrary &operator=(TranslationLibrary &&) = delete;

    void AddTranslation(std::string_view language, std::unordered_map<std::string, std::string> translation) {
        if (m_LanguageToTranslationMap.contains(language.data())) {
            m_LanguageToTranslationMap[language.data()].insert(translation.begin(), translation.end());
        } else {
            m_LanguageToTranslationMap[language.data()] = std::move(translation);
        }
    }

    [[nodiscard]] std::string_view GetTranslation(std::string_view language, std::string_view key) const {
        if (m_LanguageToTranslationMap.contains(language.data())) {
            const auto &translations = m_LanguageToTranslationMap.at(language.data());
            if (translations.contains(key.data())) {
                return translations.at(key.data());
            }
        }
        return key; // Return the key itself if no translation is found
    }

    const std::unordered_map<std::string, std::unordered_map<std::string, std::string>> &GetAllTranslations() const {
        return m_LanguageToTranslationMap;
    }

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_LanguageToTranslationMap{};
};

class TranslatableEntry {
public:
    TranslatableEntry() = default;

    TranslatableEntry(TranslationLibrary *translationLibrary, std::string_view language, std::string_view key)
        : m_TranslationLibrary(translationLibrary) {
        m_Result = m_TranslationLibrary->GetTranslation(language, key);
    }

    void SetTranslation(std::string_view language, std::string_view key) {
        m_Result = m_TranslationLibrary->GetTranslation(language, key);
    }

    [[nodiscard]] const std::string &GetResult() const {
        return m_Result;
    }

    [[nodiscard]] operator std::string_view() const {
        return m_Result;
    }

private:
    TranslationLibrary *m_TranslationLibrary{nullptr};

    std::string m_Result;
};

class TranslatableLabel {
public:
    TranslatableLabel() = default;

    TranslatableLabel(TranslationLibrary *translationLibrary, std::string_view language, std::string_view key)
        : m_TranslationLibrary(translationLibrary) {
        m_Result = std::string(m_TranslationLibrary->GetTranslation(language, key)) + "###" + std::string(key);
    }

    void SetTranslation(std::string_view language, std::string_view key) {
        m_Result = std::string(m_TranslationLibrary->GetTranslation(language, key)) + "###" + std::string(key);
    }

    [[nodiscard]] const std::string &GetResult() const {
        return m_Result;
    }

    [[nodiscard]] operator std::string_view() const {
        return m_Result;
    }

private:
    TranslationLibrary *m_TranslationLibrary{nullptr};

    std::string m_Result;
};

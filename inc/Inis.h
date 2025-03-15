#pragma once
#include <Util.h>

#include <filesystem>

class Inis : public Singleton<Inis> {
    // User Inis
  public:
    void LoadMainIni();
    void LoadCategoryFiles();

  private:
    inline static constexpr const char* cInisPath{R"(.\Data\SKSE\Plugins\NUDE)"};
    inline static constexpr const char* cCategories{"categories"};
    inline static constexpr const char* cParent{"parent"};
    inline static constexpr const char* cName{"name"};
    inline static constexpr const char* cWildcards{"wildcards"};
    inline static constexpr const char* cKeywords{"keywords"};
    inline static constexpr const char* cFactions{"factions"};
    inline static constexpr const char* cRaces{"races"};
    inline static constexpr const char* cNPCs{"npcs"};
    inline static constexpr const char* cUnderwear{"underwear"};
    inline static constexpr const char* cGender{"gender"};
    inline static constexpr const char* cChance{"chance"};

    void LoadCategories(std::filesystem::path path, const bool hasParent);

    template <typename T>
    void LoadFilters(const std::vector<std::string>& filters, std::vector<T*>& vec, const std::string& type) {
      std::string ty = type.substr(0, type.size() - 1);
      for (const auto& filter : filters) {
        auto loc = ut->StrToLoc(filter);
        if (loc.first) {
          auto rec = ut->LoadForm<T>(loc);
          if (rec) {
            vec.push_back(rec);
            SKSE::log::info("\t\tAdded [{}] filter [0x{:x}].", ty, rec->formID);
          } else {
            SKSE::log::error("\t\tParsed [{}] filter [{}] but could not find it in the game.", ty, filter);
          }
        } else {
          SKSE::log::error("\t\tCould not parse the [{}] filter [{}]!,", ty, filter);
        }
      }
    }
};
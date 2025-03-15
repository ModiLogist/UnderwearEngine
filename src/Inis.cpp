#include <Core.h>
#include <Inis.h>

#include <nlohmann/json.hpp>

Inis* inis = Inis::GetSingleton();

void Inis::LoadMainIni() {}

void Inis::LoadCategoryFiles() {
  SKSE::log::info("Loading categories...");
  if (std::filesystem::exists(cInisPath)) {
    for (const auto& entry : std::filesystem::directory_iterator(cInisPath)) {
      if (entry.path().extension() == ".json") {
        LoadCategories(entry.path(), false);
      }
    }
    for (const auto& entry : std::filesystem::directory_iterator(cInisPath)) {
      if (entry.path().extension() == ".json") {
        LoadCategories(entry.path(), true);
      }
    }
  }
  SKSE::log::info("Finished loading categories.");
}

void Inis::LoadCategories(std::filesystem::path path, const bool hasParent) {
  std::ifstream file(path);
  if (!file.is_open()) {
    SKSE::log::error("Could not open file: {}", path.filename().string());
  }

  nlohmann::json jsonData;
  file >> jsonData;
  SKSE::log::info("\tProcessing categories from file [{}]...", path.filename().string());
  if (jsonData.contains(cCategories)) {
    for (const auto& category : jsonData[cCategories]) {
      std::string parent = "";
      if (category.contains(cParent)) parent = category[cParent];
      if (!hasParent && !parent.empty()) continue;
      if (hasParent && parent.empty()) continue;

      std::string name = "";
      std::vector<std::string> wildcards;
      std::vector<RE::BGSKeyword*> kys;
      std::vector<RE::TESFaction*> fcs;
      std::vector<RE::TESRace*> rcs;
      std::vector<RE::TESNPC*> nps;
      std::set<SEFormLoc> uws;
      RE::SEX sxs = RE::SEX::kTotal;
      float chance = 0.0f;

      wildcards.clear();
      kys.clear();
      fcs.clear();
      rcs.clear();
      nps.clear();
      uws.clear();

      if (category.contains(cName)) name = category[cName];
      SKSE::log::info("\tLoading category [{}]...", name);
      if (category.contains(cWildcards)) wildcards = category[cWildcards].get<std::vector<std::string>>();

      if (category.contains(cKeywords)) {
        auto keywords = category[cKeywords].get<std::vector<std::string>>();
        LoadFilters<RE::BGSKeyword>(keywords, kys, cKeywords);
      }

      if (category.contains(cFactions)) {
        auto factions = category[cFactions].get<std::vector<std::string>>();
        LoadFilters<RE::TESFaction>(factions, fcs, cFactions);
      }

      if (category.contains(cRaces)) {
        auto races = category[cRaces].get<std::vector<std::string>>();
        LoadFilters<RE::TESRace>(races, rcs, cRaces);
      }

      if (category.contains(cNPCs)) {
        auto npcs = category[cNPCs].get<std::vector<std::string>>();
        LoadFilters<RE::TESNPC>(npcs, nps, cNPCs);
      }

      if (category.contains(cUnderwear)) {
        auto undies = category[cUnderwear].get<std::vector<std::string>>();
        for (const auto& item : undies) {
          auto loc = ut->StrToLoc(item);
          if (loc.first) {
            uws.insert(loc);
          } else {
            SKSE::log::error("\t\tCould not parse the underwear [{}] in file [{}].", item, path.filename().string());
          }
        }
      }

      if (category.contains(cGender)) {
        std::string gender = category[cGender];
        if (gender == "M" || gender == "Male" || gender == "m" || gender == "male") {
          sxs = RE::SEX::kMale;
        } else if (gender == "F" || gender == "Female" || gender == "f" || gender == "female") {
          sxs = RE::SEX::kFemale;
        } else {
          SKSE::log::error("\t\tCould not parse the gender [{}] in file [{}].", gender, path.filename().string());
        }
      }

      if (category.contains(cChance)) {
        chance = category[cChance];
        if (parent.empty() && chance > 0.0f) {
          SKSE::log::error("\t\tMain category [{}] in file [{}] cannot have a chance.", name, path.filename().string());
        }
      }
      core->AddCategory(parent, name, wildcards, kys, fcs, rcs, nps, uws, sxs, chance);
      if (parent.empty()) {
        SKSE::log::info("\tLoaded main category [{}].", name);

      } else {
        SKSE::log::info("\tLoaded sub-category [{}] under [{}].", name, parent);
      }
    }
  } else {
    SKSE::log::error("\tCould not find the [{}] section in file [{}].", cCategories, path.filename().string());
    return;
  }
  SKSE::log::info("\tFinished processing categories from file [{}].", path.filename().string());
}

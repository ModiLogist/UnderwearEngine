#pragma once

class Core;

extern Core* core;

class Util {
  public:
    inline static constexpr std::string_view cName{"UnderwearEngine.esp"};
    inline static constexpr std::string_view cTng{"TheNewGentleman.esp"};
    inline static constexpr std::string_view cSkyrim{"Skyrim.esm"};
    inline static constexpr char cDelimChar{'~'};
    inline static constexpr char cColonChar{':'};
    inline static constexpr int cNul{-1};
    inline static constexpr int cDef{-2};

    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlot32{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlot52{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

    enum eRes { resOk = 0, resFail = 1, resWarn = 2 };

    enum eBoolSetting { bsSomething, BoolSettingCount };
    enum eKeywords { kyManMer, kyBeast, kyCreature, kyVampire, kyTngUw, kyTngProcessed, kyTngReady, kyTngIgnored, kyItemM, kyItemF, KeywordsCount };
    enum eRaces { raceDefault, raceDefBeast, RacesCount };
    enum eFLs { flExcluded, FLCount };

  public:
    static RE::TESDataHandler* SEDH() {
      if (!sedh) sedh = RE::TESDataHandler::GetSingleton();
      return sedh;
    }

    template <typename T>
    static constexpr auto LoadForm(const SEFormLocView& loc) {
      return SEDH()->LookupForm<T>(loc.first, loc.second);
    }

    static bool GetBoolSetting(const size_t idx) { return idx < BoolSettingCount ? boolSettings[idx] : false; };
    static void SetBoolSetting(const size_t idx, const bool value) {
      if (idx < BoolSettingCount) boolSettings[idx] = value;
    };

    static RE::BGSKeyword* Key(const size_t idx) {
      if (idx >= KeywordsCount) return nullptr;
      if (!keywords[idx]) keywords[idx] = LoadForm<RE::BGSKeyword>(keywordIDs[idx]);
      return keywords[idx];
    }

    static std::vector<RE::BGSKeyword*> Keys(const size_t first, const size_t last) {
      std::vector<RE::BGSKeyword*> res = {};
      if (last >= KeywordsCount) return res;
      for (auto i = first; i <= last; i++) res.push_back(Key(i));
      return res;
    }

    static RE::TESRace* Race(const size_t idx) {
      if (idx >= RacesCount) return nullptr;
      if (!races[idx]) races[idx] = LoadForm<RE::TESRace>(raceIDs[idx]);
      return races[idx];
    }

    static RE::BGSListForm* FormList(const size_t idx) {
      if (idx >= FLCount) return nullptr;
      if (!fls[idx]) fls[idx] = LoadForm<RE::BGSListForm>(flIDs[idx]);
      return fls[idx];
    }

    static RE::TESObjectARMO* TngCover() {
      if (!cover) cover = LoadForm<RE::TESObjectARMO>(coverLoc);
      return cover;
    }

    static RE::BGSListForm* ProduceOrGetFormList(const std::string& edid) {
      auto& allFLs = SEDH()->GetFormArray<RE::BGSListForm>();
      auto it = std::find_if(allFLs.begin(), allFLs.end(), [&](const auto& fl) { return fl && fl->GetFormEditorID() == edid.c_str(); });
      if (it != allFLs.end()) {
        return *it;
      }
      auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSListForm>();
      auto fl = factory ? factory->Create() : nullptr;
      if (fl) {
        fl->SetFormEditorID(edid.c_str());
        allFLs.push_back(fl);
      }
      return fl;
    }

    static RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword) {
      auto& allKeywords = SEDH()->GetFormArray<RE::BGSKeyword>();
      auto it = std::find_if(allKeywords.begin(), allKeywords.end(), [&](const auto& kw) { return kw && kw->formEditorID == keyword.c_str(); });
      if (it != allKeywords.end()) {
        return *it;
      }
      const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
      auto kw = factory ? factory->Create() : nullptr;
      if (kw) {
        kw->formEditorID = keyword;
        allKeywords.push_back(kw);
      }
      return kw;
    }

    static void MsgBox(const char* message) { RE::DebugMessageBox(message); }

    static SEFormLoc FormToLoc(const RE::TESForm* form) {
      std::string filename = form->GetFile(0) ? std::string(form->GetFile(0)->GetFilename()) : "NoFile";
      auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
      return {formID, filename};
    }

    static SEFormLocView FormToLocView(RE::TESForm* form) {
      auto filename = form->GetFile(0) ? form->GetFile(0)->GetFilename() : "NoFile";
      auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
      return {formID, filename};
    }

    static SEFormLoc StrToLoc(const std::string recordStr) {
      const size_t sepLoc = recordStr.find(cDelimChar);
      if (sepLoc == std::string::npos) return {0, ""};
      const RE::FormID formID = std::strtol(recordStr.substr(0, sepLoc).data(), nullptr, 0);
      const std::string modName = recordStr.substr(sepLoc + 1);
      return std::make_pair(formID, modName);
    }

    static std::string FormToStr(RE::TESForm* form) {
      if (!form || !form->GetFile(0)) return "";
      std::ostringstream oss;
      auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
      oss << std::hex << formID;
      return "0x" + oss.str() + cDelimChar + std::string(form->GetFile(0)->GetFilename());
    }

  private:
    inline static RE::TESDataHandler* sedh;
    inline static bool boolSettings[BoolSettingCount];

    inline static constexpr SEFormLocView keywordIDs[KeywordsCount] = {{0x13794, cSkyrim}, {0xD61D1, cSkyrim}, {0x13795, cSkyrim}, {0xA82BB, cSkyrim}, {0xFFE, cTng},
                                                                       {0xFF0, cTng},      {0xFF1, cTng},      {0xFF3, cTng},      {0xFFF, cName},     {0xFFE, cName}};
    inline static RE::BGSKeyword* keywords[KeywordsCount];

    inline static constexpr SEFormLocView raceIDs[RacesCount]{{0x19, cSkyrim}, {0x13745, cSkyrim}};
    inline static RE::TESRace* races[RacesCount];

    inline static constexpr SEFormLocView flIDs[FLCount]{{0xEFF, cName}};
    inline static RE::BGSListForm* fls[FLCount];

    inline static constexpr SEFormLocView coverLoc{0xAFF, cTng};
    inline static RE::TESObjectARMO* cover;
};

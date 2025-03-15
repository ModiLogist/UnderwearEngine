#pragma once

class Util;
class Inis;
class Core;
class Events;

extern Util* ut;
extern Inis* inis;
extern Core* core;
extern Events* events;

class Util : public Singleton<Util> {
  public:
    inline static constexpr char cDelimChar{'~'};
    inline static constexpr char cColonChar{':'};
    inline static constexpr int cNul{-1};
    inline static constexpr int cDef{-2};

    inline static constexpr std::string_view cName{"UnderwearEngine.esp"};
    inline static constexpr std::string_view cTng{"TheNewGentleman.esp"};
    inline static constexpr std::string_view cSkyrim{"Skyrim.esm"};
    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlot32{RE::BGSBipedObjectForm::BipedObjectSlot::kBody};
    inline static constexpr RE::BGSBipedObjectForm::BipedObjectSlot cSlot52{RE::BGSBipedObjectForm::BipedObjectSlot::kModPelvisSecondary};

    enum eBoolSetting { bsSomething, BoolSettingCount };
    enum eKeywords {
      kyManMer,
      kyBeast,
      kyCreature,
      kyVampire,
      kyTngUw,
      kyTngProcessed,
      kyTngReady,
      kyTngIgnored,
      kyTngCovering,
      kyTngFemCovering,
      kyTngMalCovering,
      kyItemM,
      kyItemF,
      kyItemFake,
      KeywordsCount
    };
    enum eRaces { raceDefault, raceDefBeast, RacesCount };
    enum eFLs { flExcluded, flPCUndies, FLCount };

    RE::TESDataHandler* SEDH();

    template <typename T>
    constexpr auto LoadForm(const SEFormLocView& loc) {
      return SEDH()->LookupForm<T>(loc.first, loc.second);
    }

    bool GetBoolSetting(const size_t idx);
    void SetBoolSetting(const size_t idx, const bool value);

    RE::BGSKeyword* Key(const size_t idx);
    std::vector<RE::BGSKeyword*> Keys(const size_t first, const size_t last);

    RE::TESRace* Race(const size_t idx);
    bool IsBaseCover(RE::TESObjectARMO* down);

    RE::BGSListForm* FormList(const size_t idx);
    RE::BGSKeyword* ProduceOrGetKw(const std::string& keyword);

    void MsgBox(const char* message);

    SEFormLoc FormToLoc(RE::TESForm* form);
    SEFormLocView FormToLocView(RE::TESForm* form);
    SEFormLoc StrToLoc(const std::string& recordStr);
    std::string FormToStr(RE::TESForm* form);

  private:
    inline static RE::TESDataHandler* sedh;
    inline static bool boolSettings[BoolSettingCount];

    inline static constexpr SEFormLocView keywordIDs[KeywordsCount] = {{0x13794, cSkyrim}, {0xD61D1, cSkyrim}, {0x13795, cSkyrim}, {0xA82BB, cSkyrim}, {0xFFE, cTng},
                                                                       {0xFF0, cTng},      {0xFF1, cTng},      {0xFF3, cTng},      {0xFFD, cTng},      {0xFFC, cTng},
                                                                       {0xFFB, cTng},      {0xFFF, cName},     {0xFFE, cName},     {0xFFD, cName}};
    inline static RE::BGSKeyword* keywords[KeywordsCount];

    inline static constexpr SEFormLocView raceIDs[RacesCount]{{0x19, cSkyrim}, {0x13745, cSkyrim}};
    inline static RE::TESRace* races[RacesCount];

    inline static constexpr SEFormLocView flIDs[FLCount]{{0xF00, cName}, {0xF01, cName}};
    inline static RE::BGSListForm* fls[FLCount];

    inline static constexpr SEFormLocView coverID{0xAFF, cTng};

    bool try_strtoul(const std::string& str, std::uint32_t& result, int base = 0);
};

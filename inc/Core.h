#pragma once

#include <Util.h>

class Core : public Singleton<Core> {
  public:
    void LoadRaces();
    void LoadNPCs();

  private:
    inline static const std::string cFL[2]{"NudeMalTexts:", "NudeFemTexts:"};

    struct NudeGroup {
        std::string mesh;
        RE::BGSListForm* texts;
        std::vector<size_t> items;
    };

    std::set<RE::TESObjectARMO*> processedSkins;
    std::vector<NudeGroup> malGroups;
    std::vector<NudeGroup> femGroups;
    bool IsValidRace(RE::TESRace* race, bool postLoad = false);
    NudeGroup* GetSkinRaceMesh(RE::TESRace* race, RE::TESObjectARMO* skin, const bool isFemale);

  public:
    void LoadItems();

  private:
    std::set<RE::FormID> allUndies;
    std::vector<RE::TESObjectARMO*> malUndies;
    std::vector<RE::TESObjectARMO*> femUndies;
    void ProcessItem(RE::TESObjectARMO* item, const bool isFemale);
    bool IsUnderwear(RE::TESForm* item) { return allUndies.find(item->formID) != allUndies.end(); };

  public:
    void AddCategory(const std::string& parent, const std::string& name, const std::vector<std::string>& wildCards, const std::vector<RE::BGSKeyword*>& keywords,
                     const std::vector<RE::TESFaction*>& factions, const float chance = 0.0f);
    void AddItemsToCategory(const std::string& name, const std::vector<SEFormLoc>& undies, const bool isFemale);

  private:
    struct NudeCategory {
        std::string name;
        std::string parent;
        std::vector<std::string> wildCards;
        std::vector<RE::BGSKeyword*> keywords;
        std::vector<RE::TESFaction*> factions;
        float chance;
        std::map<size_t, bool> undies[2];
        size_t npcCount;
    };

    std::vector<NudeCategory> categories;

  public:
    bool GetItemStatus(const bool isFemale, const size_t cat, const size_t choice);
    void SetItemStatus(const bool isFemale, const size_t cat, const size_t choice, const bool value);
    std::vector<size_t> GetItems(const bool isFemale, const size_t groupIdx, const size_t cat, const bool onlyActive);

  private:
    std::vector<size_t> GetItems(const bool isFemale, const NudeGroup* meshGroup, const size_t cat, const bool onlyActive);
    /**
     * @brief Retrieves the underwear object based on the specified parameters.
     *
     * @param isFemale A boolean indicating if the character is female.
     * @param cat The category of the underwear.
     * @param meshGroup The mesh group of the underwear.
     * @param choice The choice index starting from 0 up to the count of available items.
     * @param onlyActive A boolean indicating if only active underwear should be considered.
     * @return A pointer to the TESObjectARMO representing the selected underwear.
     */
    RE::TESObjectARMO* GetUnderwear(const bool isFemale, const NudeGroup* meshGroup, const size_t cat, const size_t choice, const bool onlyActive);

  public:
    Util::eRes ProcessActor(RE::Actor* actor);
    RE::TESObjectARMO* GetActorItem(RE::Actor* actor);
    Util::eRes SetActorItem(RE::Actor* actor, const int itemIdx, const bool isUser = false);

  private:
    std::map<RE::Actor*, Util::eRes> processedActors;
    std::pair<NudeGroup*, size_t> CategorizeNPC(RE::TESNPC* npc);
    std::pair<RE::TESBoundObject*, bool> ForceUnderwear(RE::Actor* actor, RE::TESObjectARMO* underwear, const bool ifUpdate = false);
};

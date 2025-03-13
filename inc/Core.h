#pragma once

#include <Util.h>

#include <unordered_set>

class Core : public Singleton<Core> {
  public:
    bool LoadRaces();
    void LoadNPCs();

  private:
    struct NudeGroup {
        std::string mesh;
        std::vector<size_t> items;
    };

    std::set<RE::TESObjectARMO*> processedSkins;
    std::vector<NudeGroup> malGroups;
    std::vector<NudeGroup> femGroups;
    bool IsValidRace(RE::TESRace* race, bool postLoad = false);
    NudeGroup* GetSkinRaceMesh(RE::TESRace* race, RE::TESObjectARMO* skin, const bool isFemale);

  public:
    void LoadItems();
    bool IsUnderwear(RE::TESForm* item) { return item && allUndies.find(item->formID) != allUndies.end(); };
    std::vector<std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>>::iterator FindPair(RE::TESObjectARMO* undies, const bool isFemale, const bool isFake = false);
    RE::TESObjectARMO* GetOther(RE::TESObjectARMO* undies, const bool isFemale, const bool isFake = false);
    std::pair<RE::SEX, size_t> GetIdx(RE::TESObjectARMO* undies, const bool isFake = false);

  private:
    struct FormComparator {
        bool operator()(const RE::TESForm* lhs, const RE::TESForm* rhs) const { return lhs->formID < rhs->formID; }
    };
    std::unordered_set<RE::FormID> allUndies;
    std::set<RE::TESObjectARMO*, FormComparator> fakeUndies;
    std::vector<std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>> malUndies;
    std::vector<std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>> femUndies;
    void ProcessItem(RE::TESObjectARMO* item, const bool isFemale);
    void ProcessFakes(std::vector<std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>>& undies);

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
    std::vector<size_t> GetItems(const bool isFemale, const NudeGroup* meshGroup, const size_t cat, const bool onlyActive);
    /**
     * @brief Retrieves the underwear object based on the specified parameters.
     *
     * @param isFemale A boolean indicating if the character is female.
     * @param meshGroup The mesh group of the underwear.
     * @param cat The category of the underwear.
     * @param choice The choice index starting from 0 up to the count of available items.
     * @param onlyActive A boolean indicating if only active underwear should be considered.
     * @return A pointer to the TESObjectARMO representing the selected underwear.
     */
    std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>* GetItem(const bool isFemale, const NudeGroup* meshGroup, const size_t cat, const size_t choice, const bool onlyActive);

  public:
    void ResetProcessed() { processedActors.clear(); }
    void ProcessPlayer(RE::Actor* actor, RE::TESObjectARMO* armor = nullptr, const bool equipped = true);
    void ProcessActor(RE::Actor* actor);
    bool IsProcessed(RE::Actor* actor) { return processedActors.find(actor) != processedActors.end(); }
    std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>* GetActorItem(RE::Actor* actor, const bool noUpdate = false);
    void SetActorItem(RE::Actor* actor, const int itemIdx, const bool isUser = false);
    bool IsExcluded(RE::Actor* actor);
    void TryExclude(RE::Actor* actor, const bool toExclude);

  private:
    bool loggedExcluded = false;

    std::set<RE::Actor*> processedActors;
    std::pair<NudeGroup*, size_t> CategorizeNPC(RE::TESNPC* npc, const bool isUser);
    void UpdateActorItems(RE::Actor* actor, RE::TESObjectREFR::InventoryItemMap& inv);

  public:
    bool ShouldHave(RE::Actor* actor);
    void WearUndies(RE::Actor* actor, std::pair<RE::TESObjectARMO*, RE::TESObjectARMO*>* undies, RE::TESObjectARMO* except = nullptr);
    void TakeOffUndies(RE::Actor* actor, const bool justUp = false);

  private:
    RE::ActorEquipManager* eq;

  public:
    bool IsCovering(RE::Actor* actor, RE::TESObjectARMO* armor);
    bool HasCover(RE::Actor* actor, RE::TESObjectARMO* except = nullptr);

  private:
    std::vector<RE::BGSKeyword*> relevantKeys[2];
};

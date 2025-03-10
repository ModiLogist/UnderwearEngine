#include <Core.h>

Core *core = Core::GetSingleton();

void Core::LoadRaces() {
  SKSE::log::info("Processing races...");
  const auto &allRaces = Util::SEDH()->GetFormArray<RE::TESRace>();
  for (const auto &race : allRaces) {
    if (!IsValidRace(race, false)) continue;
    GetSkinRaceMesh(race, race->skin, false);
    GetSkinRaceMesh(race, race->skin, true);
  }
  SKSE::log::info("Finished processing races and found [{}] mesh groups for men, and [{}] mesh groups for women among them.", malGroups.size(), femGroups.size());
}

void Core::LoadNPCs() {
  SKSE::log::info("Processing NPCs...");
  auto malSize = malGroups.size();
  auto femSize = femGroups.size();
  auto &allNPCs = Util::SEDH()->GetFormArray<RE::TESNPC>();
  for (const auto &npc : allNPCs) {
    if (!npc || !npc->race || !IsValidRace(npc->race, false) || !npc->skin) continue;
    auto grpSize = npc->IsFemale() ? femGroups.size() : malGroups.size();
    GetSkinRaceMesh(npc->race, npc->skin, npc->IsFemale());
    auto grpSize2 = npc->IsFemale() ? femGroups.size() : malGroups.size();
    if (grpSize2 > grpSize)
      SKSE::log::debug("\tThe NPC [0x{:x}:{}] is using a custom mesh. Without a proper patch, they would not have an underwear.", npc->GetFormID(), npc->GetFormEditorID());
  }
  if (malGroups.size() > malSize)
    SKSE::log::warn("After processing NPCs, found [{}] new mesh groups for men. Actors using those groups would need a patch!", malGroups.size() - malSize);
  if (femGroups.size() > femSize)
    SKSE::log::warn("After processing NPCs, found [{}] new mesh groups for women. Actors using those groups would need a patch!", femGroups.size() - femSize);
}

bool Core::IsValidRace(RE::TESRace *race, bool postLoad) {
  if (!race || !race->HasKeyword(Util::Key(Util::kyManMer)) || !race->skin || race->HasKeyword(Util::Key(Util::kyCreature)) || race->IsChildRace()) return false;
  if (postLoad && race->HasKeyword(Util::Key(Util::kyTngIgnored))) return false;
  if (postLoad && !(race->HasKeyword(Util::Key(Util::kyTngProcessed)) || race->HasKeyword(Util::Key(Util::kyTngReady)))) return false;
  return true;
}

Core::NudeGroup *Core::GetSkinRaceMesh(RE::TESRace *race, RE::TESObjectARMO *skin, const bool isFemale) {
  RE::BGSTextureSet *text{nullptr};
  std::string mesh;
  if (!race || !skin) {
    SKSE::log::critical("\tSomething triggered nude to check a skin mesh for race:[0x{:x}] and skin:[0x{:x}]", race ? race->formID : 0x0, skin ? skin->formID : 0x0);
    return nullptr;
  }
  for (auto &aa : skin->armorAddons) {
    if (aa->IsValidRace(race) && aa->HasPartOf(Util::cSlot32)) {
      mesh = std::string(aa->bipedModels[isFemale ? 1 : 0].model.data());
      text = aa->skinTextures[isFemale ? 1 : 0];
      break;
    }
  }
  if (mesh.empty()) return nullptr;
  auto &groups = isFemale ? femGroups : malGroups;
  auto it = std::find_if(groups.begin(), groups.end(), [&](const auto &group) { return group.mesh == mesh; });
  if (it == groups.end()) {
    auto &newGroup = groups.emplace_back();
    newGroup.mesh = mesh;
    newGroup.texts = Util::ProduceOrGetFormList(cFL[isFemale ? 1 : 0] + std::to_string(groups.size()));
    if (newGroup.texts->forms.size() > 0) {
      SKSE::log::debug("\tThe formlist [0x{:x}] already has some forms in it.", newGroup.texts->forms[0]->formID);
    }
    if (text && newGroup.texts) newGroup.texts->AddForm(text);
    SKSE::log::debug("\tAdded mesh [{}] to {} meshes.", mesh, isFemale ? "women" : "men");
    SKSE::log::debug("\tAdded TextureSet [0x{:x}] to textures list of mesh [{}].", text->GetFormID(), mesh);
    return &newGroup;
  }
  if (text && (*it).texts && !(*it).texts->HasForm(text)) {
    it->texts->AddForm(text);
    SKSE::log::debug("\tAdded TextureSet [0x{:x}] to textures list of mesh [{}].", text->GetFormID(), mesh);
  }
  return &(*it);
}

void Core::LoadItems() {
  categories.clear();
  SKSE::log::info("Processing items...");
  AddCategory("", "default", {"*"}, {}, {});
  const auto &armorList = Util::SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto &armor : armorList) {
    if (armor->HasKeyword(Util::Key(Util::kyTngUw)) && armor->armorAddons.size() > 0) {
      allUndies.insert(armor->formID);
      if (armor->HasKeyword(Util::Key(Util::kyItemM))) ProcessItem(armor, false);
      if (armor->HasKeyword(Util::Key(Util::kyItemF))) ProcessItem(armor, true);
    }
  }
  SKSE::log::info("Finished processing items and found [{}] undies for men and [{}] undies for women.", malUndies.size(), femUndies.size());
}

void Core::ProcessItem(RE::TESObjectARMO *item, const bool isFemale) {
  auto &allList = isFemale ? femUndies : malUndies;
  auto &itemRace = item->armorAddons[0]->race;
  if (itemRace == Util::Race(Util::raceDefault) && item->armorAddons[0]->additionalRaces.size() > 0) itemRace = item->armorAddons[0]->additionalRaces[0];
  if (!IsValidRace(itemRace)) {
    SKSE::log::debug("\tThe item[0x{:x}:{}]'s race[{}] is not a  valid race.", item->formID, item->GetFormEditorID(), itemRace ? itemRace->GetFormEditorID() : "null");
    return;
  }
  auto group = GetSkinRaceMesh(itemRace, itemRace->skin, isFemale);
  if (!group) {
    SKSE::log::debug("\tThe item[0x{:x}:{}] is not categorized properly in the underwear list.", item->formID, item->GetFormEditorID());
    return;
  }
  auto idx = allList.size();
  allList.push_back(item);
  categories[0].undies[isFemale ? 1 : 0][idx] = true;
  group->items.push_back(idx);
  for (auto aa : item->armorAddons) {
    if (!aa->skinTextureSwapLists[isFemale ? 1 : 0]) aa->skinTextureSwapLists[isFemale ? 1 : 0] = group->texts;
  }
}

void Core::AddCategory(const std::string &parent, const std::string &name, const std::vector<std::string> &wildCards, const std::vector<RE::BGSKeyword *> &keywords,
                       const std::vector<RE::TESFaction *> &factions, const float chance) {
  for (auto &cat : categories) {
    if (cat.name == name) {
      SKSE::log::error("\tCategory [{}] already exists.", name);
      return;
    }
  }
  auto &newCat = categories.emplace_back();
  bool isMain = parent.empty();
  newCat.parent = parent;
  newCat.name = name;
  for (auto &wc : wildCards) newCat.wildCards.push_back(wc);
  for (auto &kw : keywords) newCat.keywords.push_back(kw);
  for (auto &fc : factions) newCat.factions.push_back(fc);
  newCat.chance = isMain ? 0.0f : chance;
  if (isMain && chance > 0.0f) SKSE::log::error("\tMain category [{}] cannot have a chance.", name);
  newCat.npcCount = 0;
  if (isMain) {
    SKSE::log::info("\tAdded main category [{}].", name);

  } else {
    SKSE::log::info("\tAdded sub-category [{}] under [{}].", name, parent);
  }
}

void Core::AddItemsToCategory(const std::string &name, const std::vector<SEFormLoc> &undies, const bool isFemale) {
  auto &undiesRef = isFemale ? femUndies : malUndies;
  for (auto &cat : categories) {
    if (cat.name == name) {
      for (auto &underwear : undies) {
        auto item = Util::LoadForm<RE::TESObjectARMO>(underwear);
        if (item && IsUnderwear(item)) {
          auto it = std::find(undiesRef.begin(), undiesRef.end(), item);
          if (it == undiesRef.end()) {
            SKSE::log::critical("\tThe item [0x{:x}] is not categorized properly in the underwear list.", item->formID);
            continue;
          }
          auto idx = std::distance(undiesRef.begin(), it);
          cat.undies[isFemale ? 1 : 0][idx] = true;
          categories[0].undies[isFemale ? 1 : 0][idx] = false;
          SKSE::log::info("\tAdded item [0x{:x}] to category [{}].", item->formID, name);
        } else {
          SKSE::log::error("\tThe item [0x{:x}] from file [{}] is not an underwear!", underwear.first, underwear.second);
        }
      }
      return;
    }
  }
  SKSE::log::error("\tCategory [{}] was not found.", name);
}

bool Core::GetItemStatus(const bool isFemale, const size_t cat, const size_t choice) {
  if (cat >= categories.size() || choice >= (isFemale ? femUndies.size() : malUndies.size())) return false;
  return categories[cat].undies[isFemale ? 1 : 0][choice];
}

void Core::SetItemStatus(const bool isFemale, const size_t cat, const size_t choice, const bool value) {
  if (cat >= categories.size() || choice >= (isFemale ? femUndies.size() : malUndies.size())) return;
  categories[cat].undies[isFemale ? 1 : 0][choice] = value;
}

std::vector<size_t> Core::GetItems(const bool isFemale, const size_t groupIdx, const size_t cat, const bool onlyActive) {
  if (groupIdx >= (isFemale ? femGroups.size() : malGroups.size()) || cat >= categories.size()) return {};
  auto meshGroup = &(isFemale ? femGroups : malGroups)[groupIdx];
  return GetItems(isFemale, meshGroup, cat, onlyActive);
}

std::vector<size_t> Core::GetItems(const bool isFemale, const NudeGroup *meshGroup, const size_t cat, const bool onlyActive) {
  if (!meshGroup || cat > categories.size()) return {};
  auto meshGroupItems = meshGroup->items;
  auto &catItems = categories[cat].undies[isFemale ? 1 : 0];
  std::vector<std::size_t> availableItems;
  for (auto &idx : meshGroupItems) {
    if (catItems.find(idx) != catItems.end() && (!onlyActive || catItems[idx])) availableItems.push_back(idx);
  }
  if (availableItems.size() == 0 && cat > 0) return GetItems(isFemale, meshGroup, 0, onlyActive);
  return availableItems;
}

RE::TESObjectARMO *Core::GetUnderwear(const bool isFemale, const NudeGroup *meshGroup, const size_t cat, const size_t choice, const bool onlyActive) {
  auto items = GetItems(isFemale, meshGroup, cat, onlyActive);
  return choice < items.size() ? malUndies[items[choice]] : nullptr;
}

Util::eRes Core::ProcessActor(RE::Actor *actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race || !IsValidRace(npc->race, true)) return Util::resFail;
  if (processedActors.find(actor) != processedActors.end()) return processedActors[actor];
  if (!npc->race->skin) {
    SKSE::log::error("The NPC [0x{:x}:{}] does not have a skin mesh.", npc->GetFormID(), npc->GetFormEditorID());
    processedActors[actor] = Util::resFail;
    return Util::resFail;
  }
  auto down = actor->GetWornArmor(Util::cSlot52, false);
  if (down && down != Util::TngCover()) {
    if (!IsUnderwear(down))
      SKSE::log::debug("The NPC [0x{:x}:{}] is wearing an item [0x{:x}] on slot 52! They won't get an underwear.", npc->GetFormID(), npc->GetFormEditorID(), down->formID);
    processedActors[actor] = Util::resOk;
    return Util::resOk;
  }
  auto fl = Util::FormList(Util::flExcluded);
  if (!fl) SKSE::log::critical("NUDE could not find the exclusion form list! Please make sure the plugin is enabled and the form list is not overridden");
  if (fl && fl->HasForm(actor)) {
    SKSE::log::debug("The NPC [0x{:x}:{}] is in the exclusion list. They won't get an underwear.", npc->GetFormID(), npc->GetFormEditorID());
    processedActors[actor] = Util::resOk;
    return Util::resOk;
  }
  processedActors[actor] = SetActorItem(actor, Util::cDef, false);
  return processedActors[actor];
}

RE::TESObjectARMO *Core::GetActorItem(RE::Actor *actor) {
  if (!actor) return nullptr;
  auto down = actor->GetWornArmor(Util::cSlot52, false);
  if (down && IsUnderwear(down)) return down;
  return nullptr;
}

Util::eRes Core::SetActorItem(RE::Actor *actor, const int itemIdx, const bool isUser) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Util::resFail;
  auto fl = Util::FormList(Util::flExcluded);
  if (!fl) SKSE::log::critical("NUDE could not find the exclusion form list! Please make sure the plugin is enabled and the form list is not overridden");
  if (itemIdx == Util::cNul) {
    if (auto underwear = GetActorItem(actor)) {
      RE::ActorEquipManager::GetSingleton()->UnequipObject(actor, underwear, nullptr, 1, nullptr, false, false, false, true);
      actor->RemoveItem(underwear, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    }
    if (fl) {
      fl->AddForm(actor);
      return Util::resOk;
    } else {
      return Util::resFail;
    }
  }
  if (fl && fl->HasForm(actor)) {
    for (RE::BSTArray<RE::TESForm *>::const_iterator it = fl->forms.begin(); it < fl->forms.end(); it++) {
      if ((*it)->formID == actor->formID) {
        fl->forms.erase(it);
        break;
      }
    }
  }
  auto npcCat = CategorizeNPC(npc);
  auto availableItems = GetItems(npc->IsFemale(), npcCat.first, npcCat.second, itemIdx == Util::cDef || !isUser);
  if (availableItems.size() == 0) {
    SKSE::log::warn("The NPC [0x{:x}:{}] does not have any underwear available to them.", npc->GetFormID(), npc->GetFormEditorID());
    return Util::resWarn;
  }
  auto idx = (itemIdx == Util::cDef) ? (npc->formID % availableItems.size()) : static_cast<size_t>(itemIdx);
  auto underwear = GetUnderwear(npc->IsFemale(), npcCat.first, npcCat.second, idx, true);
  if (!underwear) {
    SKSE::log::error("The NPC [0x{:x}:{}] could not find the underwear [{}] in the list.", npc->GetFormID(), npc->GetFormEditorID(), idx);
    return Util::resFail;
  }
  auto toWear = ForceUnderwear(actor, underwear);
  if (!toWear.first) {
    SKSE::log::error("The NPC [0x{:x}:{}] could not find the underwear [0x{:x}] in their inventory.", npc->GetFormID(), npc->GetFormEditorID(), underwear->formID);
    return Util::resFail;
  }

  SKSE::log::debug("The NPC [0x{:x}:{}] would wear the underwear [0x{:x}].", npc->GetFormID(), npc->GetFormEditorID(), toWear.first->formID);
  if (!toWear.second) RE::ActorEquipManager::GetSingleton()->EquipObject(actor, toWear.first);
  return Util::resOk;
}

std::pair<Core::NudeGroup *, size_t> Core::CategorizeNPC(RE::TESNPC *npc) {
  if (!npc) return {nullptr, 0};
  auto meshGroup = GetSkinRaceMesh(npc->race, npc->skin ? npc->skin : npc->race->skin, npc->IsFemale());
  if (!meshGroup) return {nullptr, 0};
  size_t cat = 0;  // TODO make the category to match the right categories
  return std::make_pair(meshGroup, cat);
}

std::pair<RE::TESBoundObject *, bool> Core::ForceUnderwear(RE::Actor *actor, RE::TESObjectARMO *underwear, const bool ifUpdate) {
  auto inv = actor->GetInventory();
  for (const auto &[item, invData] : inv) {
    if (!item || !IsUnderwear(item)) continue;
    const auto &[count, entry] = invData;
    if (count >= 1 && entry) {
      return {item, entry->IsWorn()};
    }
  }
  if (ifUpdate) return {nullptr, false};
  // SKSE::log::debug("Adding underwear [0x{:x}] to the inventory of the actor [0x{:x}:{}].", underwear->formID, actor->GetFormID(), actor->GetFormEditorID());
  actor->AddObjectToContainer(underwear, nullptr, 1, nullptr);
  return ForceUnderwear(actor, underwear, true);
}

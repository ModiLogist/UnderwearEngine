#include <Core.h>

Core *core = Core::GetSingleton();

bool Core::LoadRaces() {
  eq = RE::ActorEquipManager::GetSingleton();
  relevantKeys[0].clear();
  relevantKeys[1].clear();
  if (!eq) {
    SKSE::log::critical("Could not get the ActorEquipManager singleton!");
    return false;
  }
  SKSE::log::info("Processing races...");
  const auto &allRaces = Util::SEDH()->GetFormArray<RE::TESRace>();
  for (const auto &race : allRaces) {
    if (!IsValidRace(race, false)) continue;
    GetSkinRaceMesh(race, race->skin, false);
    GetSkinRaceMesh(race, race->skin, true);
  }
  SKSE::log::info("Finished processing races and found [{}] mesh groups for men, and [{}] mesh groups for women among them.", malGroups.size(), femGroups.size());
  return true;
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
  if (!race || !skin) {
    SKSE::log::critical("\tSomething triggered nude to check a skin mesh for race:[0x{:x}] and skin:[0x{:x}]", race ? race->GetFormID() : 0x0, skin ? skin->GetFormID() : 0x0);
    return nullptr;
  }
  std::string mesh{""};
  for (auto &aa : skin->armorAddons) {
    if (aa->IsValidRace(race) && aa->HasPartOf(Util::cSlot32)) {
      mesh = std::string(aa->bipedModels[isFemale ? 1 : 0].model.data());
    }
  }
  if (mesh.empty()) return nullptr;
  auto &groups = isFemale ? femGroups : malGroups;
  auto it = std::find_if(groups.begin(), groups.end(), [&](const auto &group) { return group.mesh == mesh; });
  if (it == groups.end()) {
    auto &newGroup = groups.emplace_back();
    newGroup.mesh = mesh;
    SKSE::log::debug("\tAdded mesh [{}] to {} meshes.", mesh, isFemale ? "women" : "men");
    return &newGroup;
  }
  return &(*it);
}

void Core::LoadItems() {
  categories.clear();
  SKSE::log::info("Processing items...");
  AddCategory("", "default", {"*"}, {}, {});
  const auto &armorList = Util::SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto &armor : armorList) {
    if (armor->HasKeyword(Util::Key(Util::kyTngUw))) {
      allUndies.insert(armor->GetFormID());
      if (armor->HasKeyword(Util::Key(Util::kyItemFake))) fakeUndies.insert(armor);
      if (armor->armorAddons.size() == 0) continue;
      if (armor->HasKeyword(Util::Key(Util::kyItemM))) ProcessItem(armor, false);
      if (armor->HasKeyword(Util::Key(Util::kyItemF))) ProcessItem(armor, true);
    }
  }
  ProcessFakes(malUndies);
  ProcessFakes(femUndies);
  SKSE::log::info("Finished processing items and found [{}] undies for men and [{}] undies for women.", malUndies.size(), femUndies.size());
}

std::vector<std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *>>::iterator Core::FindPair(RE::TESObjectARMO *item, const bool isFemale, const bool isFake) {
  auto &undies = isFemale ? femUndies : malUndies;
  auto it = std::find_if(undies.begin(), undies.end(), [&](const auto &pair) {
    auto toCompare = isFake ? pair.second : pair.first;
    return toCompare->GetFormID() == item->GetFormID();
  });
  return it;
}

void Core::ProcessItem(RE::TESObjectARMO *item, const bool isFemale) {
  auto &allList = isFemale ? femUndies : malUndies;
  auto &itemRace = item->armorAddons[0]->race;
  if (itemRace == Util::Race(Util::raceDefault) && item->armorAddons[0]->additionalRaces.size() > 0) itemRace = item->armorAddons[0]->additionalRaces[0];
  if (!IsValidRace(itemRace)) {
    SKSE::log::debug("\tThe item[0x{:x}:{}]'s race[{}] is not a  valid race.", item->GetFormID(), item->GetFormEditorID(), itemRace ? itemRace->GetFormEditorID() : "null");
    return;
  }
  auto group = GetSkinRaceMesh(itemRace, itemRace->skin, isFemale);
  if (!group) {
    SKSE::log::debug("\tThe item[0x{:x}:{}] is not categorized properly in the underwear list.", item->GetFormID(), item->GetFormEditorID());
    return;
  }
  auto idx = allList.size();
  allList.push_back({item, nullptr});
  categories[0].undies[isFemale ? 1 : 0][idx] = true;
  group->items.push_back(idx);
}

void Core::ProcessFakes(std::vector<std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *>> &undies) {
  for (auto it = undies.begin(); it != undies.end(); ++it) {
    if (fakeUndies.empty()) {
      SKSE::log::error("\tThe item [0x{:x}] cannot be distributed.", (*it).first->GetFormID());
      undies.erase(it);
      continue;
    }
    auto item = (*it).first;
    auto fakeCopy = *fakeUndies.begin();
    fakeCopy->SetFullName(item->GetFullName());
    fakeCopy->value = item->value;
    (*it).second = fakeCopy;
    fakeUndies.erase(fakeUndies.begin());
  }
  SKSE::log::info("\tProcessed item relations.");
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
          auto itemPair = FindPair(item, isFemale);
          if (itemPair == undiesRef.end()) {
            SKSE::log::critical("\tThe item [0x{:x}] is not categorized properly in the underwear list.", item->GetFormID());
            continue;
          }
          auto idx = std::distance(undiesRef.begin(), itemPair);
          cat.undies[isFemale ? 1 : 0][idx] = true;
          categories[0].undies[isFemale ? 1 : 0][idx] = false;
          SKSE::log::info("\tAdded item [0x{:x}] to category [{}].", item->GetFormID(), name);
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

std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *> *Core::GetItem(const bool isFemale, const NudeGroup *meshGroup, const size_t cat, const size_t choice, const bool onlyActive) {
  auto items = GetItems(isFemale, meshGroup, cat, onlyActive);
  if (choice >= items.size()) return nullptr;
  return isFemale ? &femUndies[items[choice]] : &malUndies[items[choice]];
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
  if (down && !IsUnderwear(down) && Util::FormToLocView(down) != Util::coverID) {
    SKSE::log::debug("The NPC [0x{:x}] is wearing an item [0x{:x}] on slot 52! They won't get an underwear.", npc->GetFormID(), down->GetFormID());
    processedActors[actor] = Util::resOk;
    return Util::resOk;
  }
  auto up = actor->GetWornArmor(Util::cSlot32, false);
  if (up && !IsCovering(actor, up)) {
    SKSE::log::debug("The NPC [0x{:x}] is wearing a revealing item [0x{:x}] on slot 32! They won't get an underwear.", npc->GetFormID(), up->GetFormID());
    processedActors[actor] = Util::resOk;
    return Util::resOk;
  }
  if (IsExcluded(actor)) {
    processedActors[actor] = SetActorItem(actor, Util::cNul);
    SKSE::log::debug("The NPC [0x{:x}] is in the exclusion list. They won't get an underwear and was processed [{}]", npc->GetFormID(), processedActors[actor] == Util::resOk);
    return processedActors[actor];
  }
  processedActors[actor] = SetActorItem(actor, Util::cDef);
  return processedActors[actor];
}

Util::eRes Core::ProcessPlayer(RE::Actor *actor) {
  processingPlayer = true;
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race || !IsValidRace(npc->race, true)) {
    processingPlayer = false;
    return Util::resFail;
  }
  if (processedActors.find(actor) != processedActors.end()) {
    processingPlayer = false;
    return processedActors[actor];
  }
  if (!npc->race->skin) {
    SKSE::log::error("The player does not have a skin mesh!");
    processedActors[actor] = Util::resFail;
    processingPlayer = false;
    return Util::resFail;
  }
  auto inv = actor->GetInventory([=](RE::TESBoundObject &obj) { return IsUnderwear(&obj); });
  UpdateActorItems(actor, inv);
  auto up = actor->GetWornArmor(Util::cSlot32);
  if (up && IsUnderwear(up)) {
    auto pair = GetOther(up, npc->IsFemale());
    if (pair.first) eq->EquipObject(actor, pair.second, nullptr, 1, nullptr, false, false, false, true);
  } else {
    if (auto pcUndies = Util::FormList(Util::flPCUndies)) {
      auto undies = pcUndies->forms.size() > 0 && pcUndies->forms[0] ? pcUndies->forms[0]->As<RE::TESObjectARMO>() : nullptr;
      if (undies && IsUnderwear(undies) && !undies->HasKeyword(Util::Key(Util::kyItemFake))) {
        auto pair = GetOther(undies, npc->IsFemale());
        if (!pair.first) {
          SKSE::log::critical("NUDE could not restore the player's underwear. They would go commando!");
          processedActors[actor] = Util::resOk;
        }
        auto down = actor->GetWornArmor(Util::cSlot52);
        if (!down || IsUnderwear(down)) eq->EquipObject(actor, pair.second, nullptr, 1, nullptr, false, false, false, true);
      }
    }
  }
  processedActors[actor] = Util::resOk;
  processingPlayer = false;
  return Util::resOk;
}

std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *> *Core::GetActorItem(RE::Actor *actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || IsExcluded(actor)) return nullptr;
  RE::TESObjectARMO *undies = nullptr;
  auto inv = actor->GetInventory([=](RE::TESBoundObject &obj) { return IsUnderwear(&obj); });
  if (processedActors.find(actor) == processedActors.end()) UpdateActorItems(actor, inv);
  for (const auto &[item, invData] : inv) {
    if (invData.first > 0) {
      auto armo = item->As<RE::TESObjectARMO>();
      if (!armo) continue;
      if (armo->HasKeyword(Util::Key(Util::kyItemFake))) continue;
      if (actor->IsDead() && invData.second && invData.second->IsWorn()) {
        undies = armo;
        break;
      }
      if (!undies || armo->value > undies->value) undies = armo;
    }
  }
  if (!undies) return nullptr;
  auto res = FindPair(undies, npc->IsFemale());
  if (res == (npc->IsFemale() ? femUndies.end() : malUndies.end())) {
    SKSE::log::critical("Something went very wrong with the underwear items for the NPC [0x{:x}:{}].", npc->GetFormID(), npc->GetFormEditorID());
    actor->RemoveItem(undies, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
    return nullptr;
  }
  return &(*res);
}

Util::eRes Core::SetActorItem(RE::Actor *actor, const int itemIdx, const bool isUser) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return Util::resFail;
  auto uPair = GetActorItem(actor);
  if (itemIdx == Util::cNul) {
    while (uPair != nullptr) {
      if (actor->GetWornArmor(uPair->first->GetFormID())) eq->UnequipObject(actor, uPair->first, nullptr, 1, nullptr, false, false, false, true);
      if (actor->GetWornArmor(uPair->second->GetFormID())) eq->UnequipObject(actor, uPair->second, nullptr, 1, nullptr, false, false, false, true);
      actor->RemoveItem(uPair->first, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
      actor->RemoveItem(uPair->second, 1, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
      uPair = GetActorItem(actor);
    }
    return TryExclude(actor, true);
  }
  TryExclude(actor, false);
  auto npcCat = CategorizeNPC(npc);
  if (!npcCat.first) {
    SKSE::log::error("CategorizeNPC returned null for NPC [0x{:x}:{}].", npc->GetFormID(), npc->GetFormEditorID());
    return Util::resFail;
  }
  if (!uPair) {
    auto availableItems = GetItems(npc->IsFemale(), npcCat.first, npcCat.second, itemIdx == Util::cDef || !isUser);
    if (availableItems.size() == 0) {
      SKSE::log::warn("The NPC [0x{:x}:{}] does not have any underwear available to them.", npc->GetFormID(), npc->GetFormEditorID());
      return Util::resWarn;
    }
    auto idx = (itemIdx == Util::cDef) ? (npc->GetFormID() % availableItems.size()) : static_cast<size_t>(itemIdx);
    auto newPair = GetItem(npc->IsFemale(), npcCat.first, npcCat.second, idx, true);
    if (!newPair) {
      SKSE::log::error("Could not find the underwear for the NPC [0x{:x}:{}] in the list.", npc->GetFormID(), npc->GetFormEditorID());
      return Util::resFail;
    }
    actor->AddObjectToContainer(newPair->second, nullptr, 1, nullptr);
    actor->AddObjectToContainer(newPair->first, nullptr, 1, nullptr);
    uPair = newPair;
  }
  eq->EquipObject(actor, uPair->second, nullptr, 1, nullptr, false, false, false, false);
  if (!HasCover(actor)) eq->EquipObject(actor, uPair->first, nullptr, 1, nullptr, false, false, false, true);
  return Util::resOk;
}

bool Core::IsExcluded(RE::Actor *actor) {
  auto fl = Util::FormList(Util::flExcluded);
  if (!fl) {
    if (!loggedExcluded) SKSE::log::critical("NUDE could not find the exclusion form list! Please make sure the plugin is enabled and the form list is not overridden");
    loggedExcluded = true;
    return false;
  }
  return fl->HasForm(actor);
}

Util::eRes Core::TryExclude(RE::Actor *actor, const bool toExclude) {
  auto fl = Util::FormList(Util::flExcluded);
  if (!fl) {
    if (!loggedExcluded) SKSE::log::critical("NUDE could not find the exclusion form list! Please make sure the plugin is enabled and the form list is not overridden");
    loggedExcluded = true;
    return Util::resFail;
  }
  if (toExclude && !fl->HasForm(actor)) fl->AddForm(actor);
  if (!toExclude && fl->HasForm(actor)) {
    for (RE::BSTArray<RE::TESForm *>::const_iterator it = fl->forms.begin(); it < fl->forms.end(); it++) {
      if ((*it)->GetFormID() == actor->GetFormID()) {
        fl->forms.erase(it);
        break;
      }
    }
  }
  return Util::resOk;
}

std::pair<Core::NudeGroup *, size_t> Core::CategorizeNPC(RE::TESNPC *npc) {
  if (!npc) return {nullptr, 0};
  auto meshGroup = GetSkinRaceMesh(npc->race, npc->skin ? npc->skin : npc->race->skin, npc->IsFemale());
  if (!meshGroup) return {nullptr, 0};
  size_t cat = 0;  // TODO make the category to match the right categories
  return std::make_pair(meshGroup, cat);
}

void Core::UpdateActorItems(RE::Actor *actor, RE::TESObjectREFR::InventoryItemMap &inv) {
  std::unordered_map<RE::TESObjectARMO *, int32_t> toRemove{};
  std::unordered_set<RE::TESObjectARMO *> toAdd{};
  for (const auto &[item, invData] : inv) {
    if (invData.first > 0) {
      auto armo = item->As<RE::TESObjectARMO>();
      if (!armo) continue;
      if (armo->HasKeyword(Util::Key(Util::kyItemFake))) {
        toRemove[armo] = invData.first;
        continue;
      }
      auto pair = GetOther(armo, actor->GetActorBase()->IsFemale());
      if (pair.first) toAdd.insert(pair.second);
    }
  }
  for (const auto &item : toRemove) {
    if (toAdd.find(item.first) != toAdd.end()) {
      toRemove[item.first]--;
      toAdd.erase(item.first);
    }
    if (toRemove[item.first] > 0) actor->RemoveItem(item.first, item.second, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
  }
  for (const auto &item : toAdd) {
    actor->AddObjectToContainer(item, nullptr, 1, nullptr);
  }
}

bool Core::IsCovering(RE::Actor *actor, RE::TESObjectARMO *armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (IsUnderwear(armor) || Util::FormToLocView(armor) == Util::coverID) return false;
  if (relevantKeys[0].empty() || relevantKeys[1].empty()) {
    relevantKeys[0].push_back(Util::Key(Util::kyTngCovering));
    relevantKeys[0].push_back(Util::Key(Util::kyTngMalCovering));
    relevantKeys[1].push_back(Util::Key(Util::kyTngCovering));
    relevantKeys[1].push_back(Util::Key(Util::kyTngFemCovering));
  }
  return armor->HasKeywordInArray(relevantKeys[npc->IsFemale() ? 1 : 0], false);
}

bool Core::HasCover(RE::Actor *actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return false;
  auto inv = actor->GetInventory([&](RE::TESBoundObject &obj) { return IsCovering(actor, obj.As<RE::TESObjectARMO>()); });
  for (const auto &[item, invData] : inv) {
    const auto &[count, entry] = invData;
    if (entry->IsWorn()) return true;
  }
  return false;
}

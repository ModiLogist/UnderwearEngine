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
  const auto &allRaces = ut->SEDH()->GetFormArray<RE::TESRace>();
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
  auto &allNPCs = ut->SEDH()->GetFormArray<RE::TESNPC>();
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
  SKSE::log::info("Finished processing NPCs.");
}

bool Core::IsValidRace(RE::TESRace *race, bool postLoad) {
  if (!race || !race->skin || !race->HasKeyword(ut->Key(ut->kyManMer)) || race->HasKeyword(ut->Key(ut->kyCreature)) || race->IsChildRace()) return false;
  if (postLoad && race->HasKeyword(ut->Key(ut->kyTngIgnored))) return false;
  if (postLoad && !(race->HasKeyword(ut->Key(ut->kyTngProcessed)) || race->HasKeyword(ut->Key(ut->kyTngReady)))) return false;
  return true;
}

Core::NudeGroup *Core::GetSkinRaceMesh(RE::TESRace *race, RE::TESObjectARMO *skin, const bool isFemale) {
  if (!race || !skin) {
    SKSE::log::critical("\tSomething triggered nude to check a skin mesh for race:[0x{:x}] and skin:[0x{:x}]", race ? race->GetFormID() : 0x0, skin ? skin->GetFormID() : 0x0);
    return nullptr;
  }
  std::string mesh{""};
  for (auto &aa : skin->armorAddons) {
    if (aa->IsValidRace(race) && aa->HasPartOf(ut->cSlot32)) {
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
  AddCategory("", "default", {"*"}, {}, {}, {}, {}, {}, RE::SEX::kTotal, 0.0f);
  const auto &armorList = ut->SEDH()->GetFormArray<RE::TESObjectARMO>();
  for (const auto &armor : armorList) {
    if (armor->HasKeyword(ut->Key(ut->kyTngUw))) {
      allUndies.insert(armor->GetFormID());
      if (armor->HasKeyword(ut->Key(ut->kyItemFake))) fakeUndies.insert(armor);
      if (armor->armorAddons.size() == 0) continue;
      if (armor->HasKeyword(ut->Key(ut->kyItemM))) ProcessItem(armor, false);
      if (armor->HasKeyword(ut->Key(ut->kyItemF))) ProcessItem(armor, true);
    }
  }
  ProcessFakes(false);
  ProcessFakes(true);
  SKSE::log::info("\tProcessed item relations.");
  SKSE::log::info("Finished processing items and found [{}] undies for men and [{}] undies for women.", malUndies.size(), femUndies.size());
}

RE::TESObjectARMO *Core::GetOther(RE::TESObjectARMO *undies, const RE::SEX sex, const bool isFake) {
  bool isFemale;
  switch (sex) {
    case RE::SEX::kMale:
      isFemale = false;
      break;
    case RE::SEX::kFemale:
      isFemale = true;
      break;
    default:
      auto idx = GetIdx(undies->formID, isFake);
      if (idx.first == RE::SEX::kNone) {
        return nullptr;
      } else {
        return GetOther(undies, idx.first, isFake);
      }
      break;
  }
  if (!undies) return nullptr;
  auto it = FindPair(undies->formID, isFemale, isFake);
  if (it == (isFemale ? femUndies : malUndies).end()) return nullptr;
  return isFake ? it->first : it->second;
}

void Core::ProcessItem(RE::TESObjectARMO *item, const bool isFemale) {
  auto &allList = isFemale ? femUndies : malUndies;
  auto &itemRace = item->armorAddons[0]->race;
  if (itemRace == ut->Race(ut->raceDefault) && item->armorAddons[0]->additionalRaces.size() > 0) itemRace = item->armorAddons[0]->additionalRaces[0];
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

void Core::ProcessFakes(const bool isFemale) {
  auto &undies = isFemale ? femUndies : malUndies;
  for (auto it = undies.begin(); it != undies.end(); ++it) {
    if (fakeUndies.empty()) {
      SKSE::log::error("\tThe item [0x{:x}] cannot be distributed.", (*it).first->GetFormID());
      undies.erase(it);
      continue;
    }
    auto item = (*it).first;
    if (isFemale && item->HasKeyword(ut->Key(ut->kyItemM))) {
      auto pair = FindPair(item->formID, false);
      (*it).second = (*pair).second;
      SKSE::log::trace("\tThe item [{:x}] is designed for both men and women.", item->GetFormID());
      continue;
    }
    auto fakeCopy = *fakeUndies.begin();
    fakeCopy->SetFullName(item->GetFullName());
    fakeCopy->value = item->value;
    (*it).second = fakeCopy;
    fakeUndies.erase(fakeUndies.begin());
  }
}

std::vector<std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *>>::iterator Core::FindPair(const RE::FormID &formID, const bool isFemale, const bool isFake) {
  auto &undies = isFemale ? femUndies : malUndies;
  auto it = std::find_if(undies.begin(), undies.end(), [&](const auto &pair) {
    auto toCompare = isFake ? pair.second : pair.first;
    return toCompare->GetFormID() == formID;
  });
  return it;
}

std::vector<std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *>>::iterator Core::FindPair(const SEFormLoc &formLoc, const bool isFemale, const bool isFake) {
  auto &undies = isFemale ? femUndies : malUndies;
  auto it = std::find_if(undies.begin(), undies.end(), [&](const auto &pair) {
    auto toCompare = isFake ? pair.second : pair.first;
    return ut->FormToLoc(toCompare) == formLoc;
  });
  return it;
}

std::pair<RE::SEX, size_t> Core::GetIdx(const RE::FormID &formID, const bool isFake) {
  auto it = FindPair(formID, true, isFake);
  if (it != femUndies.end()) return {RE::SEX::kFemale, std::distance(femUndies.begin(), it)};
  it = FindPair(formID, false, isFake);
  if (it != malUndies.end()) return {RE::SEX::kMale, std::distance(malUndies.begin(), it)};
  return {RE::SEX::kNone, 0};
}

std::pair<RE::SEX, size_t> Core::GetIdx(const SEFormLoc &formLoc, const bool isFake) {
  auto it = FindPair(formLoc, true, isFake);
  if (it != femUndies.end()) return {RE::SEX::kFemale, std::distance(femUndies.begin(), it)};
  it = FindPair(formLoc, false, isFake);
  if (it != malUndies.end()) return {RE::SEX::kMale, std::distance(malUndies.begin(), it)};
  return {RE::SEX::kNone, 0};
}

void Core::AddCategory(const std::string &parent, const std::string &name, const std::vector<std::string> &wildCards, const std::vector<RE::BGSKeyword *> &keywords,
                       const std::vector<RE::TESFaction *> &factions, const std::vector<RE::TESRace *> &races, const std::vector<RE::TESNPC *> &npcs,
                       const std::set<SEFormLoc> &undies, RE::SEX sex, const float chance) {
  auto &catGroup = categories;
  if (!parent.empty()) {
    auto it = std::find_if(categories.begin(), categories.end(), [&](const auto &cat) { return cat.name == parent; });
    if (it == categories.end()) {
      SKSE::log::error("\tParent category [{}] could not be found.", parent);
      return;
    }
    catGroup = (*it).children;
  }
  for (auto &cat : catGroup) {
    if (cat.name == name) {
      SKSE::log::error("\tCategory [{}] already exists in scope.", name);
      return;
    }
  }
  auto &newCat = catGroup.emplace_back();
  bool isMain = parent.empty();
  newCat.parent = parent;
  newCat.name = name;
  for (auto &wc : wildCards) newCat.wildCards.push_back(wc);
  for (auto &kw : keywords) newCat.keywords.push_back(kw);
  for (auto &fc : factions) newCat.factions.push_back(fc);
  for (auto &rc : races) newCat.races.push_back(rc);
  for (auto &npc : npcs) newCat.npcs.push_back(npc);
  newCat.sex = sex;
  newCat.chance = isMain ? 0.0f : chance;
  newCat.npcCount = 0;
  AddItemsToCategory(parent, name, undies);
}

void Core::AddItemsToCategory(const std::string &parent, const std::string &name, const std::set<SEFormLoc> &undies) {
  auto catGroup = categories;
  if (!parent.empty()) {
    auto itP = std::find_if(categories.begin(), categories.end(), [&](const auto &cat) { return cat.name == parent; });
    if (itP == categories.end()) {
      SKSE::log::error("\tParent category [{}] could not be found.", parent);
      return;
    }
    catGroup = (*itP).children;
  }
  auto itC = std::find_if(catGroup.begin(), catGroup.end(), [&](const auto &cat) { return cat.name == name; });
  if (itC == catGroup.end()) {
    SKSE::log::error("\tCategory [{}] was not found.", name);
    return;
  }
  auto &cat = *itC;
  auto AddItemsByGender = [&](const bool isFemale) {
    auto &undiesList = isFemale ? femUndies : malUndies;
    for (auto itU = undiesList.begin(); itU != undiesList.end(); ++itU) {
      cat.undies[isFemale ? 1 : 0][std::distance(undiesList.begin(), itU)] = undies.find(ut->FormToLoc((*itU).first)) != undies.end();
    }
  };
  AddItemsByGender(false);
  AddItemsByGender(true);
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

void Core::ProcessPlayer(RE::Actor *actor, RE::TESObjectARMO *armor, const bool equipped) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  auto &undiesList = npc->IsFemale() ? femUndies : malUndies;
  auto undies = armor ? FindPair(armor->formID, npc->IsFemale(), true) : undiesList.end();
  auto pcUndies = ut->FormList(ut->flPCUndies);
  auto inv = actor->GetInventory([=](RE::TESBoundObject &obj) { return IsUnderwear(&obj); });
  UpdateActorItems(actor, inv);
  if (!armor) {
    auto up = actor->GetWornArmor(ut->cSlot32);
    auto down = actor->GetWornArmor(ut->cSlot52);
    if (up && IsUnderwear(up)) {
      undies = FindPair(up->formID, npc->IsFemale());
    } else if (down && IsUnderwear(down)) {
      undies = FindPair(down->formID, npc->IsFemale(), true);
    } else {
      if (pcUndies) {
        auto vis = pcUndies->forms.size() > 0 && pcUndies->forms[0] ? pcUndies->forms[0]->As<RE::TESObjectARMO>() : nullptr;
        if (vis && IsUnderwear(vis) && !vis->HasKeyword(ut->Key(ut->kyItemFake))) undies = FindPair(vis->formID, npc->IsFemale());
      }
    }
  }
  if (undies != undiesList.end() && equipped) {
    if (pcUndies && !pcUndies->HasForm(undies->first)) {
      pcUndies->scriptAddedTempForms->erase(pcUndies->scriptAddedTempForms->begin());
      pcUndies->scriptAddedFormCount--;
      pcUndies->AddForm(undies->first);
    }
    WearUndies(actor, &(*undies), armor);
  } else {
    TakeOffUndies(actor, !equipped);
    if (pcUndies && pcUndies->HasForm(undies->first)) {
      pcUndies->scriptAddedTempForms->erase(pcUndies->scriptAddedTempForms->begin());
      pcUndies->scriptAddedFormCount--;
    }
  }
}

void Core::ProcessActor(RE::Actor *actor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !npc->race || !IsValidRace(npc->race, true)) return;
  if (IsProcessed(actor)) return;
  if (!ShouldHave(actor)) {
    SKSE::log::trace("The actor [0x{:x}] should not have underwear.", actor->GetFormID());
    return;
  }
  if (auto undies = GetActorItem(actor)) {
    WearUndies(actor, undies);
    return;
  }
  SetActorItem(actor, ut->cDef);
}

bool Core::ShouldHave(RE::Actor *actor) {
  if (IsExcluded(actor)) return false;
  auto down = actor->GetWornArmor(ut->cSlot52, false);
  if (down && ut->IsBaseCover(down) && !IsUnderwear(down)) return false;
  auto up = actor->GetWornArmor(ut->cSlot32, false);
  if (up && !IsUnderwear(up) && !IsCovering(actor, up)) return false;
  return true;
}

std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *> *Core::GetActorItem(RE::Actor *actor, const bool noUpdate) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || IsExcluded(actor)) return nullptr;
  RE::TESObjectARMO *vis = nullptr;
  auto inv = actor->GetInventory([=](RE::TESBoundObject &obj) { return IsUnderwear(&obj); });
  auto undies = npc->IsFemale() ? femUndies.end() : malUndies.end();
  if (!IsProcessed(actor) && !noUpdate) UpdateActorItems(actor, inv);
  for (const auto &[item, invData] : inv) {
    if (invData.first > 0) {
      auto armo = item->As<RE::TESObjectARMO>();
      if (!armo) continue;
      if (armo->HasKeyword(ut->Key(ut->kyItemFake))) continue;
      undies = FindPair(armo->formID, npc->IsFemale());
      if (undies == (npc->IsFemale() ? femUndies.end() : malUndies.end())) continue;
      if (actor->IsDead() && invData.second && invData.second->IsWorn()) {
        vis = armo;
        break;
      }
      if (!vis || armo->value > vis->value) vis = armo;
    }
  }
  if (!vis) return nullptr;
  if (inv.find(undies->first) == inv.end() || inv.find(undies->second) == inv.end()) {
    return nullptr;
  }
  return &(*undies);
}

void Core::SetActorItem(RE::Actor *actor, const int itemIdx, const bool isUser) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return;
  if (itemIdx == ut->cNul) {
    TakeOffUndies(actor);
    TryExclude(actor, true);
    if (isUser) {
      auto inv = actor->GetInventory([=](RE::TESBoundObject &obj) { return IsUnderwear(&obj); });
      for (const auto &[item, invData] : inv) {
        if (item && invData.first > 0) actor->RemoveItem(item, invData.first, RE::ITEM_REMOVE_REASON::kRemove, nullptr, nullptr);
      }
    }
    return;
  }
  TryExclude(actor, false);
  auto npcCat = CategorizeNPC(npc, isUser);
  if (!npcCat.first) {
    SKSE::log::error("CategorizeNPC returned null for NPC [0x{:x}:{}].", npc->GetFormID(), npc->GetFormEditorID());
    return;
  }
  auto availableItems = GetItems(npc->IsFemale(), npcCat.first, npcCat.second, itemIdx == ut->cDef || !isUser);
  if (availableItems.size() == 0) {
    SKSE::log::warn("The NPC [0x{:x}:{}] does not have any underwear available to them.", npc->GetFormID(), npc->GetFormEditorID());
    return;
  }
  auto idx = (itemIdx == ut->cDef) ? (npc->GetFormID() % availableItems.size()) : static_cast<size_t>(itemIdx);
  auto newUndies = GetItem(npc->IsFemale(), npcCat.first, npcCat.second, idx, true);
  if (!newUndies) {
    SKSE::log::error("Could not find the underwear for the NPC [0x{:x}:{}] in the list.", npc->GetFormID(), npc->GetFormEditorID());
    return;
  }
  actor->AddObjectToContainer(newUndies->second, nullptr, 1, nullptr);
  actor->AddObjectToContainer(newUndies->first, nullptr, 1, nullptr);
  WearUndies(actor, newUndies);
  return;
}

bool Core::IsExcluded(RE::Actor *actor) {
  auto fl = ut->FormList(ut->flExcluded);
  if (!fl) {
    if (!loggedExcluded) SKSE::log::critical("NUDE could not find the exclusion form list! Please make sure the plugin is enabled and the form list is not overridden");
    loggedExcluded = true;
    return false;
  }
  return fl->HasForm(actor);
}

void Core::TryExclude(RE::Actor *actor, const bool toExclude) {
  auto fl = ut->FormList(ut->flExcluded);
  if (!fl) {
    if (!loggedExcluded) SKSE::log::critical("NUDE could not find the exclusion form list! Please make sure the plugin is enabled and the form list is not overridden");
    loggedExcluded = true;
    return;
  }
  if (toExclude && !fl->HasForm(actor)) {
    SKSE::log::trace("Adding actor [0x{:x}] to the exclusion list.", actor->GetFormID());
    fl->AddForm(actor);
  }
  if (!toExclude && fl->HasForm(actor)) {
    SKSE::log::trace("Removing actor [0x{:x}] from the exclusion list from formlist with [{}] forms!", actor->GetFormID(), fl->scriptAddedTempForms->size());
    for (RE::BSTArray<RE::FormID>::const_iterator it = fl->scriptAddedTempForms->begin(); it < fl->scriptAddedTempForms->end(); it++) {
      if (*it == actor->formID) {
        fl->scriptAddedTempForms->erase(it);
        fl->scriptAddedFormCount--;
        SKSE::log::trace("Removed actor [0x{:x}] from the exclusion list.", actor->GetFormID());
        break;
      }
    }
  }
  return;
}

std::pair<Core::NudeGroup *, size_t> Core::CategorizeNPC(RE::TESNPC *npc, const bool isUser) {
  if (!npc) return {nullptr, 0};
  auto meshGroup = GetSkinRaceMesh(npc->race, npc->skin ? npc->skin : npc->race->skin, npc->IsFemale());
  if (!meshGroup) return {nullptr, 0};
  size_t cat = isUser ? 0 : 0;  // TODO make the category to match the right categories
  return std::make_pair(meshGroup, cat);
}

void Core::UpdateActorItems(RE::Actor *actor, RE::TESObjectREFR::InventoryItemMap &inv) {
  std::unordered_map<RE::TESObjectARMO *, int32_t> toRemove{};
  std::unordered_set<RE::TESObjectARMO *> toAdd{};
  for (const auto &[item, invData] : inv) {
    if (invData.first > 0) {
      auto armo = item->As<RE::TESObjectARMO>();
      if (!armo) continue;
      if (armo->HasKeyword(ut->Key(ut->kyItemFake))) {
        toRemove[armo] = invData.first;
        continue;
      }
      auto pair = GetOther(armo, actor->GetActorBase()->GetSex());
      if (pair) toAdd.insert(pair);
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

void Core::WearUndies(RE::Actor *actor, std::pair<RE::TESObjectARMO *, RE::TESObjectARMO *> *undies, RE::TESObjectARMO *except) {
  if (!actor || !undies || !ShouldHave(actor)) return;
  if (!except) {
    auto down = actor->GetWornArmor(ut->cSlot52);
    if (!down || IsUnderwear(down) || ut->IsBaseCover(down)) {
      if (!down || down->formID != undies->second->formID) eq->EquipObject(actor, undies->second, nullptr, 1, nullptr, false, false, false, true);
    } else {
      return;
    }
  }
  if (HasCover(actor, except)) return;
  auto up = actor->GetWornArmor(ut->cSlot32);
  if ((!up || up->GetFormID() == except->GetFormID())) {
    if (!up || up->formID != undies->first->formID) eq->EquipObject(actor, undies->first, nullptr, 1, nullptr, false, false, false, true);
  }
}

void Core::TakeOffUndies(RE::Actor *actor, const bool justUp) {
  if (IsUnderwear(actor->GetWornArmor(ut->cSlot32))) eq->UnequipObject(actor, actor->GetWornArmor(ut->cSlot32), nullptr, 1, nullptr, false, false, false, true);
  if (justUp) return;
  if (IsUnderwear(actor->GetWornArmor(ut->cSlot52))) eq->UnequipObject(actor, actor->GetWornArmor(ut->cSlot52), nullptr, 1, nullptr, false, false, false, true);
}

bool Core::IsCovering(RE::Actor *actor, RE::TESObjectARMO *armor) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return false;
  if (relevantKeys[0].empty() || relevantKeys[1].empty()) {
    relevantKeys[0].push_back(ut->Key(ut->kyTngCovering));
    relevantKeys[0].push_back(ut->Key(ut->kyTngMalCovering));
    relevantKeys[1].push_back(ut->Key(ut->kyTngCovering));
    relevantKeys[1].push_back(ut->Key(ut->kyTngFemCovering));
  }
  return armor->HasKeywordInArray(relevantKeys[npc->IsFemale() ? 1 : 0], false);
}

bool Core::HasCover(RE::Actor *actor, RE::TESObjectARMO *except) {
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc) return true;
  RE::FormID compFormID = except ? except->GetFormID() : 0;
  auto inv = actor->GetInventory([&](RE::TESBoundObject &obj) { return IsCovering(actor, obj.As<RE::TESObjectARMO>()); });
  for (const auto &[item, invData] : inv) {
    const auto &[count, entry] = invData;
    if (entry->IsWorn() && item->GetFormID() != compFormID) {
      return true;
    }
  }
  return false;
}

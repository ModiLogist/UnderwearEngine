#include <Core.h>
#include <Events.h>

Events* events = Events::GetSingleton();

void Events::RegisterEvents() {
  // coverKeys.clear();
  // coverKeys.push_back(Tng::Key(Tng::kyCovering));
  // coverKeys.push_back(Tng::Key(Tng::kyRevealingF));
  // coverKeys.push_back(Tng::Key(Tng::kyRevealingM));
  // showErrMessage = true;
  const auto sourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
  sourceHolder->AddEventSink<RE::TESEquipEvent>(GetSingleton());
  sourceHolder->AddEventSink<RE::TESObjectLoadedEvent>(GetSingleton());
  SKSE::log::info("Registered for necessary events.");
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = event->actor ? event->actor->As<RE::Actor>() : nullptr;
  auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(event->baseObject);
  auto npc = actor ? actor->GetActorBase() : nullptr;
  if (!npc || !armor) return RE::BSEventNotifyControl::kContinue;
  if (justProcessed.find(actor) != justProcessed.end() && justProcessed[actor] == armor) {
    justProcessed.erase(actor);
    return RE::BSEventNotifyControl::kContinue;
  }
  if (actor->IsPlayerRef()) {
    if (core->processingPlayer) return RE::BSEventNotifyControl::kContinue;
    playerRes = core->ProcessPlayer(actor);
    if (playerRes == Util::resFail) return RE::BSEventNotifyControl::kContinue;
    if (armor->HasKeyword(Util::Key(Util::kyItemFake))) {
      auto pair = core->FindPair(armor, npc->IsFemale());
      if (event->equipped) {
        if (!core->HasCover(actor) && !actor->GetWornArmor(Util::cSlot32)) core->eq->EquipObject(actor, pair->first, nullptr, 1, nullptr, false, false, false, true);
        if (auto pcUndies = Util::FormList(Util::flPCUndies)) {
          Util::CleanFormList(pcUndies);
          pcUndies->AddForm(pair->first);
        }
      } else {
        if (actor->GetWornArmor(pair->first->GetFormID())) core->eq->UnequipObject(actor, pair->first, nullptr, 1, nullptr, false, false, false, true);
        if (auto pcUndies = Util::FormList(Util::flPCUndies)) Util::CleanFormList(pcUndies);
      }
    }
  }
  if (core->IsCovering(actor, armor) && !event->equipped) {
    justProcessed[actor] = armor;
    auto pair = core->GetActorItem(actor);
    if (!pair) return RE::BSEventNotifyControl::kContinue;
    if (actor->GetWornArmor(pair->second->GetFormID())) core->eq->EquipObject(actor, pair->first, nullptr, 1, nullptr, true, false, false, false);
  }
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  // const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->GetFormID());
  // if (actor && actor->IsPlayerRef()) core->PostProcessActors();
  // auto base = Core::GetSingleton();
  // base->ProcessActor(actor);
  return RE::BSEventNotifyControl::kContinue;
}
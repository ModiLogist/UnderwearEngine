#include <Core.h>
#include <Events.h>

Events* events = Events::GetSingleton();

void Events::RegisterEvents() {
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
  if (armor->HasKeyword(Util::Key(Util::kyItemFake)) && actor->IsPlayerRef()) {
    core->ProcessPlayer(actor, armor, event->equipped);
    return RE::BSEventNotifyControl::kContinue;
  }
  if (core->IsUnderwear(armor)) {
    if (actor->IsPlayerTeammate() && armor->HasKeyword(Util::Key(Util::kyItemFake))) {
      auto undies = core->GetActorItem(actor);
      if (event->equipped) {
        if (undies) core->WearUndies(actor, undies, armor);
      } else {
        if (undies) core->TakeOffUndies(actor, true);
      }
    }
    return RE::BSEventNotifyControl::kContinue;
  }
  if (!event->equipped && core->IsCovering(actor, armor) && !core->HasCover(actor, armor)) {
    justProcessed[actor] = armor;
    auto undies = core->GetActorItem(actor);
    if (!undies || !actor->GetWornArmor(undies->second->formID)) return RE::BSEventNotifyControl::kContinue;
    core->WearUndies(actor, undies, armor);
  }
  return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl Events::ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) {
  if (!event) return RE::BSEventNotifyControl::kContinue;
  const auto actor = RE::TESForm::LookupByID<RE::Actor>(event->formID);
  if (actor && actor->IsPlayerRef()) {
    core->ProcessPlayer(actor);
  }
  return RE::BSEventNotifyControl::kContinue;
}
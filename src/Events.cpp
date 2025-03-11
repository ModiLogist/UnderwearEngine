#include <Core.h>
#include <Events.h>

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
  // const auto actor = event->actor ? event->actor->As<RE::Actor>() : nullptr;
  // auto armor = RE::TESForm::LookupByID<RE::TESObjectARMO>(event->baseObject);
  // auto base = Core::GetSingleton();
  // if (!actor || !armor || !base->IsUnderwear(armor)) return RE::BSEventNotifyControl::kContinue;
  // if (event->equipped) {
  //   armor->AddSlotToMask(Util::cSlot32);
  // } else {
  //   armor->RemoveSlotFromMask(Util::cSlot32);
  // }
  // SKSE::log::info("Processed equip event.");
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
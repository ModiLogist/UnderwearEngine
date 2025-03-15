#include <Core.h>
#include <Hooks.h>

void Hooks::Install() {
  InstallHook<Load3D>();
  InstallHook<RemoveItem>();
  InstallHook<AddObjectToContainer>();
}

void Hooks::ReProcessActors(bool doReset) {
  InternalGuard guard(isInternal);
  if (doReset) core->ResetProcessed();
  RE::TES::GetSingleton()->ForEachReference([=](RE::TESObjectREFR* ref) {
    if (ref->Is(RE::FormType::ActorCharacter)) {
      auto actor = ref->As<RE::Actor>();
      if (actor->IsPlayerRef()) return RE::BSContainer::ForEachResult::kContinue;
      core->ProcessActor(actor);
    }
    return RE::BSContainer::ForEachResult::kContinue;
  });
}

RE::NiAVObject* Hooks::Load3D::thunk(RE::Character* actor, bool backgroundLoading) {
  InternalGuard guard(isInternal);
  auto res = Load3D::func(actor, backgroundLoading);
  core->ProcessActor(actor);
  return res;
}

RE::ObjectRefHandle* Hooks::RemoveItem::thunk(RE::Character* fromRef, RE::ObjectRefHandle* returnValue, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason,
                                              RE::ExtraDataList* extraList, RE::TESObjectREFR* toRef, const RE::NiPoint3* dropLoc, const RE::NiPoint3* rotate) {
  auto res = RemoveItem::func(fromRef, returnValue, item, count, reason, extraList, toRef, dropLoc, rotate);
  if (isInternal) return res;
  InternalGuard guard(isInternal);
  auto pair = item ? item->As<RE::TESObjectARMO>() : nullptr;
  if (!fromRef || !pair || !pair->HasKeyword(ut->Key(ut->kyItemFake))) return res;
  auto vis = core->GetOther(pair, RE::SEX::kNone, true);
  if (!vis) {
    SKSE::log::critical("NUDE could not process the underwear paired with [{:x}] that was transferred from [{:x}:{}]!", pair->GetFormID(), fromRef->GetFormID(),
                        fromRef->GetFormEditorID());
    return res;
  } else {
    res = RemoveItem::func(fromRef, res, vis, count, reason, extraList, toRef, dropLoc, rotate);
    if (!core->GetActorItem(fromRef, true) || (fromRef->IsDead() && !fromRef->GetWornArmor(ut->cSlot52))) {
      core->SetActorItem(fromRef, ut->cNul, false);
    }
  }
  return res;
}

void Hooks::AddObjectToContainer::thunk(RE::Character* toRef, RE::TESBoundObject* item, RE::ExtraDataList* extraList, std::int32_t count, RE::TESObjectREFR* fromRefr) {
  AddObjectToContainer::func(toRef, item, extraList, count, fromRefr);
  if (isInternal || !fromRefr) return;
  InternalGuard guard(isInternal);
  auto pair = item ? item->As<RE::TESObjectARMO>() : nullptr;
  if (!toRef || !pair || !pair->HasKeyword(ut->Key(ut->kyItemFake))) return;
  if (!fromRefr->IsPlayerRef()) return;
  auto vis = core->GetOther(pair, RE::SEX::kNone, true);
  if (!vis) {
    SKSE::log::critical("NUDE could not process the underwear paired with [{:x}] that was transferred to [{:x}:{}]!", pair->GetFormID(), toRef->GetFormID(),
                        toRef->GetFormEditorID());
    return;
  } else {
    fromRefr->RemoveItem(vis, count, RE::ITEM_REMOVE_REASON::kRemove, nullptr, toRef);
    auto toNPC = toRef->GetActorBase();
    if (toNPC
        //&& !toRef->IsDead()
    ) {
      auto undies = core->GetActorItem(toRef, true);
      if (undies) {
        core->TryExclude(toRef, false);
        core->WearUndies(toRef, undies);
      }
    }
  }
}

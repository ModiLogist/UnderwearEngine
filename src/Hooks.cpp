#include <Core.h>
#include <Hooks.h>

void Hooks::Install() {
  InstallHook<Load3D>();
  // InstallHook<RemoveItem>();
}

RE::NiAVObject* Hooks::Load3D::thunk(RE::Character* actor, bool backgroundLoading) {
  auto res = Load3D::func(actor, backgroundLoading);
  auto processed = core->ProcessActor(actor);
  return res;
}

RE::ObjectRefHandle Hooks::RemoveItem::thunk(RE::Character* actor, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, RE::ExtraDataList* extraList,
                                             RE::TESObjectREFR* moveToRef, const RE::NiPoint3* dropLoc, const RE::NiPoint3* rotate) {
  auto res = RemoveItem::func(actor, item, count, reason, extraList, moveToRef, dropLoc, rotate);
  if (!moveToRef || !moveToRef->IsPlayerRef() || !core->IsUnderwear(item)) return res;
}

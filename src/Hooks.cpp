#include <Core.h>
#include <Hooks.h>

void Hooks::Install() { InstallHook<Load3D>(); }

RE::NiAVObject* Hooks::Load3D::thunk(RE::Character* actor, bool backgroundLoading) {
  auto res = Load3D::func(actor, backgroundLoading);
  if (const auto npc = actor->GetActorBase()) {
    auto processed = core->ProcessActor(actor);
    SKSE::log::debug("Processed npc [0x{:X}:{}] with result [{}]", npc->formID, npc->GetName(), processed == Util::eRes::resOk);
  }
  return res;
}

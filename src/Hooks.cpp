#include <Core.h>
#include <Hooks.h>

void Hooks::Install() { InstallHook<Load3D>(); }

RE::NiAVObject* Hooks::Load3D::thunk(RE::Character* actor, bool backgroundLoading) {
  if (actor) SKSE::log::trace("Processing actor [0x{:x}].", actor->GetFormID());
  auto res = Load3D::func(actor, backgroundLoading);
  auto processed = core->ProcessActor(actor);
  SKSE::log::trace("Processed actor [0x{:x}] with result [{}].", actor->GetFormID(), processed == Util::resOk);
  return res;
}

#include <Core.h>
#include <Events.h>
#include <Hooks.h>
#include <Util.h>

#include <thread>

static void InitializeLogging() {
  auto path{SKSE::log::log_directory()};
  if (!path) {
    SKSE::stl::report_and_fail("Unable to lookup SKSE logs directory.");
  }
  *path /= Version::PROJECT;
  *path += ".log"sv;

  std::shared_ptr<spdlog::logger> log;
  log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
  log->set_level(spdlog::level::trace);  // Inis::GetLogLvl());
  log->flush_on(spdlog::level::trace);
  spdlog::set_default_logger(std::move(log));
  spdlog::set_pattern("[%H:%M:%S.%e] [%l] %v");
}

static void EventListener(SKSE::MessagingInterface::Message* message) {
  if (message->type == SKSE::MessagingInterface::kDataLoaded) {
    if (!Util::SEDH()->LookupModByName(Util::cName)) {
      const char* err = fmt::format("Mod [{}] was not found! Make sure that the mod is active in your plugin load order!", Util::cName).c_str();
      Util::MsgBox(err);
      return;
    }
    if (!core->LoadRaces()) {
      Util::MsgBox("NUDE: Could not load necessary data, it won't apply the underwear to NPCs!");
      return;
    }
    core->LoadNPCs();
    core->LoadItems();
    SKSE::log::info("{} finished initialization.", Util::cName);
    Events::RegisterEvents();
    Hooks::Install();
  }
}

#ifdef SKYRIMFLATRIM
extern "C" __declspec(dllexport) constinit auto SKSEPlugin_Version = []() {
  SKSE::PluginVersionData v;
  v.PluginVersion(Version::MAJOR);
  v.PluginName(Version::PROJECT);
  v.AuthorName("ModiLogist");
  v.UsesAddressLibrary();
  v.UsesUpdatedStructs();
  v.CompatibleVersions({SKSE::RUNTIME_SSE_LATEST});
  return v;
}();
#endif

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface*, SKSE::PluginInfo* aInfo) {
  aInfo->infoVersion = SKSE::PluginInfo::kVersion;
  aInfo->name = Version::PROJECT.data();
  aInfo->version = Version::MAJOR;
  return true;
}

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Load(const SKSE::LoadInterface* aSkse) {
  InitializeLogging();
  SKSE::Init(aSkse, false);
  SKSE::log::info("Initializing UnderwearEngine {}!", Version::NAME.data());
  SKSE::log::info("Game version : {}", aSkse->RuntimeVersion().string());
  SKSE::GetMessagingInterface()->RegisterListener(EventListener);
  // SKSE::GetPapyrusInterface()->Register(Papyrus::BindPapyrus);
  return true;
}
#ifdef SKYRIMVR
extern "C" __declspec(dllexport) const char* APIENTRY GetPluginVersion() { return Version::NAME.data(); }
#endif

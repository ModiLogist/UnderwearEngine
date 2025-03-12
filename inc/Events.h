#pragma once

class Events : public Singleton<Events>, public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent> {
  public:
    void RegisterEvents();
    bool gameLoaded{false};

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;

  private:
    Util::eRes playerRes{Util::resFail};
    std::map<RE::Actor*, RE::TESObjectARMO*> justProcessed;
};

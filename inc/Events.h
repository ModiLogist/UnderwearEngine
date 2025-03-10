#pragma once

class Events : public RE::BSTEventSink<RE::TESObjectLoadedEvent>, public RE::BSTEventSink<RE::TESEquipEvent> {
  public:
    static void RegisterEvents();

    friend class Papyrus;

  protected:
    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) override;
    RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent* event, RE::BSTEventSource<RE::TESObjectLoadedEvent>*) override;

    Events() = default;
    Events(const Events&) = delete;
    Events(Events&&) = delete;

    ~Events() override = default;

    Events& operator=(const Events&) = delete;
    Events& operator=(Events&&) = delete;

    static Events* GetSingleton() {
      static Events singleton;
      return &singleton;
    }
};

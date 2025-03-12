#pragma once

class Hooks {
  public:
    static void Install();

  private:
    struct Load3D {
        using Target = RE::Character;
        static constexpr std::string_view name{"Load3D"};
        inline static constexpr size_t index{0x6a};

        static RE::NiAVObject* thunk(RE::Character* actor, bool backgroundLoading);

        inline static REL::Relocation<decltype(thunk)> func;
    };

    struct RemoveItem {
        using Target = RE::Character;
        static constexpr std::string_view name{"RemoveItem"};
        inline static constexpr size_t index{0x56};

        static RE::ObjectRefHandle thunk(RE::Character* actor, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason, RE::ExtraDataList* extraList,
                                         RE::TESObjectREFR* moveToRef, const RE::NiPoint3* dropLoc = 0, const RE::NiPoint3* rotate = 0);

        inline static REL::Relocation<decltype(thunk)> func;
    };

    template <class T>
    static constexpr auto InstallHook() {
      REL::Relocation<std::uintptr_t> vt{T::Target::VTABLE[0]};
      T::func = vt.write_vfunc(T::index, T::thunk);
      SKSE::log::info("Installed {} hook.", T::name);
    }
};

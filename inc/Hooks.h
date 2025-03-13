#pragma once

class Hooks {
  public:
    static void Install();
    static void ReProcessActors(bool doReset);

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

        static RE::ObjectRefHandle* thunk(RE::Character* fromRef, RE::ObjectRefHandle* returnValue, RE::TESBoundObject* item, std::int32_t count, RE::ITEM_REMOVE_REASON reason,
                                          RE::ExtraDataList* extraList, RE::TESObjectREFR* toRef, const RE::NiPoint3* dropLoc = 0, const RE::NiPoint3* rotate = 0);

        inline static REL::Relocation<decltype(thunk)> func;
    };

    struct AddObjectToContainer {
        using Target = RE::Character;
        static constexpr std::string_view name{"AddObjectToContainer"};
        inline static constexpr size_t index{0x5a};

        static void thunk(RE::Character* toRef, RE::TESBoundObject* item, RE::ExtraDataList* extraList, std::int32_t count, RE::TESObjectREFR* fromRefr);

        inline static REL::Relocation<decltype(thunk)> func;
    };

    template <class T>
    static constexpr auto InstallHook() {
      REL::Relocation<std::uintptr_t> vt{T::Target::VTABLE[0]};
      T::func = vt.write_vfunc(T::index, T::thunk);
      SKSE::log::info("Installed {} hook.", T::name);
    }

    struct InternalGuard {
        InternalGuard(bool& flag) : flag_(flag) { flag_ = true; }
        ~InternalGuard() { flag_ = false; }
        bool& flag_;
    };

    inline static bool isInternal = false;
};

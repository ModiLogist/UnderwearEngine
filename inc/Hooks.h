#pragma once

class Hooks {
  public:
    static void Install();

  private:
    struct Load3D {
        using Target = RE::Character;
        inline static constexpr size_t index{0x6A};

        static RE::NiAVObject* thunk(RE::Character* actor, bool backgroundLoading);

        inline static REL::Relocation<decltype(thunk)> func;

        static constexpr std::string_view name{"Load3D"};
    };

    template <class T>
    static constexpr auto InstallHook() {
      REL::Relocation<std::uintptr_t> vt{T::Target::VTABLE[0]};
      T::func = vt.write_vfunc(T::index, T::thunk);
      SKSE::log::info("Installed {} hook.", T::name);
    }
};

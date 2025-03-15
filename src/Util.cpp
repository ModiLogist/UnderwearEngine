
#include <Util.h>

#include <limits>

Util* ut = Util::GetSingleton();

bool Util::GetBoolSetting(const size_t idx) { return idx < BoolSettingCount ? boolSettings[idx] : false; }

void Util::SetBoolSetting(const size_t idx, const bool value) {
  if (idx < BoolSettingCount) boolSettings[idx] = value;
}

RE::BGSKeyword* Util::Key(const size_t idx) {
  if (idx >= KeywordsCount) return nullptr;
  if (!keywords[idx]) keywords[idx] = LoadForm<RE::BGSKeyword>(keywordIDs[idx]);
  return keywords[idx];
}

std::vector<RE::BGSKeyword*> Util::Keys(const size_t first, const size_t last) {
  std::vector<RE::BGSKeyword*> res = {};
  if (last >= KeywordsCount) return res;
  for (auto i = first; i <= last; i++) res.push_back(Key(i));
  return res;
}

RE::TESRace* Util::Race(const size_t idx) {
  if (idx >= RacesCount) return nullptr;
  if (!races[idx]) races[idx] = LoadForm<RE::TESRace>(raceIDs[idx]);
  return races[idx];
}

bool Util::IsBaseCover(RE::TESObjectARMO* down) { return FormToLocView(down) != Util::coverID; }

RE::BGSListForm* Util::FormList(const size_t idx) {
  if (idx >= FLCount) return nullptr;
  if (!fls[idx]) fls[idx] = LoadForm<RE::BGSListForm>(flIDs[idx]);
  return fls[idx];
}

RE::BGSKeyword* Util::ProduceOrGetKw(const std::string& keyword) {
  auto& allKeywords = SEDH()->GetFormArray<RE::BGSKeyword>();
  auto it = std::find_if(allKeywords.begin(), allKeywords.end(), [&](const auto& kw) { return kw && kw->formEditorID == keyword.c_str(); });
  if (it != allKeywords.end()) {
    return *it;
  }
  const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::BGSKeyword>();
  auto kw = factory ? factory->Create() : nullptr;
  if (kw) {
    kw->formEditorID = keyword;
    allKeywords.push_back(kw);
  }
  return kw;
}

void Util::MsgBox(const char* message) { RE::DebugMessageBox(message); }

SEFormLoc Util::FormToLoc(RE::TESForm* form) {
  std::string filename = form->GetFile(0) ? std::string(form->GetFile(0)->GetFilename()) : "NoFile";
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  return {formID, filename};
}

SEFormLocView Util::FormToLocView(RE::TESForm* form) {
  auto filename = form->GetFile(0) ? form->GetFile(0)->GetFilename() : "NoFile";
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  return {formID, filename};
}

SEFormLoc Util::StrToLoc(const std::string& recordStr) {
  const size_t sepLoc = recordStr.find(cDelimChar);
  RE::FormID formID;
  if (sepLoc == std::string::npos) {
    if (try_strtoul(recordStr, formID)) {
      return {formID, std::string(cSkyrim)};
    } else {
      return {0, ""};
    }
  }
  if (try_strtoul(recordStr.substr(0, sepLoc).data(), formID)) {
    const std::string modName = recordStr.substr(sepLoc + 1);
    return {formID, modName};
  } else {
    return {0, ""};
  }
}

std::string Util::FormToStr(RE::TESForm* form) {
  if (!form || !form->GetFile(0)) return "";
  std::ostringstream oss;
  auto formID = form->GetFormID() < 0xFF000000 ? form->GetLocalFormID() : form->GetFormID();
  oss << std::hex << formID;
  return "0x" + oss.str() + cDelimChar + std::string(form->GetFile(0)->GetFilename());
}

bool Util::try_strtoul(const std::string& str, std::uint32_t& result, int base) {
  char* end;
  errno = 0;
  unsigned long value = std::strtoul(str.c_str(), &end, base);
  if (errno == ERANGE || end == str.c_str() || *end != '\0') {
    return false;
  }
  if (value > UINT_MAX) {
    return false;
  }
  result = static_cast<std::uint32_t>(value);
  return true;
}

RE::TESDataHandler* Util::SEDH() {
  if (!sedh) sedh = RE::TESDataHandler::GetSingleton();
  return sedh;
}

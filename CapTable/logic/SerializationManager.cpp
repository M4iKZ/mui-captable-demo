
#include <fstream>
#include <sstream>
#include <iomanip>

#include <nlohmann/json.hpp>

#include "SerializationManager.hpp"

using json = nlohmann::json;

namespace mUI::Demo::CapTable
{
    // ─── Serialization helpers ────────────────────────────────────────────────

    static json founderToJson(const Founder& f)
    {
        return json
        {
            {"id",    f.id},
            {"name",  f.name},
            {"pct",   f.pct},
            {"color", f.color}
        };
    }

    static json roundToJson(const FundingRound& r)
    {
        return json
        {
            {"id",    r.id},
            {"title", r.title},
            {"cash",  r.cash},
            {"pct",   r.pct},
            {"color", r.color}
        };
    }

    static Founder founderFromJson(const json& j)
    {
        Founder f;

        f.id   = j.value("id", uint32_t(0));
        f.name = j.value("name", std::string("Founder"));
        f.pct  = j.value("pct", 0.0f);

        // Handle color as string (preferred) or integer (legacy)
        f.color = "#AAAAAA";
        if (j.contains("color"))
        {
            const auto& jc = j["color"];
            if (jc.is_string())
            {
                f.color = jc.get<std::string>();
            }
            else if (jc.is_number_unsigned())
            {
                // Convert integer back to hex string
                uint32_t rgb = jc.get<uint32_t>();
                std::ostringstream ss;
                ss << "#" << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (rgb & 0xFFFFFF);
                f.color = ss.str();
            }
        }

        return f;
    }

    static FundingRound roundFromJson(const json& j)
    {
        FundingRound r;

        r.id    = j.value("id", uint64_t(0));
        r.title = j.value("title", std::string("Round"));
        r.cash  = j.value("cash", 0.0);
        
        if (j.contains("pct") && j["pct"].is_number()) r.pct = j["pct"].get<float>();
        else if (j.contains("equityPct") && j["equityPct"].is_number()) r.pct = j["equityPct"].get<float>();
        else r.pct = 0.0f;

        r.color = j.value("color", std::string("#aaaaaa"));

        return r;
    }

    // ─── toJson ───────────────────────────────────────────────────────────────

    std::string SerializationManager::toJson(const CapTableState& state, bool pretty) const
    {
        json j;

        j["title"]            = state.title;
        j["currency"]         = state.currency;
        j["defaultRoundName"] = state.defaultRoundName;

        json foundersArr = json::array();
        for (const auto& f : state.founders) foundersArr.push_back(founderToJson(f));

        j["founders"] = foundersArr;

        json roundsArr = json::array();
        for (const auto& r : state.rounds) roundsArr.push_back(roundToJson(r));

        j["rounds"] = roundsArr;

        return pretty ? j.dump(2) : j.dump();
    }

    // ─── fromJson ─────────────────────────────────────────────────────────────

    bool SerializationManager::fromJson(const std::string& jsonStr, CapTableState& state) const
    {
        try
        {
            const json j = json::parse(jsonStr);

            state.title            = j.value("title",            state.title);
            state.currency         = j.value("currency",         state.currency);
            state.defaultRoundName = j.value("defaultRoundName", state.defaultRoundName);

            if (j.contains("founders") && j["founders"].is_array())
            {
                state.founders.clear();
                for (const auto& jf : j["founders"]) state.founders.push_back(founderFromJson(jf));
            }

            if (j.contains("rounds") && j["rounds"].is_array())
            {
                state.rounds.clear();
                for (const auto& jr : j["rounds"]) state.rounds.push_back(roundFromJson(jr));
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    // ─── saveToFile ───────────────────────────────────────────────────────────

    bool SerializationManager::saveToFile(const std::string& path, const CapTableState& state) const
    {
        std::ofstream ofs(path, std::ios::out | std::ios::trunc | std::ios::binary);
        if (!ofs.is_open()) return false;

        ofs << toJson(state, /*pretty=*/false);
        return ofs.good();
    }

    // ─── loadFromFile ─────────────────────────────────────────────────────────

    bool SerializationManager::loadFromFile(const std::string& path, CapTableState& state) const
    {
        std::ifstream ifs(path, std::ios::binary);
        if (!ifs.is_open()) return false;

        std::ostringstream ss;
        ss << ifs.rdbuf();

        std::string str = ss.str();

        // Strip UTF-8 BOM (\xEF\xBB\xBF) — emitted by Notepad and many editors
        if (str.size() >= 3 && (unsigned char)str[0] == 0xEFu && (unsigned char)str[1] == 0xBBu && (unsigned char)str[2] == 0xBFu)
        {
            str = str.substr(3);
        }

        return fromJson(str, state);
    }

} // namespace mUI::Demo::CapTable
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace mUI::Demo::CapTable
{
    // ─── Founder ─────────────────────────────────────────────────────────────

    struct Founder
    {
        uint32_t    id;           ///< Monotonically increasing, unique per session
        std::string name;         ///< Displayed name
        float       pct;          ///< Initial ownership percentage (0–100)
        std::string color;        ///< Hex tag: "#RRGGBB"
    };

    // ─── FundingRound ────────────────────────────────────────────────────────

    struct FundingRound
    {
        uint64_t    id;           ///< Epoch-ms stamp used as stable key
        std::string title;        ///< e.g. "Series A"
        double      cash;         ///< Capital raised (in chosen currency units)
        float       pct;          ///< Equity sold this round (0–100)
        std::string color = "#aaaaaa"; ///< Hex tag for chart display
    };

    // ─── RoundResult (computed, not persisted) ────────────────────────────────

    struct RoundResult
    {
        std::string roundTitle;
        double      preMoney  = 0.0;
        double      postMoney = 0.0;
    };

    // ─── StakeRow (computed, not persisted) ───────────────────────────────────

    /// One row in the cap table: a shareholder and their current stake.
    struct StakeRow
    {
        std::string name;
        float       ownershipPct = 0.0f; ///< After all rounds, 0–100
        double      value        = 0.0;  ///< ownershipPct * postMoney
        bool        isFounder    = false;
        std::string color        = "#666666"; ///< Hex tag: "#RRGGBB"
    };

    // ─── CapTableState ────────────────────────────────────────────────────────

    /// Full snapshot of the user-editable state (persisted to JSON).
    struct CapTableState
    {
        std::string              title           = "MIDE Co.";
        std::string              currency        = "€";
        std::string              defaultRoundName= "Round";

        std::vector<Founder>      founders;
        std::vector<FundingRound> rounds;
    };

    // ─── CalculationResult (computed from CapTableState) ─────────────────────

    struct ProgressionPoint
    {
        std::string label;                      ///< "Foundation", "Round #1", …
        std::vector<float> founderQuotaPcts;    ///< Per-founder, index matches CapTableState::founders
        std::vector<double> founderValues;      ///< Per-founder stake value at this point
    };

    struct CalculationResult
    {
        std::vector<StakeRow>        stakes;         ///< Final cap-table rows (founders + investors)
        std::vector<RoundResult>     roundResults;   ///< Pre/post money per round
        std::vector<ProgressionPoint>progression;    ///< Timeline: Foundation + each round
        double                       finalPostMoney = 0.0;
        float                        foundersTotal  = 0.0f; ///< Combined final founder stake %
    };

} // namespace mUI::Demo::CapTable
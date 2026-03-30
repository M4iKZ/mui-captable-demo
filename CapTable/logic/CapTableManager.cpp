
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

#include "CapTableManager.hpp"

namespace mUI::Demo::CapTable
{
    // ─── formatCurrency ───────────────────────────────────────────────────────

    std::string CapTableManager::formatCurrency(const std::string& symbol, double amount) const
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);

        const double abs_amount = std::abs(amount);

        if (abs_amount >= 1e9) oss << symbol << (amount / 1e9) << "B";
        else if (abs_amount >= 1e6) oss << symbol << (amount / 1e6) << "M";
        else if (abs_amount >= 1e3) oss << symbol << (amount / 1e3) << "k";
        else oss << symbol << amount;

        return oss.str();
    }

    // ─── makeFoundationPoint ─────────────────────────────────────────────────

    ProgressionPoint CapTableManager::makeFoundationPoint(const CapTableState& state) const
    {
        ProgressionPoint pt;
        pt.label = "Foundation";

        for (const auto& f : state.founders)
        {
            pt.founderQuotaPcts.push_back(f.pct);
            pt.founderValues.push_back(0.0); // no valuation yet
        }

        return pt;
    }

    // ─── calculate ───────────────────────────────────────────────────────────

    CalculationResult CapTableManager::calculate(const CapTableState& state) const
    {
        CalculationResult result;

        if (state.founders.empty()) return result;

        // Running stakes: name → fraction (0.0–1.0)
        std::unordered_map<std::string, double> stakes;
        for (const auto& f : state.founders) stakes[f.name] = f.pct / 100.0;

        // Timeline starts at Foundation
        result.progression.push_back(makeFoundationPoint(state));

        double lastPostMoney = 0.0;

        for (const auto& round : state.rounds)
        {
            const double roundFraction = round.pct / 100.0;
            const double postMoney     = (roundFraction > 0.0) ? (round.cash / roundFraction) : 0.0;
            const double preMoney      = postMoney - round.cash;

            // Dilute every current holder
            for (auto& [name, stake] : stakes) stake *= (1.0 - roundFraction);

            // Record founder progression at this point
            ProgressionPoint pt;
            pt.label = round.title;
            for (const auto& f : state.founders)
            {
                const double stake = stakes.count(f.name) ? stakes.at(f.name) : 0.0;
                pt.founderQuotaPcts.push_back(static_cast<float>(stake * 100.0));
                pt.founderValues.push_back(stake * postMoney);
            }

            result.progression.push_back(std::move(pt));

            // Add new investor
            stakes[round.title] = roundFraction;

            // Record round metrics
            RoundResult rr;
            rr.roundTitle = round.title;
            rr.preMoney   = preMoney;
            rr.postMoney  = postMoney;
            result.roundResults.push_back(rr);

            lastPostMoney = postMoney;
        }

        result.finalPostMoney = lastPostMoney;

        // Build final cap-table rows
        float foundersTotal = 0.0f;

        for (const auto& [name, stake] : stakes)
        {
            StakeRow row;
            row.name         = name;
            row.ownershipPct = static_cast<float>(stake * 100.0);
            row.value        = stake * lastPostMoney;

            // Is this a founder?
            for (const auto& f : state.founders)
            {
                if (f.name == name)
                {
                    row.isFounder = true;
                    row.color     = f.color;
                    foundersTotal += row.ownershipPct;
                    break;
                }
            }

            // If not a founder, check if it matches a round title and use its color
            if (!row.isFounder)
            {
                for (const auto& round : state.rounds)
                {
                    if (round.title == name)
                    {
                        row.color = round.color;
                        break;
                    }
                }
            }

            result.stakes.push_back(row);
        }

        // Sort: founders first (by descending stake), then investors
        std::sort(result.stakes.begin(), result.stakes.end(), [](const StakeRow& a, const StakeRow& b)
        {
            if (a.isFounder != b.isFounder) return a.isFounder > b.isFounder;

            return a.ownershipPct > b.ownershipPct;
        });

        result.foundersTotal = foundersTotal;

        return result;
    }

} // namespace mUI::Demo::CapTable
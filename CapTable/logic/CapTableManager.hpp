#pragma once

#include "Models.hpp"

namespace mUI::Demo::CapTable
{
    /**
     * @brief Pure calculation engine for the cap table.
     *
     * Stateless: takes a CapTableState, returns a CalculationResult.
     * No UI dependencies, no singletons, no global state.
     */
    class CapTableManager
    {
    public:

        CapTableManager() = default;

        /**
         * @brief Run the full dilution cascade and produce all derived data.
         *
         * Algorithm (mirrors the JS calculate() in table.html):
         *   1. Start with each founder's initial pct as their running stake.
         *   2. For each funding round (in order):
         *      a. Dilute every existing holder by (1 - roundPct).
         *      b. Add the new round investor at roundPct.
         *      c. Compute pre/post money from cash and equityPct.
         *   3. Record a ProgressionPoint after each step (Foundation + rounds).
         *
         * @param state  Current user-editable state.
         * @return       Populated CalculationResult.
         */
        CalculationResult calculate(const CapTableState& state) const;

        /// Convenience: format a monetary amount using standard SI suffixes.
        std::string formatCurrency(const std::string& symbol, double amount) const;

    private:

        // Build the "Foundation" progression point (before any rounds).
        ProgressionPoint makeFoundationPoint(const CapTableState& state) const;
        
    };

} // namespace mUI::Demo::CapTable
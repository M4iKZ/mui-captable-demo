#pragma once

#include <memory>
#include <string>
#include <cstdint>

#include <IUI.hpp>

#include "logic/Models.hpp"
#include "logic/CapTableManager.hpp"
#include "logic/SerializationManager.hpp"

namespace mUI::Demo::CapTable
{
    /**
     * @brief Top-level application controller for the Cap Table demo.
     *
     * Owns the mUI lifecycle, the UI state, and all logic managers.
     * The orchestrator bridges HTML/CSS rendering and native chart widgets;
     * this class drives the data layer and event handling.
     *
     * Dependency graph (all owned by value — no shared state):
     *   CapTableState  ← user-editable model
     *   CapTableManager  ← pure calculation engine (stateless)
     *   SerializationManager  ← JSON export/import (stateless)
     *   ILayoutOrchestrator  ← mUI bridge (owned as shared_ptr)
     */
    class CapTableApp
    {
    public:

        CapTableApp();
        ~CapTableApp();

        bool initialize();
        void run();

    private:

        // ── Owned subsystems ──────────────────────────────────────────────────
        
        CapTableState        m_state;
        mutable std::recursive_mutex m_stateMutex;
        
        CapTableManager      m_engine;
        SerializationManager m_serializer;

        std::shared_ptr<mUI::Scene::ILayoutOrchestrator> m_orch;

        // Event subscription IDs (for cleanup on shutdown)
        size_t m_dblClickSubId = 0;
        size_t m_keyCallbackId = 0;

        // Initialization / deferred widget-bind state
        bool              m_initialized       = false; ///< true after first-tick DOM setup
        int               m_pendingBindTicks  = 0;     ///< >0: updateCharts in N more ticks
        CalculationResult m_lastResult;                ///< cached for deferred chart binding

        // Next unique id for newly created founders / rounds
        uint32_t m_nextFounderId = 100;
        uint64_t m_nextRoundId   = 1000;

        // Track the currently-active tab so fullRebuild() can restore it
        struct TabConfig 
        { 
            const char* tabId; 
            const char* paneId; 
            const char* key; 
        };
        
        static inline constexpr TabConfig m_tabSet[] =
        {
            { "tab-table",    "pane-table",    "table"    },
            { "tab-founders", "pane-founders", "founders" },
            { "tab-charts",   "pane-charts",   "charts"   },
        };

        std::string m_activeTabId = "table";

        // Background import handling 
        CapTableState m_pendingImportState;
        bool          m_hasPendingImport = false;

        // Debounce live-edit DOM rebuilds: only call updateCapTableDOM after
        // the user stops typing for ~3 ticks (~50 ms at 60 fps).
        int  m_editSettleCounter = 0;
        bool m_pendingRecalc     = false;

        // ── Event handlers ────────────────────────────────────────────────────
        void onButtonClick(const std::string& cssClasses, const std::string& text);
        void onUpdate(float dt);
        void onDoubleClick(int x, int y);
        void onKeyEvent(const mUI::Events::eventData& e);

        // Pull editable field values from the DOM back into m_state.
        void readStateFromDOM();

        // ── UI update helpers ─────────────────────────────────────────────────
        
        using Doc = std::shared_ptr<mUI::Parser::DOM::Document>;

        // Re-run the calculation and push all results to the DOM + charts.
        void recalculate(Doc doc = nullptr);

        // Full page re-parse via prepareDocument — rounds and founders are
        // injected as HTML strings so Win32 EDIT controls are seeded at parse time.
        void fullRebuild();

        // Build the inner HTML for the rounds sidebar and founders panel.
        std::string buildRoundCardHTML(const FundingRound& r, bool showRemove) const;
        std::string buildFounderRowHTML(const Founder& f, bool showRemove) const;
        std::string buildRoundsHTML() const;
        std::string buildFoundersHTML() const;

        // Update the founder totals / progress bar from m_state (DOM-only, no inputs).
        void updateFounderTotals(Doc doc);

        // Update the cap-table tab DOM with a CalculationResult.
        void updateCapTableDOM(const CalculationResult& r, Doc doc = nullptr);

        // Push updated data to all chart widgets (called after recalculate).
        void updateCharts(const CalculationResult& r);

        // Bind color-picker onChange callbacks for each founder swatch.
        void bindColorPickers(Doc doc = nullptr);

        // ── Import / export ───────────────────────────────────────────────────
        void exportToFile();
        void importFromFile();

        // ── Utility ───────────────────────────────────────────────────────────
        std::string fmtCurrency(double amount) const;
        std::string colorToHex(uint32_t rgb) const;  ///< 0x00RRGGBB → "#RRGGBB"

    };

} // namespace mUI::Demo::CapTable
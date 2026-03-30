
#include <array>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

#include <IUI.hpp>
#include <IWidgets.hpp>
#include <IEvents.hpp>

#include <InputsManager/Common/InputEvents.hpp>

#include <Widgets/Common/Chart/ChartStyle.hpp>

#include "ui/font/Outfit.hpp"
#include "ui/CapTableHTML.hpp"
#include "ui/CapTableCSS.hpp"

#include "CapTableApp.hpp"

// ─── Type aliases ───────────────────────────────────────────────────────────────

using Doc  = std::shared_ptr<mUI::Parser::DOM::Document>;
using Elem = std::shared_ptr<mUI::Parser::DOM::Element>;

// Bring widget types into scope unqualified
using IChartElement = mUI::Widgets::IChartElement;
using ChartData     = mUI::Widgets::ChartData;
using ChartSeries   = mUI::Widgets::ChartSeries;
using ChartStyle    = mUI::Widgets::ChartStyle;
using PointStyle    = mUI::Widgets::PointStyle;

namespace mUI::Demo::CapTable
{
    // ─── Internal DOM helpers ─────────────────────────────────────────────────────

    /// Replace or create the first text-node child of an element.
    static void setElemText(const Doc& doc, const Elem& el, const std::string& text)
    {
        if (!el) return;

        if (el->children.empty())
        {
            el->appendChild(doc->createTextNode(text));
            return;
        }

        auto txt = std::dynamic_pointer_cast<mUI::Parser::DOM::Text>(el->children[0]);
        if (txt) 
        { 
            if (txt->data != text) txt->setNodeValue(text); 
        }
        else     
        { 
            el->children.clear(); 
            el->appendChild(doc->createTextNode(text)); 
        }
    }

    static void setText(const Doc& doc, const std::string& id, const std::string& text)
    {
        if (!doc) return;

        setElemText(doc, doc->getElementById(id), text);
    }

    static void setAttr(const Doc& doc, const std::string& id, const std::string& attr, const std::string& val)
    {
        if (!doc) return;
        
        auto el = doc->getElementById(id);
        if (el) el->setAttribute(attr, val);
    }

    /// HTML-escape the five XML special characters so values are safe in attributes.
    static std::string htmlEncode(const std::string& s)
    {
        std::string r;
        r.reserve(s.size());

        for (unsigned char c : s)
        {
            switch (c)
            {
                case '&':  r += "&amp;";  break;
                case '<':  r += "&lt;";   break;
                case '>':  r += "&gt;";   break;
                case '"':  r += "&quot;"; break;
                default:   r += static_cast<char>(c); break;
            }
        }

        return r;
    }

    // ─── Default colour palette ───────────────────────────────────────────────────

    static const std::string kDefaultColors[] = 
    {
        "#6C63FF", "#FF6B6B", "#51CF66", "#FFD43B", "#339AF0", "#FF922B"
    };

    static constexpr size_t kDefaultColorCount = sizeof(kDefaultColors) / sizeof(kDefaultColors[0]);

    // ─── Constructor / Destructor ─────────────────────────────────────────────────

    CapTableApp::CapTableApp()
    {
        m_state.title            = "MIDE Co.";
        m_state.currency         = "\xe2\x82\xac";  // UTF-8 €
        m_state.defaultRoundName = "Round";

        m_state.founders.push_back({ 913, "Founder A",   65.0f, "#A3F913" });
        m_state.founders.push_back({ 0,   "Founder B",   25.0f, "#4FC3F7" });
        m_state.founders.push_back({ 1,   "Employee #1",  4.0f, "#818CF8" });
        m_state.founders.push_back({ 2,   "Employee #2",  3.0f, "#6366F1" });
        m_state.founders.push_back({ 3,   "Employee #3",  3.0f, "#4F46E5" });
        
        m_nextFounderId = 4;

        // Greyscale palette matching HTML generateColors() — index 1..2 (after founder colors)
        static const char* kRoundColors[] = { "#ffffff", "#aaaaaa", "#777777", "#555555", "#333333", "#222222" };
        m_state.rounds.push_back({ m_nextRoundId++, "Round #1", 100'000'000.0, 20.0f, kRoundColors[1] });
        m_state.rounds.push_back({ m_nextRoundId++, "Round #2", 1'000'000'000.0, 15.0f, kRoundColors[2] });
    }

    CapTableApp::~CapTableApp()
    {
        if (mUI::Events::events)
        {
            if (m_dblClickSubId != 0) mUI::Events::events->UnregisterCallback(m_dblClickSubId);

            if (m_keyCallbackId != 0) mUI::Events::events->UnregisterCallback(m_keyCallbackId);
        }

        mUI::shutdownWidgets();
        mUI::shutdownUI();
    }

    // ─── initialize ───────────────────────────────────────────────────────────────

    bool CapTableApp::initialize()
    {
        mUI::UIConfig config;
        
        config.windowConfig.title  = L"Cap Table & Progression Analytics";
        config.windowConfig.width  = 1600;
        config.windowConfig.height = 900;

        config.windowConfig.style  = mUI::WindowStyle::BORDERED    |
                                     mUI::WindowStyle::RESIZABLE   |
                                     mUI::WindowStyle::MAXIMIZABLE |
                                     mUI::WindowStyle::MINIMIZABLE;

        if (!mUI::initializeUI(config)) return false;

        mUI::initializeWidgets(mUI::WidgetType::FILE_DIALOG | mUI::WidgetType::MESSAGE_DIALOG);

        m_orch = mUI::artist->getOrchestrator();
        if (!m_orch) return false;

        // Load Outfit font into the D2D canvas
        auto canvas = mUI::artist->getCanvas();
        if (canvas)
        {
            for (uint32_t i = 0; i < mUI::Fonts::Outfit::FontCount; ++i)
            {
                const auto& f = mUI::Fonts::Outfit::AllFonts[i];
                canvas->addFontFromMemory(mUI::Fonts::Outfit::FamilyName, f.data, f.size);
            }
        }

        // Wire event callbacks
        m_orch->onButtonClick([this](const std::string& cls, const std::string& text)
        {
            onButtonClick(cls, text);
        });

        m_orch->onUpdate([this](float dt) { onUpdate(dt); });

        // Subscribe to keyboard events for debugging
        mUI::Events::eventConfiguration keyCfg;
        keyCfg.type     = mUI::Events::eventType::SIGNAL_KEY_DOWN;
        keyCfg.callback = [this](const mUI::Events::eventData& e) { onKeyEvent(e); };

        if (mUI::Events::events) m_keyCallbackId = mUI::Events::events->RegisterCallback(keyCfg);

        // Subscribe to mouse double-click events for inline title editing
        mUI::Events::eventConfiguration dblCfg;
        dblCfg.type     = mUI::Events::eventType::SIGNAL_MOUSE_DOUBLE_CLICK;

        dblCfg.callback = [this](const mUI::Events::eventData& e)
        {
            if (const auto* me = e.get<mUI::Inputs::MouseEvent>()) onDoubleClick(me->x, me->y);
        };

        m_dblClickSubId = mUI::Events::events->RegisterCallback(dblCfg);

        // Full rebuild seeds all native Win32 EDIT controls by injecting rounds
        // and founders as HTML strings before the initial layout pass.
        fullRebuild();

        return true;
    }

    // ─── run ─────────────────────────────────────────────────────────────────────

    void CapTableApp::run()
    {
        if (mUI::artist) mUI::artist->run();
    }

    // ─── readStateFromDOM ─────────────────────────────────────────────────────────

    /// Pull all editable field values from the DOM back into m_state.
    void CapTableApp::readStateFromDOM()
    {
        if (!m_orch) return;

        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        auto cur = m_orch->getInputValue("currencySelector");
        if (!cur.empty()) m_state.currency = cur;

        auto defName = m_orch->getInputValue("defaultRoundName");
        if (!defName.empty()) m_state.defaultRoundName = defName;

        for (auto& f : m_state.founders)
        {
            const std::string sid = std::to_string(f.id);

            auto name = m_orch->getInputValue("founder-name-" + sid);
            if (!name.empty()) f.name = name;

            auto pctStr = m_orch->getInputValue("founder-pct-" + sid);
            if (!pctStr.empty())
            {
                try { f.pct = std::stof(pctStr); } catch (...) {}
            }
        }

        for (auto& r : m_state.rounds)
        {
            const std::string sid = std::to_string(r.id);

            auto title = m_orch->getInputValue("round-title-input-" + sid);
            if (!title.empty()) r.title = title;

            auto cashStr = m_orch->getInputValue("round-cash-" + sid);
            if (!cashStr.empty())
            {
                try { r.cash = std::stod(cashStr); } catch (...) {}
            }

            auto eqStr = m_orch->getInputValue("round-equity-" + sid);
            if (!eqStr.empty())
            {
                try { r.pct = std::stof(eqStr); } catch (...) {}
            }
        }
    }

    // ─── onButtonClick ───────────────────────────────────────────────────────────

    void CapTableApp::onButtonClick(const std::string& cssClasses, const std::string& text)
    {
        if (!m_orch) return;

        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        auto doc = m_orch->getDocument();
        if (!doc) return;

        // ── Tab switching ─────────────────────────────────────────────────────────
        if (cssClasses.find("tab-btn") != std::string::npos)
        {
            auto doc = m_orch->getDocument();
            if (!doc) return;

            for (const auto& t : m_tabSet)
            {
                auto tabEl  = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById(t.tabId));
                auto paneEl = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById(t.paneId));
                
                bool isActive = (cssClasses.find(t.tabId) != std::string::npos);
                if (isActive) m_activeTabId = t.key;

                if (tabEl)  tabEl->setAttribute("class", isActive ? "tab-btn active " + std::string(t.tabId) : "tab-btn " + std::string(t.tabId));
                if (paneEl) paneEl->setAttribute("class", isActive ? "tab-pane active" : "tab-pane");
            }

            m_pendingBindTicks = 2;
            if (m_activeTabId == "charts") updateCharts(m_lastResult);

            m_orch->invalidateLayout();
            
            return;
        }

        // ── Navbar title confirm / cancel ──────────────────────────────────────
        if (cssClasses.find("navbar-title-done") != std::string::npos || cssClasses.find("navbar-title-cancel") != std::string::npos)
        {
            auto doc = m_orch->getDocument();
            if (doc)
            {
                if (cssClasses.find("navbar-title-done") != std::string::npos)
                {
                    auto val = m_orch->getInputValue("navbar-title-input");
                    if (!val.empty()) m_state.title = val;

                    setText(doc, "navbar-title", m_state.title);
                }

                if (auto b = doc->getElementById("navbar-title")) 
                {
                    b->setAttribute("style", "display:block;");
                    setText(doc, "navbar-title", m_state.title); // Revert UI label
                }

                if (auto c = doc->getElementById("navbar-edit-container")) c->setAttribute("style", "display:none;");
               
                m_orch->invalidateLayout();
            }

            return;
        }

        // ── Round Title Save / Cancel ──────────────────────────────────────────
        if (cssClasses.find("round-title-done-") != std::string::npos || cssClasses.find("round-title-cancel-") != std::string::npos)
        {
            bool isDone = cssClasses.find("round-title-done-") != std::string::npos;
            
            size_t pos  = isDone ? cssClasses.find("round-title-done-") : cssClasses.find("round-title-cancel-");
            size_t idStart = pos + (isDone ? 17 : 19);
            size_t idEnd   = cssClasses.find(' ', idStart);

            std::string sid = (idEnd == std::string::npos) ? cssClasses.substr(idStart) : cssClasses.substr(idStart, idEnd - idStart);

            if (isDone)
            {
                auto val = m_orch->getInputValue("round-title-input-" + sid);
                bool changed = false;

                for (auto& r : m_state.rounds)
                {
                    if (std::to_string(r.id) == sid) 
                    {
                        if (!val.empty() && r.title != val) 
                        {
                            r.title = val;
                            changed = true;
                        }

                        break;
                    }
                }

                if (changed) recalculate(doc);
                
                auto viewEl = doc->getElementById("round-title-view-" + sid);
                if (viewEl && !val.empty()) setElemText(doc, viewEl, val);
            }

            auto viewEl = doc->getElementById("round-title-view-" + sid);
            auto editEl = doc->getElementById("round-title-edit-container-" + sid);

            if (viewEl) viewEl->setAttribute("style", "display:block; padding: 8px 12px; font-size: 1.1rem; border: 1px solid transparent; width: 100%;");
            
            if (editEl) editEl->setAttribute("style", "display:none;");
            
            m_orch->invalidateLayout();
            
            return;
        }

        // ── Round Removal ─────────────────────────────────────────────────────
        if (cssClasses.find("btn-remove-round-") != std::string::npos)
        {
            size_t pos     = cssClasses.find("btn-remove-round-");
            size_t idStart = pos + 17;
            size_t idEnd   = cssClasses.find(' ', idStart);
            std::string sid = (idEnd == std::string::npos) ? cssClasses.substr(idStart) : cssClasses.substr(idStart, idEnd - idStart);

            auto it = std::find_if(m_state.rounds.begin(), m_state.rounds.end(), [&](const auto& r) { return std::to_string(r.id) == sid; });
            if (it != m_state.rounds.end())
            {
                readStateFromDOM();
                m_state.rounds.erase(it);
                fullRebuild();
            }
            
            return;
        }

        // ── Export ────────────────────────────────────────────────────────────────
        if (cssClasses.find("export-btn") != std::string::npos)
        {
            exportToFile();
            return;
        }

        // ── Import ────────────────────────────────────────────────────────────────
        if (cssClasses.find("import-btn") != std::string::npos)
        {
            importFromFile();
            return;
        }

        // Remaining actions require reading form state first
        readStateFromDOM();

        // ── Add funding round ─────────────────────────────────────────────────────
        if (cssClasses.find("btn-add-round") != std::string::npos)
        {
            const std::string title = m_state.defaultRoundName + " #" + std::to_string(m_state.rounds.size() + 1);

            static const char* kRoundColors[] = { "#ffffff", "#aaaaaa", "#777777", "#555555", "#333333", "#222222" };
            
            const std::string roundColor = kRoundColors[m_state.rounds.size() % 6];
            m_state.rounds.push_back({ m_nextRoundId++, title, 0.0, 10.0f, roundColor });
            
            fullRebuild();
            
            return;
        }

        // ── Add founder ───────────────────────────────────────────────────────────
        if (cssClasses.find("btn-add-founder") != std::string::npos)
        {
            uint32_t colorIdx = static_cast<uint32_t>(m_state.founders.size()) % kDefaultColorCount;
            
            m_state.founders.push_back({ m_nextFounderId++, "Founder", 10.0f, kDefaultColors[colorIdx] });
            
            fullRebuild();
            
            return;
        }

        // ── Remove founder ────────────────────────────────────────────────────────
        if (cssClasses.find("btn-remove-founder-") != std::string::npos)
        {
            auto pos = cssClasses.find("btn-remove-founder-");
            if (pos != std::string::npos)
            {
                auto idStart = pos + std::string("btn-remove-founder-").size();
                auto idEnd   = cssClasses.find(' ', idStart);
                
                std::string idStr = (idEnd == std::string::npos) ? cssClasses.substr(idStart) : cssClasses.substr(idStart, idEnd - idStart);

                try
                {
                    uint32_t fid = static_cast<uint32_t>(std::stoul(idStr));
                    readStateFromDOM();
                    m_state.founders.erase(std::remove_if(m_state.founders.begin(), m_state.founders.end(), [fid](const Founder& f){ return f.id == fid; }), m_state.founders.end());
                }
                catch (...) {}
            }

            fullRebuild();
            return;
        }
    }

    // ─── onUpdate ────────────────────────────────────────────────────────────────

    void CapTableApp::onUpdate(float /*dt*/)
    {
        if (!m_orch || !m_initialized) return;

        auto doc = m_orch->getDocument();
        if (!doc) return;

        // Take a lock for the whole update as we might iterate founders/rounds or read currency
        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        // ── Applied deferred import state if any ──────────────────────────────────
        if (m_hasPendingImport)
        {
            m_hasPendingImport = false;
            m_state = std::move(m_pendingImportState);

            // Re-seed ID counters so new items don't collide
            for (const auto& f : m_state.founders) if (f.id >= m_nextFounderId) m_nextFounderId = f.id + 1;
            
            for (const auto& r : m_state.rounds)   if (r.id >= m_nextRoundId)  m_nextRoundId   = r.id + 1;

            // Full re-parse — this now happens on the UI thread safely
            fullRebuild();
        }

        // ── Deferred widget binding ───────────────────────────────────────────────
        // Charts / color-pickers inside display:none tabs are not registered by the
        // layout engine until their tab first becomes visible.  We count down from 2
        // after startup and after every tab-switch, then attempt to bind.
        if (m_pendingBindTicks > 0)
        {
            --m_pendingBindTicks;
            if (m_pendingBindTicks == 0)
            {
                // bindColorPickers(doc); // Polling used instead
                updateCharts(m_lastResult);
            }

            m_orch->invalidateLayout();
            return;
        }

        // ── Poll currency selector ────────────────────────────────────────────────
        auto newCurrency = m_orch->getInputValue("currencySelector");
        if (!newCurrency.empty() && newCurrency != m_state.currency)
        {
            readStateFromDOM();
            m_state.currency = newCurrency;
            fullRebuild();
        }

        // ── Live EDIT sync — poll all editable fields and recalculate on change ─────
        bool anyFieldChanged = false;

        for (auto& f : m_state.founders)
        {
            const std::string sid = std::to_string(f.id);

            const auto nameVal = m_orch->getInputValue("founder-name-" + sid);
            if (!nameVal.empty() && nameVal != f.name)
            {
                f.name = nameVal;
                anyFieldChanged = true;
            }

            const auto pctStr = m_orch->getInputValue("founder-pct-" + sid);
            if (!pctStr.empty())
            {
                try
                {
                    const float v = std::stof(pctStr);
                    if (std::abs(v - f.pct) > 0.001f) 
                    { 
                        f.pct = v; 
                        anyFieldChanged = true; 
                    }
                }
                catch (...) {}
            }

            const auto colorVal = m_orch->getInputValue("founder-color-picker-" + sid);
            if (!colorVal.empty() && colorVal != f.color)
            {
                f.color = colorVal;
                anyFieldChanged = true;
            }
        }

        for (auto& rd : m_state.rounds)
        {
            const std::string sid = std::to_string(rd.id);

            const auto cashStr = m_orch->getInputValue("round-cash-" + sid);
            if (!cashStr.empty())
            {
                try
                {
                    const double v = std::stod(cashStr);
                    if (std::abs(v - rd.cash) > 0.5) { rd.cash = v; anyFieldChanged = true; }

                    // Always refresh the preview span (cheap DOM text update)
                    const std::string preview = fmtCurrency(v);
                    if (auto spanEl = doc->getElementById("round-preview-" + sid))
                    {
                        std::string cur;
                        if (!spanEl->children.empty()) if (auto txt = std::dynamic_pointer_cast<mUI::Parser::DOM::Text>(spanEl->children.front())) cur = txt->data;

                        if (cur != preview) setElemText(doc, spanEl, preview);
                    }
                }
                catch (...) {}
            }

            const auto eqStr = m_orch->getInputValue("round-equity-" + sid);
            if (!eqStr.empty())
            {
                try
                {
                    const float v = std::stof(eqStr);
                    if (std::abs(v - rd.pct) > 0.001f) 
                    { 
                        rd.pct = v; 
                        anyFieldChanged = true; 
                    }
                }
                catch (...) {}
            }
        }

        if (anyFieldChanged)
        {
            m_pendingRecalc     = true;
            m_editSettleCounter = 3;
        }

        if (m_pendingRecalc)
        {
            if (m_editSettleCounter > 0)
            {
                --m_editSettleCounter;
                m_orch->invalidateLayout(); // keep preview spans and totals fresh
            }
            else
            {
                m_pendingRecalc = false;

                // Sync the 'value' attribute on each founder <input> to match m_state.
                // The layout engine reads this attribute when recreating native EDIT
                // HWNDs after a tab switch (pane-founders uses display:none, so its
                // EDIT controls are destroyed when inactive and rebuilt on re-entry).
                // Without this sync the controls revert to the values from the last
                // fullRebuild when the user returns to the Founders tab.
                for (const auto& f : m_state.founders)
                {
                    const std::string sid = std::to_string(f.id);
                    
                    std::ostringstream pss;
                    pss << std::fixed << std::setprecision(2) << f.pct;

                    if (auto el = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById("founder-name-" + sid))) el->setAttribute("value", htmlEncode(f.name));

                    if (auto el = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById("founder-pct-" + sid))) el->setAttribute("value", pss.str());

                    if (auto el = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById("founder-color-picker-" + sid))) el->setAttribute("value", f.color);
                }

                m_lastResult = m_engine.calculate(m_state);
                updateCapTableDOM(m_lastResult, doc);
                updateFounderTotals(doc);
                updateCharts(m_lastResult);

                m_orch->invalidateLayout();
            }
        }
    }

    // ─── onDoubleClick ───────────────────────────────────────────────────────────

    void CapTableApp::onDoubleClick(int x, int y)
    {
        auto orch = m_orch;
        if (!orch) return;

        auto doc = orch->getDocument();
        if (!doc) return;
        
        auto engine = orch->getLayoutEngine();
        if (!engine) return;

        // Scale physical pixels back to logical DIPs.
        float scale = orch->getDpiScale();

        // Use hitTest (which is the correct way to find clicked boxes)
        auto box = engine->hitTest((float)x / scale, (float)y / scale);
        while (box)
        {
            auto node = box->getNode();
            if (node)
            {
                auto el = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(node);
                if (el)
                {
                    std::string id = el->getId();

                    // ── Company Title (Navbar) ────────────────────────────────────
                    if (id == "navbar-title")
                    {
                        auto container = doc->getElementById("navbar-edit-container");
                        auto input = doc->getElementById("navbar-title-input");
                        if (container && input) 
                        {
                            el->setAttribute("style", "display:none;");
                            
                            container->setAttribute("style", "display:flex; flex-direction:row; align-items:center; gap:8px;");
                            
                            input->setAttribute("value", m_state.title);
                            
                            orch->invalidateLayout();

                            return;
                        }
                    }

                    // ── Round Titles (Sidebar) ────────────────────────────────────
                    if (id.find("round-title-view-") == 0)
                    {
                        std::string sid = id.substr(17); // skip "round-title-view-"
                        auto editContainer = doc->getElementById("round-title-edit-container-" + sid);
                        if (editContainer) 
                        {
                            el->setAttribute("style", "display:none;");
                            editContainer->setAttribute("style", "display:flex; align-items:center; gap:8px;");

                            // Pre-fill the input with the current round title from state
                            auto titleInput = doc->getElementById("round-title-input-" + sid);
                            if (titleInput)
                            {
                                for (const auto& r : m_state.rounds)
                                {
                                    if (std::to_string(r.id) == sid)
                                    {
                                        titleInput->setAttribute("value", r.title);
                                        break;
                                    }
                                }
                            }

                            orch->invalidateLayout();
                            
                            return;
                        }
                    }
                }
            }

            box = box->getParent();
        }
    }

    // ─── onKeyEvent ──────────────────────────────────────────────────────────────
    
    void CapTableApp::onKeyEvent(const mUI::Events::eventData& event)
    {
        if (auto keyEventPtr = event.get<mUI::Inputs::KeyEvent>())
        {
            if (keyEventPtr->key == mUI::Inputs::Key::F1 && keyEventPtr->pressed)
            {
                if (m_orch && m_orch->getLayoutEngine()) m_orch->getLayoutEngine()->debugDOM();
            }
        }
    }

    // ─── recalculate ─────────────────────────────────────────────────────────────

    void CapTableApp::recalculate(Doc doc)
    {
        if (!doc && m_orch) doc = m_orch->getDocument();
        
        {
            std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
            m_lastResult = m_engine.calculate(m_state);
        }

        updateCapTableDOM(m_lastResult, doc);
        updateCharts(m_lastResult);

        if (m_orch) m_orch->invalidateLayout();
    }

    // ─── buildRoundCardHTML ──────────────────────────────────────────────────────

    std::string CapTableApp::buildRoundCardHTML(const FundingRound& r, bool showRemove) const
    {
        const std::string sid = std::to_string(r.id);

        std::ostringstream cashOss;
        cashOss << std::fixed << std::setprecision(0) << r.cash;

        std::ostringstream eqOss;
        eqOss << std::fixed << std::setprecision(2) << r.pct;

        const std::string displayTitle = r.title.empty() ? (m_state.defaultRoundName + " ?") : r.title;

        std::ostringstream h;
        h << "<div class=\"card round-card\">";

        if (showRemove) h << "<div class=\"remove-round btn-remove-round-" << sid << " clickable position-absolute\" style=\"top: 15px; right: 15px;\">REMOVE</div>";

        // Capital Raised row
        h << "<div class=\"mb-3\">"
          << "<label class=\"form-label text-muted text-uppercase fw-bold\""
          << " style=\"font-size: 0.7rem; margin-bottom: 4px; display: block;\">"
          << "Capital Raised (" << htmlEncode(m_state.currency) << ")</label>"
          << "<div class=\"d-flex align-items-center gap-2\">"
          << "<input type=\"number\" id=\"round-cash-" << sid
          << "\" class=\"form-control flex-grow-1\" value=\"" << cashOss.str() << "\" step=\"1\" min=\"1000\" max=\"1000000000000\" />"
          << "<span id=\"round-preview-" << sid
          << "\" class=\"round-preview text-muted small fw-bold\">"
          << htmlEncode(fmtCurrency(r.cash)) << "</span>"
          << "</div></div>";

        // Round Name row
        h << "<div class=\"mb-3\">"
          << "<label class=\"form-label text-muted text-uppercase fw-bold\""
          << " style=\"font-size: 0.7rem; margin-bottom: 4px; display: block;\">Round Name</label>"
          << "<div id=\"round-title-view-" << sid
          << "\" class=\"clickable fw-bold text-white\""
          << " style=\"padding: 8px 12px; font-size: 1.1rem; border: 1px solid transparent; width: 100%;\">"
          << htmlEncode(displayTitle) << "</div>"
          << "<div id=\"round-title-edit-container-" << sid
          << "\" style=\"display: none; align-items: center; gap: 8px; width: 100%;\">"
          << "<input type=\"text\" id=\"round-title-input-" << sid
          << "\" class=\"form-control fw-bold text-white flex-grow-1\" value=\""
          << htmlEncode(r.title) << "\" />"
          << "<button class=\"btn btn-sm btn-outline-secondary round-title-done-" << sid
          << "\">\xe2\x9c\x93</button>"   // UTF-8 ✓
          << "<button class=\"btn btn-sm btn-outline-secondary round-title-cancel-" << sid
          << "\" style=\"color: #ff5252;\">\xe2\x9c\x95</button>"   // UTF-8 ✕
          << "</div></div>";

        // Equity Sold row
        h << "<div class=\"mb-0\">"
          << "<label class=\"form-label text-muted text-uppercase fw-bold\""
          << " style=\"font-size: 0.7rem; margin-bottom: 4px; display: block;\">Equity Sold (%)</label>"
          << "<input type=\"number\" id=\"round-equity-" << sid
          << "\" class=\"form-control\" value=\"" << eqOss.str() << "\" step=\"any\" />"
          << "</div>";

        h << "</div>"; // card round-card

        return h.str();
    }

    std::string CapTableApp::buildRoundsHTML() const
    {
        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        const bool multiRound = m_state.rounds.size() > 1;
        
        std::string html;
        for (const auto& r : m_state.rounds) html += buildRoundCardHTML(r, multiRound);

        return html;
    }

    // ─── buildFounderRowHTML / buildFoundersHTML / updateFounderTotals ────────────

    std::string CapTableApp::buildFounderRowHTML(const Founder& f, bool showRemove) const
    {
        const std::string sid = std::to_string(f.id);

        std::ostringstream pctOss;
        pctOss << std::fixed << std::setprecision(2) << f.pct;

        std::ostringstream h;
        h << "<div class=\"founder-row\" id=\"founder-row-" << sid << "\">";

        // Name column
        h << "<div>"
          << "<div class=\"form-label\">Name</div>"
          << "<input type=\"text\" id=\"founder-name-" << sid
          << "\" class=\"form-control\" value=\"" << htmlEncode(f.name) << "\" />"
          << "</div>";

        // Ownership column
        h << "<div>"
          << "<div class=\"form-label\">Initial Ownership (%)</div>"
          << "<input type=\"number\" id=\"founder-pct-" << sid
          << "\" class=\"form-control\" value=\"" << pctOss.str() << "\" step=\"any\" min=\"0\" max=\"100\" />"
          << "</div>";

        // Color picker
        h << "<div class=\"d-flex flex-column align-items-center\">"
          << "<div class=\"form-label\">Color</div>"
          << "<input type=\"color\" id=\"founder-color-picker-" << sid
          << "\" data-color-mode=\"wheel\" data-inherit-color=\"true\" class=\"founder-color-picker\" value=\"" << f.color << "\" />"
          << "</div>";

        // Remove button column
        h << "<div class=\"d-flex flex-column align-items-center\">"
          << "<div class=\"form-label\">\xc2\xa0</div>";  // UTF-8 &nbsp;

        if (showRemove) h << "<button class=\"remove-founder btn-remove-founder-" << sid << " clickable\">\xc3\x97</button>";        // UTF-8 ×
        else h << "<div style=\"width:38px;height:38px;\"></div>";

        h << "</div>";

        h << "</div>"; // founder-row

        return h.str();
    }

    std::string CapTableApp::buildFoundersHTML() const
    {
        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        const bool multiFounder = m_state.founders.size() > 1;

        std::string html;
        for (const auto& f : m_state.founders) html += buildFounderRowHTML(f, multiFounder);

        return html;
    }

    void CapTableApp::updateFounderTotals(Doc doc)
    {
        if (!doc) return;

        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        float totalPct = 0.0f;
        for (const auto& f : m_state.founders) totalPct += f.pct;

        const bool over  = totalPct > 100.0f + 0.005f;
        const bool exact = std::abs(totalPct - 100.0f) < 0.01f;

        std::ostringstream totalOss;
        totalOss << "Total: " << std::fixed << std::setprecision(2) << totalPct << "%";
        setText(doc, "totalSharesLabel", totalOss.str());

        if (auto labelEl = doc->getElementById("totalSharesLabel"))
        {
            std::string cls = "total-shares-display";

            if (over)  cls += " total-over";
            
            if (exact) cls += " total-ok";
            
            labelEl->setAttribute("class", cls);
        }

        std::string hintText;
        if (over)       hintText = "\xe2\x9a\xa0 Exceeds 100%";   // UTF-8 ⚠
        else if (exact) hintText = "\xe2\x9c\x93 Fully allocated"; // UTF-8 ✓
        else
        {
            std::ostringstream h;
            h << std::fixed << std::setprecision(2) << (100.0f - totalPct) << "% remaining";
            hintText = h.str();
        }

        setText(doc, "totalSharesPct", hintText);

        if (auto hintEl = doc->getElementById("totalSharesPct"))
        {
            std::string hintColor = over ? "#ff5252" : exact ? "#00e676" : "#888";
            hintEl->setAttribute("style", "color:" + hintColor + ";");
        }

        const float barWidth = std::min(totalPct, 100.0f);
        const std::string fillColor = over ? "#ff5252" : exact ? "#00e676" : "#4fc3f7";
        
        std::ostringstream barOss;
        barOss << "width:" << std::fixed << std::setprecision(2) << barWidth << "%;background:" << fillColor << ";";
        
        setAttr(doc, "founderTotalFill", "style", barOss.str());
    }

    // ─── updateCapTableDOM ───────────────────────────────────────────────────────

    void CapTableApp::updateCapTableDOM(const CalculationResult& r, Doc doc)
    {
        if (!doc && m_orch) doc = m_orch->getDocument();

        if (!doc) return;

        // ── Summary stats ─────────────────────────────────────────────────────────
        setText(doc, "summary_post", fmtCurrency(r.finalPostMoney));

        std::ostringstream founderPctOss;
        founderPctOss << std::fixed << std::setprecision(2) << r.foundersTotal << "%";
        setText(doc, "summary_founders", founderPctOss.str());

        // ── Cap table body ────────────────────────────────────────────────────────
        // In-place update when the row count matches (always true during live
        // editing since structural changes go through fullRebuild first).
        // Avoids DOM node creation/destruction: the incremental layout engine
        // only runs invalidateAllStyles() on a live-edit tick, which doesn't 
        // create layout boxes for newly-created Element nodes.
        auto tbody = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById("capTableBody"));
        if (tbody)
        {
            if (tbody->children.size() == r.stakes.size())
            {
                // ── In-place: patch text + attributes on existing nodes ────────
                size_t rowIdx = 0;
                for (auto& rowNode : tbody->children)
                {
                    const auto& stake = r.stakes[rowIdx++];

                    auto tr = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(rowNode);
                    if (!tr || tr->children.size() < 4) continue;

                    // Row class (founder highlight)
                    const std::string rowClass = stake.isFounder ? "founder-row-highlight" : "";
                    tr->setAttribute("class", rowClass);

                    // td[0]: [dot_span, text_node]
                    auto tdName = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(tr->children[0]);
                    if (tdName && tdName->children.size() >= 2)
                    {
                        if (auto dot = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(tdName->children[0])) dot->setAttribute("style", "background:" + stake.color + ";");

                        if (auto txt = std::dynamic_pointer_cast<mUI::Parser::DOM::Text>(tdName->children[1])) if (txt->data != " " + stake.name) txt->setNodeValue(" " + stake.name);
                    }

                    // td[1]: badge span with percentage text
                    auto tdPct = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(tr->children[1]);
                    if (tdPct && !tdPct->children.empty())
                    {
                        if (auto badge = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(tdPct->children[0]))
                        {
                            std::ostringstream pOss;
                            pOss << std::fixed << std::setprecision(2) << stake.ownershipPct << "%";

                            setElemText(doc, badge, pOss.str());
                        }
                    }

                    // td[2]: value
                    auto tdVal = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(tr->children[2]);
                    if (tdVal) setElemText(doc, tdVal, fmtCurrency(stake.value));

                    // td[3]: status
                    auto tdStatus = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(tr->children[3]);
                    if (tdStatus) setElemText(doc, tdStatus, stake.isFounder ? "Founder" : "Investor");
                }
            }
            else
            {
                // ── Structural fallback: full clear + rebuild ──────────────────
                // Safety net — normally fullRebuild() is called before stake
                // count changes (add/remove founder or round).
                while (!tbody->children.empty()) tbody->removeChild(tbody->children.front());

                for (const auto& stake : r.stakes)
                {
                    auto tr = doc->createElement("tr");
                    if (stake.isFounder) tr->classList_add("founder-row-highlight");

                    auto tdName = doc->createElement("td");
                    auto dot = doc->createElement("span");
                    dot->setAttribute("class", "color-dot");
                    dot->setAttribute("style", "background:" + stake.color + ";");
                    tdName->appendChild(dot);
                    tdName->appendChild(doc->createTextNode(" " + stake.name));
                    tr->appendChild(tdName);

                    auto tdPct = doc->createElement("td");
                    tdPct->setAttribute("class", "text-center");
                    std::ostringstream pOss;
                    pOss << std::fixed << std::setprecision(2) << stake.ownershipPct << "%";
                    auto badge = doc->createElement("span");
                    badge->setAttribute("class", "badge-percentage");
                    badge->appendChild(doc->createTextNode(pOss.str()));
                    tdPct->appendChild(badge);
                    tr->appendChild(tdPct);

                    auto tdVal = doc->createElement("td");
                    tdVal->setAttribute("class", "text-end");
                    tdVal->appendChild(doc->createTextNode(fmtCurrency(stake.value)));
                    tr->appendChild(tdVal);

                    auto tdStatus = doc->createElement("td");
                    tdStatus->setAttribute("class", "text-end");
                    tdStatus->appendChild(doc->createTextNode(stake.isFounder ? "Founder" : "Investor"));
                    tr->appendChild(tdStatus);

                    tbody->appendChild(tr);
                }
            }
        }

        // ── Pre-money grid ────────────────────────────────────────────────────────
        auto preMoneyEl = doc->getElementById("preMoneyContainer");
        if (preMoneyEl)
        {
            if (preMoneyEl->children.size() == r.roundResults.size())
            {
                // ── In-place: update label + value text ───────────────────────
                size_t colIdx = 0;
                for (auto& colNode : preMoneyEl->children)
                {
                    const auto& rr = r.roundResults[colIdx++];

                    auto col = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(colNode);
                    if (!col || col->children.size() < 2) continue;

                    // children[0] = summary-label div
                    auto labelEl = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(col->children[0]);
                    if (labelEl) setElemText(doc, labelEl, rr.roundTitle + " Pre-Money");

                    // children[1] = h6 value div
                    auto valEl = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(col->children[1]);
                    if (valEl) setElemText(doc, valEl, fmtCurrency(rr.preMoney));
                }
            }
            else
            {
                // ── Structural fallback ───────────────────────────────────────
                while (!preMoneyEl->children.empty()) preMoneyEl->removeChild(preMoneyEl->children.front());

                for (const auto& rr : r.roundResults)
                {
                    auto col = doc->createElement("div");
                    col->setAttribute("class", "col");

                    auto label = doc->createElement("div");
                    label->setAttribute("class", "summary-label");
                    label->appendChild(doc->createTextNode(rr.roundTitle + " Pre-Money"));
                    col->appendChild(label);

                    auto val = doc->createElement("div");
                    val->setAttribute("class", "h6 fw-bold text-white");
                    val->appendChild(doc->createTextNode(fmtCurrency(rr.preMoney)));
                    col->appendChild(val);

                    preMoneyEl->appendChild(col);
                }
            }
        }
    }

    // ─── updateCharts ────────────────────────────────────────────────────────────

    void CapTableApp::updateCharts(const CalculationResult& r)
    {
        if (!m_orch) return;

        auto doc = m_orch->getDocument();

        // ── ownershipChart — per-stakeholder ownership split (Pie/Doughnut) ──────
        {
            auto chart = m_orch->getChart("ownershipChart");
            if (chart)
            {
                chart->configureStyle([](ChartStyle& s) 
                {
                    s.showLegend  = true;
                    s.showTooltip = true;
                });

                ChartData data;
                data.title = "Ownership Distribution";

                for (const auto& stake : r.stakes)
                {
                    ChartSeries s;
                    s.label  = stake.name;
                    s.values = { stake.ownershipPct };

                    uint32_t rgb = 0x666666;
                    try
                    {
                        std::string c = stake.color;
                        if (!c.empty() && c[0] == '#') c = c.substr(1);

                        rgb = std::stoul(c, nullptr, 16) & 0x00FFFFFF;
                    }
                    catch (...) {}

                    s.color =
                    {
                        ((rgb >> 16) & 0xFF) / 255.0f,
                        ((rgb >>  8) & 0xFF) / 255.0f,
                        ( rgb        & 0xFF) / 255.0f,
                        1.0f
                    };

                    data.series.push_back(s);
                }

                chart->setData(data);
            }
        }

        // ── ownershipLegend — shareholder breakdown list ────────────────────────
        if (doc)
        {
            auto legendEl = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(doc->getElementById("ownershipLegend"));
            if (legendEl)
            {
                while (!legendEl->children.empty()) legendEl->removeChild(legendEl->children.front());

                for (const auto& stake : r.stakes)
                {
                    std::ostringstream pctOss;
                    pctOss << std::fixed << std::setprecision(2) << stake.ownershipPct << "%";

                    auto row = doc->createElement("div");
                    row->setAttribute("class", "d-flex align-items-center gap-2");

                    auto dot = doc->createElement("span");
                    dot->setAttribute("class", "color-dot");
                    dot->setAttribute("style", "background:" + stake.color + "; flex-shrink:0;");
                    row->appendChild(dot);

                    auto nameSpan = doc->createElement("span");
                    nameSpan->setAttribute("style", "color:var(--text-muted); font-size:0.85rem; flex:1; overflow:hidden; text-overflow:ellipsis;");
                    nameSpan->appendChild(doc->createTextNode(stake.name));
                    row->appendChild(nameSpan);

                    auto pctSpan = doc->createElement("span");
                    pctSpan->setAttribute("style", "font-weight:600; color:var(--accent-secondary); min-width:52px; text-align:right;");
                    pctSpan->appendChild(doc->createTextNode(pctOss.str()));
                    row->appendChild(pctSpan);

                    legendEl->appendChild(row);
                }
            }
        }

        // ── Colour helper (same pattern as above, avoids repeating 4 lines) ─────
        const auto hexToColorArr = [](const std::string& hex) -> std::array<float, 4>
        {
            uint32_t rgb = 0x888888;
            try
            {
                std::string s = hex;
                if (!s.empty() && s[0] == '#') s = s.substr(1);

                rgb = std::stoul(s, nullptr, 16) & 0x00FFFFFFu;
            }
            catch (...) {}

            return { ((rgb >> 16) & 0xFF) / 255.f, ((rgb >>  8) & 0xFF) / 255.f, ( rgb & 0xFF) / 255.f, 1.f };
        };

        // ── founderPieChart — initial ownership split (Doughnut) ─────────────────
        {
            auto chart = m_orch->getChart("founderPieChart");
            if (chart)
            {
                chart->configureStyle([](ChartStyle& s) 
                {
                    s.showLegend  = true;
                    s.showTooltip = true;
                });

                ChartData data;
                data.title = "Initial Ownership";

                for (const auto& f : m_state.founders)
                {
                    ChartSeries s;
                    s.label  = f.name;
                    s.values = { f.pct };

                    auto c   = hexToColorArr(f.color);
                    s.color  = { c[0], c[1], c[2], c[3] };
                    data.series.push_back(s);
                }

                chart->setData(data);
            }
        }

        // ── founderValueChart — per-founder stake value progression (Line) ────────
        {
            auto chart = m_orch->getChart("founderValueChart");
            if (chart)
            {
                chart->configureStyle([](ChartStyle& s) 
                {
                    s.showLegend    = true;
                    s.showTooltip   = true;
                    s.tooltipMode   = mUI::Widgets::TooltipMode::Nearest;
                    s.beginAtZero   = true;
                    s.dotRadius     = 5.f;
                });

                ChartData data;
                data.title = "Stake Value Progression";

                for (size_t i = 0; i < m_state.founders.size(); ++i)
                {
                    ChartSeries s;
                    s.label   = m_state.founders[i].name;
                    auto c    = hexToColorArr(m_state.founders[i].color);
                    s.color   = { c[0], c[1], c[2], c[3] };
                    s.tension = 0.4f;

                    for (const auto& pt : r.progression) s.values.push_back(i < pt.founderValues.size() ? static_cast<float>(pt.founderValues[i]) : 0.f);

                    data.series.push_back(s);
                }

                for (const auto& pt : r.progression) data.labels.push_back(pt.label);

                chart->setData(data);
            }
        }

        // ── founderQuotaChart — ownership % by round (Line) ──────────────────────
        {
            auto chart = m_orch->getChart("founderQuotaChart");

            if (chart)
            {
                chart->configureStyle([](ChartStyle& s) 
                {
                    s.showLegend    = true;
                    s.showTooltip   = true;
                    s.tooltipMode   = mUI::Widgets::TooltipMode::Nearest;
                    s.beginAtZero   = true;
                    s.yAxisSuffix   = "%";
                    s.dotRadius     = 5.f;
                });

                ChartData data;
                data.title = "Ownership % by Round";

                for (size_t i = 0; i < m_state.founders.size(); ++i)
                {
                    ChartSeries s;
                    s.label = m_state.founders[i].name;

                    auto c  = hexToColorArr(m_state.founders[i].color);
                    s.color = { c[0], c[1], c[2], c[3] };

                    for (const auto& pt : r.progression) s.values.push_back(i < pt.founderQuotaPcts.size() ? pt.founderQuotaPcts[i] : 0.f);

                    data.series.push_back(s);
                }

                for (const auto& pt : r.progression) data.labels.push_back(pt.label);

                chart->setData(data);
            }
        }

        // ── valuationChart — post-money valuation per round (Line) ───────────────
        {
            auto chart = m_orch->getChart("valuationChart");
            if (chart)
            {
                // White curved line, green area fill + green dots, Foundation origin
                chart->configureStyle([](ChartStyle& s) 
                {
                    s.showLegend  = false;
                    s.showTooltip = true;
                    s.tooltipMode = mUI::Widgets::TooltipMode::Nearest;
                    s.beginAtZero = true;
                    s.lineWidth   = 2.5f;
                    s.dotRadius   = 7.f;
                });

                ChartData data;
                data.title = "Valuation Growth";

                ChartSeries s;
                s.label       = "Post-Money";
                s.color       = { 0.0f, 0.902f, 0.463f, 1.0f };  // #00e676 — fill + dot colour
                s.borderColor = { 1.0f, 1.0f,  1.0f,  1.0f };    // white line
                s.fill        = true;
                s.fillAlpha   = 0.18f;
                s.tension     = 0.4f;
                s.pointStyle  = PointStyle::Circle;
                s.pointRadius = 7.f;

                // Prepend the Foundation origin (pre-round baseline = 0)
                s.values.push_back(0.f);
                data.labels.push_back("Foundation");

                for (const auto& rr : r.roundResults)
                {
                    s.values.push_back(static_cast<float>(rr.postMoney));
                    data.labels.push_back(rr.roundTitle);
                }

                data.series.push_back(s);
                chart->setData(data);
            }
        }

        // ── dilutionChart — per-stakeholder ownership % by round (Stacked Bar) ──
        {
            auto chart = m_orch->getChart("dilutionChart");
            if (chart)
            {
                chart->configureStyle([](ChartStyle& s) {
                    s.stackedBars        = true;
                    s.beginAtZero        = true;
                    s.showLegend         = true;
                    s.categoryPercentage = 0.6f;
                    s.showTooltip        = true;
                    s.yAxisSuffix        = "%";
                    s.gridLineCount      = 10;
                    s.yMax               = 100.f;
                    s.gridColor          = { 51.f/255.f, 51.f/255.f, 51.f/255.f, 1.f };  // grid.color: #333333
                });

                ChartData data;
                data.title = "Dilution Progression";

                // ── Founder series ────────────────────────────────────────────
                for (size_t i = 0; i < m_state.founders.size(); ++i)
                {
                    ChartSeries s;
                    s.label = m_state.founders[i].name;
                    auto c  = hexToColorArr(m_state.founders[i].color);
                    s.color = { c[0], c[1], c[2], c[3] };

                    for (const auto& pt : r.progression) s.values.push_back(i < pt.founderQuotaPcts.size() ? pt.founderQuotaPcts[i] : 0.f);

                    data.series.push_back(s);
                }

                // ── Per-round investor series ─────────────────────────────────
                // Color comes directly from FundingRound::color (set at creation)
                const size_t nPts = r.progression.size();

                for (size_t ri = 0; ri < m_state.rounds.size(); ++ri)
                {
                    ChartSeries inv;
                    inv.label = m_state.rounds[ri].title;

                    // Use the color stored on the round (distinct shade per round)
                    const auto col = hexToColorArr(m_state.rounds[ri].color);
                    inv.color = { col[0], col[1], col[2], 1.f };

                    for (size_t j = 0; j < nPts; ++j)
                    {
                        // progression[0]    = Foundation (before any rounds)
                        // progression[ri+1] = point where round ri investor first appears
                        if (j <= ri)
                        {
                            inv.values.push_back(0.f);
                        }
                        else
                        {
                            // Equity at founding round ri, diluted by all subsequent rounds up to point j
                            float equity = m_state.rounds[ri].pct;
                            for (size_t k = ri + 1; k < j; ++k) equity *= (1.f - m_state.rounds[k].pct / 100.f);

                            inv.values.push_back(equity);
                        }
                    }

                    data.series.push_back(inv);
                }

                for (const auto& pt : r.progression) data.labels.push_back(pt.label);

                chart->setData(data);
            }
        }
    }

    // ─── exportToFile ────────────────────────────────────────────────────────────

    void CapTableApp::exportToFile()
    {
        readStateFromDOM();

        auto dlg = mUI::widgets->createFileDialog();
        if (!dlg) return;

        dlg->saveFile({{ "Cap Table JSON (*.json)", "*.json" }}, "json",
        [this, dlg](const std::string& path)
        {
            if (path.empty()) return;

            bool ok = false;

            {
                std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
                ok = m_serializer.saveToFile(path, m_state);
            }

            // Extract filename for a short, non-wrapping message
            const std::string fname = path.substr(path.find_last_of("/\\") + 1);

            auto msgDlg = mUI::widgets->createMessageDialog();
            if (!msgDlg) return;

            mUI::MessageDialogConfig cfg;
                        
            if (ok)
            {
                cfg.title    = "Saved";
                cfg.message  = "Exported to: " + fname;
                cfg.iconPath = "\xe2\x9c\x93"; // UTF-8 ✓
            }
            else
            {
                cfg.title    = "Export Failed";
                cfg.message  = "Could not write: " + fname;
                cfg.iconPath = "\xe2\x9a\xa0"; // UTF-8 ⚠
            }

            cfg.buttons = {{ "OK", mUI::MessageDialogResult::OK }};

            // Capture msgDlg to keep the dialog object alive until the user dismisses it.
            // Without this capture the shared_ptr goes out of scope before OK is clicked,
            // destroying the Win32 resources and causing desync / lag in the main window.
            msgDlg->show(cfg, [this, msgDlg](mUI::MessageDialogResult)
            {
                (void)msgDlg; // keep alive
                if (m_orch) m_orch->invalidateLayout();
            });
        });
    }

    void CapTableApp::importFromFile()
    {
        auto dlg = mUI::widgets->createFileDialog();
        if (!dlg) return;

        dlg->openFile({{ "Cap Table JSON (*.json)", "*.json" }},
        [this, dlg](const std::string& path)
        {
            if (path.empty()) return;

            CapTableState loaded;
            if (!m_serializer.loadFromFile(path, loaded))
            {
                const std::string fname = path.substr(path.find_last_of("/\\") + 1);
                auto msgDlg = mUI::widgets->createMessageDialog();
                if (msgDlg)
                {
                    mUI::MessageDialogConfig cfg;
                    cfg.title    = "Import Failed";
                    cfg.message  = "Could not parse: " + fname;
                    cfg.iconPath = "\xe2\x9a\xa0"; // UTF-8 ⚠
                    cfg.buttons  = {{ "OK", mUI::MessageDialogResult::OK }};

                    // Capture msgDlg — same lifetime fix as in exportToFile.
                    msgDlg->show(cfg, [this, msgDlg](mUI::MessageDialogResult)
                    {
                        (void)msgDlg; // keep alive
                        if (m_orch) m_orch->invalidateLayout();
                    });
                }
                return;
            }

            {
                // Defer state update to main thread in onUpdate() 
                // to avoid race against Hover/UI events in the mUI orchestrator.
                std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
                m_pendingImportState = std::move(loaded);
                m_hasPendingImport   = true;
            }
        });
    }

    // ─── bindColorPickers ────────────────────────────────────────────────────────

    /// Re-parse the HTML/CSS document from scratch so every input control is seeded
    /// from its `value` attribute during the initial layout pass.
    ///
    /// Rounds and founders are pre-injected as HTML strings so that Win32 EDIT
    /// controls (which ignore post-parse setAttribute calls) receive the correct
    /// values — they are only seeded at parse time from the static HTML.
    void CapTableApp::bindColorPickers(Doc doc)
    {
        if (!m_orch) return;

        if (!doc) doc = m_orch->getDocument();

        if (!doc) return;

        std::lock_guard<std::recursive_mutex> lock(m_stateMutex);

        for (const auto& f : m_state.founders)
        {
            const std::string sid = std::to_string(f.id);
            auto picker = m_orch->getColorPicker("founder-color-picker-" + sid);
            if (picker)
            {
                picker->setOnChange([this, sid](uint32_t newColor)
                {
                    std::lock_guard<std::recursive_mutex> lock(m_stateMutex);
                    auto doc = m_orch->getDocument();

                    for (auto& f : m_state.founders)
                    {
                        if (std::to_string(f.id) == sid)
                        {
                            f.color = colorToHex(newColor);
                            break;
                        }
                    }

                    // Re-calculate and push to cap table so the color dot in the
                    // ownership structure row updates immediately.
                    if (doc)
                    {
                        m_lastResult = m_engine.calculate(m_state);
                        updateCapTableDOM(m_lastResult, doc);
                        m_orch->invalidateLayout();
                    }
                });
            }
        }
    }

    // ─── fullRebuild ─────────────────────────────────────────────────────────────

    void CapTableApp::fullRebuild()
    {
        if (!m_orch) return;

        // 1. Inject dynamic content into the HTML template before parsing.
        std::string html = HTML;

        const std::string roundsMark = "<!-- Dynamic Rounds -->";

        auto rp = html.find(roundsMark);
        if (rp != std::string::npos) html.replace(rp, roundsMark.size(), buildRoundsHTML());

        const std::string foundersEmpty = "id=\"foundersContainer\" class=\"d-flex flex-column gap-2\"></div>";

        auto fp = html.find(foundersEmpty);
        if (fp != std::string::npos) html.replace(fp, foundersEmpty.size(), "id=\"foundersContainer\" class=\"d-flex flex-column gap-2\">" + buildFoundersHTML() + "</div>");

        // 2. Parse the complete document.  The callback only handles static DOM
        //    restoration — input values are already baked into the HTML above.
        m_orch->prepareDocument(html, CSS, [this](Doc document)
        {
            if (!document) return;

            // Restore static DOM state from m_state
            setText(document, "navbar-title", m_state.title);
            setAttr(document, "navbar-title-input", "value", m_state.title);
            setAttr(document, "defaultRoundName",   "value", m_state.defaultRoundName);

            // Currency selector: mark the matching <option> as selected
            if (auto sel = document->getElementById("currencySelector"))
            {
                for (auto& child : sel->children)
                {
                    auto opt = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(child);
                    if (!opt) continue;

                    if (opt->getAttribute("value") == m_state.currency) opt->setAttribute("selected", "selected");
                    else opt->removeAttribute("selected");
                }
            }

            // Active tab restore
            for (const auto& t : m_tabSet)
            {
                bool active = (m_activeTabId == t.key);

                auto tabEl  = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(document->getElementById(t.tabId));
                auto paneEl = std::dynamic_pointer_cast<mUI::Parser::DOM::Element>(document->getElementById(t.paneId));
                
                if (tabEl)  tabEl->setAttribute("class",  active ? "tab-btn active " + std::string(t.tabId) : "tab-btn " + std::string(t.tabId));
                
                if (paneEl) paneEl->setAttribute("class", active ? "tab-pane active" : "tab-pane");
            }

            // Founder totals bar (DOM elements exist in the static HTML)
            updateFounderTotals(document);

            // Recalculate and push all results to cap table DOM + charts
            recalculate(document);

            m_initialized = true;

            // Reset binding ticks to wait for layout to settle
            m_pendingBindTicks = 2;
            
        });
    }

    // ─── Utility ──────────────────────────────────────────────────────────────────

    std::string CapTableApp::fmtCurrency(double amount) const
    {
        std::ostringstream oss;

        if (amount >= 1'000'000'000.0) oss << m_state.currency << std::fixed << std::setprecision(2) << (amount / 1'000'000'000.0) << "B";
        else if (amount >= 1'000'000.0) oss << m_state.currency << std::fixed << std::setprecision(2) << (amount / 1'000'000.0) << "M";
        else if (amount >= 1'000.0) oss << m_state.currency << std::fixed << std::setprecision(1) << (amount / 1'000.0) << "K";
        else oss << m_state.currency << std::fixed << std::setprecision(2) << amount;

        return oss.str();
    }

    std::string CapTableApp::colorToHex(uint32_t rgb) const
    {
        // Input: 0x00RRGGBB
        std::ostringstream oss;

        oss << '#'
            << std::hex << std::uppercase << std::setfill('0')
            << std::setw(2) << ((rgb >> 16) & 0xFF)
            << std::setw(2) << ((rgb >>  8) & 0xFF)
            << std::setw(2) << ((rgb      ) & 0xFF);

        return oss.str();
    }

} // namespace mUI::Demo::CapTable
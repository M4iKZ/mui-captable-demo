#pragma once

// CSS for the Cap Table demo — Premium dark-mode with Outfit font.
// Re-aligned with the original design specs for maximum visual fidelity.

namespace mUI::Demo::CapTable
{
    inline constexpr char CSS[] = 
R"css(:root {
    --bg-color: #050505;
    --card-bg: #121212;
    --card-hover: #1a1a1a;
    --accent-primary: #ffffff;
    --accent-secondary: #00e676;
    --text-main: #ffffff;
    --text-muted: #888888;
    --border-color: #2a2a2a;
    --input-bg: #1a1a1a;
}

* { box-sizing: border-box; }
*, ::after, ::before { box-sizing: border-box; }


body {
    background-color: var(--bg-color);
    color: var(--text-main);
    font-family: 'Outfit', sans-serif;
    margin: 0; padding: 0;
    overflow-x: hidden;
    min-height: 100vh;
    padding-bottom: 25px;
}

.container-fluid { width: 100%; padding: 0 24px; }

.row {
    display: flex !important;
    margin: 0 -12px;
}

.col-12 { flex: 0 0 100%; max-width: 100%; padding: 0 12px; }
.col-lg-4 { flex: 0 0 33.333333%; max-width: 33.333333%; padding: 0 12px; }
.col-lg-8 { flex: 0 0 66.666667%; max-width: 66.666667%; padding: 0 12px; }
.col-auto { flex: 0 0 auto; width: auto; padding: 0 12px; }

/* ─────────────────────────────────────────────────────────────────────────────
   Navigation
   ───────────────────────────────────────────────────────────────────────────── */
.navbar {
    background: rgba(10, 10, 10, 0.9);
    backdrop-filter: blur(10px);
    border-bottom: 1px solid var(--border-color);
    margin-bottom: 40px;
    padding: 0px 25px;
    position: sticky; top: 0; z-index: 1000;
}

.navbar-edit-input {
    width: 200px; height: 36px;
    background-color: var(--input-bg); border: 1px solid var(--border-color);
    color: var(--text-main); border-radius: 6px;
    padding: 4px 10px; font-family: inherit; outline: none;
    font-size: 1rem; font-weight: 700;
}

.nav-brand {
    font-weight: 700; font-size: 1.5rem;
    background: linear-gradient(135deg, #ffffff, #888888);
    -webkit-background-clip: text; -webkit-text-fill-color: transparent;
    letter-spacing: -0.01em;
}

.nav {
    display: flex !important; flex-direction: row !important;
    gap: 16px !important; list-style: none !important; padding: 0; margin-bottom: 24px;
    border-bottom: 1px solid var(--border-color);
}

.nav-item { list-style: none !important; margin: 0; }

.tab-btn {
    background: transparent; border: 1px solid transparent !important;
    color: var(--text-muted); font-weight: 500; padding: 10px 20px;
    border-radius: 6px; cursor: pointer; font-family: inherit;
    transition: all 0.2s ease;
}

.tab-btn:hover { color: #ffffff; background: rgba(255, 255, 255, 0.05); }
.tab-btn.active { background-color: #ffffff !important; color: #000000 !important; }

.tab-pane, .valuationChart, .dilutionChart, .founderPieChart, .founderValueChart, .founderQuotaChart, .ownershipChart { display: none !important; }
.tab-pane.active { display: block !important; }

#navbar-edit-container { display: none; }

/* ─────────────────────────────────────────────────────────────────────────────
   Cards & Elements
   ───────────────────────────────────────────────────────────────────────────── */
.card {
    background: var(--card-bg); border: 1px solid var(--border-color);
    border-radius: 12px; box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
    padding: 24px; margin-bottom: 20px;
    display: flex !important; flex-direction: column !important;
}

.h6 { font-size: 0.9rem; font-weight: 700; margin-top: 5px; }
.col { flex: 1; padding: 0 10px; }

.round-card { position: relative; }

.card:hover { border-color: #444444; background: var(--card-hover); }

.card-title {
    color: var(--accent-primary); font-weight: 600; margin-bottom: 20px;
    font-size: 1rem; text-transform: uppercase; letter-spacing: 0.1em;
}

.summary-label {
    font-size: 0.7rem; color: #888; text-transform: uppercase;
    letter-spacing: 0.08em; font-weight: 700; margin-bottom: 4px;
}

.summary-value {
    font-size: 1.4rem; font-weight: 700; color: #fff;
    white-space: nowrap !important;
}

.form-label { 
    color: var(--text-muted); font-weight: 500; margin-bottom: 8px; font-size: 0.85rem; 
    display: block; 
}

.form-control {
    background-color: #1a1a1a !important; border: 1px solid #2a2a2a;
    color: #ffffff !important; border-radius: 6px; padding: 10px 14px;
    width: 100%; font-family: inherit; outline: none;
}

.text-white { color: #ffffff !important; }
.fw-bold { font-weight: 700 !important; }
.mb-3 { margin-bottom: 1rem !important; }

/* ─────────────────────────────────────────────────────────────────────────────
   Founders Tab Style
   ───────────────────────────────────────────────────────────────────────────── */
.founder-row {
    display: flex !important;
    align-items: flex-start;
    gap: 16px;
    padding: 16px;
    background: #1a1a1a;
    border: 1px solid var(--border-color);
    border-radius: 8px;
    margin-bottom: 12px;
    width: 100%;
    transition: border-color 0.15s ease;
}
.founder-row > div { flex: 1; }
.founder-row > div:nth-child(3), .founder-row > div:nth-child(4) { flex: 0 0 auto; }
.founder-row:hover { border-color: #444444; }

.founder-color-picker {
    width: 36px;
    height: 36px;
    cursor: pointer;
}

.total-shares-display { display: inline-block; font-weight: 700; font-size: 0.9rem; }
.total-over { color: #ff5252 !important; }
.total-ok { color: #00e676 !important; }

.remove-founder {
    width: 38px; height: 38px; border-radius: 6px;
    background: transparent; border: 1px solid #333;
    color: #ff5252; cursor: pointer; font-size: 1.25rem;
    display: flex; align-items: center; justify-content: center;
    transition: all 0.2s; 
}
.remove-founder:hover { background: rgba(255, 82, 82, 0.1); border-color: #ff5252; }

.remove-round {
    position: absolute !important;
    top: 15px !important;
    right: 15px !important;
    color: #ff5252; cursor: pointer; opacity: 0.6;
    transition: all 0.2s; font-size: 0.75rem;
    text-transform: uppercase; font-weight: 800;
    z-index: 100; padding: 5px;
}
.remove-round:hover { opacity: 1; text-decoration: underline; }

button {
    border-radius: 0;
}

button, input, optgroup, select, textarea {
    margin: 0;
    font-family: inherit;
    font-size: inherit;
    line-height: inherit;
}

button, select {
    text-transform: none;
}

[type=button], [type=reset], [type=submit], button {
    -webkit-appearance: button;
}

[type=button]:not(:disabled), [type=reset]:not(:disabled), [type=submit]:not(:disabled), button:not(:disabled) {
    cursor: pointer;
}

.btn {
    --bs-btn-padding-x: 0.75rem;
    --bs-btn-padding-y: 0.375rem;
    --bs-btn-font-family: ;
    --bs-btn-font-size: 1rem;
    --bs-btn-font-weight: 400;
    --bs-btn-line-height: 1.5;
    --bs-btn-color: #fff;
    --bs-btn-bg: transparent;
    --bs-btn-border-width: 1px;
    --bs-btn-border-color: transparent;
    --bs-btn-border-radius: 0.375rem;
    --bs-btn-hover-border-color: transparent;
    --bs-btn-box-shadow: inset 0 1px 0 rgba(255, 255, 255, 0.15), 0 1px 1px rgba(0, 0, 0, 0.075);
    --bs-btn-disabled-opacity: 0.65;
    --bs-btn-focus-box-shadow: 0 0 0 0.25rem rgba(13, 110, 253, .5);
    display: inline-block;
    padding: var(--bs-btn-padding-y) var(--bs-btn-padding-x);
    font-family: var(--bs-btn-font-family);
    font-size: var(--bs-btn-font-size);
    font-weight: var(--bs-btn-font-weight);
    line-height: var(--bs-btn-line-height);
    color: var(--bs-btn-color);
    text-align: center;
    text-decoration: none;
    vertical-align: middle;
    cursor: pointer;
    -webkit-user-select: none;
    -moz-user-select: none;
    user-select: none;
    border: var(--bs-btn-border-width) solid var(--bs-btn-border-color);
    border-radius: var(--bs-btn-border-radius);
    background-color: var(--bs-btn-bg);
    transition: color .15s ease-in-out, background-color .15s ease-in-out, border-color .15s ease-in-out, box-shadow .15s ease-in-out;
}

)css"
R"css(

.btn-add-round, .btn-add-founder {
    width: 100%; border: 2px dashed #333333; background: transparent; color: #888;
    padding: 20px; border-radius: 12px; transition: all 0.2s; font-weight: 600;
    margin-top: 10px;
}
.btn-add-round:hover, .btn-add-founder:hover {
    border-color: #ffffff; color: #ffffff; background: rgba(255, 255, 255, 0.05);
}

.round-default-name-row {
    display: flex; align-items: center; gap: 12px; padding: 10px 0; width: 100%;
}
.round-default-name-row label {
    font-size: 0.78rem; color: #888; text-transform: uppercase;
    letter-spacing: 0.06em; white-space: nowrap; font-weight: 600;
}
.round-default-name-row input {
    flex: 1; background: #111; border: 1px solid #333; color: #eee;
    border-radius: 6px; padding: 8px 12px; font-size: 0.9rem;
}

.btn-group-sm>.btn, .btn-sm {
    --bs-btn-padding-y: 0.25rem;
    --bs-btn-padding-x: 0.5rem;
    --bs-btn-font-size: 0.875rem;
    --bs-btn-border-radius: 0.25rem;
}

.btn-outline-secondary { background: transparent; border: 1px solid #333333; color: #888888; }
.btn-outline-secondary:hover { color: #ffffff; border-color: #888888; }

.badge-mini {
    background: #ffffff !important;
    color: #000000 !important;
    padding: 2px 10px !important;
    border-radius: 12px !important;
    font-weight: 900 !important;
    font-size: 0.65rem !important;
    text-transform: uppercase;
    letter-spacing: 0.05em;
    display: inline-block;
    vertical-align: middle;
    white-space: nowrap !important;
}

/* ─────────────────────────────────────────────────────────────────────────────
   Tables & Data Structure
   ───────────────────────────────────────────────────────────────────────────── */
.table { width: 100%; border-collapse: collapse; }
.table th {
    color: var(--text-muted); font-size: 0.8rem; text-transform: uppercase;
    letter-spacing: 0.05em; font-weight: 600; padding: 15px 10px;
    border-bottom: 1px solid var(--border-color); text-align: left;
    white-space: nowrap !important;
}
.table td { 
    padding: 15px 10px; border-bottom: 1px solid var(--border-color); 
    white-space: nowrap !important;
    vertical-align: middle;
}

.table-hover tbody tr:hover td { background: rgba(255, 255, 255, 0.04); }
.table-hover tbody tr.founder-row-highlight:hover td { background: rgba(108, 99, 255, 0.08) !important; }

.badge-percentage {
    background: #222222;
    color: #ffffff;
    font-weight: 600;
    padding: 4px 10px;
    border-radius: 4px;
    border: 1px solid #333333;
    display: inline-block;
    vertical-align: middle;
}

/* ─────────────────────────────────────────────────────────────────────────────
   Bootstrap & Layout Utilities
   ───────────────────────────────────────────────────────────────────────────── */
.row { display: flex !important; flex-wrap: wrap; margin-right: -15px; margin-left: -15px; width: calc(100% + 30px); }
.col-lg-4 { flex: 0 0 33.333333%; max-width: 33.333333%; padding: 0 15px; }
.col-lg-6 { flex: 0 0 50%; max-width: 50%; padding: 0 15px; }
.col-lg-8 { flex: 0 0 66.666667%; max-width: 66.666667%; padding: 0 15px; }
.col-lg-12 { flex: 0 0 100%; max-width: 100%; padding: 0 15px; }

.table { width: 100%; border-collapse: collapse; table-layout: fixed; }
.table th, .table td { overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }

.d-flex { display: flex !important; }
.flex-column { flex-direction: column !important; }
.align-items-center { align-items: center !important; }
.justify-content-between { justify-content: space-between !important; }
.gap-2 { gap: 8px !important; }
.gap-3 { gap: 16px !important; }
.mt-3 { margin-top: 16px !important; }
.mt-4 { margin-top: 24px !important; }
.mb-2 { margin-bottom: 8px !important; }
.mb-3 { margin-bottom: 16px !important; }
.p-3 { padding: 16px !important; }
.pt-4 { padding-top: 24px !important; }
.border-top { border-top: 1px solid var(--border-color); }
.border-white { border-color: rgba(255,255,255,0.1); }

.color-dot { display: inline-block; width: 10px; height: 10px; border-radius: 50%; margin-right: 10px; }
.color-preview { width: 100%; height: 100%; border-radius: 4px; }

.chart-container { height: 300px; width: 100%; position: relative; }
.founder-total-bar { height: 8px; background: #1a1a1a; border-radius: 4px; overflow: hidden; margin-top: 8px; }
.founder-total-fill { height: 100%; transition: width 0.3s; }

/* ─────────────────────────────────────────────────────────────────────────────
   Utilities
   ───────────────────────────────────────────────────────────────────────────── */
.text-end { text-align: right; }
.text-center { text-align: center; }
.d-flex { display: flex !important; }
.flex-column { flex-direction: column !important; }
.flex-row { flex-direction: row !important; }
.align-items-center { align-items: center !important; }
.align-items-end { align-items: flex-end !important; }
.justify-content-between { justify-content: space-between !important; }
.gap-2 { gap: 8px !important; }
.gap-3 { gap: 16px !important; }
.gap-4 { gap: 24px !important; }
.w-100 { width: 100% !important; }
.flex-grow-1 { flex-grow: 1 !important; }
.mt-4 { margin-top: 1.5rem !important; }
.mb-4 { margin-bottom: 1.5rem !important; }
.pt-4 { padding-top: 1.5rem !important; }
.rounded-pill {
    border-radius: 50rem !important;
}
.bg-white {
    --bs-bg-opacity: 1;
    background-color: rgba(255, 255, 255, var(--bs-bg-opacity)) !important;
}
.text-dark {
    --bs-text-opacity: 1;
    color: rgba(33, 37, 41, var(--bs-text-opacity)) !important;
}
.py-2 {
    padding-top: .5rem !important;
    padding-bottom: .5rem !important;
}
.px-3 {
    padding-right: 1rem !important;
    padding-left: 1rem !important;
}
.badge {
    --bs-badge-padding-x: 0.65em;
    --bs-badge-padding-y: 0.35em;
    --bs-badge-font-size: 0.75em;
    --bs-badge-font-weight: 700;
    --bs-badge-color: #fff;
    --bs-badge-border-radius: 0.375rem;
    display: inline-block;
    padding: var(--bs-badge-padding-y) var(--bs-badge-padding-x);
    font-size: var(--bs-badge-font-size);
    font-weight: var(--bs-badge-font-weight);
    line-height: 1;
    color: var(--bs-badge-color);
    text-align: center;
    white-space: nowrap;
    vertical-align: baseline;
    border-radius: var(--bs-badge-border-radius);
}

.import-btn, .export-btn {
    background: transparent;
    color: #888;
    border: 1px solid #333;
    padding: 8px 12px;
    border-radius: 6px;
    font-size: 0.85rem;
    cursor: pointer;
    transition: all 0.2s;
}

/* ─────────────────────────────────────────────────────────────────────────────
   Chart element — base visual style (replicates Chart.js dark theme)
   background-color  → ChartStyle::bgColor
   color             → ChartStyle::labelColor / legendTextColor / titleColor
   border-top-color  → ChartStyle::gridColor
   ───────────────────────────────────────────────────────────────────────────── */
chart {
    display: block;
    background-color: transparent;
    color: #888888;
    border-top-color: #2a2a2a;
    width: 100%;
    height: 100%;
}

/* ─────────────────────────────────────────────────────────────────────────────
   Chart.js-style value highlight (accent green on numbers)
   ───────────────────────────────────────────────────────────────────────────── */
.value-highlight { font-weight: 700; color: var(--accent-secondary); }

/* ─────────────────────────────────────────────────────────────────────────────
   Founder chart cards (matches original chart.js dark-mode card style)
   ───────────────────────────────────────────────────────────────────────────── */
.founder-chart-card {
    background: #121212;
    border: 1px solid var(--border-color);
    border-radius: 12px;
    padding: 20px;
    margin-bottom: 0;
}

.founder-chart-title {
    color: var(--text-muted);
    font-size: 0.75rem;
    text-transform: uppercase;
    letter-spacing: 0.1em;
    font-weight: 600;
    margin-bottom: 16px;
    display: block;
}

.founder-chart-container {
    position: relative;
    height: 250px;
    width: 100%;
}

/* ─────────────────────────────────────────────────────────────────────────────
   Legend rows beneath line/bar charts
   ───────────────────────────────────────────────────────────────────────────── */
.founder-legend {
    display: flex !important;
    flex-wrap: wrap;
    gap: 10px;
    margin-top: 12px;
}

.founder-legend-item {
    display: flex !important;
    align-items: center;
    gap: 6px;
    font-size: 0.8rem;
    color: var(--text-muted);
}

.founder-legend-dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    flex-shrink: 0;
    display: inline-block;
}
)css";


} // namespace mUI::Demo::CapTable
#pragma once

// HTML layout for the Cap Table demo — Premium dark-mode with Outfit font.
// Aligned with the original design specs for maximum visual fidelity.

namespace mUI::Demo::CapTable
{
    inline constexpr char HTML[] = R"html(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Cap Table &amp; Progression Analytics</title>
  </head>
  <body>
    <nav class="navbar sticky-top">
        <div class="container-fluid d-flex justify-content-between align-items-center" style="height: 60px;">
            <div class="d-flex align-items-center gap-3">
                <span id="navbar-title" class="nav-brand clickable">MIDE Co.</span>
                <div id="navbar-edit-container" style="display:none; flex-direction:row; align-items:center; gap:8px;">
                    <input type="text" id="navbar-title-input" class="navbar-edit-input" />
                    <button id="navbar-title-done" class="btn btn-sm btn-outline-secondary navbar-title-done">✓</button>
                    <button id="navbar-title-cancel" class="btn btn-sm btn-outline-secondary navbar-title-cancel" style="color: #ff5252;">✕</button>
                </div>
            </div>
            <div class="d-flex align-items-center gap-2">
                <button id="export-btn" class="btn btn-sm export-btn" title="Export settings">&#8595;</button>
                <button id="import-btn" class="btn btn-sm import-btn" title="Import settings">&#8593;</button>
                <input type="file" id="import-file" accept=".json" style="display: none;" />
                <span class="badge rounded-pill bg-white text-dark">CAPTABLE</span>
            </div>

        </div>
    </nav>

    <div class="container-fluid">
      <div class="row">

        <!-- Sidebar -->
        <div class="col-lg-4 sidebar">
          <div class="d-flex flex-column gap-3">
            <div class="card p-3" style="height: auto;">
              <div class="form-label">Currency</div>
              <select id="currencySelector" class="form-control">
                <option value="€">Euro (€)</option>
                <option value="$">US Dollar ($)</option>
                <option value="£">British Pound (£)</option>
                <option value="kr ">Swedish Krona</option>
                <option value="Fr. ">Swiss Franc</option>
                <option value="¥">Japanese Yen</option>
              </select>
            </div>
            
            <div id="roundsContainer" class="d-flex flex-column gap-4">
                <!-- Dynamic Rounds -->
            </div>

            <div class="round-default-name-row mt-4">
                <label>Default Name</label>
                <input type="text" id="defaultRoundName" class="form-control" value="Round" />
            </div>
            <button class="btn btn-add-round mt-2">+ Add Funding Round</button>
          </div>
        </div>

        <!-- Main Content -->
        <div class="col-lg-8">
          <ul class="nav" id="myTab">
                <li class="nav-item">
                    <button id="tab-table" class="tab-btn tab-table active" data-tab="table">Cap Table</button>
                </li>
                <li class="nav-item">
                    <button id="tab-founders" class="tab-btn tab-founders" data-tab="founders">Founders</button>
                </li>
                <li class="nav-item">
                    <button id="tab-charts" class="tab-btn tab-charts" data-tab="charts">Progression Analytics</button>
                </li>
          </ul>
      
          <div class="tab-content" id="myTabContent">                    

            <!-- Cap Table Tab -->
            <div class="tab-pane active" id="pane-table">
                <div class="card" style="height: auto;">
                    <div class="d-flex justify-content-between align-items-center mb-4">
                        <h3 class="card-title mb-0">Ownership Structure</h3>
                        <div class="d-flex gap-4">
                            <div class="text-end">
                                <div class="summary-label">Post Valuation</div>
                                <div id="summary_post" class="summary-value">€0.00</div>
                            </div>
                            <div class="text-end">
                                <div class="summary-label">Founders Stake</div>
                                <div id="summary_founders" class="summary-value">0%</div>
                            </div>
                        </div>
                    </div>
                    <div class="table-responsive">
                        <table class="table table-hover">
                            <colgroup>
                                <col style="width: 40%;" />
                                <col style="width: 20%;" />
                                <col style="width: 25%;" />
                                <col style="width: 15%;" />
                            </colgroup>
                            <thead>
                                <tr>
                                    <th>Shareholder</th>
                                    <th class="text-center">Ownership %</th>
                                    <th class="text-end">Value</th>
                                    <th class="text-end">Status</th>
                                </tr>
                            </thead>
                            <tbody id="capTableBody"></tbody>
                        </table>
                    </div>
                    <div id="metrics-summary-view" class="mt-4 pt-4 border-top border-white border-opacity-10">
                        <div class="row" id="preMoneyContainer"></div>
                    </div>
                </div>
            </div>
      
            <!-- Founders Tab -->
            <div class="tab-pane" id="pane-founders">
              <div class="card" style="height: auto;">
                  <h3 class="card-title">Founders</h3>
                  <div id="foundersContainer" class="d-flex flex-column gap-2"></div>
                  <button class="btn btn-add-founder mt-3">+ Add Founder</button>

                  <div class="mt-4 pt-4 border-top border-white border-opacity-10">
                      <div class="d-flex justify-content-between mb-2">
                          <span id="totalSharesLabel" class="form-label">Total: 0 shares</span>
                          <span id="totalSharesPct" class="form-label"></span>
                      </div>
                      <div class="founder-total-bar">
                          <div class="founder-total-fill" id="founderTotalFill" style="width:0%; background:#00e676;"></div>
                      </div>
                  </div>
              </div>

              <!-- Founder Charts -->
              <div class="row mt-3">
                  <div class="col-lg-6">
                    <div class="card founder-chart-card" style="height: auto;">
                        <div class="founder-chart-title">Initial Ownership Split</div>
                        <div class="chart-container">
                            <chart id="founderPieChart" type="doughnut" style="width:100%;height:100%;"></chart>
                        </div>
                    </div>
                  </div>
                  <div class="col-lg-6">
                    <div class="card founder-chart-card" style="height: auto;">
                        <div class="founder-chart-title">Stake Value Progression</div>
                        <div class="chart-container">
                            <chart id="founderValueChart" type="line" style="width:100%;height:100%;"></chart>
                        </div>
                    </div>
                  </div>
              </div>
              <div class="row mt-3">
                  <div class="col-lg-12">
                    <div class="card founder-chart-card" style="height: auto;">
                        <div class="founder-chart-title">Ownership % Progression (After Each Round)</div>
                        <div class="chart-container">
                            <chart id="founderQuotaChart" type="line" style="width:100%;height:100%;"></chart>
                        </div>
                    </div>
                  </div>
              </div>
            </div>

            <!-- Progression Analytics Tab -->
            <div class="tab-pane" id="pane-charts">
                <div class="row">
                    <div class="col-lg-6">
                        <div class="card" style="height: auto;">
                            <h3 class="card-title">Ownership Distribution</h3>
                            <div class="chart-container">
                                <chart id="ownershipChart" type="doughnut" style="width:100%;height:100%;"></chart>
                            </div>
                        </div>
                    </div>
                    <div class="col-lg-6">
                        <div class="card" style="height: auto;">
                            <h3 class="card-title">Valuation Growth</h3>
                            <div class="chart-container">
                                <chart id="valuationChart" type="line" style="width:100%;height:100%;"></chart>
                            </div>
                        </div>
                    </div>
                </div>
                <div class="row mt-3">
                    <div class="col-lg-12">
                        <div class="card" style="height: auto;">
                            <h3 class="card-title">Dilution Progression</h3>
                            <div class="chart-container" style="height: 400px;">
                                <chart id="dilutionChart" type="bar" style="width:100%;height:100%;"></chart>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
          </div>
        </div>
      </div>        
    </div>
</body>
</html>
)html";

} // namespace mUI::Demo::CapTable
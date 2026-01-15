#include "project.h"
#include <cstdlib> 

// 定義全域購物車
std::vector<CartItem> g_cart;

// 定義共用的飲料選單
struct OptionItem { wxString name; int extraPrice; };
const std::vector<OptionItem> COMMON_DRINKS = {
    {wxT("後韻紅茶"), 0}, {wxT("茉香綠茶"), 0}, {wxT("非基因豆漿"), 5},
    {wxT("奶綠"), 10}, {wxT("奶茶"), 10}, {wxT("可口可樂"), 15}, {wxT("紅茶拿鐵"), 20}
};

// ============================================================================
// 新增：結帳介面實作
// ============================================================================
CheckoutFrame::CheckoutFrame()
    : wxFrame(NULL, wxID_ANY, wxT("結帳確認"), wxDefaultPosition, wxSize(400, 500))
{
    SetBackgroundColour(*wxWHITE);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 計算總金額
    int total = 0;
    for (const auto& item : g_cart) total += (item.unitPrice * item.quantity);

    wxStaticText* title = new wxStaticText(this, wxID_ANY, wxT("結帳資訊"));
    wxFont font = title->GetFont(); font.SetPointSize(16); font.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(font);
    mainSizer->Add(title, 0, wxALL | wxALIGN_CENTER, 20);

    // 顯示金額
    wxStaticText* priceTxt = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("應付總金額: $%d"), total));
    priceTxt->SetFont(wxFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    priceTxt->SetForegroundColour(*wxRED);
    mainSizer->Add(priceTxt, 0, wxALL | wxALIGN_CENTER, 10);

    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    // 付款方式
    wxArrayString payChoices;
    payChoices.Add(wxT("現金"));
    payChoices.Add(wxT("信用卡"));
    payChoices.Add(wxT("LinePay"));
    payChoices.Add(wxT("悠遊卡"));
    wxRadioBox* rbPay = new wxRadioBox(this, wxID_ANY, wxT("選擇付款方式"), wxDefaultPosition, wxDefaultSize, payChoices, 1, wxRA_SPECIFY_COLS);
    mainSizer->Add(rbPay, 0, wxEXPAND | wxALL, 15);

    // 發票選擇
    wxArrayString invChoices;
    invChoices.Add(wxT("紙本發票"));
    invChoices.Add(wxT("手機載具"));
    invChoices.Add(wxT("公司統編"));
    wxRadioBox* rbInv = new wxRadioBox(this, wxID_ANY, wxT("發票類型"), wxDefaultPosition, wxDefaultSize, invChoices, 1, wxRA_SPECIFY_COLS);
    mainSizer->Add(rbInv, 0, wxEXPAND | wxALL, 15);

    mainSizer->AddStretchSpacer();

    // 確認按鈕
    wxButton* btnConfirm = new wxButton(this, wxID_ANY, wxT("確認結帳"), wxDefaultPosition, wxSize(200, 50));
    btnConfirm->SetBackgroundColour(wxColour(255, 100, 100)); // 紅色按鈕
    btnConfirm->SetForegroundColour(*wxWHITE);
    btnConfirm->Bind(wxEVT_BUTTON, &CheckoutFrame::OnConfirm, this);
    mainSizer->Add(btnConfirm, 0, wxALIGN_CENTER | wxBOTTOM, 30);

    SetSizer(mainSizer);
    Layout();
    Centre();
}

void CheckoutFrame::OnConfirm(wxCommandEvent& event) {
    // 1. 產生 1 ~ 999 的隨機訂單編號
    int orderId = std::rand() % 999 + 1;

    // 2. 整理餐點清單字串
    wxString mealListStr = "";
    if (g_cart.empty()) {
        mealListStr = wxT("(無餐點)");
    }
    else {
        for (const auto& item : g_cart) {
            mealListStr += wxString::Format(wxT("? %s x %d\n"), item.name, item.quantity);
            if (!item.details.IsEmpty()) {
                mealListStr += wxString::Format(wxT("   └ %s\n"), item.details);
            }
        }
    }

    // 3. 組合最終顯示訊息
    wxString msg;
    // ★★★ 修正點：mealListStr 必須加上 .c_str() 才能在 %s 中正確顯示 ★★★
    msg.Printf(wxT("付款成功！\n廚房正在準備您的餐點。\n\n====================\n訂單編號：#%03d\n====================\n\n您的餐點內容：\n%s"),
        orderId, mealListStr.c_str());

    // 4. 彈出視窗
    wxMessageBox(msg, wxT("系統通知"), wxOK | wxICON_INFORMATION);

    // 5. 清除購物車並關閉視窗
    g_cart.clear();
    Close();
}
// ============================================================================
// 新增：購物車介面實作
// ============================================================================
CartFrame::CartFrame()
    : wxFrame(NULL, wxID_ANY, wxT("我的購物車"), wxDefaultPosition, wxSize(400, 600))
{
    SetBackgroundColour(*wxWHITE);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* header = new wxStaticText(this, wxID_ANY, wxT("已選購商品"));
    wxFont font = header->GetFont(); font.SetPointSize(16); font.SetWeight(wxFONTWEIGHT_BOLD);
    header->SetFont(font);
    mainSizer->Add(header, 0, wxALL | wxALIGN_CENTER, 20);

    // 清單列表
    m_listBox = new wxListBox(this, wxID_ANY);
    mainSizer->Add(m_listBox, 1, wxEXPAND | wxALL, 10);

    // 總金額顯示
    m_txtTotal = new wxStaticText(this, wxID_ANY, wxT("總金額: $0"));
    m_txtTotal->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    mainSizer->Add(m_txtTotal, 0, wxALIGN_RIGHT | wxRIGHT, 20);

    mainSizer->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    // 底部按鈕區
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* btnBack = new wxButton(this, wxID_ANY, wxT("繼續點餐"), wxDefaultPosition, wxSize(120, 40));
    wxButton* btnCheckout = new wxButton(this, wxID_ANY, wxT("前往結帳"), wxDefaultPosition, wxSize(120, 40));

    btnBack->Bind(wxEVT_BUTTON, &CartFrame::OnBack, this);
    btnCheckout->Bind(wxEVT_BUTTON, &CartFrame::OnCheckout, this);

    btnSizer->Add(btnBack, 0, wxALL, 10);
    btnSizer->Add(btnCheckout, 0, wxALL, 10);
    mainSizer->Add(btnSizer, 0, wxALIGN_CENTER | wxBOTTOM, 20);

    SetSizer(mainSizer);
    RefreshList(); // 載入資料
    Layout();
    Centre();
}

void CartFrame::RefreshList() {
    m_listBox->Clear();
    int total = 0;

    if (g_cart.empty()) {
        m_listBox->Append(wxT("購物車是空的"));
    }
    else {
        for (const auto& item : g_cart) {
            int subtotal = item.unitPrice * item.quantity;
            total += subtotal;
            // 格式: 商品名稱 x 數量 ($小計) [詳細]
            wxString line = wxString::Format(wxT("%s x %d ($%d)"), item.name, item.quantity, subtotal);
            m_listBox->Append(line);
            if (!item.details.IsEmpty()) {
                m_listBox->Append(wxT("  └ ") + item.details); // 縮排顯示附餐細節
            }
        }
    }
    m_txtTotal->SetLabel(wxString::Format(wxT("總金額: $%d"), total));
}

void CartFrame::OnBack(wxCommandEvent& event) {
    Close();
}

void CartFrame::OnCheckout(wxCommandEvent& event) {
    if (g_cart.empty()) {
        wxMessageBox(wxT("購物車是空的，無法結帳！"), wxT("錯誤"));
        return;
    }
    CheckoutFrame* cf = new CheckoutFrame();
    cf->Show(true);
    Close();
}

// ============================================================================
// 修改：諾一超值餐介面 (加入購物車邏輯)
// ============================================================================
NuoyiMealFrame::NuoyiMealFrame(const wxString& title, const wxString& itemName, int basePrice)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 600)), m_itemName(itemName), m_basePrice(basePrice)
{
    // ... (介面排版程式碼與上次相同，省略以節省篇幅) ...
    // 請複製上一次 NuoyiMealFrame 的建構子 UI 程式碼到這裡
    // 這裡只展示為了完整性重寫一遍 UI 建構
    SetBackgroundColour(*wxWHITE);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this, wxID_ANY, itemName);
    wxFont font = header->GetFont(); font.SetPointSize(16); font.SetWeight(wxFONTWEIGHT_BOLD);
    header->SetFont(font);
    mainSizer->Add(header, 0, wxALL | wxALIGN_CENTER, 20);

    wxStaticBoxSizer* sideSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("附餐 (請選擇一種)"));
    std::vector<wxString> sides = { wxT("花生粒粒"), wxT("巧克力厚片"), wxT("奶酥"), wxT("藍莓乳酪") };
    for (size_t i = 0; i < sides.size(); ++i) {
        wxRadioButton* rb = new wxRadioButton(sideSizer->GetStaticBox(), wxID_ANY, sides[i], wxDefaultPosition, wxDefaultSize, (i == 0) ? wxRB_GROUP : 0);
        sideSizer->Add(rb, 0, wxALL, 5);
        m_sideDishes.push_back(rb);
    }
    mainSizer->Add(sideSizer, 0, wxEXPAND | wxALL, 10);

    wxStaticBoxSizer* drinkSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("附餐飲料"));
    for (size_t i = 0; i < COMMON_DRINKS.size(); ++i) {
        wxString label = COMMON_DRINKS[i].name;
        if (COMMON_DRINKS[i].extraPrice > 0) label += wxString::Format(wxT(" (+%d元)"), COMMON_DRINKS[i].extraPrice);
        wxRadioButton* rb = new wxRadioButton(drinkSizer->GetStaticBox(), wxID_ANY, label, wxDefaultPosition, wxDefaultSize, (i == 0) ? wxRB_GROUP : 0);
        drinkSizer->Add(rb, 0, wxALL, 5);
        m_drinks.push_back(rb);
    }
    mainSizer->Add(drinkSizer, 0, wxEXPAND | wxALL, 10);

    mainSizer->AddStretchSpacer();
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->Add(new wxStaticText(this, wxID_ANY, wxT("數量:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_spinQty = new wxSpinCtrl(this, wxID_ANY, wxT("1"));
    bottomSizer->Add(m_spinQty, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    wxButton* btnAdd = new wxButton(this, wxID_ANY, wxT("加入購物車"), wxDefaultPosition, wxSize(120, 40));
    btnAdd->Bind(wxEVT_BUTTON, &NuoyiMealFrame::OnAddToCart, this);
    bottomSizer->Add(btnAdd, 0, wxALIGN_CENTER_VERTICAL);
    mainSizer->Add(bottomSizer, 0, wxALIGN_CENTER | wxBOTTOM, 20);
    SetSizer(mainSizer);
    Layout();
    Centre();
}

void NuoyiMealFrame::OnAddToCart(wxCommandEvent& event) {
    int extraPrice = 0;
    wxString detailStr = "";

    // 取得附餐
    for (auto rb : m_sideDishes) {
        if (rb->GetValue()) {
            detailStr += rb->GetLabel();
            break;
        }
    }
    // 取得飲料與加價
    for (size_t i = 0; i < m_drinks.size(); ++i) {
        if (m_drinks[i]->GetValue()) {
            detailStr += wxT(" + ") + COMMON_DRINKS[i].name;
            extraPrice += COMMON_DRINKS[i].extraPrice;
            break;
        }
    }

    // ★★★ 存入全域購物車 ★★★
    CartItem newItem;
    newItem.name = m_itemName;
    newItem.details = detailStr;
    newItem.unitPrice = m_basePrice + extraPrice;
    newItem.quantity = m_spinQty->GetValue();
    g_cart.push_back(newItem);

    wxMessageBox(wxT("已加入購物車！"), wxT("成功"));
    Close();
}

// ============================================================================
// 修改：套餐選擇介面 (加入購物車邏輯)
// ============================================================================
ComboSelectFrame::ComboSelectFrame(const wxString& title, const wxString& itemName, int basePrice)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 600)), m_itemName(itemName), m_basePrice(basePrice)
{
    // ... UI 程式碼 (複製前一次，省略) ...
    SetBackgroundColour(*wxWHITE);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this, wxID_ANY, itemName + wxT(" (套餐)"));
    wxFont font = header->GetFont(); font.SetPointSize(16); font.SetWeight(wxFONTWEIGHT_BOLD);
    header->SetFont(font);
    mainSizer->Add(header, 0, wxALL | wxALIGN_CENTER, 20);

    wxStaticBoxSizer* sideSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("附餐 (二選一)"));
    wxRadioButton* rb1 = new wxRadioButton(sideSizer->GetStaticBox(), wxID_ANY, wxT("薯餅"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    wxRadioButton* rb2 = new wxRadioButton(sideSizer->GetStaticBox(), wxID_ANY, wxT("黃金脆薯 (+15元)"));
    sideSizer->Add(rb1, 0, wxALL, 5); sideSizer->Add(rb2, 0, wxALL, 5);
    m_sideDishes.push_back(rb1); m_sideDishes.push_back(rb2);
    mainSizer->Add(sideSizer, 0, wxEXPAND | wxALL, 10);

    wxStaticBoxSizer* drinkSizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("附餐飲料"));
    for (size_t i = 0; i < COMMON_DRINKS.size(); ++i) {
        wxString label = COMMON_DRINKS[i].name;
        if (COMMON_DRINKS[i].extraPrice > 0) label += wxString::Format(wxT(" (+%d元)"), COMMON_DRINKS[i].extraPrice);
        wxRadioButton* rb = new wxRadioButton(drinkSizer->GetStaticBox(), wxID_ANY, label, wxDefaultPosition, wxDefaultSize, (i == 0) ? wxRB_GROUP : 0);
        drinkSizer->Add(rb, 0, wxALL, 5); m_drinks.push_back(rb);
    }
    mainSizer->Add(drinkSizer, 0, wxEXPAND | wxALL, 10);
    mainSizer->AddStretchSpacer();
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->Add(new wxStaticText(this, wxID_ANY, wxT("數量:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_spinQty = new wxSpinCtrl(this, wxID_ANY, wxT("1"));
    bottomSizer->Add(m_spinQty, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    wxButton* btnAdd = new wxButton(this, wxID_ANY, wxT("加入購物車"), wxDefaultPosition, wxSize(120, 40));
    btnAdd->Bind(wxEVT_BUTTON, &ComboSelectFrame::OnAddToCart, this);
    bottomSizer->Add(btnAdd, 0, wxALIGN_CENTER_VERTICAL);
    mainSizer->Add(bottomSizer, 0, wxALIGN_CENTER | wxBOTTOM, 20);
    SetSizer(mainSizer);
    Layout();
    Centre();
}

void ComboSelectFrame::OnAddToCart(wxCommandEvent& event) {
    int extraPrice = 0;
    wxString detailStr = "";

    // 附餐邏輯
    if (m_sideDishes[0]->GetValue()) { detailStr += wxT("薯餅"); }
    else { detailStr += wxT("黃金脆薯"); extraPrice += 15; }

    // 飲料邏輯
    for (size_t i = 0; i < m_drinks.size(); ++i) {
        if (m_drinks[i]->GetValue()) {
            detailStr += wxT(" + ") + COMMON_DRINKS[i].name;
            extraPrice += COMMON_DRINKS[i].extraPrice;
            break;
        }
    }

    // ★★★ 存入全域購物車 ★★★
    CartItem newItem;
    newItem.name = m_itemName + wxT(" (套餐)");
    newItem.details = detailStr;
    newItem.unitPrice = m_basePrice + extraPrice;
    newItem.quantity = m_spinQty->GetValue();
    g_cart.push_back(newItem);

    wxMessageBox(wxT("已加入購物車！"), wxT("成功"));
    Close();
}

// ============================================================================
// 修改：單點介面 (加入購物車邏輯)
// ============================================================================
SingleOrderFrame::SingleOrderFrame(const wxString& title, const wxString& itemName, int basePrice)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(300, 250)), m_itemName(itemName), m_basePrice(basePrice)
{
    // ... UI 程式碼 (複製前一次，省略) ...
    SetBackgroundColour(*wxWHITE);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this, wxID_ANY, itemName);
    wxFont font = header->GetFont(); font.SetPointSize(14); font.SetWeight(wxFONTWEIGHT_BOLD);
    header->SetFont(font);
    mainSizer->Add(header, 0, wxALL | wxALIGN_CENTER, 20);
    wxStaticText* priceTxt = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("單價: %d元"), basePrice));
    mainSizer->Add(priceTxt, 0, wxALL | wxALIGN_CENTER, 10);
    mainSizer->AddStretchSpacer();
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
    bottomSizer->Add(new wxStaticText(this, wxID_ANY, wxT("數量:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_spinQty = new wxSpinCtrl(this, wxID_ANY, wxT("1"));
    bottomSizer->Add(m_spinQty, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 20);
    wxButton* btnAdd = new wxButton(this, wxID_ANY, wxT("加入購物車"), wxDefaultPosition, wxSize(100, 40));
    btnAdd->Bind(wxEVT_BUTTON, &SingleOrderFrame::OnAddToCart, this);
    bottomSizer->Add(btnAdd, 0, wxALIGN_CENTER_VERTICAL);
    mainSizer->Add(bottomSizer, 0, wxALIGN_CENTER | wxBOTTOM, 20);
    SetSizer(mainSizer);
    Layout();
    Centre();
}

void SingleOrderFrame::OnAddToCart(wxCommandEvent& event) {
    // ★★★ 存入全域購物車 ★★★
    CartItem newItem;
    newItem.name = m_itemName;
    newItem.details = wxT(""); // 單點無細項
    newItem.unitPrice = m_basePrice;
    newItem.quantity = m_spinQty->GetValue();
    g_cart.push_back(newItem);

    wxMessageBox(wxT("已加入購物車！"), wxT("成功"));
    Close();
}

// ============================================================================
// 是否升級介面 (保持不變)
// ============================================================================
UpgradeOrSingleFrame::UpgradeOrSingleFrame(const wxString& itemName, int basePrice)
    : wxFrame(NULL, wxID_ANY, wxT("選擇點餐方式"), wxDefaultPosition, wxSize(350, 250)), m_itemName(itemName), m_basePrice(basePrice)
{
    SetBackgroundColour(*wxWHITE);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this, wxID_ANY, wxT("請問要升級套餐嗎？"));
    wxFont font = header->GetFont(); font.SetPointSize(14); font.SetWeight(wxFONTWEIGHT_BOLD);
    header->SetFont(font);
    mainSizer->Add(header, 0, wxALL | wxALIGN_CENTER, 20);
    mainSizer->Add(new wxStaticText(this, wxID_ANY, wxT("目前選擇: ") + itemName), 0, wxALL | wxALIGN_CENTER, 5);
    mainSizer->AddStretchSpacer();
    wxButton* btnCombo = new wxButton(this, wxID_ANY, wxT("升級套餐"), wxDefaultPosition, wxSize(120, 50));
    wxButton* btnSingle = new wxButton(this, wxID_ANY, wxT("單點"), wxDefaultPosition, wxSize(120, 50));
    btnCombo->Bind(wxEVT_BUTTON, [=](wxCommandEvent& e) {
        ComboSelectFrame* cf = new ComboSelectFrame(wxT("套餐選擇"), m_itemName, m_basePrice + 50);
        cf->Show(true); Close();
        });
    btnSingle->Bind(wxEVT_BUTTON, [=](wxCommandEvent& e) {
        SingleOrderFrame* sf = new SingleOrderFrame(wxT("單點"), m_itemName, m_basePrice);
        sf->Show(true); Close();
        });
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(btnCombo, 0, wxALL, 10);
    btnSizer->Add(btnSingle, 0, wxALL, 10);
    mainSizer->Add(btnSizer, 0, wxALIGN_CENTER | wxBOTTOM, 20);
    SetSizer(mainSizer);
    Layout();
    Centre();
}

// ============================================================================
// 主視窗修改 (加入購物車按鈕)
// ============================================================================
MainFrame::MainFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(500, 800))
{
    this->SetBackgroundColour(wxColour(255, 255, 255));

    // 使用垂直 sizer 作為 Frame 的主要排版
    wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

    m_scrollWindow = new wxScrolledWindow(this, wxID_ANY);
    m_scrollWindow->SetScrollRate(5, 5);
    m_mainSizer = new wxBoxSizer(wxVERTICAL);

    // 標題
    wxStaticText* topTitle = new wxStaticText(m_scrollWindow, wxID_ANY, wxT("歡迎點餐 - 主介面"));
    wxFont font = topTitle->GetFont(); font.SetPointSize(18); font.SetWeight(wxFONTWEIGHT_BOLD);
    topTitle->SetFont(font);
    m_mainSizer->Add(topTitle, 0, wxALL | wxALIGN_CENTER, 20);

    InitMenuData();

    m_scrollWindow->SetSizer(m_mainSizer);
    m_scrollWindow->FitInside();

    // 將捲動視窗加入 rootSizer，佔滿剩餘空間
    rootSizer->Add(m_scrollWindow, 1, wxEXPAND);

    // ★★★ 在底部加入購物車按鈕 ★★★
    wxPanel* botPanel = new wxPanel(this);
    botPanel->SetBackgroundColour(wxColour(240, 240, 240));
    wxBoxSizer* botSizer = new wxBoxSizer(wxHORIZONTAL);

    m_btnCart = new wxButton(botPanel, wxID_ANY, wxT("查看購物車 / 結帳"), wxDefaultPosition, wxSize(-1, 50));
    m_btnCart->SetFont(wxFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    m_btnCart->SetBackgroundColour(wxColour(255, 165, 0)); // 橘色
    m_btnCart->SetForegroundColour(*wxWHITE);
    m_btnCart->Bind(wxEVT_BUTTON, &MainFrame::OnViewCart, this);

    botSizer->Add(m_btnCart, 1, wxALL | wxEXPAND, 10);
    botPanel->SetSizer(botSizer);

    // 加入 rootSizer
    rootSizer->Add(botPanel, 0, wxEXPAND);

    this->SetSizer(rootSizer);
}

void MainFrame::OnViewCart(wxCommandEvent& event) {
    CartFrame* cart = new CartFrame();
    cart->Show(true);
}

// ... (剩下的 AddCategoryHeader, AddItemButton, OnItemClick, InitMenuData 保持不變) ...
// 為了編譯通過，以下重複必要的實作 (請保留原有的內容，只需確保 MainFrame 構造函數已如上修改)

void MainFrame::AddCategoryHeader(const wxString& text) {
    wxStaticLine* line = new wxStaticLine(m_scrollWindow, wxID_ANY);
    m_mainSizer->Add(line, 0, wxEXPAND | wxALL, 10);
    wxStaticText* header = new wxStaticText(m_scrollWindow, wxID_ANY, text);
    wxFont font = header->GetFont(); font.SetPointSize(14); font.SetWeight(wxFONTWEIGHT_BOLD);
    header->SetFont(font);
    header->SetForegroundColour(wxColour(0, 51, 102));
    m_mainSizer->Add(header, 0, wxALL | wxALIGN_LEFT, 15);
}

void MainFrame::AddItemButton(const wxString& text, const wxString& category) {
    wxButton* btn = new wxButton(m_scrollWindow, wxID_ANY, text, wxDefaultPosition, wxSize(400, 50));
    wxFont font = btn->GetFont(); font.SetPointSize(11); btn->SetFont(font);
    wxString nameStr = text;
    int price = 0;
    wxString nums = "0123456789";
    int lastDigit = text.find_last_of(nums);
    if (lastDigit != wxString::npos) {
        int startDigit = lastDigit;
        while (startDigit >= 0 && nums.Find(text[startDigit]) != wxString::npos) startDigit--;
        wxString priceStr = text.SubString(startDigit + 1, lastDigit);
        long temp; if (priceStr.ToLong(&temp)) price = (int)temp;
        nameStr = text.SubString(0, startDigit).Trim();
    }
    btn->Bind(wxEVT_BUTTON, [=](wxCommandEvent& e) { OnItemClick(nameStr, price, category); });
    m_mainSizer->Add(btn, 0, wxALL | wxALIGN_CENTER, 5);
}

void MainFrame::OnItemClick(const wxString& itemName, int price, const wxString& category) {
    if (category == wxT("諾伊超值餐")) {
        NuoyiMealFrame* frame = new NuoyiMealFrame(wxT("諾伊超值餐"), itemName, price);
        frame->Show(true);
    }
    else if (category == wxT("活力餐餐配") || category == wxT("麵食套餐") || category == wxT("經典堡") || category == wxT("吐司") || category == wxT("漢堡")) {
        UpgradeOrSingleFrame* frame = new UpgradeOrSingleFrame(itemName, price);
        frame->Show(true);
    }
    else {
        SingleOrderFrame* frame = new SingleOrderFrame(wxT("單點"), itemName, price);
        frame->Show(true);
    }
}

void MainFrame::InitMenuData() {
    // 這裡放入上一篇回答的 InitMenuData 內容，保持不變
    AddCategoryHeader(wxT("諾伊超值餐"));
    std::vector<wxString> cat1 = { wxT("諾一超值餐 95元"), wxT("職人里肌蛋吐司餐 95元"), wxT("勁辣卡拉雞套餐 95元"), wxT("美式牛肉起司堡餐 129元"), wxT("活力大早餐 159元") };
    for (const auto& item : cat1) AddItemButton(item, wxT("諾伊超值餐"));

    AddCategoryHeader(wxT("活力餐餐配"));
    std::vector<wxString> cat2 = { wxT("金塊蘿菠糕 60元"), wxT("比薩煎餃 75元"), wxT("香煎豬里肌沙拉 95元"), wxT("嫩煎雞胸肉沙拉 95元") };
    for (const auto& item : cat2) AddItemButton(item, wxT("活力餐餐配"));

    AddCategoryHeader(wxT("麵食套餐"));
    std::vector<wxString> cat3 = { wxT("黑胡椒鐵板麵 65元"), wxT("蘑菇鐵板麵 65元"), wxT("奶油白醬起司麵 70元") };
    for (const auto& item : cat3) AddItemButton(item, wxT("麵食套餐"));

    AddCategoryHeader(wxT("經典堡"));
    std::vector<wxString> cat4 = { wxT("經典大亨堡 65元"), wxT("蜜蘋蛋沙拉熱狗餐 75元"), wxT("經典辣肉醬熱狗堡 85元") };
    for (const auto& item : cat4) AddItemButton(item, wxT("經典堡"));

    AddCategoryHeader(wxT("吐司"));
    std::vector<wxString> cat5 = { wxT("薯餅蛋吐司 45元"), wxT("玉米鮪魚蛋吐司 50元"), wxT("職人里肌蛋吐司 55元"), wxT("職人肌肉蛋吐司 55元"), wxT("花生培根牛肉起司吐司 65元"), wxT("美式牛肉起司吐司 65元"), wxT("勁辣卡拉蛋吐司 70元"), wxT("板烤雞肉蛋吐司 70元") };
    for (const auto& item : cat5) AddItemButton(item, wxT("吐司"));

    AddCategoryHeader(wxT("漢堡"));
    std::vector<wxString> cat6 = { wxT("薯餅蛋漢堡 55元"), wxT("玉米鮪魚蛋漢堡 60元"), wxT("職人里肌蛋漢堡 65元"), wxT("職人肌肉蛋漢堡 65元"), wxT("花生培根牛肉起司漢堡 75元"), wxT("美式牛肉起司吐漢堡 75元"), wxT("勁辣卡拉蛋漢堡 80元"), wxT("板烤雞肉蛋漢堡 80元") };
    for (const auto& item : cat6) AddItemButton(item, wxT("漢堡"));

    AddCategoryHeader(wxT("抹醬麵包"));
    std::vector<wxString> cat7 = { wxT("花生粒粒 25元"), wxT("巧克力厚片 25元"), wxT("奶酥 25元"), wxT("藍莓乳酪 25元"), wxT("抹茶奶酥 45元"), wxT("英式伯爵 45元") };
    for (const auto& item : cat7) AddItemButton(item, wxT("抹醬麵包"));

    AddCategoryHeader(wxT("輕鬆小點"));
    std::vector<wxString> cat8 = { wxT("荷包蛋 15元"), wxT("熱狗 30元"), wxT("孩子王雞塊 40元"), wxT("黃金脆薯 45元"), wxT("檸檬雞柳條 45元"), wxT("六塊雞塊 45元") };
    for (const auto& item : cat8) AddItemButton(item, wxT("輕鬆小點"));

    AddCategoryHeader(wxT("喝點什麼"));
    std::vector<wxString> cat9 = { wxT("後韻紅茶 25 元"), wxT("茉香綠茶 25元"), wxT("非基因豆漿 30元"), wxT("奶綠 35元"), wxT("奶茶 35元"), wxT("可口可樂 35元"), wxT("紅茶拿鐵 40元") };
    for (const auto& item : cat9) AddItemButton(item, wxT("喝點什麼"));
}   

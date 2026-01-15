#pragma once

#include <wx/wx.h>
#include <wx/scrolwin.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/radiobut.h>
#include <wx/listbox.h>
#include <vector>

// ----------------------------------------------------------------------------
// 資料結構：購物車項目
// ----------------------------------------------------------------------------
struct CartItem {
    wxString name;      // 商品名稱
    wxString details;   // 詳細內容 (附餐、飲料等)
    int unitPrice;      // 單價 (含加價)
    int quantity;       // 數量
};

// 宣告全域購物車容器 (讓所有視窗都能存取)
extern std::vector<CartItem> g_cart;

// 前置宣告
class MainFrame;

// ----------------------------------------------------------------------------
// 5. 結帳介面 (新增)
// ----------------------------------------------------------------------------
class CheckoutFrame : public wxFrame {
public:
    CheckoutFrame();
private:
    void OnConfirm(wxCommandEvent& event);
};

// ----------------------------------------------------------------------------
// 6. 購物車介面 (新增)
// ----------------------------------------------------------------------------
class CartFrame : public wxFrame {
public:
    CartFrame();
private:
    wxListBox* m_listBox;
    wxStaticText* m_txtTotal;
    void RefreshList();
    void OnCheckout(wxCommandEvent& event);
    void OnBack(wxCommandEvent& event);
};

// ----------------------------------------------------------------------------
// 原有的視窗類別 (保持不變)
// ----------------------------------------------------------------------------
class NuoyiMealFrame : public wxFrame {
public:
    NuoyiMealFrame(const wxString& title, const wxString& itemName, int basePrice);
private:
    wxString m_itemName;
    int m_basePrice;
    std::vector<wxRadioButton*> m_sideDishes;
    std::vector<wxRadioButton*> m_drinks;
    wxSpinCtrl* m_spinQty;
    void OnAddToCart(wxCommandEvent& event);
};

class ComboSelectFrame : public wxFrame {
public:
    ComboSelectFrame(const wxString& title, const wxString& itemName, int basePrice);
private:
    wxString m_itemName;
    int m_basePrice;
    std::vector<wxRadioButton*> m_sideDishes;
    std::vector<wxRadioButton*> m_drinks;
    wxSpinCtrl* m_spinQty;
    void OnAddToCart(wxCommandEvent& event);
};

class SingleOrderFrame : public wxFrame {
public:
    SingleOrderFrame(const wxString& title, const wxString& itemName, int basePrice);
private:
    wxString m_itemName;
    int m_basePrice;
    wxSpinCtrl* m_spinQty;
    void OnAddToCart(wxCommandEvent& event);
};

class UpgradeOrSingleFrame : public wxFrame {
public:
    UpgradeOrSingleFrame(const wxString& itemName, int basePrice);
private:
    wxString m_itemName;
    int m_basePrice;
};

// ----------------------------------------------------------------------------
// 主視窗 (新增購物車按鈕)
// ----------------------------------------------------------------------------
class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title);

private:
    wxScrolledWindow* m_scrollWindow;
    wxBoxSizer* m_mainSizer;
    wxButton* m_btnCart; // 新增購物車按鈕

    void AddCategoryHeader(const wxString& text);
    void AddItemButton(const wxString& text, const wxString& category);
    void InitMenuData();
    void OnItemClick(const wxString& itemName, int price, const wxString& category);
    void OnViewCart(wxCommandEvent& event); // 查看購物車
};

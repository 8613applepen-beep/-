#include <wx/wx.h>
#include <cstdlib> // 加入這行
#include <ctime>   // 加入這行
#include "project.h"

// 定義應用程式類別
class MyApp : public wxApp
{
public:
    virtual bool OnInit() override
    {
        // ★★★ 設定隨機數種子 (讓每次亂數都不同) ★★★
        std::srand(std::time(nullptr));

        // 確保圖片處理器已初始化
        wxInitAllImageHandlers();

        // 建立並顯示主視窗
        MainFrame* frame = new MainFrame(wxT("點餐系統主介面"));
        frame->Show(true);

        return true;
    }
};

wxIMPLEMENT_APP(MyApp);

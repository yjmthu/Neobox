#include <windows.h>
#include <atlbase.h>
#include <commctrl.h>
#include <exdisp.h>
#include <Shlobj.h>
#include <shobjidl_core.h>
#include <QEnterEvent>
#include <QEvent>

#include "desktopmask.h"
#include "funcbox.h"

class CCoInitialize {
public:
    CCoInitialize() : m_hr(CoInitialize(NULL)) { }
    ~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
    operator HRESULT() const { return m_hr; }
    HRESULT m_hr;
};

// find desktop file view
void FindDesktopFolderView(REFIID riid, void** ppv)
{
    CComPtr<IShellWindows> spShellWindows;
    spShellWindows.CoCreateInstance(CLSID_ShellWindows);
    CComVariant vtLoc(CSIDL_DESKTOP);
    CComVariant vtEmpty;
    long lhwnd;
    CComPtr<IDispatch> spdisp;
    spShellWindows->FindWindowSW(&vtLoc, &vtEmpty,SWC_DESKTOP, &lhwnd, SWFO_NEEDDISPATCH, &spdisp);
    CComPtr<IShellBrowser> spBrowser;
    CComQIPtr<IServiceProvider>(spdisp)->QueryService(SID_STopLevelBrowser,IID_PPV_ARGS(&spBrowser));
    CComPtr<IShellView> spView;
    spBrowser->QueryActiveShellView(&spView);
    spView->QueryInterface(riid, ppv);
}

// auto
int SnapToGridEx(bool bAlign)
{
    int errorNum = 0;
    CCoInitialize initCom;
    CComPtr<IFolderView2> spView;
    FindDesktopFolderView(IID_PPV_ARGS(&spView));
    if (NULL == spView)
    {
        return errorNum;
    }
    if (bAlign)
    {
        spView->SetCurrentFolderFlags(FWF_NOICONS, FWF_NOICONS);
    }
    else
    {
        spView->SetCurrentFolderFlags(FWF_NOICONS, ~FWF_NOICONS);
    }
    return errorNum;
}

class DesktopWidget:public QWidget
{
protected:
    void enterEvent(QEnterEvent *event);    // 鼠标进入后改变图标
public:
    explicit DesktopWidget(bool flag);
    //~DesktopWidget();
private:
    const bool _flag;
};

DesktopWidget::DesktopWidget(bool flag):
    QWidget(nullptr),
    _flag(flag)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool);
    //setStyleSheet("QWidget{background-color:rgba(0, 0, 0, 0);}");
    setStyleSheet("background:transparent;border-radius: 1px;");
    QSize size(1, 12);
    setMinimumSize(size);
    setMaximumSize(size);
}

void DesktopWidget::enterEvent(QEnterEvent *event)
{
    SnapToGridEx(_flag);
    event->accept();
}

DesktopMask::DesktopMask():
    left(new DesktopWidget(false)),
    right(new DesktopWidget(true))
{
    left->move(0, VarBox->ScreenHeight/2-5);
    right->move(VarBox->ScreenWidth-1, VarBox->ScreenHeight/2-6);
    left->show(); right->show();
}

DesktopMask::~DesktopMask()
{
    delete right;
    delete left;
}


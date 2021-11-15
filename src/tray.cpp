#include <oleacc.h>
#include <QTimer>

#include "funcbox.h"
#include "tray.h"


constexpr const char szShellTray[] = "Shell_TrayWnd";                //主任务栏类名
constexpr const char szSecondaryTray[] = "Shell_SecondaryTrayWnd";   //副任务栏类名

//查找任务图标UI
bool Find(IAccessible* paccParent, int iRole, IAccessible** paccChild)
{
	long numChildren;
	unsigned long numFetched;
	VARIANT varChild;
	int indexCount = 1;
    IAccessible* pChild = nullptr;
    IEnumVARIANT* pEnum = nullptr;
    IDispatch* pDisp = nullptr;
	bool found = false;
	paccParent->QueryInterface(IID_IEnumVARIANT, (PVOID*)&pEnum);
	if (pEnum)
		pEnum->Reset();
	paccParent->get_accChildCount(&numChildren);
    for (; indexCount <= numChildren && !found; indexCount++)
	{
		pChild = NULL;
		if (pEnum)
			pEnum->Next(1, &varChild, &numFetched);
		else
		{
			varChild.vt = VT_I4;
			varChild.lVal = indexCount;
		}
		if (varChild.vt == VT_I4)
		{
			pDisp = NULL;
            paccParent->get_accChild(varChild, &pDisp);
		}
		else
			pDisp = varChild.pdispVal;
		if (pDisp)
		{
			pDisp->QueryInterface(IID_IAccessible, (void**)&pChild);
			pDisp->Release();
		}
		if (pChild)
		{
			VariantInit(&varChild);
			varChild.vt = VT_I4;
			varChild.lVal = CHILDID_SELF;
			*paccChild = pChild;
		}
		VARIANT varState;
		if (pChild)
		{
			pChild->get_accState(varChild, &varState);
			if ((varState.intVal & STATE_SYSTEM_INVISIBLE) == 0)
			{
				VARIANT varRole;
                pChild->get_accRole(varChild, &varRole); ///////////// 32位系统会出现错误提示.
				if (varRole.lVal == iRole)
				{
					paccParent->Release();
					found = true;
					break;
				}
			}
		}
		if (!found && pChild)
			pChild->Release();
	}
	if (pEnum)
		pEnum->Release();
	return found;
}

//设置任务栏图标位置
void Tray::SetTaskBarPos(HWND hTaskListWnd, HWND hTrayWnd, HWND hTaskWnd, HWND hReBarWnd, BOOL bMainTray)
{
	static int oleft = 0, otop = 0;
	IAccessible* pAcc = NULL;
    VarBox->AccessibleObjectFromWindow(hTaskListWnd, OBJID_WINDOW, IID_IAccessible, (void**)&pAcc);
	IAccessible* paccChlid = NULL;
	if (pAcc)
	{
		if (Find(pAcc, 22, &paccChlid) == FALSE)
			return;
	}
	else
		return;
	long childCount, returnCount;
	LONG left, top, width, height;
	LONG ol = 0, ot = 0;
    int tWidth = 0, tHeight = 0;         //任务栏图标 宽和， 高和
	if (paccChlid)
	{
		if (paccChlid->get_accChildCount(&childCount) == S_OK && childCount != 0)
		{
			VARIANT* pArray = (VARIANT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(VARIANT) * childCount);
            if (VarBox->AccessibleChildren(paccChlid, 0L, childCount, pArray, &returnCount) == S_OK)
			{
				for (int x = 0; x < returnCount; x++)
				{
					VARIANT vtChild = pArray[x];
					{
						VARIANT varState;
						paccChlid->get_accState(vtChild, &varState);
						if ((varState.intVal & STATE_SYSTEM_INVISIBLE) == 0)
						{
							VARIANT varRole;
							paccChlid->get_accRole(vtChild, &varRole);
							if (varRole.intVal == 0x2b || varRole.intVal == 0x39)
							{
								paccChlid->accLocation(&left, &top, &width, &height, vtChild);
								if (ol != left)
								{
									tWidth += width;
									ol = left;
								}
								if (ot != top)
								{
									tHeight += height;
									ot = top;
								}
							}
						}
					}
				}
			}
			HeapFree(GetProcessHeap(), 0, pArray);
		}
		paccChlid->Release();
	}
	else
        return;
    RECT lrc, src, trc;                        // l     r   t    b
    GetWindowRect(hTaskListWnd, &lrc);         // 984 2480 1520 1600              猜测 图标所占用区域
    GetWindowRect(hTrayWnd, &src);             // 0   2560 1520 1600              猜测 整个任务栏
    GetWindowRect(hTaskWnd, &trc);             // 384 1880 1520 1600              猜测 中部区域

	bool Vertical = false;
	if (src.right - src.left < src.bottom - src.top)
		Vertical = true;
	SendMessage(hReBarWnd, WM_SETREDRAW, TRUE, 0);
	int nleft = 0, ntop = 0;
    if (VarBox->iPos == TaskBarCenterState::TASK_RIGHT)
	{
		if (Vertical)
			ntop = trc.bottom - trc.top - tHeight;
		else
			nleft = trc.right - trc.left - tWidth;
	}
    else if (VarBox->iPos == TaskBarCenterState::TASK_LEFT)
	{
		nleft = ntop = 0;
        centerTask->stop();
	}
    else if (VarBox->iPos == TaskBarCenterState::TASK_CENTER)
	{
		if (Vertical)
            ntop = (src.top + src.bottom) / 2 - trc.top - tHeight / 2;
		else
            nleft = (src.right + src.left) / 2 - trc.left - tWidth / 2;
        if (bMainTray)
        {
            if (Vertical)
                ntop -= 2;
            else
                nleft -= 2;
        }
    }
	if (Vertical)
	{
		if (bMainTray)
		{
            if (otop == 0)
                lrc.top = ntop;
            else
                lrc.top = otop;
            otop = ntop;
			while (ntop != lrc.top)
			{
				if (ntop > lrc.top)
					++lrc.top;
				else
					--lrc.top;
                SetWindowPos(hTaskListWnd, 0, 0, lrc.top, 0, 0, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
			}
		}
        SetWindowPos(hTaskListWnd, 0, 0, ntop, 0, 0, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}
	else
	{
		if (bMainTray)
		{
            if (oleft == 0)
                lrc.left = nleft;
            else
                lrc.left = oleft;
            oleft = nleft;
            while (nleft != lrc.left)
            {
                if (nleft > lrc.left)
                    ++lrc.left;
                else
                    --lrc.left;
                SetWindowPos(hTaskListWnd, 0, lrc.left, 0, 0, 0, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
            }
		}
        SetWindowPos(hTaskListWnd, 0, nleft, 0, 0, 0, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}
    if (VarBox->iPos != TaskBarCenterState::TASK_LEFT)
		SendMessage(hReBarWnd, WM_SETREDRAW, FALSE, 0);
	ShowWindow(hTaskWnd, SW_SHOWNOACTIVATE);
}

BOOL CALLBACK IsZoomedFunc(HWND hWnd, LPARAM lpAram)
{
    if (::IsWindowVisible(hWnd) && IsZoomed(hWnd))
    {
        if (MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST) == (HMONITOR)lpAram)
        {
            BOOL Attribute = FALSE;
            if (VarBox->pDwmGetWindowAttribute)
                VarBox->pDwmGetWindowAttribute(hWnd, 14, &Attribute, sizeof(BOOL));
            if (!Attribute)
            {
                //qout << "找到全屏";
                VarBox->isMax = 1;
                return FALSE;
            }
            //qout << "未找到全屏";
        }
    }
    return TRUE;
}


void Tray::GetShellAllWnd()
{
	while (IsWindow(hTray) == FALSE)
	{
        hTray = FindWindowA(szShellTray, NULL);
		if (hTray == NULL)
			Sleep(100);
	}
	while (IsWindow(hReBarWnd) == FALSE)
	{
        hReBarWnd = FindWindowExA(hTray, 0, "ReBarWindow32", NULL);
		if (hReBarWnd == NULL)
			Sleep(100);
	}
	if (IsWindow(hTaskWnd) == FALSE)
        hTaskWnd = FindWindowExA(hReBarWnd, NULL, "MSTaskSwWClass", NULL);
	if (IsWindow(hTaskListWnd) == FALSE)
        hTaskListWnd = FindWindowExA(hTaskWnd, NULL, "MSTaskListWClass", NULL);
}

Tray::Tray()
{
    beautifyTask = new QTimer;
    beautifyTask->setInterval(VarBox->RefreshTime);
    connect(beautifyTask, SIGNAL(timeout()), this, SLOT(keepTaskBar()));
    beautifyTask->start();

    if (VarBox->WinVersion == 0xA)
    {
        centerTask = new QTimer;
        centerTask->setInterval(500);
        connect(centerTask, SIGNAL(timeout()), this, SLOT(centerTaskBar()));
        centerTask->start();
    }
}

Tray::~Tray()
{
    if (VarBox->WinVersion == 0xA)
    {
        if (centerTask->isActive()) centerTask->stop();
        delete centerTask;
    }
    if (beautifyTask->isActive()) beautifyTask->stop();
    delete beautifyTask;
}

void Tray::keepTaskBar()
{
    VarBox->isMax = FALSE;
    HWND hTray = FindWindowA(szShellTray, NULL);
	if (hTray)
	{
        EnumWindows(IsZoomedFunc, (LPARAM)MonitorFromWindow(hTray, MONITOR_DEFAULTTONEAREST));
        VarBox->SetWindowCompositionAttribute(hTray, VarBox->aMode[VarBox->isMax], VarBox->dAlphaColor[VarBox->isMax]);
		LONG_PTR exStyle = GetWindowLongPtr(hTray, GWL_EXSTYLE);
		exStyle |= WS_EX_LAYERED;
        SetWindowLongPtrA(hTray, GWL_EXSTYLE, exStyle);
        SetLayeredWindowAttributes(hTray, NULL, (BYTE)VarBox->bAlpha[VarBox->isMax], LWA_ALPHA);
	}
    HWND hSecondaryTray = FindWindowA(szSecondaryTray, NULL);
	while (hSecondaryTray)
	{
        EnumWindows(IsZoomedFunc, (LPARAM)MonitorFromWindow(hSecondaryTray, MONITOR_DEFAULTTONEAREST));
        VarBox->SetWindowCompositionAttribute(hSecondaryTray, VarBox->aMode[VarBox->isMax], VarBox->dAlphaColor[VarBox->isMax]);
		LONG_PTR exStyle = GetWindowLongPtr(hSecondaryTray, GWL_EXSTYLE);
		exStyle |= WS_EX_LAYERED;
        SetWindowLongPtrA(hSecondaryTray, GWL_EXSTYLE, exStyle);
        SetLayeredWindowAttributes(hSecondaryTray, NULL, (BYTE)VarBox->bAlpha[VarBox->isMax], LWA_ALPHA);
        hSecondaryTray = FindWindowExA(NULL, hSecondaryTray, szSecondaryTray, NULL);
	}
    if ((VarBox->aMode[1] == ACCENT_STATE::ACCENT_DISABLED) && (VarBox->aMode[0] == ACCENT_STATE::ACCENT_DISABLED))
        beautifyTask->stop();
}

void Tray::centerTaskBar()
{
	GetShellAllWnd();
	SetTaskBarPos(hTaskListWnd, hTray, hTaskWnd, hReBarWnd, TRUE);
    HWND hSecondaryTray = FindWindowA(szSecondaryTray, NULL);
	while (hSecondaryTray)
	{
        HWND hSReBarWnd = FindWindowExA(hSecondaryTray, 0, "WorkerW", NULL);
		if (hSReBarWnd)
		{
            HWND hSTaskListWnd = FindWindowExA(hSReBarWnd, NULL, "MSTaskListWClass", NULL);
			if (hSTaskListWnd)
				SetTaskBarPos(hSTaskListWnd, hSecondaryTray, hSReBarWnd, hSReBarWnd, FALSE);
		}
        hSecondaryTray = FindWindowExA(NULL, hSecondaryTray, szSecondaryTray, NULL);
	}
}

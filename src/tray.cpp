#include <oleacc.h>
#include <QTimer>

#include "funcbox.h"
#include "tray.h"

typedef ULONG(WINAPI* pfnAccessibleObjectFromWindow)(_In_ HWND hwnd, _In_ DWORD dwId, _In_ REFIID riid, _Outptr_ void** ppvObject);
typedef ULONG(WINAPI* pfnAccessibleChildren)(_In_ IAccessible* paccContainer, _In_ LONG iChildStart, _In_ LONG cChildren, _Out_writes_(cChildren) VARIANT* rgvarChildren, _Out_ LONG* pcObtained);
typedef BOOL(WINAPI* pfnDwmGetWindowAttribute)(HWND hwnd, DWORD dwAttribute, PVOID pvAttribute, DWORD cbAttribute);

pfnAccessibleObjectFromWindow AccessibleObjectFromWindowT;
pfnAccessibleChildren AccessibleChildrenT;
pfnDwmGetWindowAttribute pDwmGetWindowAttribute;

const char szShellTray[] = "Shell_TrayWnd";                //主任务栏类名
const char szSecondaryTray[] = "Shell_SecondaryTrayWnd";   //副任务栏类名

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
				pChild->get_accRole(varChild, &varRole); ///////////////////////////////////////////
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
    if (VarBox.hOleacc == NULL)        //  获取函数
	{
		VarBox.hOleacc = LoadLibrary(TEXT("oleacc.dll"));
		if (VarBox.hOleacc)
		{
            AccessibleObjectFromWindowT = (pfnAccessibleObjectFromWindow)GetProcAddress(VarBox.hOleacc, "AccessibleObjectFromWindow");
			AccessibleChildrenT = (pfnAccessibleChildren)GetProcAddress(VarBox.hOleacc, "AccessibleChildren");
		}
		if (VarBox.hOleacc == NULL)
			return;
	}
	IAccessible* pAcc = NULL;
	AccessibleObjectFromWindowT(hTaskListWnd, OBJID_WINDOW, IID_IAccessible, (void**)&pAcc);
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
	int tWidth = 0, tHeight = 0;
	if (paccChlid)
	{
		if (paccChlid->get_accChildCount(&childCount) == S_OK && childCount != 0)
		{
			VARIANT* pArray = (VARIANT*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(VARIANT) * childCount);
			if (AccessibleChildrenT(paccChlid, 0L, childCount, pArray, &returnCount) == S_OK)
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
	RECT lrc, src, trc;
	GetWindowRect(hTaskListWnd, &lrc);
	GetWindowRect(hTrayWnd, &src);
	GetWindowRect(hTaskWnd, &trc);
	bool Vertical = false;
	if (src.right - src.left < src.bottom - src.top)
		Vertical = true;
	SendMessage(hReBarWnd, WM_SETREDRAW, TRUE, 0);
	int lr, tb;
	if (Vertical)
	{
		int t = trc.left - src.left, b = src.bottom - trc.bottom;
		if (t > b)
			tb = t;
		else
			tb = b;
	}
	else
	{
		int l = trc.left - src.left, r = src.right - trc.right;
		if (l > r)
			lr = l;
		else
			lr = r;
	}
	int nleft = 0, ntop = 0;
	if ((VarBox.iPos == TaskBarCenterState::TASK_RIGHT || (!Vertical && tWidth >= trc.right - trc.left - lr) || (Vertical && tHeight >= trc.bottom - trc.top - tb)) && VarBox.iPos != TaskBarCenterState::TASK_LEFT)
	{
		if (Vertical)
			ntop = trc.bottom - trc.top - tHeight;
		else
			nleft = trc.right - trc.left - tWidth;
	}
	else if (VarBox.iPos == TaskBarCenterState::TASK_LEFT)
	{
		nleft = ntop = 0;
        centerTask->stop();
	}
	else if (VarBox.iPos == TaskBarCenterState::TASK_CENTER)
	{
		if (Vertical)
			ntop = src.top + (src.bottom - src.top) / 2 - trc.top - tHeight / 2;
		else
			nleft = src.left + (src.right - src.left) / 2 - trc.left - tWidth / 2;
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
				SetWindowPos(hTaskListWnd, 0, 0, lrc.top, lrc.right - lrc.left, lrc.bottom - lrc.top, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
			}
		}
		SetWindowPos(hTaskListWnd, 0, 0, ntop, lrc.right - lrc.left, lrc.bottom - lrc.top, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
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
				SetWindowPos(hTaskListWnd, 0, lrc.left, 0, lrc.right - lrc.left, lrc.bottom - lrc.top, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
			}
		}
		SetWindowPos(hTaskListWnd, 0, nleft, 0, lrc.right - lrc.left, lrc.bottom - lrc.top, SWP_NOSIZE | SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}
	if (VarBox.iPos != TaskBarCenterState::TASK_LEFT)
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
            if (pDwmGetWindowAttribute)
                pDwmGetWindowAttribute(hWnd, 14, &Attribute, sizeof(BOOL));
            if (!Attribute)
            {
                VarBox.WHEN_FULL = 1;
                return FALSE;
            }
        }
    }
    return TRUE;
}


void Tray::GetShellAllWnd()
{
	while (IsWindow(hTray) == FALSE)
	{
		hTray = FindWindow(szShellTray, NULL);
		if (hTray == NULL)
			Sleep(100);
	}
	while (IsWindow(hReBarWnd) == FALSE)
	{
		hReBarWnd = FindWindowEx(hTray, 0, TEXT("ReBarWindow32"), NULL);
		if (hReBarWnd == NULL)
			Sleep(100);
	}
	if (IsWindow(hTaskWnd) == FALSE)
		hTaskWnd = FindWindowEx(hReBarWnd, NULL, TEXT("MSTaskSwWClass"), NULL);
	if (IsWindow(hTaskListWnd) == FALSE)
		hTaskListWnd = FindWindowEx(hTaskWnd, NULL, TEXT("MSTaskListWClass"), NULL);
}

Tray::Tray()
{
    beautifyTask = new QTimer;
    beautifyTask->setInterval(VarBox.RefreshTime);
    connect(beautifyTask, SIGNAL(timeout()), this, SLOT(keepTaskBar()));
    beautifyTask->start();

    centerTask = new QTimer;
    centerTask->setInterval(500);
    connect(centerTask, SIGNAL(timeout()), this, SLOT(centerTaskBar()));
    centerTask->start();
}

Tray::~Tray()
{
    if (centerTask->isActive()) centerTask->stop();
    if (beautifyTask->isActive()) beautifyTask->stop();
    delete centerTask; delete beautifyTask;
}

void Tray::keepTaskBar()
{
    VarBox.WHEN_FULL = FALSE;
	HWND hTray = FindWindow(szShellTray, NULL);
	if (hTray)
	{
        EnumWindows(IsZoomedFunc, (LPARAM)MonitorFromWindow(hTray, MONITOR_DEFAULTTONEAREST));
		FuncBox::SetWindowCompositionAttribute(hTray, VarBox.aMode[VarBox.WHEN_FULL], VarBox.dAlphaColor[VarBox.WHEN_FULL]);
		LONG_PTR exStyle = GetWindowLongPtr(hTray, GWL_EXSTYLE);
		exStyle |= WS_EX_LAYERED;
		SetWindowLongPtr(hTray, GWL_EXSTYLE, exStyle);
		SetLayeredWindowAttributes(hTray, NULL, (BYTE)VarBox.bAlpha[VarBox.WHEN_FULL], LWA_ALPHA);
	}
	HWND hSecondaryTray = FindWindow(szSecondaryTray, NULL);
	while (hSecondaryTray)
	{
        EnumWindows(IsZoomedFunc, (LPARAM)MonitorFromWindow(hSecondaryTray, MONITOR_DEFAULTTONEAREST));
		FuncBox::SetWindowCompositionAttribute(hSecondaryTray, VarBox.aMode[VarBox.WHEN_FULL], VarBox.dAlphaColor[VarBox.WHEN_FULL]);
		LONG_PTR exStyle = GetWindowLongPtr(hSecondaryTray, GWL_EXSTYLE);
		exStyle |= WS_EX_LAYERED;
		SetWindowLongPtr(hSecondaryTray, GWL_EXSTYLE, exStyle);
		SetLayeredWindowAttributes(hSecondaryTray, NULL, (BYTE)VarBox.bAlpha[VarBox.WHEN_FULL], LWA_ALPHA);
		hSecondaryTray = FindWindowEx(NULL, hSecondaryTray, szSecondaryTray, NULL);
	}
	if ((VarBox.aMode[1] == ACCENT_STATE::ACCENT_DISABLED) && (VarBox.aMode[0] == ACCENT_STATE::ACCENT_DISABLED))
        beautifyTask->stop();
}

void Tray::centerTaskBar()
{
	GetShellAllWnd();
	SetTaskBarPos(hTaskListWnd, hTray, hTaskWnd, hReBarWnd, TRUE);
	HWND hSecondaryTray = FindWindow(szSecondaryTray, NULL);
	while (hSecondaryTray)
	{
        HWND hSReBarWnd = FindWindowEx(hSecondaryTray, 0, "WorkerW", NULL);
		if (hSReBarWnd)
		{
            HWND hSTaskListWnd = FindWindowEx(hSReBarWnd, NULL, "MSTaskListWClass", NULL);
			if (hSTaskListWnd)
				SetTaskBarPos(hSTaskListWnd, hSecondaryTray, hSReBarWnd, hSReBarWnd, FALSE);
		}
		hSecondaryTray = FindWindowEx(NULL, hSecondaryTray, szSecondaryTray, NULL);
	}
}

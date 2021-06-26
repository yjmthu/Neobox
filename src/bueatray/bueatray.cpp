#include "bueatray.h"

pfnAccessibleObjectFromWindow AccessibleObjectFromWindowT;
pfnAccessibleChildren AccessibleChildrenT;
pfnDwmGetWindowAttribute pDwmGetWindowAttribute;
const WCHAR szShellTray[] = L"Shell_TrayWnd";                //主任务栏类名
const WCHAR szSecondaryTray[] = L"Shell_SecondaryTrayWnd";   //副任务栏类名
HMODULE hOleacc = NULL;

TRAYSAVE BueaTray::TraySave =
{
	FALSE,
	FALSE,
	{ACCENT_DISABLED, ACCENT_DISABLED},
	{254, 254},
	{0x01111111, 0x01111111},
	33
};

BOOL Find(IAccessible* paccParent, int iRole, IAccessible** paccChild)         //查找任务图标UI
{
	long numChildren;
	unsigned long numFetched;
	VARIANT varChild;
	int indexCount;
	IAccessible* pChild = NULL;
	IEnumVARIANT* pEnum = NULL;
	IDispatch* pDisp = NULL;
	BOOL found = false;
	paccParent->QueryInterface(IID_IEnumVARIANT, (PVOID*)&pEnum);
	if (pEnum)
		pEnum->Reset();
	paccParent->get_accChildCount(&numChildren);
	for (indexCount = 1; indexCount <= numChildren && !found; indexCount++)
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
				pChild->get_accRole(varChild, &varRole);
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


int oleft = 0, otop = 0;

void BueaTray::SetTaskBarPos(HWND hTaskListWnd, HWND hTrayWnd, HWND hTaskWnd, HWND hReBarWnd, BOOL bMainTray)//设置任务栏图标位置
{
	if (hOleacc == NULL)
	{
		hOleacc = LoadLibrary(L"oleacc.dll");
		if (hOleacc)
		{
			AccessibleObjectFromWindowT = (pfnAccessibleObjectFromWindow)GetProcAddress(hOleacc, "AccessibleObjectFromWindow");
			AccessibleChildrenT = (pfnAccessibleChildren)GetProcAddress(hOleacc, "AccessibleChildren");
		}
		if (hOleacc == NULL)
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
	long childCount;
	long returnCount;
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
	BOOL Vertical = FALSE;
	if (src.right - src.left < src.bottom - src.top)
		Vertical = TRUE;
	SendMessage(hReBarWnd, WM_SETREDRAW, TRUE, 0);
	int lr, tb;
	if (Vertical)
	{
		int t = trc.left - src.left;
		int b = src.bottom - trc.bottom;
		if (t > b)
			tb = t;
		else
			tb = b;
	}
	else
	{
		int l = trc.left - src.left;
		int r = src.right - trc.right;
		if (l > r)
			lr = l;
		else
			lr = r;
	}
	int nleft = 0, ntop = 0;
	if (((int)iPos == 2 || (Vertical == FALSE && tWidth >= trc.right - trc.left - lr) || (Vertical && tHeight >= trc.bottom - trc.top - tb)) && iPos != 0)
	{
		if (Vertical)
			ntop = trc.bottom - trc.top - tHeight;
		else
			nleft = trc.right - trc.left - tWidth;
	}
	else if ((int)iPos == 0)
	{
		nleft = 0;
		ntop = 0;                                               //SetTimer(hMain, 11, 1000, NULL);
		centerTask->stop();
	}
	else if ((int)iPos == 1)
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
	if (iPos != 0)
		SendMessage(hReBarWnd, WM_SETREDRAW, FALSE, 0);
	ShowWindow(hTaskWnd, SW_SHOWNOACTIVATE);
}


//BOOL CALLBACK IsZoomedFunc(const HWND hwnd, const LPARAM lpAram)
//{
//	if (IsWindow(hwnd) && IsWindowVisible(hwnd) && IsWindowEnabled(hwnd))
//	{                                                     // 传入的句柄是否为窗口，是否活动，是否可见
//		int win_lenth = GetWindowTextLength(hwnd) + 1;    // 获取窗口名称字符串长度
//		char* lpstring = new char[win_lenth];             // 用于存储窗口名称
//		GetWindowTextA(hwnd, lpstring, win_lenth);        // 获取窗口名称
//		RECT rctA = { 0, 0, 0, 0 };                         // 用于存储窗口大小
//		LPRECT win_size = &rctA;
//		GetWindowRect(hwnd, win_size);                    // 获取窗口几何信息
//		int W = win_size->right - win_size->left;         // 计算窗口宽度
//		int H = win_size->bottom - win_size->top;         // 计算窗口高度
//		if ((W / 2 == VarBox.SCREEN_WIDTH) && (H / 2 == VarBox.SCREEN_HEIGHT))  // 算出来的窗口大小为实际大小的两倍，所以要除以2
//		{
//			if ((QString(lpstring).compare(QString("Microsoft Text Input Application")))
//				&&
//				(QString(lpstring).compare(QString("Program Manager")))
//				&&
//				(QString(lpstring).compare(QString("")))) //排除三个无影响的窗口
//			{
//				VarBox.FULL_SCRENN = true;                 // 检测到全屏窗口
//				delete[] lpstring;
//				return FALSE;
//			}
//			else
//			{
//				delete[] lpstring;
//			}
//		}
//		else
//		{
//            delete[] lpstring;
//            if (MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST) == (HMONITOR)lpAram)
//            {
//                BOOL Attribute = FALSE;
//                if (pDwmGetWindowAttribute)
//                    pDwmGetWindowAttribute(hwnd, 14, &Attribute, sizeof(BOOL));
//                if (Attribute == FALSE)
//                {
//                    BueaTray::TraySave.WHEN_FULL = FALSE;
//                    return FALSE;
//                }
//            }
//		}
//	}
//    return TRUE;
//}

BOOL CALLBACK IsZoomedFunc(HWND hWnd, LPARAM lpAram)
{
    if (::IsWindowVisible(hWnd) && IsZoomed(hWnd))
    {
        if (MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST) == (HMONITOR)lpAram)
        {
            BOOL Attribute = FALSE;
            if (pDwmGetWindowAttribute)
                pDwmGetWindowAttribute(hWnd, 14, &Attribute, sizeof(BOOL));
            if (Attribute == FALSE)
            {
                BueaTray::TraySave.WHEN_FULL = 1;
                return FALSE;
            }
        }
    }
    return TRUE;
}


void BueaTray::GetShellAllWnd()
{
	while (IsWindow(hTray) == FALSE)
	{
		hTray = FindWindow(szShellTray, NULL);
		if (hTray == NULL)
			Sleep(100);
	}
	while (IsWindow(hReBarWnd) == FALSE)
	{
		hReBarWnd = FindWindowEx(hTray, 0, L"ReBarWindow32", NULL);
		if (hReBarWnd == NULL)
			Sleep(100);
	}
	if (IsWindow(hTaskWnd) == FALSE)
		hTaskWnd = FindWindowEx(hReBarWnd, NULL, L"MSTaskSwWClass", NULL);
	if (IsWindow(hTaskListWnd) == FALSE)
		hTaskListWnd = FindWindowEx(hTaskWnd, NULL, L"MSTaskListWClass", NULL);
}

void BueaTray::getValue(const WCHAR key[], int& value)
{
	HKEY pKey;
	RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_ALL_ACCESS, &pKey);
	if (pKey)
	{
		DWORD dType = REG_DWORD;
        DWORD cbData = sizeof(int);
		RegQueryValueEx(pKey, key, 0, &dType, (BYTE*)&value, &cbData);
		RegCloseKey(pKey);
	}
}

bool BueaTray::delKey(const WCHAR key[])
{
    HKEY pKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_ALL_ACCESS, &pKey);
    if (pKey)
    {
        DWORD dType = REG_DWORD;
        DWORD cbData = sizeof(int);
        int enable_sec;
        if ((RegQueryValueEx(pKey, key, 0, &dType, (BYTE*)&enable_sec, &cbData) == ERROR_SUCCESS)
            &&
            (RegDeleteValue(pKey, key) == ERROR_SUCCESS))
        {
            RegCloseKey(pKey);
            return true;
        }
        else
            RegCloseKey(pKey);
    }
    return false;
}

void BueaTray::setKey(const WCHAR key[], BOOL value)
{
    HKEY pKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_ALL_ACCESS, &pKey);
    if (pKey)
    {
        RegSetValueEx(pKey, key, 0, REG_DWORD, (BYTE*)&value, sizeof(value));
        RegCloseKey(pKey);
    }
}

bool BueaTray::checkKey(const WCHAR key[])
{
    HKEY pKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, TASK_DESK_SUB, 0, KEY_READ, &pKey);
    if (pKey)
    {
        DWORD dType = REG_DWORD;
        DWORD cbData = sizeof(BOOL);
        BOOL enable_sec;
        if (RegQueryValueEx(pKey, key, 0, &dType, (BYTE*)&enable_sec, &cbData) == ERROR_SUCCESS)
        {
            RegCloseKey(pKey);
            return true;
        }
        else
            RegCloseKey(pKey);
    }
    return false;
}

BueaTray::BueaTray()
{
	readstyle();

	beautifyTask = new QTimer;
	beautifyTask->setInterval(TraySave.refresh_time);
	connect(beautifyTask, SIGNAL(timeout()), this, SLOT(keepTaskBar()));
	beautifyTask->start();

	centerTask = new QTimer;
	centerTask->setInterval(500);
	connect(centerTask, SIGNAL(timeout()), this, SLOT(centerTaskBar()));
	centerTask->start();
}

BueaTray::~BueaTray()
{
	A(delete centerTask;)
		A(delete beautifyTask;)
		if (hOleacc)
			FreeLibrary(hOleacc);
}

void BueaTray::readstyle()
{
	QString file_name = FuncBox::get_son_dir(FuncBox::get_dat_path() + "/ini") + "/TaskStyle.ini";
	if (QFile::exists(file_name))
	{
        D("开始读取风格配置文件。");
			QSettings IniRead(file_name, QSettings::IniFormat);
		TraySave.aMode[0] = (_ACCENT_STATE)(IniRead.value("aMode_f").toUInt());
		TraySave.aMode[1] = (_ACCENT_STATE)(IniRead.value("aMode_s").toUInt());
		TraySave.dAlphaColor[0] = IniRead.value("dAlphaColor_f").toULongLong();
		TraySave.dAlphaColor[1] = IniRead.value("dAlphaColor_s").toULongLong();
		TraySave.bAlpha[0] = IniRead.value("bAlpha_f").toUInt();
		TraySave.bAlpha[1] = IniRead.value("bAlpha_s").toUInt();
		TraySave.refresh_time = IniRead.value("refresh_time").toInt();
		iPos = (TaskBarCenterState)IniRead.value("iPos").toInt();
        D("风格配置文件读取完毕");
	}
}

void BueaTray::savestyle()
{
    D("开始保存任务栏风格文件。");
		QSettings IniWrite(FuncBox::get_son_dir(FuncBox::get_dat_path() + "/ini") + "/TaskStyle.ini", QSettings::IniFormat);
	IniWrite.setValue("aMode_f", (int)TraySave.aMode[0]);
	IniWrite.setValue("aMode_s", (int)TraySave.aMode[1]);
	IniWrite.setValue("dAlphaColor_f", (unsigned long long)TraySave.dAlphaColor[0]);
	IniWrite.setValue("dAlphaColor_s", (unsigned long long)TraySave.dAlphaColor[1]);
	IniWrite.setValue("bAlpha_f", (int)TraySave.bAlpha[0]);
	IniWrite.setValue("bAlpha_s", (int)TraySave.bAlpha[1]);
	IniWrite.setValue("refresh_time", TraySave.refresh_time);
	IniWrite.setValue("iPos", (int)iPos);
    D("配置任务栏风格文件保存完毕。");
}

BOOL SetWindowCompositionAttribute(HWND hWnd, ACCENT_STATE mode, DWORD AlphaColor)//设置窗口WIN10风格
{
    pfnSetWindowCompositionAttribute pSetWindowCompositionAttribute = NULL;
    if (mode == ACCENT_DISABLED)
    {
//		if (bAccentNormal == FALSE)
        {
            SendMessage(hWnd, WM_THEMECHANGED, 0, 0);
//			bAccentNormal = TRUE;
        }
        return TRUE;
    }
//	bAccentNormal = FALSE;
    BOOL ret = FALSE;
    HMODULE hUser = GetModuleHandle(L"user32.dll");
    if (hUser)
        pSetWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
    if (pSetWindowCompositionAttribute)
    {
        ACCENT_POLICY accent = { mode, 2, AlphaColor, 0 };
        _WINDOWCOMPOSITIONATTRIBDATA data;
        data.Attrib = WCA_ACCENT_POLICY;
        data.pvData = &accent;
        data.cbData = sizeof(accent);
        ret = pSetWindowCompositionAttribute(hWnd, &data);
    }
    return ret;
}

void BueaTray::keepTaskBar()
{
	TraySave.WHEN_FULL = FALSE;
	HWND hTray = FindWindow(szShellTray, NULL);
	if (hTray)
	{
        EnumWindows(IsZoomedFunc, (LPARAM)MonitorFromWindow(hTray, MONITOR_DEFAULTTONEAREST));
		SetWindowCompositionAttribute(hTray, TraySave.aMode[TraySave.WHEN_FULL], TraySave.dAlphaColor[TraySave.WHEN_FULL]);
		LONG_PTR exStyle = GetWindowLongPtr(hTray, GWL_EXSTYLE);
		exStyle |= WS_EX_LAYERED;
		SetWindowLongPtr(hTray, GWL_EXSTYLE, exStyle);
		SetLayeredWindowAttributes(hTray, NULL, (BYTE)TraySave.bAlpha[TraySave.WHEN_FULL], LWA_ALPHA);
	}
	HWND hSecondaryTray = FindWindow(szSecondaryTray, NULL);
	while (hSecondaryTray)
	{
        EnumWindows(IsZoomedFunc, (LPARAM)MonitorFromWindow(hSecondaryTray, MONITOR_DEFAULTTONEAREST));
		SetWindowCompositionAttribute(hSecondaryTray, TraySave.aMode[TraySave.WHEN_FULL], TraySave.dAlphaColor[TraySave.WHEN_FULL]);
		LONG_PTR exStyle = GetWindowLongPtr(hSecondaryTray, GWL_EXSTYLE);
		exStyle |= WS_EX_LAYERED;
		SetWindowLongPtr(hSecondaryTray, GWL_EXSTYLE, exStyle);
		SetLayeredWindowAttributes(hSecondaryTray, NULL, (BYTE)TraySave.bAlpha[TraySave.WHEN_FULL], LWA_ALPHA);
		hSecondaryTray = FindWindowEx(NULL, hSecondaryTray, szSecondaryTray, NULL);
	}
    if ((TraySave.aMode[1] == ACCENT_DISABLED)&&(TraySave.aMode[0] == ACCENT_DISABLED))
        beautifyTask->stop();
}

void BueaTray::centerTaskBar()
{
	GetShellAllWnd();
	SetTaskBarPos(hTaskListWnd, hTray, hTaskWnd, hReBarWnd, TRUE);
	HWND hSecondaryTray;
	hSecondaryTray = FindWindow(szSecondaryTray, NULL);
	while (hSecondaryTray)
	{
		HWND hSReBarWnd = FindWindowEx(hSecondaryTray, 0, L"WorkerW", NULL);
		if (hSReBarWnd)
		{
			HWND hSTaskListWnd = FindWindowEx(hSReBarWnd, NULL, L"MSTaskListWClass", NULL);
			if (hSTaskListWnd)
				SetTaskBarPos(hSTaskListWnd, hSecondaryTray, hSReBarWnd, hSReBarWnd, FALSE);
		}
		hSecondaryTray = FindWindowEx(NULL, hSecondaryTray, szSecondaryTray, NULL);
	}
}

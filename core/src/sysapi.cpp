#include "sysapi.h"
#include <yjson.h>
#include <stdio.h>
#include <iostream>

std::unique_ptr<YJson> m_GlobalSetting;
const char* m_szClobalSettingFile = "Setting.json";

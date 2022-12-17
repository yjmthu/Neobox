#include <plugincenter.h>
#include <yjson.h>

PluginCenter::PluginCenter(YJson& setting):
  QDialog(nullptr),
  m_Setting(setting)
{
}

PluginCenter::~PluginCenter()
{
}



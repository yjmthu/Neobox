#include <weatherdlg.h>
#include <yjson.h>

WeatherDlg::WeatherDlg(YJson& settings)
  : WidgetBase(nullptr, false, true)
  , m_Settings(settings)
{
  //
}

WeatherDlg::~WeatherDlg() {
  //
}

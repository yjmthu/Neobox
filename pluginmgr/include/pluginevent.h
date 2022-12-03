#ifndef PLUGINEVENT_H
#define PLUGINEVENT_H

enum class PluginEvent: char8_t
{
  Bool,         // bool
  Int8,         // int8_t
  Int16,        // int16_t
  Int32,        // int32_t
  Int64,        // int64_t
  Uint8,        // uint8_t
  Uint16,       // uint16_t
  Uint32,       // uint32_t
  Uint64,       // uint64_t
  Float,        // float
  Double,       // double
  Pointer,      // _Ty *
  String,       // std::string
  Wstring,      // std::wstring
  U8string,     // std::u8string
  U16string,    // std::u16string
  U32string,    // std::u32string
  Show,
  KeyPress,
  KeyRelease,
  MouseMove,
  MouseRress,
  MouseRelease,
  MouseEnter,
  MouseLeave,
  MouseDoubleClick,
  Drop,
  HotKey
};

#endif // PLUGINEVENT_H
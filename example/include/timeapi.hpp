#pragma once

#include <cstdint>
#include <string>
#include <array>

using namespace std::literals;

inline uint8_t GetWeekNumber3(std::u8string_view week) {
  static const auto strWeeks = u8"SunMonTueWedThuFriSat"s;
  return strWeeks.find(week) / 3;
}

inline uint8_t GetWeekNumber3(std::string_view week) {
  return GetWeekNumber3(std::u8string_view(reinterpret_cast<const char8_t*>(week.data()), week.size()));
}

inline uint8_t GetMonthNumber3(std::u8string_view month) {
  static const auto strMonths = u8"JanFebMarAprMayJunJulAugSepOctNovDec"s;
  return strMonths.find(month) / 3 + 1;
}

inline uint8_t GetMonthNumber3(std::string_view month) {
  return GetMonthNumber3(std::u8string_view(reinterpret_cast<const char8_t*>(month.data()), month.size()));
}

inline uint8_t GetWeekNumber(std::u8string week) {
  static const std::array strWeeks = {
    u8"Sunday"s, u8"Monday"s, u8"Tuesday"s, u8"Wednesday"s, u8"Thursday"s, u8"Friday"s, u8"Saturday"s
  };
  return std::find(strWeeks.begin(), strWeeks.end(), week) - strWeeks.begin();
}

inline uint8_t GetWeekNumber(std::string_view week) {
  return GetWeekNumber(std::u8string(reinterpret_cast<const char8_t*>(week.data()), week.size()));
}

inline uint8_t GetMonthNumber(std::u8string_view month) {
  static const std::array strMonths = {
    u8"January"s, u8"February"s, u8"March"s, u8"April"s, u8"May"s, u8"June"s, u8"July"s, u8"August"s, u8"September"s, u8"October"s, u8"November"s, u8"December"s
  };
  return std::find(strMonths.begin(), strMonths.end(), month) - strMonths.begin() + 1;
}

inline uint8_t GetMonthNumber(std::string_view month) {
  return GetMonthNumber(std::u8string_view(reinterpret_cast<const char8_t*>(month.data()), month.size()));
}

template<typename _Return>
inline _Return GetWeekDay3(int number) {
  static const auto strWeeks = u8"SunMonTueWedThuFriSat";
  return _Return(reinterpret_cast<const _Return::value_type*>(strWeeks) + number * 3, 3);
}

template<typename _Return>
inline _Return GetWeekDay(int number) {
  static const std::array strWeeks = {
    u8"Sunday"s, u8"Monday"s, u8"Tuesday"s, u8"Wednesday"s, u8"Thursday"s, u8"Friday"s, u8"Saturday"s
  };
  return _Return(reinterpret_cast<const _Return::type*>(strWeeks[number].data()), strWeeks[number].size());
}

template<typename _Return>
inline _Return GetMonth3(int number) {
  static const auto strMonths = u8"JanFebMarAprMayJunJulAugSepOctNovDec";
  return _Return(reinterpret_cast<const _Return::type*>(strMonths) + number * 3, 3);
}

template<typename _Return>
inline _Return GetMonth(int number) {
  static const std::array strMonths = {
    u8"January"s, u8"February"s, u8"March"s, u8"April"s, u8"May"s, u8"June"s, u8"July"s, u8"August"s, u8"September"s, u8"October"s, u8"November"s, u8"December"s
  };
  if (--number > 11) number = 11;
  if (number < 0) number = 0;
  return _Return(reinterpret_cast<const _Return::type*>(strMonths[number].data()), strMonths[number].size());
}


#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace white::davisbase::sdl {

/* TODO: Use strong typedefs instead. */
using PageNo = int32_t;
using PageCount = PageNo;
using RowId = int32_t;
using PageLength = uint16_t;
using CellOffset = PageLength;
using CellIndex = PageLength;
using CellCount = PageLength;
using PayloadLength = PageLength;

constexpr PageNo NULL_PAGE_NO = -1;
constexpr RowId NULL_ROW_ID = -1;

enum class PageType : uint8_t
{
  INDEX_INTERIOR = 0x02,
  TABLE_INTERIOR = 0x05,
  INDEX_LEAF = 0x0A,
  TABLE_LEAF = 0x0D,
};

/* I am looking at you std::endian... in 2019. */
static const bool is_machine_big_endian = false;
// (*reinterpret_cast<const uint8_t*>((uint16_t){0x00FF}) == 0x00);

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr>
static constexpr T endian_reverse(T x) noexcept
{
  auto cx = reinterpret_cast<char*>(&x);
  for (size_t i = 0; i < sizeof(x) / 2; i++)
    std::swap(cx[i], cx[sizeof(x) - 1 - i]);
  return x;
}

template<typename T>
static constexpr auto serialized(T data)
{
  if (is_machine_big_endian)
    return data;
  return endian_reverse(data);
}

template<typename T>
static constexpr auto deserialized(T data)
{
  if (is_machine_big_endian)
    return data;
  return endian_reverse(data);
}

template<typename T>
static constexpr auto enum_cast(const T& e)
{
  return static_cast<std::underlying_type_t<T>>(e);
};

template<typename T>
static constexpr const T& offset_cast(const char* base_addr, size_t offset = 0)
{
  return *reinterpret_cast<const T*>(base_addr + offset);
}

template<typename T>
static constexpr T& offset_cast(char* base_addr, size_t offset = 0)
{
  return *reinterpret_cast<T*>(base_addr + offset);
}

} // namespace white::davisbase::sdl

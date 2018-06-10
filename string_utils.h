#ifndef NONSTD_STRING_UTILS_H
#define NONSTD_STRING_UTILS_H


#include <string>
#include <string_view>
#include <locale>
#include <vector>


namespace nonstd::string_utils::detail
{


template<typename I, typename F> void transform(I start, I stop, F func)
{
  while (start != stop) {
    *start = func(*start);
    start++;
  }
}


template<typename I> inline bool compare(I a, I b, const I last)
{
  if (*a != *b)
    return false;

  while (b != last && *(++a) == *(++b));

  if (b == last)
    return true;

  return false;
}


template <typename T> std::vector<T> split_keep_empty(std::string_view sv, std::string_view token)
{
  std::size_t start = 0;
  auto i = sv.find(token);
  std::vector<T> parts;

  while (i != sv.npos) {
    parts.emplace_back(sv.substr(start, i - start));
    start = i + token.size();
    i = sv.find(token, start);
  }
  parts.emplace_back(sv.substr(start));

  return parts;
}


template <typename T> std::vector<T> split_ignore_empty(std::string_view sv, std::string_view token)
{
  std::size_t start = 0;
  auto i = sv.find(token);
  std::vector<T> parts;

  while (i != sv.npos) {
    if (auto len = i - start; len > 0)
      parts.emplace_back(sv.substr(start, len));
    start = i + token.size();
    i = sv.find(token, start);
  }
  if (sv.size() - start > 0)
    parts.emplace_back(sv.substr(start));

  return parts;
}


}  // namepsace nonstd::string_utils::detail


// The following functions expect ASCII (or single byte) encoding
//

namespace nonstd::string_utils::ascii  
{


void to_upper(std::string& s)
{
  detail::transform(std::begin(s), std::end(s), ::toupper);
}


void to_lower(std::string& s)
{
  detail::transform(std::begin(s), std::end(s), ::tolower);
}


std::string as_upper(std::string_view sv)
{
  std::string s;
  s.reserve(sv.size());
  for (auto c : sv)
    s += ::toupper(c);
  return s;
}


std::string as_lower(std::string_view sv)
{
  std::string s;
  s.reserve(sv.size());
  for (auto c : sv)
    s += ::tolower(c);
  return s;
}


// Splits sv each character_count characters, skipping skip characters after each split
// E.g.: split(sv, 3)    : "abcdef123"   -> "abc", "def" and "123"
//       split(sv, 3, 1) : "abc,def,123" -> "abc", "def" and "123"
std::vector<std::string_view> split(std::string_view sv, std::size_t character_count,
    std::size_t skip = 0)
{
  if (character_count == 0)
    return {};

  std::vector<std::string_view> v;
  auto size = sv.size();
  std::size_t i = 0;
  while (i < size) {
    v.push_back(sv.substr(i, character_count));
    i += character_count + skip;
  }

  return v;
}


}  // namespace nonstd::string_utils::ascii


namespace nonstd::string_utils
{


// The following functions are not Unicode aware and simply do byte comparisons
//

bool starts_with(std::string_view sv, std::string_view test)
{
  if (sv.empty() || test.empty() || test.size() > sv.size())
    return false;
  return detail::compare(std::begin(sv), std::begin(test), std::end(test));
}


bool ends_with(std::string_view sv, std::string_view test)
{
  if (sv.empty() || test.empty() || test.size() > sv.size())
    return false;
  return detail::compare(std::rbegin(sv), std::rbegin(test), std::rend(test));
}


std::vector<std::string_view> split(std::string_view sv, std::string_view token,
    bool keep_empty_parts = true)
{
  if (keep_empty_parts)
    return detail::split_keep_empty<std::string_view>(sv, token);
  return detail::split_ignore_empty<std::string_view>(sv, token);
}


std::vector<std::string> split_copy(std::string_view sv, std::string_view token,
    bool keep_empty_parts = true)
{
  if (keep_empty_parts)
    return detail::split_keep_empty<std::string>(sv, token);
  return detail::split_ignore_empty<std::string>(sv, token);
}


std::tuple<std::string_view, std::string_view> split_first(std::string_view sv,
    std::string_view token)
{
  if (auto i = sv.find(token); i != sv.npos) {
    return {sv.substr(0, i), sv.substr(i+token.size())};
  }

  return {sv, {}};
}


std::tuple<std::string, std::string> split_first_copy(std::string_view sv,
    std::string_view token)
{
  if (auto i = sv.find(token); i != sv.npos) {
    return {std::string{sv.substr(0, i)}, std::string{sv.substr(i+token.size())}};
  }

  return {std::string{sv}, {}};
}


std::string replace(std::string_view sv, std::string_view search_token,
    std::string_view replace_token)
{
  std::vector<std::size_t> positions;
  auto pos = sv.find(search_token);
  while (pos != sv.npos) {
    positions.push_back(pos);
    pos = sv.find(search_token, pos + search_token.size());
  }
  if (positions.empty())
    return std::string{sv};

  std::string result;
  auto n = positions.size();
  result.resize(sv.size() - search_token.size() * n + replace_token.size() * n);
  auto result_it = std::begin(result);
  auto source_it = std::begin(sv);
  for (auto p : positions) {
    result_it = std::copy(source_it, std::begin(sv) + p, result_it);
    result_it = std::copy(std::begin(replace_token), std::end(replace_token), result_it);
    source_it = std::begin(sv) + p + search_token.size();
  }
  if (result_it != std::end(result)) {
    std::copy(source_it, std::end(sv), result_it);
  }

  return result;
}


}  // namespace nonstd::string_utils


#endif  // NONSTD_STRING_UTILS_H

#include "extensions/quic_listeners/quiche/platform/string_utils.h"

// NOLINT(namespace-envoy)
//
// This file is part of the QUICHE platform implementation, and is not to be
// consumed or referenced directly by other Envoy code. It serves purely as a
// porting layer for QUICHE.

#include <cstring>
#include <string>

#include "envoy/common/platform.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_format.h"
#include "common/common/assert.h"
#include "common/common/base64.h"

namespace quiche {

void Base64Encode(const uint8_t* data, size_t data_len, std::string* output) {
  *output =
      Envoy::Base64::encode(reinterpret_cast<const char*>(data), data_len, /*add_padding=*/false);
}

std::string HexDump(absl::string_view data) {
  const int kBytesPerLine = 16;
  const char* buf = data.data();
  int bytes_remaining = data.size();
  int offset = 0;
  std::string out;
  const char* p = buf;
  while (bytes_remaining > 0) {
    const int line_bytes = std::min(bytes_remaining, kBytesPerLine);
    absl::StrAppendFormat(&out, "0x%04x:  ", offset); // Do the line header
    for (int i = 0; i < kBytesPerLine; ++i) {
      if (i < line_bytes) {
        absl::StrAppendFormat(&out, "%02x", p[i]);
      } else {
        out += "  "; // two-space filler instead of two-space hex digits
      }
      if (i % 2) {
        out += ' ';
      }
    }
    out += ' ';
    for (int i = 0; i < line_bytes; ++i) { // Do the ASCII dump
      out += absl::ascii_isgraph(p[i]) ? p[i] : '.';
    }

    bytes_remaining -= line_bytes;
    offset += line_bytes;
    p += line_bytes;
    out += '\n';
  }
  return out;
}

char HexDigitToInt(char c) {
  ASSERT(std::isxdigit(c));

  if (std::isdigit(c)) {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return 0;
}

bool HexDecodeToUInt32(absl::string_view data, uint32_t* out) {
  if (data.empty() || data.size() > 8u) {
    return false;
  }

  for (char c : data) {
    if (!absl::ascii_isxdigit(c)) {
      return false;
    }
  }

  // Pad with leading zeros.
  std::string data_padded(data.data(), data.size());
  data_padded.insert(0, 8u - data.size(), '0');

  std::string byte_string = absl::HexStringToBytes(data_padded);

  ASSERT(byte_string.size() == 4u);
  *out = ntohl(*reinterpret_cast<const uint32_t*>(byte_string.c_str()));
  return true;
}

} // namespace quiche

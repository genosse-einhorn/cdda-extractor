// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef SENSE_H
#define SENSE_H

#include <QString>

#include "toc.h"

namespace cdda {

#pragma pack(push, 1)
struct sense_data {
  unsigned char errorCode :7;
  unsigned char valid :1;
  unsigned char segmentNumber;
  unsigned char senseKey  :4;
  unsigned char reserved  :1;
  unsigned char incorrectLength  :1;
  unsigned char endOfMedia  :1;
  unsigned char fileMark  :1;
  unsigned char information[4];
  unsigned char additionalSenseLength;
  unsigned char commandSpecificInformation[4];
  unsigned char additionalSenseCode;
  unsigned char additionalSenseCodeQualifier;
  unsigned char fieldReplaceableUnitCode;
  unsigned char senseKeySpecific[3];
};
#pragma pack(pop)

QString sense_to_string(const sense_data &sense);

enum result_sense sense_to_enum(const sense_data &sense);

} // namespace cdda

#endif // SENSE_H

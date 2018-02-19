#ifndef SENSE_H
#define SENSE_H

#include <QString>

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

} // namespace cdda

#endif // SENSE_H

#include "sense.h"

namespace {

struct sense_table_row {
    unsigned char key;
    unsigned char asc;
    unsigned char ascq;
    const char *desc;
};

sense_table_row sense_table[] = {
    // some text stolen from the MMC-5 spec
    { 0x0, 0x00, 0x00, "NO ADDITIONAL SENSE INFORMATION"},
    { 0xB, 0x00, 0x06, "I/O PROCESS TERMINATED"},
    { 0x5, 0x00, 0x16, "OPERATION IN PROGRESS"},
    { 0x4, 0x00, 0x17, "CLEANING REQUESTED"},
    { 0x3, 0x02, 0x00, "NO SEEK COMPLETE"},
    { 0x2, 0x04, 0x00, "LOGICAL UNIT NOT READY, CAUSE NOT REPORTABLE"},
    { 0x2, 0x04, 0x01, "LOGICAL UNIT IS IN PROCESS OF BECOMING READY"},
    { 0x2, 0x04, 0x02, "LOGICAL UNIT NOT READY, INITIALIZING CMD. REQUIRED"},
    { 0x2, 0x04, 0x03, "LOGICAL UNIT NOT READY, MANUAL INTERVENTION REQUIRED"},
    { 0x0, 0x04, 0x04, "LOGICAL UNIT NOT READY, FORMAT IN PROGRESS"},
    { 0x2, 0x04, 0x04, "LOGICAL UNIT NOT READY, FORMAT IN PROGRESS"},
    { 0x2, 0x04, 0x07, "LOGICAL UNIT NOT READY, OPERATION IN PROGRESS"},
    { 0x2, 0x04, 0x08, "LOGICAL UNIT NOT READY, LONG WRITE IN PROGRESS"},
    { 0x2, 0x04, 0x09, "LOGICAL UNIT NOT READY, SELF-TEST IN PROGRESS"},
    { 0x4, 0x05, 0x00, "LOGICAL UNIT DOES NOT RESPOND TO SELECTION"},
    { 0x3, 0x06, 0x00, "NO REFERENCE POSITION FOUND"},
    { 0x4, 0x08, 0x00, "LOGICAL UNIT COMMUNICATION FAILURE"},
    { 0x4, 0x08, 0x01, "LOGICAL UNIT COMMUNICATION TIMEOUT"},
    { 0x4, 0x08, 0x02, "LOGICAL UNIT COMMUNICATION PARITY ERROR"},
    { 0x4, 0x08, 0x03, "LOGICAL UNIT COMMUNICATION CRC ERROR (ULTRA-DMA/32)"},
    { 0x4, 0x09, 0x00, "TRACK FOLLOWING ERROR"},
    { 0x4, 0x09, 0x01, "TRACKING SERVO FAILURE"},
    { 0x4, 0x09, 0x02, "FOCUS SERVO FAILURE"},
    { 0x4, 0x09, 0x03, "SPINDLE SERVO FAILURE"},
    { 0x4, 0x09, 0x04, "HEAD SELECT FAULT"},
    { 0x6, 0x0A, 0x00, "ERROR LOG OVERFLOW"},
    { 0x1, 0x0B, 0x00, "WARNING"},
    { 0x1, 0x0B, 0x01, "WARNING - SPECIFIED TEMPERATURE EXCEEDED"},
    { 0x1, 0x0B, 0x02, "WARNING - ENCLOSURE DEGRADED"},
    { 0x1, 0x0B, 0x03, "WARNING - BACKGROUND SELF-TEST FAILED"},
    { 0x1, 0x0B, 0x04, "WARNING - BACKGROUND PRE-SCAN DETECTED MEDIUM ERROR"},
    { 0x1, 0x0B, 0x05, "WARNING - BACKGROUND MEDIUM SCAN DETECTED MEDIUM ERROR"},
    { 0x3, 0x0C, 0x00, "WRITE ERROR"},
    { 0x1, 0x0C, 0x01, "WRITE ERROR - RECOVERED WITH AUTO-REALLOCATION"},
    { 0x3, 0x0C, 0x02, "WRITE ERROR - AUTO-REALLOCATION FAILED"},
    { 0x3, 0x0C, 0x03, "WRITE ERROR - RECOMMEND REASSIGNMENT"},
    { 0x2, 0x0C, 0x07, "WRITE ERROR - RECOVERY NEEDED"},
    { 0x3, 0x0C, 0x07, "WRITE ERROR - RECOVERY NEEDED"},
    { 0x3, 0x0C, 0x08, "WRITE ERROR - RECOVERY FAILED"},
    { 0x3, 0x0C, 0x09, "WRITE ERROR - LOSS OF STREAMING"},
    { 0x3, 0x0C, 0x0A, "WRITE ERROR - PADDING BLOCKS ADDED"},
    { 0x2, 0x0C, 0x0F, "DEFECTS IN ERROR WINDOW"},
    { 0x3, 0x11, 0x00, "UNRECOVERED READ ERROR"},
    { 0x3, 0x11, 0x01, "READ RETRIES EXHAUSTED"},
    { 0x3, 0x11, 0x02, "ERROR TOO LONG TO CORRECT"},
    { 0x3, 0x11, 0x05, "L-EC UNCORRECTABLE ERROR"},
    { 0x3, 0x11, 0x06, "CIRC UNRECOVERED ERROR"},
    { 0x3, 0x11, 0x0F, "ERROR READING UPC/EAN NUMBER"},
    { 0x3, 0x11, 0x10, "ERROR READING ISRC NUMBER"},
    { 0xB, 0x11, 0x11, "READ ERROR - LOSS OF STREAMING"},
    { 0x3, 0x15, 0x00, "RANDOM POSITIONING ERROR"},
    { 0x4, 0x15, 0x00, "RANDOM POSITIONING ERROR"},
    { 0x3, 0x15, 0x01, "MECHANICAL POSITIONING ERROR"},
    { 0x4, 0x15, 0x01, "MECHANICAL POSITIONING ERROR"},
    { 0x3, 0x15, 0x02, "POSITIONING ERROR DETECTED BY READ OF MEDIUM"},
    { 0x1, 0x17, 0x00, "RECOVERED DATA WITH NO ERROR CORRECTION APPLIED"},
    { 0x1, 0x17, 0x01, "RECOVERED DATA WITH RETRIES"},
    { 0x1, 0x17, 0x02, "RECOVERED DATA WITH POSITIVE HEAD OFFSET"},
    { 0x1, 0x17, 0x03, "RECOVERED DATA WITH NEGATIVE HEAD OFFSET"},
    { 0x1, 0x17, 0x04, "RECOVERED DATA WITH RETRIES AND/OR CIRC APPLIED"},
    { 0x1, 0x17, 0x05, "RECOVERED DATA USING PREVIOUS SECTOR ID"},
    { 0x1, 0x17, 0x07, "RECOVERED DATA WITHOUT ECC - RECOMMEND REASSIGNMENT"},
    { 0x1, 0x17, 0x08, "RECOVERED DATA WITHOUT ECC - RECOMMEND REWRITE"},
    { 0x1, 0x17, 0x09, "RECOVERED DATA WITHOUT ECC - DATA REWRITTEN"},
    { 0x1, 0x18, 0x00, "RECOVERED DATA WITH ERROR CORRECTION APPLIED"},
    { 0x1, 0x18, 0x01, "RECOVERED DATA WITH ERROR CORR. & RETRIES APPLIED"},
    { 0x1, 0x18, 0x02, "RECOVERED DATA - DATA AUTO-REALLOCATED"},
    { 0x1, 0x18, 0x03, "RECOVERED DATA WITH CIRC"},
    { 0x1, 0x18, 0x04, "RECOVERED DATA WITH L-EC"},
    { 0x1, 0x18, 0x05, "RECOVERED DATA - RECOMMEND REASSIGNMENT"},
    { 0x1, 0x18, 0x06, "RECOVERED DATA - RECOMMEND REWRITE"},
    { 0x1, 0x18, 0x08, "RECOVERED DATA WITH LINKING"},
    { 0x5, 0x1A, 0x00, "PARAMETER LIST LENGTH ERROR"},
    { 0x4, 0x1B, 0x00, "SYNCHRONOUS DATA TRANSFER ERROR"},
    { 0xA, 0x1D, 0x00, "MISCOMPARE DURING VERIFY OPERATION"},
    { 0x5, 0x20, 0x00, "INVALID COMMAND OPERATION CODE"},
    { 0x5, 0x21, 0x00, "LOGICAL BLOCK ADDRESS OUT OF RANGE"},
    { 0x5, 0x21, 0x01, "INVALID ELEMENT ADDRESS"},
    { 0x5, 0x21, 0x02, "INVALID ADDRESS FOR WRITE"},
    { 0x5, 0x21, 0x03, "INVALID WRITE CROSSING LAYER JUMP"},
    { 0x5, 0x22, 0x00, "ILLEGAL FUNCTION"},
    { 0x5, 0x24, 0x00, "INVALID FIELD IN CDB"},
    { 0x5, 0x25, 0x00, "LOGICAL UNIT NOT SUPPORTED"},
    { 0x5, 0x26, 0x00, "INVALID FIELD IN PARAMETER LIST"},
    { 0x5, 0x26, 0x01, "PARAMETER NOT SUPPORTED"},
    { 0x5, 0x26, 0x02, "PARAMETER VALUE INVALID"},
    { 0x5, 0x26, 0x03, "THRESHOLD PARAMETERS NOT SUPPORTED"},
    { 0x5, 0x26, 0x04, "INVALID RELEASE OF PERSISTENT RESERVATION"},
    { 0x7, 0x27, 0x00, "WRITE PROTECTED"},
    { 0x7, 0x27, 0x01, "HARDWARE WRITE PROTECTED"},
    { 0x7, 0x27, 0x02, "LOGICAL UNIT SOFTWARE WRITE PROTECTED"},
    { 0x7, 0x27, 0x03, "ASSOCIATED WRITE PROTECT"},
    { 0x7, 0x27, 0x04, "PERSISTENT WRITE PROTECT"},
    { 0x7, 0x27, 0x05, "PERMANENT WRITE PROTECT"},
    { 0x7, 0x27, 0x06, "CONDITIONAL WRITE PROTECT"},
    { 0x6, 0x28, 0x00, "NOT READY TO READY CHANGE, MEDIUM MAY HAVE CHANGED"},
    { 0x6, 0x28, 0x01, "IMPORT OR EXPORT ELEMENT ACCESSED"},
    { 0x6, 0x28, 0x02, "FORMAT-LAYER MAY HAVE CHANGED"},
    { 0x6, 0x29, 0x00, "POWER ON, RESET, OR BUS DEVICE RESET OCCURRED"},
    { 0x6, 0x29, 0x01, "POWER ON OCCURRED"},
    { 0x6, 0x29, 0x02, "BUS RESET OCCURRED"},
    { 0x6, 0x29, 0x03, "BUS DEVICE RESET FUNCTION OCCURRED"},
    { 0x6, 0x29, 0x04, "DEVICE INTERNAL RESET"},
    { 0x6, 0x2A, 0x00, "PARAMETERS CHANGED"},
    { 0x6, 0x2A, 0x01, "MODE PARAMETERS CHANGED"},
    { 0x6, 0x2A, 0x02, "LOG PARAMETERS CHANGED"},
    { 0x6, 0x2A, 0x03, "RESERVATIONS PREEMPTED"},
    { 0x5, 0x2C, 0x00, "COMMAND SEQUENCE ERROR"},
    { 0x5, 0x2C, 0x03, "CURRENT PROGRAM AREA IS NOT EMPTY"},
    { 0x5, 0x2C, 0x04, "CURRENT PROGRAM AREA IS EMPTY"},
    { 0x6, 0x2E, 0x00, "INSUFFICIENT TIME FOR OPERATION"},
    { 0x6, 0x2F, 0x00, "COMMANDS CLEARED BY ANOTHER INITIATOR"},
    { 0x2, 0x30, 0x00, "INCOMPATIBLE MEDIUM INSTALLED"},
    { 0x5, 0x30, 0x00, "INCOMPATIBLE MEDIUM INSTALLED"},
    { 0x2, 0x30, 0x01, "CANNOT READ MEDIUM - UNKNOWN FORMAT"},
    { 0x5, 0x30, 0x01, "CANNOT READ MEDIUM - UNKNOWN FORMAT"},
    { 0x2, 0x30, 0x02, "CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"},
    { 0x5, 0x30, 0x02, "CANNOT READ MEDIUM - INCOMPATIBLE FORMAT"},
    { 0x2, 0x30, 0x03, "CLEANING CARTRIDGE INSTALLED"},
    { 0x5, 0x30, 0x03, "CLEANING CARTRIDGE INSTALLED"},
    { 0x2, 0x30, 0x04, "CANNOT WRITE MEDIUM - UNKNOWN FORMAT"},
    { 0x5, 0x30, 0x04, "CANNOT WRITE MEDIUM - UNKNOWN FORMAT"},
    { 0x2, 0x30, 0x05, "CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"},
    { 0x5, 0x30, 0x05, "CANNOT WRITE MEDIUM - INCOMPATIBLE FORMAT"},
    { 0x2, 0x30, 0x06, "CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"},
    { 0x5, 0x30, 0x06, "CANNOT FORMAT MEDIUM - INCOMPATIBLE MEDIUM"},
    { 0x2, 0x30, 0x07, "CLEANING FAILURE"},
    { 0x5, 0x30, 0x07, "CLEANING FAILURE"},
    { 0x5, 0x30, 0x08, "CANNOT WRITE - APPLICATION CODE MISMATCH"},
    { 0x5, 0x30, 0x09, "CURRENT SESSION NOT FIXATED FOR APPEND"},
    { 0x5, 0x30, 0x10, "MEDIUM NOT FORMATTED"},
    { 0x2, 0x30, 0x11, "CANNOT WRITE MEDIUM - UNSUPPORTED MEDIUM VERSION"},
    { 0x5, 0x30, 0x11, "CANNOT WRITE MEDIUM - UNSUPPORTED MEDIUM VERSION"},
    { 0x3, 0x31, 0x00, "MEDIUM FORMAT CORRUPTED"},
    { 0x3, 0x31, 0x01, "FORMAT COMMAND FAILED"},
    { 0x3, 0x31, 0x02, "ZONED FORMATTING FAILED DUE TO SPARE LINKING"},
    { 0x3, 0x32, 0x00, "NO DEFECT SPARE LOCATION AVAILABLE"},
    { 255, 0x34, 0x00, "ENCLOSURE FAILURE"},
    { 255, 0x35, 0x00, "ENCLOSURE SERVICES FAILURE"},
    { 255, 0x35, 0x01, "UNSUPPORTED ENCLOSURE FUNCTION"},
    { 255, 0x35, 0x02, "ENCLOSURE SERVICES UNAVAILABLE"},
    { 255, 0x35, 0x03, "ENCLOSURE SERVICES TRANSFER FAILURE"},
    { 255, 0x35, 0x04, "RNCLOSURE SERVICES TRANSFER REFUSED"},
    { 255, 0x35, 0x05, "ENCLOSURE SERVICES CHECKSUM ERROR"},
    { 0x1, 0x37, 0x00, "ROUNDED PARAMETER"},
    { 0x5, 0x39, 0x00, "SAVING PARAMETERS NOT SUPPORTED"},
    { 0x2, 0x3A, 0x00, "MEDIUM NOT PRESENT"},
    { 0x2, 0x3A, 0x01, "MEDIUM NOT PRESENT - TRAY CLOSED"},
    { 0x2, 0x3A, 0x02, "MEDIUM NOT PRESENT - TRAY OPEN"},
    { 0x2, 0x3A, 0x03, "MEDIUM NOT PRESENT - LOADABLE"},
    { 0x6, 0x3B, 0x0D, "MEDIUM DESTINATION ELEMENT FULL"},
    { 0x6, 0x3B, 0x0E, "MEDIUM SOURCE ELEMENT EMPTY"},
    { 0x6, 0x3B, 0x0F, "END OF MEDIUM REACHED"},
    { 0x6, 0x3B, 0x11, "MEDIUM MAGAZINE NOT ACCESSIBLE"},
    { 0x6, 0x3B, 0x12, "MEDIUM MAGAZINE REMOVED"},
    { 0x6, 0x3B, 0x13, "MEDIUM MAGAZINE INSERTED"},
    { 0x6, 0x3B, 0x14, "MEDIUM MAGAZINE LOCKED"},
    { 0x6, 0x3B, 0x15, "MEDIUM MAGAZINE UNLOCKED"},
    { 0x4, 0x3B, 0x16, "MECHANICAL POSITIONING OR CHANGER ERROR"},
    { 0x5, 0x3D, 0x00, "INVALID BITS IN IDENTIFY MESSAGE"},
    { 0x2, 0x3E, 0x00, "LOGICAL UNIT HAS NOT SELF-CONFIGURED YET"},
    { 0x4, 0x3E, 0x01, "LOGICAL UNIT FAILURE"},
    { 0x4, 0x3E, 0x02, "TIMEOUT ON LOGICAL UNIT"},
    { 0x6, 0x3F, 0x00, "TARGET OPERATING CONDITIONS HAVE CHANGED"},
    { 0x6, 0x3F, 0x01, "MICROCODE HAS BEEN CHANGED"},
    { 0x6, 0x3F, 0x02, "CHANGED OPERATING DEFINITION"},
    { 0x6, 0x3F, 0x03, "INQUIRY DATA HAS CHANGED"},
    { 0x4, 0x40, 0xff, "DIAGNOSTIC FAILURE ON COMPONENT"},
    { 0x5, 0x43, 0x00, "MESSAGE ERROR"},
    { 0x4, 0x44, 0x00, "INTERNAL TARGET FAILURE"},
    { 0xB, 0x45, 0x00, "SELECT OR RESELECT FAILURE"},
    { 0x4, 0x46, 0x00, "UNSUCCESSFUL SOFT RESET"},
    { 0x4, 0x47, 0x00, "SCSI PARITY ERROR"},
    { 0xB, 0x48, 0x00, "INITIATOR DETECTED ERROR MESSAGE RECEIVED"},
    { 0xB, 0x49, 0x00, "INVALID MESSAGE ERROR"},
    { 0x4, 0x4A, 0x00, "COMMAND PHASE ERROR"},
    { 0x4, 0x4B, 0x00, "DATA PHASE ERROR"},
    { 0x4, 0x4C, 0x00, "LOGICAL UNIT FAILED SELF-CONFIGURATION"},
    { 0xB, 0x4D, 0xff, "TAGGED OVERLAPPED COMMANDS"},
    { 0xB, 0x4E, 0x00, "OVERLAPPED COMMANDS ATTEMPTED"},
    { 0x3, 0x51, 0x00, "ERASE FAILURE"},
    { 0x3, 0x51, 0x01, "ERASE FAILURE - INCOMPLETE ERASE OPERATION DETECTED"},
    { 0x4, 0x53, 0x00, "MEDIA LOAD OR EJECT FAILED"},
    { 0x5, 0x53, 0x02, "MEDIUM REMOVAL PREVENTED"},
    { 0x5, 0x55, 0x00, "SYSTEM RESOURCE FAILURE"},
    { 0x3, 0x57, 0x00, "UNABLE TO RECOVER TABLE-OF-CONTENTS"},
    { 0x6, 0x5A, 0x00, "OPERATOR REQUEST OR STATE CHANGE INPUT"},
    { 0x6, 0x5A, 0x01, "OPERATOR MEDIUM REMOVAL REQUEST"},
    { 0x6, 0x5A, 0x02, "OPERATOR SELECTED WRITE PROTECT"},
    { 0x6, 0x5A, 0x03, "OPERATOR SELECTED WRITE PERMIT"},
    { 0x6, 0x5B, 0x00, "LOG EXCEPTION"},
    { 0x6, 0x5B, 0x01, "THRESHOLD CONDITION MET"},
    { 0x6, 0x5B, 0x02, "LOG COUNTER AT MAXIMUM"},
    { 0x6, 0x5B, 0x03, "LOG LIST CODES EXHAUSTED"},
    { 0x1, 0x5D, 0x00, "FAILURE PREDICTION THRESHOLD EXCEEDED"},
    { 0x1, 0x5D, 0x01, "MEDIA FAILURE PREDICTION THRESHOLD EXCEEDED"},
    { 0x1, 0x5D, 0x02, "LOGICAL UNIT FAILURE PREDICTION THRESHOLD EXCEEDED"},
    { 0x1, 0x5D, 0x03, "SPARE AREA EXHAUSTION FAILURE PREDICTION THRESHOLD EXCEEDED"},
    { 0x1, 0x5D, 0xFF, "FAILURE PREDICTION THRESHOLD EXCEEDED (FALSE)"},
    { 0x6, 0x5E, 0x00, "LOW POWER CONDITION ON"},
    { 0x6, 0x5E, 0x01, "IDLE CONDITION ACTIVATED BY TIMER"},
    { 0x6, 0x5E, 0x02, "STANDBY CONDITION ACTIVATED BY TIMER"},
    { 0x6, 0x5E, 0x03, "IDLE CONDITION ACTIVATED BY COMMAND"},
    { 0x6, 0x5E, 0x04, "STANDBY CONDITION ACTIVATED BY COMMAND"},
    { 0x5, 0x63, 0x00, "END OF USER AREA ENCOUNTERED ON THIS TRACK"},
    { 0x5, 0x63, 0x01, "PACKET DOES NOT FIT IN AVAILABLE SPACE"},
    { 0x5, 0x64, 0x00, "ILLEGAL MODE FOR THIS TRACK"},
    { 0x5, 0x64, 0x01, "INVALID PACKET SIZE"},
    { 0x4, 0x65, 0x00, "VOLTAGE FAULT"},
    { 0x5, 0x6F, 0x00, "COPY PROTECTION KEY EXCHANGE FAILURE - AUTHENTICATION FAILURE"},
    { 0x5, 0x6F, 0x01, "COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT PRESENT"},
    { 0x5, 0x6F, 0x02, "COPY PROTECTION KEY EXCHANGE FAILURE - KEY NOT ESTABLISHED"},
    { 0x5, 0x6F, 0x03, "READ OF SCRAMBLED SECTOR WITHOUT AUTHENTICATION"},
    { 0x5, 0x6F, 0x04, "MEDIA REGION CODE IS MISMATCHED TO LOGICAL UNIT REGION"},
    { 0x5, 0x6F, 0x05, "LOGICAL UNIT REGION MUST BE PERMANENT/REGION RESET COUNT ERROR"},
    { 0x5, 0x6F, 0x06, "INSUFFICIENT BLOCK COUNT FOR BINDING NONCE RECORDING"},
    { 0x5, 0x6F, 0x07, "CONFLICT IN BINDING NONCE RECORDING"},
    { 0x3, 0x72, 0x00, "SESSION FIXATION ERROR"},
    { 0x3, 0x72, 0x01, "SESSION FIXATION ERROR WRITING LEAD-IN"},
    { 0x3, 0x72, 0x02, "SESSION FIXATION ERROR WRITING LEAD-OUT"},
    { 0x5, 0x72, 0x03, "SESSION FIXATION ERROR - INCOMPLETE TRACK IN SESSION"},
    { 0x5, 0x72, 0x04, "EMPTY OR PARTIALLY WRITTEN RESERVED TRACK"},
    { 0x5, 0x72, 0x05, "NO MORE TRACK RESERVATIONS ALLOWED"},
    { 0x5, 0x72, 0x06, "RMZ EXTENSION IS NOT ALLOWED"},
    { 0x5, 0x72, 0x07, "NO MORE TEST ZONE EXTENSIONS ARE ALLOWED"},
    { 0x3, 0x73, 0x00, "CD CONTROL ERROR"},
    { 0x1, 0x73, 0x01, "POWER CALIBRATION AREA ALMOST FULL"},
    { 0x3, 0x73, 0x02, "POWER CALIBRATION AREA IS FULL"},
    { 0x3, 0x73, 0x03, "POWER CALIBRATION AREA ERROR"},
    { 0x3, 0x73, 0x04, "PROGRAM MEMORY AREA UPDATE FAILURE"},
    { 0x3, 0x73, 0x05, "PROGRAM MEMORY AREA IS FULL"},
    { 0x1, 0x73, 0x06, "RMA/PMA IS ALMOST FULL"},
    { 0x3, 0x73, 0x10, "CURRENT POWER CALIBRATION AREA IS ALMOST FULL"},
    { 0x3, 0x73, 0x11, "CURRENT POWER CALIBRATION AREA IS FULL"},
    { 0x5, 0x73, 0x17, "RDZ IS FULL"},
    { 0x8, 0xff, 0xff, "BLANK CHECK"}
};

QString byte_buffer_to_hex_str(const unsigned char *buf, size_t len)
{
    static const char *digits = "0123456789ABCDEF";
    char str[len*2+1];
    for (size_t i = 0; i < len; ++i)
    {
        str[2*i]   = digits[buf[i] >> 4];
        str[2*i+1] = digits[buf[i] & 15];
    }
    str[2*len] = 0;
    return QString::fromLatin1(str);
}

} // anonymous namespace

namespace cdda {

QString sense_to_string(const sense_data &sense)
{
    QString senseKeyStr;

    switch (sense.senseKey)
    {
    case 0x0:
        senseKeyStr = QStringLiteral("No Sense: ");
        break;
    case 0x1:
        senseKeyStr = QStringLiteral("Recovered Error: ");
        break;
    case 0x2:
        senseKeyStr = QStringLiteral("Not Ready: ");
        break;
    case 0x3:
        senseKeyStr = QStringLiteral("Medium Error: ");
        break;
    case 0x4:
        senseKeyStr = QStringLiteral("Hardware Error: ");
        break;
    case 0x5:
        senseKeyStr = QStringLiteral("Illegal Request: ");
        break;
    case 0x6:
        senseKeyStr = QStringLiteral("Unit Attention: ");
        break;
    case 0x7:
        senseKeyStr = QStringLiteral("Data Protect: ");
        break;
    case 0x8:
        senseKeyStr = QStringLiteral("Blank Check: ");
        break;
    case 0x9:
        senseKeyStr = QStringLiteral("Vendor Specific Error: ");
        break;
    case 0xa:
        senseKeyStr = QStringLiteral("Copy Aborted: ");
        break;
    case 0xB:
        senseKeyStr = QStringLiteral("Aborted Command: ");
        break;
    case 0xC:
        senseKeyStr = QStringLiteral("Equal: ");
        break;
    case 0xD:
        senseKeyStr = QStringLiteral("Volume Overflow: ");
        break;
    case 0xE:
        senseKeyStr = QStringLiteral("Miscompare: ");
        break;
    }

    for (size_t i = 0; i < sizeof(sense_table)/sizeof(sense_table[0]); ++i)
    {
        if ((sense_table[i].key == 0xff  || sense_table[i].key == sense.senseKey)
                && (sense_table[i].asc == 0xff || sense_table[i].asc == sense.additionalSenseCode)
                && (sense_table[i].ascq == 0xff || sense_table[i].ascq == sense.additionalSenseCodeQualifier))
        {
            return QStringLiteral("%1%2").arg(senseKeyStr, QString::fromLatin1(sense_table[i].desc));
        }
    }

    // Not found? Emit sense buffer as hex for debugging
    return QStringLiteral("Unknown SCSI sense: %1%2").arg(senseKeyStr, byte_buffer_to_hex_str((unsigned char*)&sense, sizeof(sense)));
}

} // namespace cdda
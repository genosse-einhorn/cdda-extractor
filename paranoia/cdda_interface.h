/******************************************************************
 * CopyPolicy: GNU Lesser General Public License 2.1 applies
 * Copyright (C) 2001-2008 Xiph.org
 * Original version by Heiko Eissfeldt heiko@escape.colossus.de
 *
 * Toplevel interface header; applications include this
 *
 ******************************************************************/

#ifndef _cdda_interface_h_
#define _cdda_interface_h_

#ifndef CD_FRAMESIZE
#define CD_FRAMESIZE 2048
#endif
#ifndef CD_FRAMESIZE_RAW
#define CD_FRAMESIZE_RAW 2352
#endif
#define CD_FRAMESAMPLES (CD_FRAMESIZE_RAW / 4)

#include <sys/types.h>
#include <time.h>

/* cdrom access function pointer */

typedef struct cdrom_drive {
  void *userdata;
  int nsectors;
  long firstsector;

  long (*cdda_read_func)(void *userdata, void *buffer, long begin, long sectors);
} cdrom_drive;

static inline long cdda_read(cdrom_drive *d, void *buffer,
                long beginsector, long sectors)
{
    return d->cdda_read_func(d->userdata, buffer, beginsector, sectors);
}

static inline long cdda_read_timed(cdrom_drive *d, void *buffer,
                long beginsector, long sectors, int *milliseconds)
{
    struct timespec ts1, ts2;
    clock_gettime(CLOCK_MONOTONIC, &ts1);

    long rv = cdda_read(d, buffer, beginsector, sectors);

    clock_gettime(CLOCK_MONOTONIC, &ts2);

    double starttime = (double)ts1.tv_sec + 1.0e-9 * (double)ts1.tv_nsec;
    double endtime = (double)ts2.tv_sec + 1.0e-9 * (double)ts2.tv_nsec;
    *milliseconds = (int)((endtime - starttime) * 1000.0);

    return rv;
}

static inline long cdda_disc_firstsector(cdrom_drive *d)
{
    return d->firstsector;
}

#endif


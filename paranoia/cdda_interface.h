#ifndef CDDA_INTERFACE_H
#define CDDA_INTERFACE_H

#ifndef CD_FRAMESIZE
#define CD_FRAMESIZE 2048
#endif
#ifndef CD_FRAMESIZE_RAW
#define CD_FRAMESIZE_RAW 2352
#endif
#define CD_FRAMESAMPLES (CD_FRAMESIZE_RAW / 4)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int nsectors;
    long (*cdda_read_func)(void *userdata, void *buffer, long beginsector, long endsector);
    void *userdata;
} cdrom_drive;

static inline long cdda_read(cdrom_drive *d, void *buffer,
                      long beginsector, long sectors)
{
    return d->cdda_read_func(d->userdata, buffer, beginsector, sectors);
}

#ifdef __cplusplus
}
#endif

#endif // CDDA_INTERFACE_H

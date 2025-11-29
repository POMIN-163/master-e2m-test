#ifndef FILED_BUS_H
#define FILED_BUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "soem/soem.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

    typedef struct
    {
        ecx_contextt context;
        char        *iface;
        uint8        group;
        int          roundtrip_time;
        uint8        map[4096];
        boolean      is_running;
    } Fieldbus;

    void    fieldbus_initialize(Fieldbus *fieldbus, char *iface);
    int     fieldbus_roundtrip(Fieldbus *fieldbus);
    boolean fieldbus_start(Fieldbus *fieldbus);
    void    fieldbus_stop(Fieldbus *fieldbus);
    boolean fieldbus_dump(Fieldbus *fieldbus);
    void    fieldbus_check_state(Fieldbus *fieldbus);

#ifdef __cplusplus
}
#endif
#endif  // FILED_BUS_H

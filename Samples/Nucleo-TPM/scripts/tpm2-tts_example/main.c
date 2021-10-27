#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <tss2/tss2_tcti_device.h>

int main() {
    TSS2_RC rc;
    TSS2_TCTI_CONTEXT *tcti_context;
    size_t size;
//    char *conf = "/dev/ttyACM1";
    char *conf = "/tmp/ttyclient";

    rc = Tss2_Tcti_Device_Init (NULL, &size, NULL);
    if (rc != TSS2_RC_SUCCESS) {
        fprintf (stderr, "Failed to get allocation size for device TCTI context: 0x%x", rc);
        exit (EXIT_FAILURE);
    }
    tcti_context = calloc (1, size);
    if (tcti_context == NULL) {
        fprintf (stderr, "Allocation for TCTI context failed: %s", strerror(errno));
        exit (EXIT_FAILURE);
    }
    rc = Tss2_Tcti_Device_Init (tcti_context, &size, conf);
    if (rc != TSS2_RC_SUCCESS) {
        fprintf (stderr, "Failed to initialize device TCTI context: 0x%x", rc);
        free (tcti_context);
        exit (EXIT_FAILURE);
    }
    exit (EXIT_SUCCESS);
}
//TSS2_TCTI_CONTEXT *tctiContext


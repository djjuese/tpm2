#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "src/Tpm.h"
#include "src/Platform_fp.h"
#include "src/PlatformACT_fp.h"
#include "src/ExecCommand_fp.h"
#include "src/Manufacture_fp.h"
#include "src/_TPM_Init_fp.h"
#include "src/_TPM_Hash_Start_fp.h"
#include "src/_TPM_Hash_Data_fp.h"
#include "src/_TPM_Hash_End_fp.h"
#include "src/TpmFail_fp.h"
#include "src/TpmTcpProtocol.h"
#include "src/Simulator_fp.h"

// extern BOOL g_inFailureMode;
static BOOL reportedFailureCommand;
typedef uint32_t TPM_RESULT;

void main(void);
static TPM_RESULT TPMLIB_MainInit(void);
static void TPMLIB_Terminate(void);
static TPM_RESULT TPMLIB_Process(unsigned char **respbuffer, uint32_t *resp_size,
                               uint32_t *respbufsize,
                               unsigned char *command, uint32_t command_size);

static void test_CC_Create_Primary(void);
static void test_CC_pcr_read(void);
static void test_CC_selftest(void);

#define TPM_BASE 0
#define TPM_BUFFER_MAX 0x0800 /* 2k bytes */
#define TPM_SIZE TPM_BASE + 23


void main(void)
{
    TPM_RC rc;
    int i;

    MemoryCopy(g_toTest,
		       g_implementedAlgorithms,
		       sizeof(g_toTest));

    for (i = TPM_ALG_FIRST; i <= TPM_ALG_LAST; i++)
    {
        printf("alg %d test start:\n", i);
        rc = TestAlgorithm(i, g_toTest);
        printf("TPM alg %d test result : %x \n", i, rc);
    }

    // rc =  CryptSelfTest(YES);
    // printf("TPM test result : %x \n", rc);

    printf("TPM test_CC_Create_Primary:");
    test_CC_Create_Primary();
    
    printf("TPM test_CC_pcr_read: ");
    test_CC_pcr_read();
    
    printf("TPM test_CC_selftest:");
    test_CC_selftest();

    printf("tpm test is DONE!\n");

}

static void test_CC_selftest(void)
{
    unsigned char *rbuffer = NULL;
    uint32_t rlength;
    uint32_t rtotal = 0;
    TPM_RESULT res;
    int ret = 1;
    unsigned char tpm2_startup[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01, 0x44, 0x00, 0x00};
    unsigned char tpm2_selftest[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00,
        0x01, 0x43, 0x01};
    const unsigned char tpm2_selftest_resp[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
        0x00, 0x00};

    res = TPMLIB_MainInit();
    if (res)
    {
        printf("TPMLIB_MainInit() failed: 0x%02x\n", res);
        goto exit;
    }

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_startup, sizeof(tpm2_startup));
    if (res)
    {
        printf("TPMLIB_Process(Startup) failed: 0x%02x\n", res);
        goto exit;
    }

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_selftest, sizeof(tpm2_selftest));
    if (res)
    {
        printf("TPMLIB_Process(TPM2_Self) failed: 0x%02x\n",
                res);
        goto exit;
    }

    if (memcmp(rbuffer, tpm2_selftest_resp, rlength))
    {
        printf("Expected response from TPM2_Selftest is different than received one.\n");
        goto exit;
    }

    printf("OK\n");

    ret = 0;

exit:
    TPMLIB_Terminate();
    free(rbuffer);

    return ;//ret;
}

static void test_CC_pcr_read(void)
{
    unsigned char *rbuffer = NULL;
    uint32_t rlength;
    uint32_t rtotal = 0;
    TPM_RESULT res;
    int ret = 1;
    unsigned char *perm = NULL;
    uint32_t permlen = 0;
    unsigned char *vol = NULL;
    uint32_t vollen = 0;
    unsigned char startup[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01, 0x44, 0x00, 0x00};

    unsigned char tpm2_pcr_read[] = {
        0x80, 0x01,             // TPM_ST_NO_SESSIONS
        0x00, 0x00, 0x00, 0x26, // command size
        0x00, 0x00, 0x01, 0x7e, // TPM_CC_PCR_Read
        0x00, 0x00, 0x00, 0x04, // TPML_PCR_SELECTION
        0x00, 0x04,             // TPMI_ALG_HASH, SHA1=4
        0x03,                   // size of the select
        0x01, 0x00, 0x10,       // pcrSelect
        0x00, 0x0b,             // TPMI_ALG_HASH, SHA256=11
        0x03,                   // size of the select
        0x01, 0x00, 0x10,       // pcrSelect
        0x00, 0x0c,             // TPMI_ALG_HASH, SHA384=12
        0x03,                   // size of the select
        0x01, 0x00, 0x10,       // pcrSelect
        0x00, 0x0d,             // TPMI_ALG_HASH, SHA512=13
        0x03,                   // size of the select
        0x01, 0x00, 0x10        // pcrSelect
    };
    const unsigned char tpm2_pcr_read_exp_resp[] = {
        0x80, 0x01, 0x00, 0x00, 0x01, 0x86, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
        0x00, 0x04, 0x00, 0x04, 0x03, 0x01, 0x00, 0x10,
        0x00, 0x0b, 0x03, 0x01, 0x00, 0x10, 0x00, 0x0c,
        0x03, 0x01, 0x00, 0x10, 0x00, 0x0d, 0x03, 0x01,
        0x00, 0x10, 0x00, 0x00, 0x00, 0x08, 0x00, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x30,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x30, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    res = TPMLIB_MainInit();
    if (res)
    {
        printf("TPMLIB_MainInit() failed: 0x%02x\n", res);
        goto exit;
    }

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal, startup, sizeof(startup));
    if (res)
    {
        printf("TPMLIB_Process(Startup) failed: 0x%02x\n", res);
        goto exit;
    }

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_pcr_read, sizeof(tpm2_pcr_read));
    if (res)
    {
        printf("TPMLIB_Process(TPM2_PCR_Read) failed: 0x%02x\n",
                res);
        goto exit;
    }

    if (rlength != sizeof(tpm2_pcr_read_exp_resp))
    {
        printf("Expected response is %zu bytes, but got %u.\n",
                sizeof(tpm2_pcr_read_exp_resp), rlength);
        goto exit;
    }

    if (memcmp(rbuffer, tpm2_pcr_read_exp_resp, rlength))
    {
        printf("Expected response is different than received one.\n");
        // dump_array("actual:", rbuffer, rlength);
        // dump_array("expected:", tpm2_pcr_read_exp_resp, sizeof(tpm2_pcr_read_exp_resp));
        goto exit;
    }

    /* Extend PCR 10 with string '1234' */
    unsigned char tpm2_pcr_extend[] = {
        0x80, 0x02, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00,
        0x01, 0x82, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
        0x00, 0x09, 0x40, 0x00, 0x00, 0x09, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x0b, 0x31, 0x32, 0x33, 0x34, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_pcr_extend, sizeof(tpm2_pcr_extend));
    if (res != TPM_RC_SUCCESS)
    {
        printf(
                "TPMLIB_Process(TPM2_PCR_Extend) failed: 0x%02x\n", res);
        goto exit;
    }

    /* Read value of PCR 10 */
    unsigned char tpm2_pcr10_read[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
        0x01, 0x7e, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0b,
        0x03, 0x00, 0x04, 0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_pcr10_read, sizeof(tpm2_pcr10_read));
    if (res)
    {
        printf("TPMLIB_Process(PCR10 Read) failed: 0x%02x\n", res);
        goto exit;
    }

    const unsigned char tpm2_pcr10_read_resp[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x0b, 0x03, 0x00, 0x04, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x20, 0x1f, 0x7f,
        0xb1, 0x00, 0xe1, 0xb2, 0xd1, 0x95, 0x19, 0x4b,
        0x58, 0xe7, 0xc3, 0x09, 0xa5, 0x86, 0x30, 0x7c,
        0x34, 0x64, 0x19, 0xdc, 0xb2, 0xd5, 0x9f, 0x52,
        0x2b, 0xe7, 0xf0, 0x94, 0x51, 0x01};

    if (memcmp(tpm2_pcr10_read_resp, rbuffer, rlength))
    {
        printf("TPM2_PCRRead(PCR10) did not return expected result\n");
        // dump_array("actual:", rbuffer, rlength);
        // dump_array("expected:", tpm2_pcr10_read_resp, sizeof(tpm2_pcr10_read_resp));
        goto exit;
    }

    unsigned char tpm2_shutdown[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01, 0x45, 0x00, 0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_shutdown, sizeof(tpm2_shutdown));
    if (res)
    {
        printf(
                "TPMLIB_Process(Shutdown) after SetState failed: 0x%02x\n",
                res);
        goto exit;
    }

    unsigned char tpm2_shutdown_resp[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
        0x00, 0x00};

    if (memcmp(tpm2_shutdown_resp, rbuffer, rlength))
    {
        printf(
                "TPM2_PCRRead(Shutdown) after SetState did not return expected "
                "result\n");
        goto exit;
    }

    ret = 0;

    printf("OK\n");

exit:
    free(perm);
    free(vol);
    TPMLIB_Terminate();
    free(rbuffer);

    return ;//ret;
}

static void test_CC_Create_Primary(void)
{
    unsigned char *rbuffer = NULL;
    uint32_t rlength;
    uint32_t rtotal = 0;
    TPM_RESULT res;
    int ret = 1;
    unsigned char *perm = NULL;
    unsigned char *vol = NULL;
    unsigned char startup[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01, 0x44, 0x00, 0x00};

    res = TPMLIB_MainInit();
    if (res)
    {
        printf("TPMLIB_MainInit() failed: 0x%02x\n", res);
        goto exit;
    }

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal, startup, sizeof(startup));
    if (res)
    {
        printf("TPMLIB_Process(Startup) failed: 0x%02x\n",
               res);
        goto exit;
    }

    unsigned char tpm2_createprimary[] = {
        0x80, 0x02, 0x00, 0x00, 0x00, 0x43, 0x00, 0x00,
        0x01, 0x31, 0x40, 0x00, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x09, 0x40, 0x00, 0x00, 0x09, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x1a, 0x00, 0x01, 0x00, 0x0b, 0x00,
        0x03, 0x04, 0x72, 0x00, 0x00, 0x00, 0x06, 0x00,
        0x80, 0x00, 0x43, 0x00, 0x10, 0x08, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_createprimary, sizeof(tpm2_createprimary));
    if (res)
    {
        printf("TPMLIB_Process(TPM2_CreatePrimary) failed: 0x%02x\n",
               res);
        goto exit;
    }

    if (rlength != 506)
    {
        printf("Expected response is %u bytes, but got %u.\n",
               506, rlength);
        goto exit;
    }

    unsigned char tpm2_evictcontrol[] = {
        0x80, 0x02, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00,
        0x01, 0x20, 0x40, 0x00, 0x00, 0x01, 0x80, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x40, 0x00,
        0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81,
        0x00, 0x00, 0x00};

    const unsigned char tpm2_evictcontrol_exp_resp[] = {
        0x80, 0x02, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_evictcontrol, sizeof(tpm2_evictcontrol));
    if (res)
    {
        printf("TPMLIB_Process(TPM2_EvictControl) failed: %02x\n",
               res);
        goto exit;
    }

    if (rlength != sizeof(tpm2_evictcontrol_exp_resp))
    {
        printf("Expected TPM2_EvictControl response is %zu bytes, "
               "but got %u.\n",
               sizeof(tpm2_evictcontrol_exp_resp), rlength);
        goto exit;
    }

    if (memcmp(rbuffer, tpm2_evictcontrol_exp_resp, rlength))
    {
        printf("Expected TPM2_EvictControl response is different than "
               "received one.\n");
        goto exit;
    }

    /* Expecting a handle 0x81000000 for the persisted key now */
    unsigned char tpm2_getcapability[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x16, 0x00, 0x00,
        0x01, 0x7a, 0x00, 0x00, 0x00, 0x01, 0x81, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40};
    const unsigned char tpm2_getcapability_exp_resp[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x81, 0x00, 0x00, 0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_getcapability, sizeof(tpm2_getcapability));
    if (res)
    {
        printf("TPMLIB_Process(TPM2_GetCapability) failed: 0x%02x\n",
               res);
        goto exit;
    }

    if (rlength != sizeof(tpm2_getcapability_exp_resp))
    {
        printf("Expected TPM2_GetCapability response is %zu bytes, "
               "but got %u.\n",
               sizeof(tpm2_getcapability_exp_resp), rlength);
        goto exit;
    }

    if (memcmp(rbuffer, tpm2_getcapability_exp_resp, rlength))
    {
        printf(
            "Expected TPM2_GetCapability response is different than "
            "received one.\n");
        goto exit;
    }

    TPMLIB_Terminate();

    res = TPMLIB_MainInit();
    if (res)
    {
        printf("TPMLIB_MainInit() after SetState failed: 0x%02x\n",
               res);
        goto exit;
    }

    //RCP added for starup manually
    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal, startup, sizeof(startup));
    if (res)
    {
        printf("TPMLIB_Process(Startup) failed: 0x%02x\n",
               res);
        goto exit;
    }

    /* Again expecting the handle 0x81000000 for the persisted key */
    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_getcapability, sizeof(tpm2_getcapability));
    if (res)
    {
        printf(
            "TPMLIB_Process(TPM2_GetCapability) failed: 0x%02x\n",
            res);
        goto exit;
    }

    if (rlength != sizeof(tpm2_getcapability_exp_resp))
    {
        printf(
            "Expected TPM2_GetCapability response is %zu bytes,"
            "but got %u.\n",
            sizeof(tpm2_getcapability_exp_resp), rlength);
        goto exit;
    }

    if (memcmp(rbuffer, tpm2_getcapability_exp_resp, rlength))
    {
        printf(
            "Expected TPM2_GetCapability response is different than "
            "received one.\n");
        goto exit;
    }

    /* Shutdown */
    unsigned char tpm2_shutdown[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
        0x01, 0x45, 0x00, 0x00};

    res = TPMLIB_Process(&rbuffer, &rlength, &rtotal,
                         tpm2_shutdown, sizeof(tpm2_shutdown));
    if (res)
    {
        printf(
            "TPMLIB_Process(Shutdown) after SetState failed: 0x%02x\n",
            res);
        goto exit;
    }

    unsigned char tpm2_shutdown_resp[] = {
        0x80, 0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
        0x00, 0x00};

    if (memcmp(tpm2_shutdown_resp, rbuffer, rlength))
    {
        printf(
            "TPM2_PCRRead(Shutdown) after SetState did not return "
            "expected result\n");
        goto exit;
    }

    ret = 0;

    printf("OK\n");

exit:
    free(perm);
    free(vol);
    TPMLIB_Terminate();
    free(rbuffer);

    return;
}


static TPM_RESULT TPMLIB_MainInit(void)
{
    TPM_RESULT ret = TPM_RC_SUCCESS;

    g_inFailureMode = FALSE;
    reportedFailureCommand = FALSE;

    _rpc__Signal_PowerOff();

    _plat__NVDisable(0);

    _rpc__Signal_PowerOn(FALSE);

    _rpc__Signal_NvOn();
    

    ret = _plat__NVEnable(NULL);
    if (_plat__NVNeedsManufacture())
    {
        TPM_Manufacture(1);
        TPM_Manufacture(0);
        //TPM_TearDown();
        //TPM_Manufacture(1)
        printf("hashAlg=%d\n", gp.auditHashAlg);
    }
    // _plat__NVDisable(0);

    // _rpc__Signal_PowerOn(FALSE);

    // _rpc__Signal_NvOn();

    if (ret == TPM_RC_SUCCESS)
    {
        if (g_inFailureMode)
            ret = TPM_RC_FAILURE;
    }

    return ret;
}

static void TPMLIB_Terminate(void)
{
    TPM_TearDown();

    _rpc__Signal_PowerOff();
}

static TPM_RESULT TPMLIB_Process(unsigned char **respbuffer, uint32_t *resp_size,
                                 uint32_t *respbufsize,
                                 unsigned char *command, uint32_t command_size)
{
    uint8_t locality = 0;
    _IN_BUFFER req;
    _OUT_BUFFER resp;
    unsigned char *tmp;

    req.BufferSize = command_size;
    req.Buffer = command;

    /* have the TPM 2 write directly into the response buffer */
    if (*respbufsize < TPM_BUFFER_MAX || !*respbuffer)
    {
        tmp = realloc(*respbuffer, TPM_BUFFER_MAX);
        if (!tmp)
        {
            printf("Could not allocated %u bytes.\n",
                                TPM_BUFFER_MAX);
            return TPM_SIZE;
        }
        *respbuffer = tmp;
        *respbufsize = TPM_BUFFER_MAX;
    }
    resp.BufferSize = *respbufsize;
    resp.Buffer = *respbuffer;

    /*
     * signals for cancellation have to come after we start processing
     */
    _rpc__Signal_CancelOff();

    _rpc__Send_Command(locality, req, &resp);

    /* it may come back with a different buffer, especially in failure mode */
    if (resp.Buffer != *respbuffer)
    {
        if (resp.BufferSize > *respbufsize)
            resp.BufferSize = *respbufsize;
        memcpy(*respbuffer, resp.Buffer, resp.BufferSize);
    }

    *resp_size = resp.BufferSize;

    if (g_inFailureMode && !reportedFailureCommand)
    {
        reportedFailureCommand = TRUE;
        printf("%s: Entered failure mode through command:\n",
                            __func__);
        // TPMLIB_LogArray(~0, command, command_size);
    }

    return TPM_RC_SUCCESS;
}

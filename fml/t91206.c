#include <string.h>
#include "t91206.h"
#include "regdef.h"
#include "typdef.h"
#include "printk.h"
#include "delay.h"
#include "i2cm.h"


/* algorithm object string TLV: ecdsa-with-SHA256 */
const unsigned char ALGO_OBJECT[10] = {0x06,0x08,0x2a,0x86,0x48,0xce,0x3d,0x04,0x03,0x02};
/* public key object string TLV: ecPublicKey | prime256v1/secp256r1/P-256 */
const unsigned char PUBKEY_INFOR[21] = {0x30,0x13,0x06,0x07,0x2a,0x86,0x48,0xce,0x3d,0x02,0x01,0x06,0x08,0x2a,0x86,0x48,0xce,0x3d,0x03,0x01,0x07};
/* common name object string TLV:2.5.4.3 :commonName  */
const unsigned char COMMON_NAME_OBJECT[5] = {0x06,0x03,0x55,0x04,0x03};

/* Qi2.0 authentication */
unsigned char g_MaxprotocolVersion = 0;
unsigned char g_SlotPopulatedMask = 0;
/* Cache digests LSB * 4 */
unsigned char g_DigestsLSB[4] = {0};
/* Cache Certificate Chain length * 4 */
unsigned char g_CertificateLen[8] = {0};
/* Cache Manufacturer CA length * 4 */
unsigned short g_ManuCALen[4] = {0};

unsigned char apduBuf[TMC_SEND_MAX];
unsigned char RecvBuf[TMC_RECE_MAX];

/* CRC8 table */
static const unsigned char crc8Table1[16] =
{
	0x00, 0x64, 0xC8, 0xAC, 0xE1, 0x85, 0x29, 0x4D,
	0xB3, 0xD7, 0X7B, 0x1F, 0x52, 0x36, 0x9A, 0xFE,
};

/* CRC table */
static const unsigned char crc8Table2[16] =
{
	0x00, 0x17, 0x2E, 0x39, 0x5C, 0x4B, 0x72, 0x65,
	0xB8, 0xAF, 0X96, 0x81, 0xE4, 0xF3, 0xCA, 0xDD,
};

/*
 * @function     i2c_DevPowerUp
 * @brief    Power up the slave.
 */
static void i2c_PowerUp(void)
{
    GPA->DOUT.BITS.PIN7 = 1;
    delay_1us(SWI_DELAY_POWERUP_5MS);

    GPA->DOUT.BITS.PIN7 = 0;
    delay_1us(SWI_DELAY_POWERUP_5MS);

    GPA->DOUT.BITS.PIN7 = 1;
    delay_1us(SWI_DELAY_POWERUP_8MS);

    GPA->ODEN.BITS.PIN6 = 1;
    GPA->ODEN.BITS.PIN7 = 1;
}

void t91206_init(void)
{
    hal_i2cm_init(100000);
    i2c_PowerUp();
    printk("\r\n t91206_init done");
}

int t91206_get_qi_id(uint8_t *rbuf)
{
    /* Get Qi ID */
    /* Set 0 to indicate slot 0 */
    uint8_t slotNumReq = 0;
    /* length of Qi ID buffer >= 6 */
    rbuf[0] = 6;
    int ret = TMC_GetQiID(rbuf, slotNumReq);

	printk("\r\n t91206_qi_id: ");
	for (int i=0; i<6; i++)
	{
		printk("%c", *(rbuf + i));
	}

    return ret;
}
int t91206_read_cert_hash(uint8_t *rbuf)
{
    if (rbuf == NULL)
    {
        return -1;
    }

    /* Get digests from slot 0 */
    /* Set bit0 to indicate slot 0 */
    unsigned char slotMaskRet = 0x00;
    uint8_t slotMaskReq = 0x0001;
    unsigned int read_digest_len = 32;
    int ret = TMC_ReadDigests(&slotMaskRet, rbuf, &read_digest_len, slotMaskReq);

//	printk("\r\n ReadDigests ret->%X %x %x %x\r\n", ret, slotMaskRet, slotMaskReq, read_digest_len);
    printk("\r\n DIGEST:");
    for (int i = 0; i < read_digest_len; i++)
    {
        printk(" %02X",rbuf[i]);
    }

    return ret;
}

int t91206_read_se_cert(uint8_t *rbuf, uint32_t *rlen)
{
    /* prepare Get Digests response: (slots populated mask | slots returned mask) + digestsbuf */
    /* Read Certificate Chain from slot 0 */
    /* Set 0 to indicate slot 0 */
    uint8_t slotNumReq = 0;

    rbuf[0] = DIGESTS_RESPONSE;

    /* Read out the whole certificate chain in one time */
    int ret = TMC_ReadCertification(rbuf, 0, (unsigned int *)rlen, slotNumReq);

//    printk("\r\n ReadCertificationret->%X len: %d \r\n", ret, *rlen);

    int i = 0;

    printk("\r\n CERT_LEN:");
    printk("\r\n");
    for (i=0; i<2; i++)
    {
    	printk(" %02X", rbuf[i]);
    }

    printk("\r\n Root CA Hash");
    printk("\r\n");
    for (i=2; i<2+32; i++)
    {
    	printk(" %02X", rbuf[i]);
    }

    printk("\r\n Manufacturer CA Certificate");
    printk("\r\n");
    for (i=2+32; i<2+32+4; i++)
    {
    	printk(" %02X", rbuf[i]);
    }

    for (i=2+32+4; i<2+32+4+(rbuf[2+32+4-2]*256+rbuf[2+32+4-1]); i++)
    {
    	if ((i-(2+32+4)) % 32 == 0)
    	{
    		printk("\r\n");
    	}
    	printk(" %02X", rbuf[i]);
    }

    printk("\r\n Product Unit Certificate");
    printk("\r\n");
    for (i=2+32+4+(rbuf[2+32+4-2]*256+rbuf[2+32+4-1]); i<2+32+4+(rbuf[2+32+4-2]*256+rbuf[2+32+4-1])+4; i++)
    {
    	printk(" %02X", rbuf[i]);
    }

    for (i=2+32+4+(rbuf[2+32+4-2]*256+rbuf[2+32+4-1])+4; i<*rlen; i++)
    {
    	if ((i-(2+32+4+(rbuf[2+32+4-2]*256+rbuf[2+32+4-1])+4)) % 32 == 0)
    	{
    		printk("\r\n");
    	}
    	printk(" %02X", rbuf[i]);
    }


//    printk("\r\n");
//    printk("\r\n");
//    printk("\r\n");

    return ret;
}

int t91206_get_cert_chain(uint8_t *wpc_cert_hash, uint8_t *manufacturer_cert, uint16_t manufacturer_cert_len, uint8_t *rbuf, uint16_t *rlen)
{
	return 0;
}

int t91206_get_tbs_auth(uint8_t *signature , uint8_t *array_random)
{
    /* Challenge Auth with slot 0 */
    /* 16 bytes random number as challenge */

    /* Set 0 to indicate slot 0 */
    unsigned short  slotNumReq = 0;
    /* Set receive buffer size */
    unsigned int  len_Resp = 64;
    // signature[0] = CERTIFICATE_RESPONSE;
    int ret = TMC_SignChallenge(array_random, LEN_OF_CHALLENGE, slotNumReq, signature, &len_Resp);

    printk("\r\n CHALL_AUTH");

    for (int i = 0; i < len_Resp; i++)
    {
    	if (i % 32 == 0)
    	{
    		printk("\r\n");
    	}
        printk(" %02X", signature[i]);
    }

    return 0;
}


/************************************************SDK API*****************************************/

/*
 * @function    TMC_TLVParse
 * @brief       ASN.1 TLV parse.
 * @param[in]   src        points to TLV structure buffer.
 * @param[out]  pValue     point to value of current TLV structure.
 * @return      Length current TLV structure.
 */
int TMC_TLVParse(unsigned char *src, unsigned char **pValue)
{
    int Len = 0;

    if (src[0] != TAG_DER_SEQUENCE)
    {
        Len = src[1] + 2U;
        *pValue = &src[2];
    }
    else
    {
        switch (src[1])
        {
        case 0x81U:
            Len = (uint16_t)src[2] + 3U;
            *pValue = &src[3];
            break;

        case 0x82U:
            Len = (((uint16_t)src[2]) << 8) + src[3] + 4U;
            *pValue = &src[4];
            break;

        default:
            if (src[1] < 0x81U)
            {
                Len = src[1] + 2U;
                *pValue = &src[2];
            }
            break;
        }
    }

    return Len;
}

/*
 * @function    TMC_GetCAPubKey
 * @brief       certificate parse and get publick key in certificate.
 * @param[in]   pCertificate        points to current certificate TLV structure buffer.
 * @param[in]   mode        0: Get CA public key; 1: Get Qi ID.
 * @param[out]  pPubKey     Public key of current CA.
 * @return      Length current TLV structure.
 */
int TMC_GetCAPubKey(unsigned char *pCertificate, unsigned char mode, unsigned char *pPubKey)
{
    unsigned char *pSubCertificate = NULL;
    int subOffset, Len = 0;
    int ret = SUCCEED;

    /* Verify CA certificate TLV structure and get public key */
    subOffset = 0;
    pSubCertificate = pCertificate;

    /* Get sub TLV length and pointer to value */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
    /* Update subOffset and point to sub TLV */
    subOffset = pSubCertificate - pCertificate;

    /* X.509v3 version checking */
    /* Get sub TLV length and pointer to value */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
    if ((*pSubCertificate++ != TAG_INTEGER) && (*pSubCertificate++ != 0x01) && (*pSubCertificate++ != X509_V3_VERSION))
    {
        return ERR_CERTIFICATE_FORMAT;
    }
    /* Update subOffset and point to next TLV */
    subOffset += Len;

    /* tbd, SN checking */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
    subOffset += Len;

    /* Signature algorithm checking, Algo objec = ecdsa-with-SHA256 */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
    if (memcmp(pSubCertificate, ALGO_OBJECT, 10) != 0)
    {
        return ERR_CERTIFICATE_FORMAT;
    }
    subOffset += Len;

    /* tbd, Issuer checking */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
    subOffset += Len;

    /* tbd, validity checking */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
    subOffset += Len;

    /* tbd, subject name checking */
    Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);

    /* Get CA public key */
    if (mode == 0)
    {
        subOffset += Len;

        /* subject key checking */
        Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
        if (memcmp(pSubCertificate, PUBKEY_INFOR, sizeof(PUBKEY_INFOR)) != 0)
        {
            return ERR_CERTIFICATE_FORMAT;
        }
        pSubCertificate += sizeof(PUBKEY_INFOR);
        /* offset points to next TLV */
        subOffset += Len;
        Len = TMC_TLVParse(pSubCertificate, &pSubCertificate);
        /* Remove unused bits, update subOffset and point to sub TLV */
        Len = Len - (*pSubCertificate++ / 8);
        if (*pSubCertificate++ != 0x04)
        {
            return ERR_CERTIFICATE_FORMAT;
        }
        /* r value */
        memcpy(pPubKey, pSubCertificate, 64);

        /* tbd, externsion checking */
    }
    /* Get Qi ID */
    else if (mode == 1)
    {
        /* Update subOffset and point to sub TLV */
        subOffset = pSubCertificate - pCertificate;
        Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
        /* Update subOffset and point to sub TLV */
        subOffset = pSubCertificate - pCertificate;
        Len = TMC_TLVParse(pCertificate + subOffset, &pSubCertificate);
        /* object ID checking: common name, 06:03:55:04:03 */
        if (memcmp(pSubCertificate, COMMON_NAME_OBJECT, sizeof(COMMON_NAME_OBJECT)) != 0)
        {
            return ERR_CERTIFICATE_FORMAT;
        }
        pSubCertificate += sizeof(COMMON_NAME_OBJECT);
        Len = TMC_TLVParse(pSubCertificate, &pSubCertificate);
        if (Len < 8)
        {
            return ERR_CERTIFICATE_FORMAT;
        }
        /* Qi ID string */
        memcpy(pPubKey, pSubCertificate, 6);
    }
    return ret;
}

/************************************************SDK API*****************************************/

/*
 * @function    TMC_GetSEID
 * @brief       TMC_GetSEID.    Get SEID of chipset.
 * @param[out]  SEID            points to SEID buffer
 * @param[out]  length          Length of SEID information
 * @return                      SWI_SUCCEED/other
 */
int TMC_GetSEID(unsigned char *SEID, unsigned int length)
{
    int ret = SUCCEED;

    if (SEID == NULL)
    {
        return ERR_WRONG_PARAMETER;
    }
    if (length > LEN_SEID)
    {
        return ERR_WRONG_PARAMETER;
    }
    if (*SEID < length)
    {
        return ERR_WRONG_PARAMETER;
    }

    /* Read out SEID */
    ret = tmc_read_data(SEID, OFFSET_SEID, length, ID_READ_DATA);
    if (ret != SUCCEED)
    {
        return ERR_COMMUNICATION | ret;
    }

    return SUCCEED;
}

/*
 * @function    TMC_GetPopulatedMask
 * @brief       Read and initialize slot populated mask and max protocol version.
 * @param[in]   slotMaskReq         slot number of targeted digests.
 * @param[out]  slotMaskRet         Returned slots populated mask.
 * @return    SWI_SUCCEED/other
 */
int TMC_GetPopulatedMask(unsigned char *slotMaskRet, unsigned short slotMaskReq)
{
    unsigned char slotInfor[5] = {0};
    int ret = SUCCEED;
    unsigned short slotID = 0;
    int i = 0;

    if (slotMaskRet == NULL)
    {
        return ERR_WRONG_PARAMETER;
    }
    if (slotMaskReq == 0 || slotMaskReq > 0x0F)
    {
        return ERR_INVALID_REQUEST;
    }

    /* Get populated mask */
    if (g_MaxprotocolVersion == 0 || g_SlotPopulatedMask == 0)
    {
        /* Set bit0~7 of slot ID as slot number */
        slotID = 0x8000 | slotMaskReq;
        ret = tmc_read_data(slotInfor, 0, 1, slotID);
        if (ret != SUCCEED)
        {
            return ERR_COMMUNICATION | ret;
        }

        /* Get max protocol version, store in bit4-7 */
        if ((slotInfor[0] & 0xF0) != 0x10)
        {
            /* Error code UNSUPPORTED_PROTOCOL*/
            return ERR_UNSUPPORTED_PROTOCOL;
        }
        else
        {
            g_MaxprotocolVersion = slotInfor[0] >> 4;
        }

        /* Get slot populated mask */
        if ((slotInfor[0] & 0x01) != 0x01)
        {
            /* Error code handling, slot 0 not initialized */
            return ERR_UNINITIALIZED_SE;
        }
        else
        {
            g_SlotPopulatedMask = slotInfor[0] & 0x0F;
        }

        /* Get Certification Chain Hash LSB for each populated slot */
        i = 1;
        g_DigestsLSB[0] = slotInfor[i++];
        if (g_SlotPopulatedMask & 0x02)
        {
            g_DigestsLSB[1] = slotInfor[i++];
        }
        if (g_SlotPopulatedMask & 0x04)
        {
            g_DigestsLSB[2] = slotInfor[i++];
        }
        if (g_SlotPopulatedMask & 0x08)
        {
            g_DigestsLSB[3] = slotInfor[i++];
        }
    }

    /* Set return value */
    slotMaskRet[0] = g_SlotPopulatedMask;

    return ret;
}

/*
 * @function    TMC_GetProtocolVersion
 * @brief       Read Max rpotocol version.
 * @param[out]  maxProtocolVersion  Returned max protocol version supported by SE.
 * @return                          SWI_SUCCEED/other
 */
int TMC_GetProtocolVersion(unsigned char *maxProtocolVersion)
{
    unsigned char slotReturnedMask = 0;
    int ret = SUCCEED;

    if (maxProtocolVersion == NULL)
    {
        return ERR_WRONG_PARAMETER;
    }

    /* Get populated mask infor */
    if (g_MaxprotocolVersion == 0 || g_SlotPopulatedMask == 0)
    {
        /* Read slot infor, slot populated mask | slot returned mask */
        ret = TMC_GetPopulatedMask(&slotReturnedMask, 0x000F);
        if (ret)
        {
            return ret;
        }
    }
    /* Set protocol version */
    maxProtocolVersion[0] = g_MaxprotocolVersion;

    return ret;
}

/*
 * @function    TMC_GetCertificateChainLSB
 * @brief       Read certificate chain hash LSB indicated by slot number.
 * @param[out]  CertChainHashLSB    Returned certificate chain hash LSB indicated by slot number.
 * @param[in]   slotNumReq          Slot number of certificate chain hash LSB to be read.
 * @return    SWI_SUCCEED/other
 */
int TMC_GetCertificateChainLSB(unsigned char *CertChainHashLSB, unsigned short slotNumReq)
{
    unsigned char slotReturnedMask = 0;
    int ret = SUCCEED;

    if (CertChainHashLSB == NULL)
    {
        return ERR_WRONG_PARAMETER;
    }

    if (slotNumReq > 3)
    {
        return ERR_INVALID_REQUEST;
    }

    /* Get populated mask infor */
    if (g_MaxprotocolVersion == 0 || g_SlotPopulatedMask == 0)
    {
        /* Read slot infor, slot populated mask | slot returned mask */
        ret = TMC_GetPopulatedMask(&slotReturnedMask, 0x000F);
        if (ret)
        {
            return ret;
        }
    }

    if ((1 << slotNumReq) & g_SlotPopulatedMask)
    {
        CertChainHashLSB[0] = g_DigestsLSB[slotNumReq];
    }
    else
    {
        ret = ERR_INVALID_REQUEST;
    }

    return ret;
}

/*
 * @function    TMC_ReadDigests
 * @brief       Read digests.
 * @param[in]   slotMaskReq[bit3:0] slot number of targeted digests.
 * @param[out]  slotMaskRet         Returned slots information, slots populated mask | slots returned mask.
 * @param[out]  digests             Digests buffer.
 * @param[out]  outLen              Returned digests length.
 * @return                          SWI_SUCCEED/other
 */
int TMC_ReadDigests(unsigned char *slotMaskRet, unsigned char *digests, unsigned int *outLen, unsigned short slotMaskReq)
{

    unsigned char slotReturnedMask = 0;
    unsigned char slotPopulatedMask = 0;

    unsigned short slotID = 0;
    int i = 0;
    int offset = 0;
    int ret = SUCCEED;

    if ((slotMaskRet == NULL) || (digests == NULL) || (outLen == NULL))
    {
        printk("ERR_WRONG_PARAMETER\n");
        return ERR_WRONG_PARAMETER;
    }
    if (slotMaskReq == 0 || slotMaskReq > 0x0F)
    {
        printk("ERR_INVALID_REQUEST\n");
        return ERR_INVALID_REQUEST;
    }
    /* Get max digests length with requested slot mask */
    for (i = 0; i < 4; i++)
    {
        if (slotMaskReq & (0x01 << i))
        {
            offset += LEN_OF_DIGESTS;
        }
    }
    if (*outLen < offset)
    {
        printk("ERR_WRONG_PARAMETER\n");
        return ERR_WRONG_PARAMETER;
    }

    /* Get populated mask infor */
    if (g_MaxprotocolVersion == 0 || g_SlotPopulatedMask == 0)
    {
        /* Read slot infor, slot populated mask | slot returned mask */
        ret = TMC_GetPopulatedMask(&slotPopulatedMask, 0x000F);
        if (ret != SUCCEED)
        {
            return ret;
        }
    }
    /* Set slot returned infor */
    slotReturnedMask = (unsigned char)(slotMaskReq & g_SlotPopulatedMask);
    slotMaskRet[0] = (g_SlotPopulatedMask << 4) | slotReturnedMask;

    /* Get digests by populated mask */
    offset = 0;
    if (slotReturnedMask)
    {
        for (i = 0; i < 4; i++)
        {
            if (slotReturnedMask & 0x01)
            {
                /* Get digests */
                /* Set bit12~15 of slot ID as slot number (0:3 indicte certificate from slot 0:3; 4:7 indicate digests of slot 0:3) */
                slotID = (i + 4) << 12;

                ret = tmc_read_data(digests + offset, 0, LEN_OF_DIGESTS, slotID);
                if (ret != SUCCEED)
                {
                    return ERR_COMMUNICATION | ret;
                }
                offset += LEN_OF_DIGESTS;
            }
            slotReturnedMask >>= 1;
        }
        /* Set output digests length */
        outLen[0] = offset;
    }
    else
    {
        /* tbd, error handling in case requested slot not contains certificate chain */
        /* Set output digests length */
        outLen[0] = 0;
    }
    return ret;
}

/*
 * @function    TMC_ReadCertification
 * @brief       Read certification.
 * @param[in]   offset              Offset of certification.
 * @param[in]   length              Length to be read.
 * @param[in]   slotNumber          slot number of targeted certificte chain.
 * @param[out]  certification       Certification buffer.
 * @return                          SWI_SUCCEED/other
 */
int TMC_ReadCertification(unsigned char *certification, unsigned int offset, unsigned int *length, unsigned short slotNumber)
{
    int certOffset, certLen = 0;
    int ret = 0;
    unsigned short slotID = 0;
    unsigned char manuCALen[4] = {0};

    if ((certification == NULL))
    {
        return ERR_WRONG_PARAMETER;
    }

    if (slotNumber > 3)
    {
        return ERR_INVALID_REQUEST;
    }

    /* Check targeted certificate chain length */
    if (g_CertificateLen[slotNumber * 2] == 0)
    {
        /* Read out Certificate Chain total length */
        /* Set bit12~15 of slot ID as slot number (0:3 indicte certificate from slot 0:3; 4:7 indicate digests of slot 0:3) */
        slotID = slotNumber << 12;
        ret = tmc_read_data(g_CertificateLen + slotNumber * 2, 0, 2, slotID);
        if (ret != SUCCEED)
        {
            return ERR_COMMUNICATION | ret;
        }
    }

    /* Check targeted manufacturer CA length */
    if (g_ManuCALen[slotNumber] == 0)
    {
        /* Read out manufacturer CA length */
        /* Set bit12~15 of slot ID as slot number (0:3 indicte certificate from slot 0:3; 4:7 indicate digests of slot 0:3) */
        slotID = slotNumber << 12;
        ret = tmc_read_data(manuCALen, 2 + LEN_OF_N_RH, 4, slotID);
        if (ret != SUCCEED)
        {
            return ERR_COMMUNICATION | ret;
        }
        /* Check manufacturer length */
        if (manuCALen[0] == 0x30 && manuCALen[1] == 0x82)
        {
            g_ManuCALen[slotNumber] = manuCALen[2] * 256 + manuCALen[3] + 4;
        }
        else
        {
            return ERR_CERTIFICATE_FORMAT;
        }
    }

    /* Certification chain offset and length adaption */
    certOffset = offset;
    certLen = *length;
    if (certOffset >= 0x600)
    {
        certOffset -= 0x600;
        certOffset = certOffset + 2 + LEN_OF_N_RH + g_ManuCALen[slotNumber];
    }
    else if (certOffset >= (g_CertificateLen[slotNumber * 2] * 256 + g_CertificateLen[slotNumber * 2 + 1]))
    {
        return ERR_INVALID_REQUEST;
    }
    if (certLen == 0)
    {
        certLen = g_CertificateLen[slotNumber * 2] * 256 + g_CertificateLen[slotNumber * 2 + 1] - certOffset;
    }

    if ((certOffset + certLen) > (g_CertificateLen[slotNumber * 2] * 256 + g_CertificateLen[slotNumber * 2 + 1]))
    {
        return ERR_INVALID_REQUEST;
    }

    /* Check return buffer size */
    if ((certification[0] * 256 + certification[1]) < certLen)
    {
        return ERR_WRONG_PARAMETER;
    }

    /* Set bit12~15 of slot ID as slot number (0:3 indicte certificate from slot 0:3; 4:7 indicate digests of slot 0:3) */
    slotID = slotNumber << 12;
    ret = tmc_read_data(certification, certOffset, certLen, slotID);
    if (ret != SUCCEED)
    {
        ret = ERR_COMMUNICATION | ret;
    }
    else
    {
        *length = certLen;
    }
    return ret;
}

/*
 * @function    TMC_SignChallenge
 * @brief       Send random to slave and get signature r&s.
 * @param[in]   random          The random data send to slave, to calc sign.
 * @param[in]   length          Length of random.(16)
 * @param[in]   slotNumber      slot number of targeted certificte chain.
 * @param[out]  signature       Signature buffer.
 * @param[out]  signLen         Signature length.
 * @return                      SWI_SUCCEED/other
 */
int TMC_SignChallenge(unsigned char *random, unsigned int length, unsigned short slotNumber, unsigned char *signature, unsigned int *signLen)
{
    unsigned int outLen = 0;
    int ret = SUCCEED;
    unsigned short slotID = 0;

    if ((random == NULL) || (signature == NULL) || (signLen == NULL))
    {
        return ERR_WRONG_PARAMETER;
    }

    if (slotNumber > 3)
    {
        return ERR_INVALID_REQUEST;
    }
    if (length != LEN_OF_CHALLENGE)
    {
        return ERR_INVALID_REQUEST;
    }
    /* Buffer size checking */
    if (*signLen < LEN_OF_SIGN)
    {
        return ERR_WRONG_PARAMETER;
    }

    /* Set bit12~15 of slot ID as slot number */
    slotID = slotNumber;
    ret = tmc_ecc_signature(random, length, signature, &outLen, slotID);
    if (ret != SUCCEED)
    {
        return ERR_COMMUNICATION;
    }
    else if (outLen != LEN_OF_SIGN)
    {
        /* error handling, signature length not match */
        return ERR_CHALLENGE_SIGNING;
    }

    /* Set return value */
    signLen[0] = outLen;

    return ret;
}

/*
 * @function    TMC_GetQiID
 * @brief       Get Qi ID from product unit CA certificate.
 * @param[in]   slotNumber          slot number of targeted certificte chain.
 * @param[out]  QiID                point to Qi ID string.
 * @return                          SWI_SUCCEED/other
 */
int TMC_GetQiID(unsigned char *QiID, unsigned short slotNumber)
{
    unsigned char ProdUnitCertificate[APDU_BODY_MAX_SIZE] = {0};
    unsigned char *pCertificate = NULL;
    unsigned int Length = 0;
    int ret = SUCCEED;
    unsigned char slotPopulatedMask = 0;

    if (QiID == NULL)
    {
        return ERR_WRONG_PARAMETER;
    }
    /* Cehck return buffer length */
    if (QiID[0] < 6)
    {
        return ERR_WRONG_PARAMETER;
    }
    /* Check stlot number */
    if (slotNumber > 3)
    {
        return ERR_INVALID_REQUEST;
    }

#ifdef ENABLE_CACHING
    /* Get populated mask infor */
    if (g_MaxprotocolVersion == 0 || g_SlotPopulatedMask == 0)
#endif
    {
        /* Read slot infor, slot populated mask | slot returned mask */
        ret = TMC_GetPopulatedMask(&slotPopulatedMask, 0x000F);
        if (ret != SUCCEED)
        {
            return ret;
        }
    }

    if (((1 << slotNumber) & g_SlotPopulatedMask) == 0)
    {
        return ERR_INVALID_REQUEST;
    }

    /* Get product unit certificate with offset = 0x600 and length = 255 */
    ProdUnitCertificate[0] = sizeof(ProdUnitCertificate) >> 8;
    ProdUnitCertificate[1] = sizeof(ProdUnitCertificate) & 0x00FF;
    Length = APDU_BODY_MAX_SIZE;
    /* read out product CA first 255 bytes */
    ret = TMC_ReadCertification(ProdUnitCertificate, 0x600, &Length, slotNumber);
    if (ret != SUCCEED)
    {
        return ret;
    }

    /* Get Manufacturer CA length */
    Length = TMC_TLVParse(ProdUnitCertificate, &pCertificate);

    /* Get Qi ID with mode = 1 */
    ret = TMC_GetCAPubKey(pCertificate, 1, QiID);

    return ret;
}

// Communication Layer  -----------

/*
 *@function transmit_apdu
 *@brief  APDU ????
 *@para[in] pAPDU
 *                ????APDU ??
 *@return SUCCEED:???FAILED:??
 */
int transmit_apdu(TRANSMIT_DATA *pAPDU)
{
    int i = 0;
    int ret = SUCCEED;
    unsigned char sw1, sw2;
    unsigned short len = 0; // Record the total length of received data

    if (pAPDU->tx_len > TMC_SEND_MAX)
    {
        return FAILED;
    }

    /* Send cmd and receive response data */
    for (i = 0; i <= RETRY_COUNT; i++)
    {
        if (i > 0)
        { /* Retry after 8ms */
            delay_1us(8000);
        }
        /* IIC package length */
        len = pAPDU->tx_len + TMC_I2C_MAX + 1;

        /* Start + Salve address + 0xAA + length (exclude CRC8) + APDU + CRC8 */
        pAPDU->tx[1] = 0xAA;
        pAPDU->tx[2] = (pAPDU->tx_len) >> 8;
        pAPDU->tx[3] = (pAPDU->tx_len) >> 0;

        pAPDU->tx[len - 1] = tmc_i2c_crc(0, pAPDU->tx + TMC_I2C_MAX, pAPDU->tx_len);

        ret = I2C_Write(pAPDU->tx + 1, len - 1);

        if (ret != SUCCEED)
        {
            continue;
        }
        /* check expected receive length,skip receiving if rx_len = 0 */
        if (pAPDU->rx_len != 0x00)
        {
            memset(pAPDU->rx, 0x00, TMC_RECE_MAX);

            /* Execution delay, try receiving SE response after that */
            delay_1us(pAPDU->execution_time);
            delay_1us(8000);
            /* Receive incoming data with package format: AA + 2 bytes length + (data + CRC), length = data length + CRC length */
            ret = I2C_Read(pAPDU->rx + 1, &len, pAPDU->max_wait_time);

            /* Verify package format, check execution status word befor return response data */
            if ((ret == SUCCEED) && (pAPDU->rx[1] == FRAME_TAG) && (len == ((pAPDU->rx[2] << 8) | (pAPDU->rx[3]))))
            {
                ret = FAILED;
                /* Returned length exclude CRC8 for incoming package */
                if (len >= 2)
                {
                    // crc = tmc_i2c_crc(inBuf + 4, inlen-1);
                    if (tmc_i2c_crc(0, pAPDU->rx + 4, len + 1) != 0)
                    {
                        continue;
                    }

                    memcpy(pAPDU->rx, pAPDU->rx + 4, len);

                    sw1 = pAPDU->rx[len - 2];
                    sw2 = pAPDU->rx[len - 1];
                    if ((sw1 == 0x00) && (sw2 == 0x00))
                    {
                        continue;
                    }
                    ret = SUCCEED;
                    break;
                }
            }
        }
        else
        {
            /* No response expected, return directly */
            return ret;
        }
    }

    if (ret != SUCCEED)
    {
        return ret;
    }

    /* In case returned SW != 9000, return error SW directly */
    if ((sw1 == 0x00) && (sw2 == 0x00))
    {
        return FAILED;
    }
    else if ((sw1 != 0x90) || (sw2 != 0x00))
    {
        return (int)((sw1 << 8) | sw2);
    }
    /* SW = 0x9000 */
    /* remove SW length */
    len -= 2;
    pAPDU->rx_len = len;

    return ret;
}

/*
 *@function tmc_read_data
 *@brief ???????????
 *@para[out] data
 *               ?????
 *@para[in] data_off
 *               ??????????
 *@para[in] data_len
 *               ???????
 *@para[in] slotID
 *                Certificate Chain slot ID
 *                0x80 0X, get polulated mask, X: slot mask, SE returns (max protocol version | Populated mask)+(CC Hash LSB in slot populated)
 *                0xN0 00, get digests or certificate, N: slot number, SE returns digests or certificate indicated by slot number
 *                0xF0 00, read data
 *@return SUCCEED:???FAILED:??
 */
int tmc_read_data(unsigned char *data, unsigned int data_off, unsigned int data_len, unsigned short slotID)
{
    int rv = SUCCEED;
    TRANSMIT_DATA apdu;
    int len = 0, off = data_off, readlen = 0;

    if (data == NULL)
    {
        rv = FAILED;
        goto end;
    }

    /* Read data */
    if (slotID == 0xF000)
    {
        /* Length checking */
        if ((data_off + data_len) > 0x1000)
        {
            rv = FAILED;
            goto end;
        }
    }
    /* Get populated mask */
    else if ((slotID & 0xFFF0) == 0x8000)
    {
        /* slot mask checking */
        if ((slotID & 0x000F) == 0)
        {
            rv = FAILED;
            goto end;
        }
        /* Length checking */
        if ((data_off != 0) || (data_len != 1))
        {
            rv = FAILED;
            goto end;
        }
    }
    /* Get digests or certificate */
    else if ((slotID & 0x0FFF) == 0)
    {
        /* slot number checking */
        if ((slotID >> 12) > 7)
        {
            rv = FAILED;
            goto end;
        }
        /* Length checking */
        if ((data_off + data_len) > LEN_OF_CERTIFICATION)
        {
            rv = FAILED;
            goto end;
        }
    }

    /* Config P1 & P2 (off) with slotID */
    off |= slotID;

    while (len < data_len)
    {
        readlen = (data_len - len) > APDU_BODY_MAX_SIZE ? APDU_BODY_MAX_SIZE : (data_len - len);
        /* Reserve TMC_I2C_MAX bytes for IIC package header */
        apdu.tx_len = TMC_I2C_MAX;
        apduBuf[apdu.tx_len++] = 0x00;

        apduBuf[apdu.tx_len++] = INS_READ;
        apduBuf[apdu.tx_len++] = (unsigned char)(off >> 8);
        apduBuf[apdu.tx_len++] = (unsigned char)(off & 0x00FF);
        apduBuf[apdu.tx_len++] = (unsigned char)readlen;
        /* Adapt tx length */
        apdu.tx_len -= TMC_I2C_MAX;

        /* Read response maximum waiting time, NOK returned if not finished receiving response */
        apdu.max_wait_time = MAX_WAIT_TIME_100MS;
        /* APDU execution time, 3ms delay before start read */
        apdu.execution_time = EXECUTION_TIME_3MS;

        apdu.tx = apduBuf;
        apdu.rx = RecvBuf;
        apdu.rx_len = sizeof(RecvBuf);
//        printk("\r\n apdu-> %d %d %d %d", readlen, data_len, len, apdu.rx_len);

        delay_1ms(200);
        rv = transmit_apdu(&apdu);

        while (rv != SUCCEED)
        {
//        	printk("\r\n T91-retry");
            delay_1ms(200);
            rv = transmit_apdu(&apdu);
        }

        if (rv != SUCCEED)
        {
//        	printk("\r\n yyy");
            rv = FAILED;
            goto end;
        }

        if (apdu.rx_len)
        {
            if (data)
            {
                memcpy(data + len, apdu.rx, apdu.rx_len);
            }
        }

        off += readlen;
        len += readlen;
    }

end:

    return rv;
}

/*
 *@function tmc_ecc_signature
 *@brief ????????ECC ???????????
 *@para[in] data
 *                ??????
 *@para[in] data_len
 *                ?????????
 *@para[out] sign_data
 *                ????
 *@para[out] sign_data_len
 *                ??????
 *@para[in] eccID
 *                Certificate Chain slot ID
 *@return SUCCEED:???FAILED:??
 */
int tmc_ecc_signature(unsigned char *data, unsigned int data_len,
                      unsigned char *sign_data, unsigned int *sign_data_len,
                      unsigned short eccID)
{
    int rv = SUCCEED;
    TRANSMIT_DATA apdu;
    int dataLen = data_len;

    if (data == NULL)
    {
        rv = FAILED;
        goto end;
    }

    if (data_len != LEN_OF_CHALLENGE)
    {
        rv = FAILED;
        goto end;
    }

    if (sign_data_len == NULL)
    {
        rv = FAILED;
        goto end;
    }

    if (sign_data == NULL)
    { // ????????
        *sign_data_len = LEN_OF_CHALLENGE_AUTH;
        goto end;
    }

    *sign_data_len = 0;

    /* 00 E4 0N 00 10 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx, N = slot number */
    /* Reserve TMC_I2C_MAX bytes for IIC package header */
    apdu.tx_len = TMC_I2C_MAX;
    apduBuf[apdu.tx_len++] = 0x00;
    apduBuf[apdu.tx_len++] = INS_ECC;
    apduBuf[apdu.tx_len++] = (unsigned char)(eccID & 0x000F);
    apduBuf[apdu.tx_len++] = 0x00;
    apduBuf[apdu.tx_len++] = dataLen;

    memcpy(apduBuf + apdu.tx_len, data, dataLen);
    /* Adapt tx length */
    apdu.tx_len -= TMC_I2C_MAX;

    /* Read response maximum waiting time, NOK returned if not finished receiving response */
    apdu.max_wait_time = MAX_WAIT_TIME_100MS;
    /* APDU execution time, 210ms delay before start read */
    apdu.execution_time = EXECUTION_TIME_210MS;
    apdu.tx = apduBuf;
    apdu.tx_len = APDU_HEADER_SIZE + dataLen;
    apdu.rx = RecvBuf;
    apdu.rx_len = sizeof(RecvBuf);
    rv = transmit_apdu(&apdu);
    if (rv != SUCCEED)
    {
        // rv = FAILED;
        goto end;
    }

    if (apdu.rx_len)
    {
        if (sign_data)
        {
            memcpy(sign_data, apdu.rx, apdu.rx_len);
        }

        if (sign_data_len)
        {
            *sign_data_len = apdu.rx_len;
        }
    }

end:

    return rv;
}

/*
 *@function tmc_i2c_crc
 *@brief  ????????????
 *@para[in] pbyBuf
 *               ?????
 *@para[in] wLen
 *               ???????
 *@para[out] ?
 *@return ????
 */
unsigned char tmc_i2c_crc(unsigned char byAccum, unsigned char *pbyBuf, unsigned short wLen)
{
    unsigned char *ptrEnd;
    unsigned char byData;
    ptrEnd = pbyBuf + wLen;
    for (; pbyBuf < ptrEnd; pbyBuf++)
    {
        byData = byAccum ^ *pbyBuf;
        byAccum = crc8Table1[byData & 0x0f] ^ crc8Table2[byData >> 4];
    }
    return byAccum;
}

/*
 *@function I2C_Write
 *@brief  ?T9 ?????
 *@para[in] TxBuf
 *               ??????
 *@para[in] len
 *               ????????
 *@para[out] ?
 *@return SUCCEED:???FAILED:??
 */
int I2C_Write(unsigned char *TxBuf, int len)
{
    // ?????I2C???????
    if (TxBuf == NULL || len <= 0)
    {
        return -1; // ???????????
    }

    // ????
    hal_i2cm_start();
    if (hal_i2cm_byte_send(T91206_I2C_ADDRESS | 0) < 0)
    {
        hal_i2cm_stop(); // ?????
        return -1;       // ????
    }

    for (int i = 0; i < len; i++)
    {
        if (hal_i2cm_byte_send(TxBuf[i]) < 0)
        {
            hal_i2cm_stop(); // ?????
            return -1;       // ????
        }
    }

    hal_i2cm_stop();
    return 0; // ????
}
/*
 *@function I2C_Read
 *@brief  ?T9 ?????
 *@para[out] RxBuf
 *               ??????
 *@para[in] len
 *               ????????
 *@return 0:???-1:??
 */
int I2C_Read(unsigned char *RxBuf, unsigned short *restrict len, unsigned long Timeout)
{
    // ?????I2C???????
    //	???3???AA+LEN1+LEN2
    //	??LEN1*256+LEN2??????????
    uint8_t byte;
    hal_i2cm_start();

    // ??????
    if (hal_i2cm_byte_send(T91206_I2C_ADDRESS | 1) < 0)
    {
        hal_i2cm_stop();
        return -1; // ????????
    }

    // ??????
    for (int i = 0; i < 3; i++)
    {
        if (hal_i2cm_byte_read(&byte, 0) < 0)
        {
            hal_i2cm_stop();
            return -1; // ????????
        }
        RxBuf[i] = byte;
    }

    // ??????
    uint8_t LEN1 = RxBuf[1];
    uint8_t LEN2 = RxBuf[2];
    *len = (LEN1 << 8) | LEN2;

    // ????????
    for (int i = 3; i < *len + 3 + 1; i++)
    {
        if (hal_i2cm_byte_read(&byte, 0) < 0)
        {
            hal_i2cm_stop();
            return -1; // ????????
        }
        RxBuf[i] = byte;
    }

    hal_i2cm_stop();
    return 0; // ????
}

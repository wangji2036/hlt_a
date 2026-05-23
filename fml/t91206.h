#ifndef T91206_H_
#define T91206_H_

#include "typdef.h"

#ifndef SUCCEED
#define SUCCEED (0)
#endif
#ifndef FAILED
#define FAILED (-1)
#endif

#define T91206_I2C_ADDRESS    0xA0
typedef struct _TRANSMIT_DATA
{
    unsigned char *tx;
    int            tx_len;
    unsigned char  *rx;
    int            rx_len;
    int            max_wait_time;
    int            execution_time;
} TRANSMIT_DATA;


#define FRAME_TAG               0xAA
#define APDU_HEADER_SIZE        5
#define APDU_BODY_MAX_SIZE      255
#define RETRY_COUNT             3

#define TMC_SEND_MAX            30
#define TMC_RECE_MAX            (APDU_BODY_MAX_SIZE+APDU_HEADER_SIZE+5)
#define TMC_I2C_MAX             4


extern unsigned char apduBuf[TMC_SEND_MAX];
extern unsigned char RecvBuf[TMC_RECE_MAX];

/* Length of Root certificate Hash */
#define LEN_OF_N_RH             32
/* Length of Manufacturer CA Certificate */
#define LEN_OF_N_MC             333
/* Length of Product Unit Certificate */
#define LEN_OF_N_PUC            442

#define LEN_OF_CERTIFICATION    (2+LEN_OF_N_RH+LEN_OF_N_MC+LEN_OF_N_PUC)     /* Length of certification */
#define LEN_OF_SIGN             64
#define LEN_OF_PUBKEY           64
#define LEN_OF_PRIKEY           32
#define LEN_OF_RANDOM           32  /* Length of random */
#define LEN_OF_DIGESTS          32
#define LEN_OF_CHALLENGE        16
#define LEN_OF_HASH_32          32  /* Length of HASH. */
#define LEN_OF_CHALLENGE_AUTH   64

#define LEN_COS_VERSION         3
#define OFFSET_COS_VERSION      0
#define LEN_SEID                12
#define OFFSET_SEID             4
#define ID_READ_DATA            0xF000

#define TAG_DER_SEQUENCE            0x30
#define TAG_OBJECT_ID               0x06
#define TAG_SIGNATURE               0x02
#define TAG_INTEGER                 0x02

#define X509_V3_VERSION             0X02

/* CLA, INS */
#define CLA_UPDATE                  0xFC
#define INS_READ                    0xB0
#define INS_ECC                     0xE4
#define INS_UPDATE                  0xFA
#define INS_POWERDOWN               0xAD
#define INS_CTRL_GPO                0xDA

/* WPC Auth Type */
#define DIGESTS_RESPONSE            0x11
#define CERTIFICATE_RESPONSE        0x12
#define CHALLENGE_AUTH_RESPONSE     0x13
#define ERROR_RESPONSE              0x17
//#define GET_DIGESTS                 0x19
//#define GET_CERTIFICATE             0x1A
#define CHALLENGE_AUTH              0x1B

/*  Error Code*/
#define ERR_INVALID_REQUEST         0x0100
#define ERR_UNSUPPORTED_PROTOCOL    0x0201
#define ERR_BUSY                    0x0300
#define ERR_UNSPECIFIED             0x0400
/* Manufacturer defined */
#define ERR_COMMUNICATION           0xF000
#define ERR_UNINITIALIZED_SE        0xF100
#define ERR_CERTIFICATE_FORMAT      0xF101
#define ERR_WRONG_VERSION_SE        0xF102
#define ERR_WRONG_PARAMETER         0xF200
#define ERR_WRONG_INDATA            0xF201
#define ERR_NO_CACHING              0xF400
#define ERR_VERIFICATION_FAILED     0xF500
#define ERR_DEVICE_NOT_INIT         0xF600
#define ERR_DEVICE_INIT_FAILED      0xF601
#define ERR_DEVICE_DEINIT_FAILED    0xF602
#define ERR_CHALLENGE_SIGNING       0XF700

/* Timer config */
#define EXECUTION_TIME_210MS        210000//10000000
#define EXECUTION_TIME_30MS         30000//1500000
#define EXECUTION_TIME_3MS          3000//150000
#define MAX_WAIT_TIME_100MS         2000

#define SWI_DELAY_POWERDOWN				330
#define SWI_DELAY_POWERUP_5MS			5000
#define SWI_DELAY_POWERUP_1MS			1000
#define SWI_DELAY_POWERUP_8MS			8000

void t91206_init(void);

int TMC_GetPopulatedMask(unsigned char* slotMaskRet, unsigned short slotMaskReq);
int TMC_GetProtocolVersion(unsigned char* maxProtocolVersion);

int TMC_GetCertificateChainLSB(unsigned char* CertChainHashLSB, unsigned short slotNumReq);

int TMC_ReadDigests(unsigned char* slotMaskRet, unsigned char* digests, unsigned int* outLen, unsigned short slotMaskReq);
int TMC_ReadCertification(unsigned char* certification, unsigned int offset, unsigned int *length, unsigned short slotNumber);
int TMC_SignChallenge(unsigned char* random, unsigned int length, unsigned short slotNumber, unsigned char* signature, unsigned int* signLen);

int TMC_GetQiID(unsigned char* QiID, unsigned short slotNumber);

int transmit_apdu(TRANSMIT_DATA *pAPDU);
int tmc_read_data(unsigned char *data, unsigned int data_off, unsigned int data_len, unsigned short slotID);
int tmc_ecc_signature(unsigned char *data, unsigned int data_len, unsigned char *sign_data, unsigned int *sign_data_len, unsigned short eccID);

int I2C_Write(unsigned char *TxBuf, int len);
int I2C_Read(unsigned char *RxBuf, unsigned short *restrict len, unsigned short rx_capacity, unsigned long Timeout);
unsigned char tmc_i2c_crc(unsigned char byAccum, unsigned char * pbyBuf, unsigned short wLen);


int t91206_get_qi_id(uint8_t *rbuf);
int t91206_read_cert_hash(uint8_t *rbuf);
int t91206_read_se_cert(uint8_t *rbuf, uint32_t *rlen);
int t91206_get_cert_chain(uint8_t *wpc_cert_hash, uint8_t *manufacturer_cert, uint16_t manufacturer_cert_len, uint8_t *rbuf, uint16_t *rlen);
int t91206_get_tbs_auth(uint8_t *rbuf , uint8_t *array_random);

#endif /* T91206_H_ */

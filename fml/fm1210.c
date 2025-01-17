#include "regdef.h"
#include "printk.h"
#include "config.h"
#include "g_data.h"
#include "fm1210.h"
#include "delay.h"
#include "osal.h"
#include "algo.h"


#define SEIC_DEV_ADDR    0x04 //8bit

#define I2C_NAD			0x00
#define I2C_CMD_IBLOCK	0x02
#define I2C_CMD_GET_ATR 0x30
#define I2C_CMD_NAK		0xBA

#define I2C_MIN_LEN 1
#define I2C_MAX_LEN 0xFF - 3

/*******************************************************/
//#define SE_CERT_MAX_SIZE 400
//uint8_t Certchain[1200] = { 0 };
uint8_t g_rbuf[400] = { 0 };

//static struct fm_head_t fm_hdr;
static struct fm_pack_t fm_pack;

#define FM1210_I2C_WKUP_DELAY    1000 //us
#define FM1210_I2C_READ_LEN_MIN     1
#define FM1210_I2C_READ_LEN_MAX  (252)

void fm1210_wakeup(void)
{
	//this for fm1210 wake up
	GPA->DOUT.BITS.PIN7 = 0;
	delay_1us(FM1210_I2C_WKUP_DELAY);
	GPA->DOUT.BITS.PIN7 = 1;
	delay_1us(FM1210_I2C_WKUP_DELAY);
    GPA->ODEN.BITS.PIN6 = 1;
    GPA->ODEN.BITS.PIN7 = 1;
}

uint8_t fm1210_device_select(uint8_t *rbuf)
{
	uint16_t slen = 0;
	uint16_t read_len;
	uint8_t  ret;

	fm_pack.cmd = 0x10;
	fm_pack.apdu_data[0] = SEIC_DEV_ADDR;
	slen = 2;

	ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf, &read_len);

	if (ret)
	{
		return ret;
	}

	return 0;
}

void fm1210_init(void)
{
	hal_i2cm_init(100000);
	fm1210_wakeup();
	//device selection must be completed within 30ms after fm1210 wake up
	fm1210_device_select(g_rbuf);
	printk("\r\n done");
}

int fm1210_i2c_send_frame(uint8_t cmd, uint8_t *sbuf, uint16_t slen)
{
	int ret;
	uint16_t i, crc;
	struct fm_head_t fm_hdr;

	fm_hdr.len = slen;
	fm_hdr.flag.cmd = cmd;

	crc = crc16_ccitt(sbuf, slen, 0xC6C6); //calc_crc

	do {
		ret = 0;

		hal_i2cm_start();

		 //send se_addr, write slaver
		if (hal_i2cm_byte_send(SEIC_DEV_ADDR) < 0) { ret = -1; break; }

		 //send len
		if (hal_i2cm_byte_send(fm_hdr.len) < 0) { ret = -2; break; }

		//send data
		for (i=0; i<slen; i++)
		{
			if (hal_i2cm_byte_send(*sbuf++) < 0)
			{
				ret = -3;
				break;
			}
		}
		if (ret < 0) { break; }

		 //send_crc
		if (hal_i2cm_byte_send((crc >> 8) & 0xff) < 0) { ret = -4; break; }
		if (hal_i2cm_byte_send((crc >> 0) & 0xff) < 0) { ret = -5; break; }
	} while(0);

	hal_i2cm_stop();

	return ret;
}

int fm1210_i2c_recv_frame(uint8_t *rbuf, uint16_t *rlen)
{
	int ret;
	uint8_t crc_msb, crc_lsb;
	uint16_t i, crc, recv_len;
	struct fm_head_t fm_hdr;

	*rlen = 0;
	recv_len = 0;

	do {
		ret = 0;

		hal_i2cm_start();

		//send se_addr,read slaver
		if ( hal_i2cm_byte_send(SEIC_DEV_ADDR + 1) < 0) { ret = -1; break; }

		//recv_length
		if (hal_i2cm_byte_read(&fm_hdr.len, 0) < 0) { ret = -2; break; }
		recv_len = fm_hdr.len;
		if (recv_len < FM1210_I2C_READ_LEN_MIN || recv_len > FM1210_I2C_READ_LEN_MAX) { printk(" \r\n length_err ..."); *rlen = 0; ret = -3; break; }

		//recv_sta & data
		for (i=0; i<recv_len; i++) { hal_i2cm_byte_read(&rbuf[i], 0); }

		//recv and send NAK
		hal_i2cm_byte_read(&crc_msb, 0);
		hal_i2cm_byte_read(&crc_lsb, 1);
	} while(0);

	hal_i2cm_stop();

	do {
		if (ret < 0) break;

		crc = crc_msb << 8 | crc_lsb;
		if (crc != crc16_ccitt(rbuf, recv_len, 0xC6C6)) { printk(" \r\n crc_err ..."); *rlen = 0; ret = -4; break; }

		fm_hdr.flag.sta = rbuf[0];
		*rlen = recv_len - 1; //recv data length
		for (i=0; i<*rlen; i++)
		{
			rbuf[i] = rbuf[i + 1];
		}
	} while(0);

	return ret;
}

int fm1210_i2c_transceive(uint8_t *sbuf, uint16_t slen, uint8_t *rbuf, uint16_t *rlen)
{
	uint8_t ret;

	*rlen = 0;
	if (fm1210_i2c_send_frame(I2C_CMD_IBLOCK, sbuf, slen) < 0) return -1;

	do {
		delay_1ms(2);
		ret = fm1210_i2c_recv_frame(rbuf, rlen);
		if (ret)
		{
//			printk("\r\n %d", ret);
		}
		else
		{
			break;
		}
	} while (1);//TODO: need timeout to avoid endless loop

	return ret;
}

int fm1210_get_qi_id(uint8_t *rbuf)
{
	uint16_t slen = 0;
	uint16_t read_len;
	uint8_t i, ret;

	fm_pack.cmd = 0x30;
	fm_pack.apdu_data[0] = 0x00;
	slen = 2;

	ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf, &read_len);

	if (ret)
	{
		return ret;
	}

//	printk(" \r\n rbuf[%d]-> ", read_len);
//	for (i=0; i<read_len; i++)
//	{
//		printk("%02X ", rbuf[i]);
//	}
//
	printk(" \r\n qi_id-> ");
	for (i=0; i<3; i++)
	{
		rbuf[i] = rbuf[i + 11];
		printk("%02X ", rbuf[i]);
	}

	return 0;
}

//uint8_t		certhash[32]		= { 0 };
extern uint8_t cert_chain[];

int fm1210_read_cert_hash(uint8_t *rbuf)
{
	uint16_t slen = 0;
	uint16_t read_len;
	uint8_t	 /*i,*/ ret;

	fm_pack.cmd = 0x30;
	fm_pack.apdu_data[0] = 0x30;
	slen = 2;

	ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf, &read_len);

    if (ret)
    {
    	printk("\r\n read cert_hash error 0x30");
        return (ret);
    }

    fm_pack.apdu_data[0] = 0x33;
    ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf + 16, &read_len);

    if (ret)
    {
    	printk("\r\n read cert_hash error 0x33");
        return (ret);
    }

//	osal_mem_copy(certhash, rbuf, 32);
//	printk(" \r\n certhash[%d]-> ", read_len);
//	for (i=0; i<32; i++)
//	{
//		printk("%02X ", certhash[i]);
//	}

    return (ret);
}

int fm1210_read_se_cert(uint8_t *rbuf, uint32_t *rlen)
{
    uint16_t slen = 0;
    uint16_t read_len;
    uint8_t  ret;
    uint8_t  i = 0;
    uint8_t	 blocknumber;
    uint8_t	 blockleft;

	fm_pack.cmd = 0x30;
	fm_pack.apdu_data[0] = 0x01;
	slen = 2;

    /*get first block to calculate the len of cert */
    ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf, &read_len);

    if (ret)
    {
    	printk("\r\n se_cert 1st err");
        return (ret);
    }
    else
    {
    	printk(" \r\n rbuf-0x3001[%d]-> ", i);
    	for (i=0; i<read_len; i++)
    	{
    		printk("%02X ", rbuf[i]);
    	}
    }
    fm_pack.apdu_data[0]++;

    *rlen = (rbuf[2] << 8 | rbuf[3]) + 4;

    /*max len 400 bytes */
    if (*rlen > 400)
    {
    	printk("\r\n se_cert len_err %d", *rlen);
    	return -10; //IF_ERR_LENGTH
    }

    blocknumber = (*rlen / 16);
    blockleft	= *rlen % 16;

    for (i = 1; i<blocknumber; i++)
    {
        ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf + i * 16, &read_len);

        if (ret)
        {
        	printk("\r\n se_cert %d block err", i);
            return (ret);
        }
        else
        {
        	printk(" \r\n rbuf-0x3001[%d]-> ", i);
        	for (uint16_t j=0; j<read_len; j++)
        	{
        		printk("%02X ", rbuf[i * 16 + j]);
        	}
        }

        if (i == 14)
        {
            fm_pack.apdu_data[0] = 0x28; /*part 2 */
        }
        else if (i == 22)
        {
            fm_pack.apdu_data[0] = 0x35; /*part 3 */
        }
        else
        {
        	fm_pack.apdu_data[0]++;
        }
    }


    if (blockleft)
    {
        ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf + blocknumber * 16, &read_len);
        if (ret)
        {
        	printk("\r\n se_cert last block err");
            return (ret);
        }
        else
        {
        	printk(" \r\n rbuf-0x3001[%d]-> ", i+1);
        	for (uint16_t j=0; j<read_len; j++)
        	{
        		printk("%02X ", rbuf[blocknumber * 16 + j]);
        	}
        }
    }

    cert_chain[0] = ((2 + 32 + 328 + *rlen) >> 8) & 0xff;
    cert_chain[1] = ((2 + 32 + 328 + *rlen) >> 0) & 0xff;

    return (ret);
}

int fm1210_get_cert_chain(uint8_t *wpc_cert_hash, uint8_t *manufacturer_cert, uint16_t manufacturer_cert_len, uint8_t *rbuf, uint32_t *rlen)
{
    unsigned char ret = 0;

    osal_mem_copy(rbuf + 0x02, wpc_cert_hash, 0x20);
    osal_mem_copy(rbuf + 0x22, manufacturer_cert, manufacturer_cert_len);

    ret = fm1210_read_se_cert(rbuf + 0x22 + manufacturer_cert_len, rlen );
    if (ret)
    {
    	printk("\r\n fm1210_get_cert_chain fail");
        return (ret);
    }

    *rlen  += (0x22 + manufacturer_cert_len);
    rbuf[0] = *rlen >> 8;
    rbuf[1] = *rlen & 0xFF;

    printk("\r\n fm1210_get_cert_chain-> start\r\n");
    for (int i=0; i<*rlen; i++)
    {
    	printk("%02X ", rbuf[i]);
    }
    printk("\r\n fm1210_get_cert_chain-> end\r\n");

    return (ret);
}

int data_compress_sha256(uint8_t *hashdata, uint8_t *rbuf)
{
    uint16_t slen = 0;
    uint16_t read_len;
    uint8_t  ret;

	fm_pack.cmd = 0x45;
	fm_pack.apdu_data[0] = 0x00;
	osal_mem_copy(fm_pack.apdu_data + 1, hashdata, 54);

	fm_pack.apdu_data[55] = 0x80;
	osal_mem_clear(fm_pack.apdu_data + 56, 63 - 55);

	fm_pack.apdu_data[63] = 0x01;
	fm_pack.apdu_data[64] = 0xB0;
    slen = 2 + 64;

    ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf, &read_len);

    if (ret)
    {
    	printk("\r\n data_compress_sha256 err");
        return (ret);
    }


//    printk("\r\n compress_sha256 start %d\r\n", read_len);
//    for (int i=0; i<read_len; i++)
//    {
//    	printk("%02X", rbuf[i]);
//    }
//    printk("\r\n compress_sha256 end \r\n");

    return (0);
}

int ecc_private_key_cal_p256r1sha256(uint8_t* hashresult, uint8_t* rbuf)
{
    uint16_t slen = 0;
    uint16_t read_len;
    uint8_t  ret;

	fm_pack.cmd = 0x41;
	fm_pack.apdu_data[0] = 0x80;
	osal_mem_copy(fm_pack.apdu_data + 1, hashresult, 32);
	slen = 2 + 32;

    ret = fm1210_i2c_transceive((uint8_t *)&fm_pack, slen, rbuf, &read_len);

    if (ret)
    {
    	printk("\r\n ecc_private_key_cal_p256r1sha256 err");
        return (ret);
    }

    return (0);
}

extern uint8_t adt_data_recv_buf[18];
uint8_t	tbs_auth[54]		= { 0 };
uint8_t	tbs_authhash[32]	= { 0 };

extern uint8_t array_digest[];

int fm1210_get_tbs_auth(uint8_t *rbuf)
{
	uint8_t ret;
	uint8_t pos; /*Prefix */

	pos = 0;
	tbs_auth[pos++] = 0x41;

	/*Certificate Chain hash */
//	osal_mem_copy(tbs_auth + pos, certhash, 32);
	osal_mem_copy(tbs_auth + pos, array_digest + 1, 32);
	pos += 32;

    /*Challenge Request */
    tbs_auth[pos++] = 0x1B;
    tbs_auth[pos++] = 0x00;
    osal_mem_copy(tbs_auth + pos, adt_data_recv_buf + 2, 16);
    pos += 16;

    /*CHALLENGE_AUTH response */
    tbs_auth[pos++] = 0x13;
    tbs_auth[pos++] = 0x11;
//    tbs_auth[pos++] = certhash[31];
    tbs_auth[pos++] = array_digest[32];

//    printk("\r\n orig_tba_auth start");
//    for (int i=0; i<54; i++)
//    {
//    	printk("%02X ", tbs_auth[i]);
//    }
//    printk("\r\n orig_tba_auth end");

    ret = data_compress_sha256(tbs_auth, tbs_authhash);
    if (ret)
    {
        printk("\r\n Get TBS Auth Digest Failed %04X", ret );
        return ret;
    }
//    else
//    {
//        printk("\r\n Get tbs_authhash Digest success \r\n");
//        for (int x=0; x<0x20; x++)
//        {
//        	printk(" %02X", tbs_authhash[x]);
//        }
//        printk("\r\n");
//    }

//    printk( "\r\n ////////Step 4.2 ECC Calculation - Get Signature of the digest //////////\r\n" );
    ret = ecc_private_key_cal_p256r1sha256(tbs_authhash, rbuf);
    if (ret)
    {
        printk("\r\n ECC Calculation Failed %04X\r\n", ret);
        return ret;
    }
//    else
//    {
//        printk( "\r\n ECC Calculation success\r\n" );
//        for (int x=0; x<0x40; x++)
//        {
//        	printk(" %02X", rbuf[x]);
//        }
//        printk("\r\n");
//    }

    return 0;
}


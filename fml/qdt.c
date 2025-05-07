#include "regdef.h"
#include "printk.h"
#include "nu103x.h"
#include "g_data.h"
#include "delay.h"
#include "ecap.h"
#include "qdt.h"

static const uint16_t _qdt_ln_tbl[100] =
{
	1789, 1786, 1782, 1779, 1776, 1772, 1769, 1766, 1763, 1759,
	1756, 1753, 1750, 1747, 1743, 1740, 1737, 1734, 1731, 1728,
	1725, 1721, 1718, 1715, 1712, 1709, 1706, 1703, 1700, 1697,
	1694, 1691, 1688, 1685, 1682, 1679, 1676, 1673, 1670, 1667,
	1664, 1661, 1658, 1655, 1652, 1650, 1647, 1644, 1641, 1638,
	1635, 1632, 1630, 1627, 1624, 1621, 1618, 1615, 1613, 1610,
	1607, 1604, 1602, 1599, 1596, 1593, 1591, 1588, 1585, 1582,
	1580, 1577, 1574, 1572, 1569, 1566, 1564, 1561, 1558, 1556,
	1553, 1551, 1548, 1545, 1543, 1540, 1538, 1535, 1532, 1530,
	1527, 1525, 1522, 1520, 1517, 1515, 1512, 1510, 1507, 1505,
};

enum _qdt_pin_chan
{
	_qdt_pin_ch0 = 0,
	_qdt_pin_ch1 = 1,
};

struct _qdt_pin_ctrl
{
	uint8_t I_EN : 1;
	uint8_t O_EN : 1;
	uint8_t DOUT : 1;
	uint8_t ODEN : 1;
	uint8_t PUEN : 1;
	uint8_t PDEN : 1;
	uint8_t MODE : 2;
};

static void qdt_pin_ctrl(enum _qdt_pin_chan chan, struct _qdt_pin_ctrl ctrl)
{
	VIC_vModuleDisable();
	if (chan == _qdt_pin_ch0)
	{
		/* PC0 */
		GPC->I_EN.BITS.PIN0 = ctrl.I_EN;
		GPC->O_EN.BITS.PIN0 = ctrl.O_EN;
		GPC->DOUT.BITS.PIN0 = ctrl.DOUT;
		GPC->ODEN.BITS.PIN0 = ctrl.ODEN;
		GPC->PUEN.BITS.PIN0 = ctrl.PUEN;
		GPC->PDEN.BITS.PIN0 = ctrl.PDEN;
		GPC->MODE.BITS.PIN0 = ctrl.MODE; //00:PC0 01:EPWM1 10:RESERVED 11:RESERVED
	}

	if (chan == _qdt_pin_ch1)
	{
		/* PC1 */
		GPC->I_EN.BITS.PIN1 = ctrl.I_EN;
		GPC->O_EN.BITS.PIN1 = ctrl.O_EN;
		GPC->DOUT.BITS.PIN1 = ctrl.DOUT;
		GPC->ODEN.BITS.PIN1 = ctrl.ODEN;
		GPC->PUEN.BITS.PIN1 = ctrl.PUEN;
		GPC->PDEN.BITS.PIN1 = ctrl.PDEN;
		GPC->MODE.BITS.PIN1 = ctrl.MODE; //00:PC1 01:EPWM2 10:RESERVED 11:RESERVED
	}
	VIC_vModuleEnable();
}

void fml_qdt_detect(uint32_t *q_fact, uint32_t *f_self)
{
	#define QDT_VPEAK_STD_THD2     300 //mV
	#define QDT_DECAY_DLY_TIME    7200 //us

	uint16_t timeout = 0;
	uint16_t vqm_decay_time_cnt = 0;
	uint16_t vqm_width_last_cnt = 0;
	uint16_t nqm_reson_freq_cnt = 0;
	uint32_t q_tmp, f_tmp;
	uint32_t vpeak_th2 = 0;

	struct _qdt_pin_ctrl ch0_ctrl = { 0, 1, 0, 0, 0, 0, 0 };
	struct _qdt_pin_ctrl ch1_ctrl = { 0, 1, 0, 0, 0, 0, 0 };

	//step-1. set PWM1/PWM2 output 0
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);
	qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);

	// --- set PWM1 output 0, PWM2 output 1, reverse charge SW2
	// ch1_ctrl.DOUT = 1;
	// qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);
	// delay_1us(250);
	// ch1_ctrl.DOUT = 0;
	// qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);

	// //step-2. enable nu103x Q-measurement
	// fml_nu103x_qdt_init();

	//step-3. set PWM1 input(High-Z)
	ch0_ctrl.I_EN = 1;
	ch0_ctrl.O_EN = 0;
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);

	//step-2. enable nu103x Q-measurement
	fml_nu103x_qdt_init();

	//step-4. wait charging time
	delay_1us(500);

	//step-5. configure ECAP as QDT measure mode
	hal_ecap_init(ECAP1, _ECAP_FUNC_MODE_QDT);
	hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_QDT);
	hal_ecap_open(ECAP1);
	hal_ecap_open(ECAP2);

	//step-6. set PWM1 output 0
	ch0_ctrl.I_EN = 0;
	ch0_ctrl.O_EN = 1;
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);

	//step-7. wait ECAP detect Q-factor and resonant frequency done
	while (timeout++ < 500)
	{
		delay_1us(4);
		if ((ECAP1->STS_FLAG.WORD & ECAP_STS_FLAG_QDT_DONE_FLAG_Msk) && (ECAP2->STS_FLAG.WORD & ECAP_STS_FLAG_QDT_DONE_FLAG_Msk))
		{
			break;
		}
	}

	if (ECAP1->STS_FLAG.WORD & ECAP_STS_FLAG_QDT_DONE_FLAG_Msk)
	{
		vqm_decay_time_cnt = ECAP1->QDT_MEAS._VQM.DECAY_TIME_CNT;
		vqm_width_last_cnt = ECAP1->QDT_MEAS._VQM.WIDTH_LAST_CNT;
	}

	if (ECAP2->STS_FLAG.WORD & ECAP_STS_FLAG_QDT_DONE_FLAG_Msk)
	{
		nqm_reson_freq_cnt = ECAP2->QDT_MEAS._NQM.RESON_FREQ_CNT;
	}

	//step-8. after measure, close ECAP and set back PWM1/PWM2 mode
	hal_ecap_close(ECAP1);
	hal_ecap_close(ECAP2);
	ch0_ctrl.MODE = 1;
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);
	ch1_ctrl.MODE = 1;
	qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);

	//step-8. calculate Q-factor and resonant frequency
	q_tmp = 1;
	f_tmp = 1;
	if (nqm_reson_freq_cnt > 0 && vqm_decay_time_cnt > QDT_DECAY_DLY_TIME)
	{
		uint32_t tmp;
		tmp = 10000 * vqm_width_last_cnt / nqm_reson_freq_cnt;
		tmp *= tmp;
		tmp = 1232 * tmp / 100000;
		tmp = (tmp < 10000) ? (10000 - tmp) : 10000;
		vpeak_th2 = (tmp != 0) ? QDT_VPEAK_STD_THD2 * 10000 / tmp : QDT_VPEAK_STD_THD2;
		tmp = (vpeak_th2 - QDT_VPEAK_STD_THD2 < sizeof(_qdt_ln_tbl) / sizeof(_qdt_ln_tbl[0])) ? vpeak_th2 - QDT_VPEAK_STD_THD2 : 0;

		q_tmp = 157000 * (vqm_decay_time_cnt - QDT_DECAY_DLY_TIME - vqm_width_last_cnt / 2 + nqm_reson_freq_cnt / 20) / (nqm_reson_freq_cnt * _qdt_ln_tbl[tmp]);
		f_tmp = 360000 * (ECAP2->QDT_CTRL._NQM.MEAS_TIMES_SET + 1) / nqm_reson_freq_cnt;
	}

	*q_fact = q_tmp;
	*f_self = f_tmp;

//	printk("\r\n QDT-> %d %d %d %d %d %d %d %d", vqm_decay_time_cnt, vqm_width_last_cnt, nqm_reson_freq_cnt, *f_self, *q_fact, vpeak_th2, *q_fact, *f_self);
}


static void fml_qdt_initialize(void)
{
	struct _qdt_pin_ctrl ch0_ctrl = { 0, 1, 0, 0, 0, 0, 0 };
	struct _qdt_pin_ctrl ch1_ctrl = { 0, 1, 0, 0, 0, 0, 0 };

	//step-1. set PWM1/PWM2 output 0
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);
	qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);

	//step-2. set PWM1 input(High-Z)
	ch0_ctrl.I_EN = 1;
	ch0_ctrl.O_EN = 0;
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);

	//step-3. enable nu103x Q-measurement
	fml_nu103x_qdt_init();

	//step-4. wait charging time
}

static void fml_qdt_precharged(void)
{
	struct _qdt_pin_ctrl ch0_ctrl = { 0, 1, 0, 0, 0, 0, 0 };
	struct _qdt_pin_ctrl ch1_ctrl = { 0, 1, 0, 0, 0, 0, 0 };

	//step-5. configure ECAP as QDT measure mode
	hal_ecap_init(ECAP1, _ECAP_FUNC_MODE_QDT);
	hal_ecap_init(ECAP2, _ECAP_FUNC_MODE_QDT);
	hal_ecap_open(ECAP1);
	hal_ecap_open(ECAP2);

	//step-6. set PWM1 output 0
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);
	qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);

	//step-7. wait ECAP detect Q-factor and resonant frequency done
}

static void fml_qdt_discharged(void)
{
	#define QDT_VPEAK_STD_THD2     300 //mV
	#define QDT_DECAY_DLY_TIME    7200 //us

	uint16_t vqm_decay_time_cnt = 0;
	uint16_t vqm_width_last_cnt = 0;
	uint16_t nqm_reson_freq_cnt = 0;
	uint32_t q_tmp, f_tmp;
	uint32_t vpeak_th2 = 0;

	struct _qdt_pin_ctrl ch0_ctrl = { 0, 1, 0, 0, 0, 0, 1 };
	struct _qdt_pin_ctrl ch1_ctrl = { 0, 1, 0, 0, 0, 0, 1 };

	if (ECAP1->STS_FLAG.WORD & ECAP_STS_FLAG_QDT_DONE_FLAG_Msk)
	{
		vqm_decay_time_cnt = ECAP1->QDT_MEAS._VQM.DECAY_TIME_CNT;
		vqm_width_last_cnt = ECAP1->QDT_MEAS._VQM.WIDTH_LAST_CNT;
	}

	if (ECAP2->STS_FLAG.WORD & ECAP_STS_FLAG_QDT_DONE_FLAG_Msk)
	{
		nqm_reson_freq_cnt = ECAP2->QDT_MEAS._NQM.RESON_FREQ_CNT;
	}

	//step-8. after measure, close ECAP and set back PWM1/PWM2 mode
	hal_ecap_close(ECAP1);
	hal_ecap_close(ECAP2);
	qdt_pin_ctrl(_qdt_pin_ch0, ch0_ctrl);
	qdt_pin_ctrl(_qdt_pin_ch1, ch1_ctrl);

	//step-9. calculate Q-factor and resonant frequency
	q_tmp = 1;
	f_tmp = 1;
	if (nqm_reson_freq_cnt > 0 && vqm_decay_time_cnt > QDT_DECAY_DLY_TIME)
	{
		uint32_t tmp;
		tmp = 10000 * vqm_width_last_cnt / nqm_reson_freq_cnt;
		tmp *= tmp;
		tmp = 1232 * tmp / 100000;
		tmp = (tmp < 10000) ? (10000 - tmp) : 10000;
		vpeak_th2 = (tmp != 0) ? QDT_VPEAK_STD_THD2 * 10000 / tmp : QDT_VPEAK_STD_THD2;
		tmp = (vpeak_th2 - QDT_VPEAK_STD_THD2 < sizeof(_qdt_ln_tbl) / sizeof(_qdt_ln_tbl[0])) ? vpeak_th2 - QDT_VPEAK_STD_THD2 : 0;

		q_tmp = 157000 * (vqm_decay_time_cnt - QDT_DECAY_DLY_TIME - vqm_width_last_cnt / 2 + nqm_reson_freq_cnt / 20) / (nqm_reson_freq_cnt * _qdt_ln_tbl[tmp]);
		f_tmp = 360000 * (ECAP2->QDT_CTRL._NQM.MEAS_TIMES_SET + 1) / nqm_reson_freq_cnt;
	}

	gd->tx_infos.q_fact = q_tmp;
	gd->tx_infos.f_self = f_tmp;

	printk("\r\n QDT-> %d %d %d %d %d %d", vqm_decay_time_cnt, vqm_width_last_cnt, nqm_reson_freq_cnt, vpeak_th2, q_tmp, f_tmp);
}

void fml_qdt_detect_1(enum qdt_state_t state)
{
	switch (state)
	{
		case QDT_STA_INITIALIZE:
			fml_qdt_initialize();
			break;
		case QDT_STA_PRECHARGED:
			fml_qdt_precharged();
			break;
		case QDT_STA_DISCHARGED:
			fml_qdt_discharged();
			break;
		default:
			break;
	}
}


/**
  ****************************(C) COPYRIGHT 2024 Nuvolta Technology****************************
  * @file       epp.c/h
  * @brief      在这个文件中可以看到EPP模式下的NEGO&ReNEGO阶段和PT阶段的处理函数。
  *          EPP模式下，NEGO阶段主要是处理设备之间的协商，PT阶段主要是处理设备之间的能量传输和数据传输
  *          数据传输通常是Authentication的处理
  * @note
  *
  @verbatim
  ==============================================================================
  *     在这个文件中可以看到EPP模式下的NEGO阶段和PT阶段的处理函数。
  * EPP模式下，NEGO阶段主要是处理设备之间的协商，PT阶段主要是处理设备之间的能量传输和数据传输
  * 数据传输通常是Authentication的处理
  *
  *
  ==============================================================================
  @endverbatim
  ****************************(C) COPYRIGHT 2024 Nuvolta Technology****************************
  */
#include "epp.h"
#include "g_data.h"
#include "_wpc.h"
#include "osal.h"
#include "algo.h"
#include "qfod.h"

static uint8_t cali_cep_flag;
// Negotiation Power Transfer Contract --- PTC
static struct power_contract contract;
// FOD
static struct epp_fod_t epp_fod;
// Authentication
static struct epp_auth_t epp_auth;

/// @brief Global memory data
extern uint8_t cert_chain[];
extern uint8_t array_chall[];
extern uint8_t array_digest[];

uint8_t rec_challenge_data[100];


/// @brief
/// @param hdr ASK包头
/// @note 
/// @return
static uint8_t is_epp_neg_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x01 || hdr == 0x05 || hdr == 0x06 || hdr == 0x09 || hdr == 0x15 ||
        hdr == 0x26 || hdr == 0x51 || hdr == 0x71 || hdr == 0x81 || 
        /*hdr == 0x03 ||*/ hdr == 0x16 || hdr == 0x17 || hdr == 0x25 || hdr == 0x27 ||
        hdr == 0x36 || hdr == 0x37 || hdr == 0x46 || hdr == 0x47 || hdr == 0x56 || hdr == 0x57 ||
        hdr == 0x66 || hdr == 0x67 || hdr == 0x76 || hdr == 0x77)
    {
        return 1;
    }
    return 0;
}

static uint8_t is_epp_neg_proprietary_pkt(uint8_t hdr)
{
    if(hdr == 0x18 || hdr == 0x19 || hdr == 0x28 || hdr == 0x29 || hdr == 0x38 || hdr == 0x48 ||
       hdr == 0x58 || hdr == 0x68 || hdr == 0x78 || hdr == 0x84 || hdr == 0xA4 || hdr == 0xC4 ||
       hdr == 0xE2)
    {
        return 1;
    }
    return 0;
}

static uint8_t is_epp_xfer_illegal_pkt(uint8_t hdr)
{
    if (hdr == 0x31 || hdr == 0x04 || hdr == 0x01 || hdr == 0x71 || hdr == 0x81 || hdr == 0x06 ||
        hdr == 0x51)
    {
        return 1;
    }
    return 0;
}

/// @brief EPP Determine whether it is an ADT data packet
/// @param value
/// @return
uint8_t is_ADT_valid_packet_type(int value)
{
    // Checks if the value is between 0x16 and 0x77 and matches the pattern of even or odd.
    if (value >= 0x16 && value <= 0x77)
    {
        uint8_t lower_nibble = value & 0x0F; // Get the lower 4 bits
        uint8_t upper_nibble = value >> 4;   // Get the upper 4 bits
        // Checks if the lower 4 bits are 6 or 7, and the upper 4 bits are between 1 and 7
        if ((lower_nibble == 0x6 || lower_nibble == 0x7) && (upper_nibble >= 1 && upper_nibble <= 7))
        {
            EPP_Debug("\r\n Authen Packet: 0x%x", value);
            return 1; // Valid packet types
        }
    }

    return 0; // Invalid packet type
}

void initializePTC(void)
{
    contract.nego_mask = 0;
    contract.ref_power = gd->rx_infos.max_power_temp;   
    contract.rcv_pwr_type = 0;
    contract.guaranteed_power = gd->rx_infos.max_power_temp;
    contract.wait_update = 0;
    contract.fsk_params.pola = gd->fsk_cfg.polar;
    contract.fsk_params.depth = gd->fsk_cfg.depth;
    contract.fsk_params.Ncycles = 0;  // 512 cycles

    contract.re_ping_delay = 0;

    epp_fod.epp_fod_mode = 0;

    contract.nego_fod_mask = 0;

    osal_mem_clear(&epp_auth, sizeof(epp_auth));
}

/**
 * @brief          GRQ packet process
 * @param[in]      *epp_ask
 * @retval         General Request
 */
void wpc_epp_GRQ_pkt_process(struct com_prx_ask_pkt_t *epp_ask)
{
    struct epp_ptx_fsk_pkt_t fsk_pkt = {};
    uint8_t data_empty[2] = {0x00, 0x00};

    if (epp_ask == NULL) // if epp_ask is NULL, return directly
    {
        return;
    }
    
    switch (epp_ask->msg.grq.request)
    {
    case WPC_PTx_PKT_TYP_ID_30:
        fsk_pkt.epp_fsk.ID_pkt.hdr_30 = 0x30;

        fsk_pkt.epp_fsk.ID_pkt.qi_version = EPP_PROTOCOL_QI_VERSION;

        fsk_pkt.epp_fsk.ID_pkt.ptmc_msb = EPP_MANUFACTURER_CODE_MSB;
        fsk_pkt.epp_fsk.ID_pkt.ptmc_lsb = EPP_MANUFACTURER_CODE_LSB;

        fml_fsk_data_send(EPWM1, T_RESPONSE, fsk_pkt.epp_fsk.data, wpc_msg_size_get(fsk_pkt.epp_fsk.ID_pkt.hdr_30) + 1);
        break;
    case WPC_PTx_PKT_TYP_CAP_31:
        fsk_pkt.epp_fsk.cap.hdr_31 = 0x31;

        if(gd->adp.adp_type ==EADP_TYPE_POWERBANK_09V)
        {
        fsk_pkt.epp_fsk.cap.neg_power = 20; //10W
        fsk_pkt.epp_fsk.cap.pot_power = 20;
        //printk("\r\n !!!EPP 10W \r\n");
        }
        else
        {
        fsk_pkt.epp_fsk.cap.neg_power = EPP_CAP_NEGOTIABLE_POWER; //15W
        fsk_pkt.epp_fsk.cap.pot_power = EPP_CAP_POTENTIAL_POWER;
        //printk("\r\n !!!EPP 15W \r\n");
        }

        fsk_pkt.epp_fsk.cap.nrs = EPP_CAP_NRS;
        fsk_pkt.epp_fsk.cap.wpid = EPP_CAP_WPID;
        fsk_pkt.epp_fsk.cap.buff_size = EPP_CAP_BUFFER_SIZE;
        fsk_pkt.epp_fsk.cap.ob = EPP_CAP_OB;
        fsk_pkt.epp_fsk.cap.ar = EPP_CAP_AR;
        fsk_pkt.epp_fsk.cap.dup = EPP_CAP_DUP;

        fml_fsk_data_send(EPWM1, T_RESPONSE, fsk_pkt.epp_fsk.data, wpc_msg_size_get(fsk_pkt.epp_fsk.cap.hdr_31) + 1);
        break;
    default:

        fml_fsk_data_send(EPWM1, T_RESPONSE, data_empty, wpc_msg_size_get(data_empty[0]) + 1);
        break;
    }
//    osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE + 100, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO); //480 ms
    //超时应该从发完FSK再开始计算
//    osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE + 100, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO); //480 ms
		gd->tx_infos.fsk_done_event |= 0x10;
}

/**
 * @brief          SRQ packet process
 * @param[in]      *epp_ask
 * @retval         Specific Request
 */
void wpc_epp_SRQ_pkt_process(struct com_prx_ask_pkt_t *epp_ask)
{
    uint8_t i, cnt = 0, data = 0; // Verify contract quantity

    if (epp_ask == NULL) // if epp_ask is NULL, return directly
    {
        return;
    }

    switch (epp_ask->msg.srq.request)
    {
    case EPP_SRQ_en_00:            // End negotiation
        data = contract.nego_mask; // check PTC mask is correct or not
        for (i = 0; i < 8; i++)
        {
            if (data & 1)
            {
                cnt++;
            }
            data >>= 1;
        }

        if (IS_BIT_SET(contract.nego_mask, EPP_SRQ_gp_01))
        {
        	printk("\r\n contract.guaranteed_power= %d gd->rx_infos.gant_power=%d", contract.guaranteed_power, gd->rx_infos.guaranteed_power);
        	if (contract.guaranteed_power == gd->rx_infos.guaranteed_power)
        	{
        		cnt--;
        	}
        }

        if (IS_BIT_SET(contract.nego_mask, EPP_SRQ_rp_04))
        {
        	printk("\r\n contract.ref_power= %d gd->rx_infos.max_power=%d", contract.ref_power, gd->rx_infos.max_power);
        	if (contract.ref_power == gd->rx_infos.max_power)
        	{
        		cnt--;
        	}
        }

        EPP_Debug("\r\n NGE Cnt:%x %d", contract.nego_mask, cnt);

        if (epp_ask->msg.srq.parameter != cnt)
        {
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
            EPP_Debug("\r\n NGE ERR:%x %d", contract.nego_mask, cnt);
        }
        else
        {
        	if (gd->rx_infos.qi_version > 0x12)
        	{
        		if (gd->nego_flag == 1) //nego
        		{
        			if (contract.nego_fod_mask != 0x03)// Qi > 1.3 must send FOD/qf and FOD/rf
        			{
                        printk("\r\n enter bpp xfer");
                        EPP_Debug("\r\n No Send FOD/rq or FOD/rf"); // for IEC 8.3.48
                        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);

                    	gd->rx_infos.power_profile_mode = BPP;
                        osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
                        osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
        			}
        			else
        			{
        				if (qfod_nego(gd->rx_infos.ref_q, gd->rx_infos.ref_f) > 0)
        				{
        	                gd->ptx_idle_phase_status = WPC_IDLE_STAT_XER_FOD;
        					wpc_stop_to_idle(ESYS_ERR_CODE_NEGO_PHASE_FODS_REF_QVALUE_ERR);
        					return;
        				}
        				else
        				{
                            if (IS_BIT_SET(contract.nego_mask, EPP_SRQ_fsk_03))
                            {
                                contract.wait_update = 1; // Update the content in PTC only after Nego Done
                            }
                            gd->rx_infos.max_power = contract.ref_power;
                            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        				}
        			}
        		}
        		else if (gd->nego_flag == 2) //renego
        		{
                    if (IS_BIT_SET(contract.nego_mask, EPP_SRQ_fsk_03))
                    {
                        contract.wait_update = 1; // Update the content in PTC only after Nego Done
                    }
                    gd->rx_infos.max_power = contract.ref_power;
                    EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        		}
        	}
        	else
        	{
                if (IS_BIT_SET(contract.nego_mask, EPP_SRQ_fsk_03))
                {
                    contract.wait_update = 1; // Update the content in PTC only after Nego Done
                }
                gd->rx_infos.max_power = contract.ref_power;
                EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        	}

            gd->nego_flag = 0;
        }

        osal_stop_timerEx(WPC_NEXT_TIMER);
        osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
        osal_start_timerEx(WPC_RPP_TIMER, EPP_RP1_TIMEOUT, 0, WPC_TASK, WPC_EVT_RPP_TO);
        gd->ptx_protocol_phase = WPC_PHASE_XFER;
        gd->ptx_end_nego_event |= EPP_END_NEGO_FLAG;


        //TODO: 优化，根据协商的功率决定切到多少duty。
        if (gd->rx_infos.power_profile_mode == EPP)
        {
        	if (gd->rx_infos.qi_version == 0x13)
        	{
        		if (gd->pid_duty < 350)
        		{
        			gd->pid_duty = 350;
        			hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
        		}
        	}
        	else
        	{
        		if (gd->pid_duty < 250)
        		{
        			gd->pid_duty = 250;
        			hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
        		}
        	}
        }
        else
        {
    		if (gd->pid_duty < 250)
    		{
    			gd->pid_duty = 250;
    			hal_epwm_pwm_start(EPWM1, gd->pid_perd, gd->pid_duty, gd->pid_phas);
    		}
        }
        return; // goto transfer phase
        break;
    case EPP_SRQ_gp_01: // Received Power reporting

        if ((epp_ask->msg.srq.parameter & 0x3F) > 0x1E)
        {
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
            BIT_CLEAR(&contract.nego_mask, EPP_SRQ_gp_01);
            break;
        }

        if (contract.guaranteed_power != epp_ask->msg.srq.parameter)
        {
            contract.guaranteed_power = epp_ask->msg.srq.parameter;
//            gd->rx_infos.gant_power = epp_ask->msg.srq.parameter;
            BIT_SET(&contract.nego_mask, EPP_SRQ_gp_01);
            EPP_Debug("\r\n MASK gp");
        }

        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
    case EPP_SRQ_rpr_02: // Received Power reporting
        EPP_Debug("\r\n rsp_type :%x",gd->rx_infos.rsp_type);

        if (epp_ask->msg.srq.parameter != 0x31)
        {
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
            BIT_CLEAR(&contract.nego_mask, EPP_SRQ_rpr_02);
            break;
        }
        contract.rcv_pwr_type = epp_ask->msg.srq.parameter;
        gd->rx_infos.rsp_type = epp_ask->msg.srq.parameter;

        if(IS_BIT_SET(contract.nego_mask, EPP_SRQ_rpr_02))
        {
        	if (gd->nego_flag == 2) //renego
        	{
        		EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        	}
        	else
        	{
                EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
                BIT_SET(&contract.nego_mask, EPP_SRQ_rpr_02);
        	}
        }
        else
        {
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
            BIT_SET(&contract.nego_mask, EPP_SRQ_rpr_02);
        }
        
        EPP_Debug("\r\n MASK rpr");
        break;
    case EPP_SRQ_fsk_03: // FSK configuration

        EPP_Debug("\r\n fsk_params.pola:%d", contract.fsk_params.pola);
        EPP_Debug("\r\n fsk_params.depth:%d", contract.fsk_params.depth);

        EPP_Debug("\r\n 3 fsk_params.depth:%d",  (epp_ask->msg.srq.parameter & 0x03));
        EPP_Debug("\r\n 1 fsk_params.pola:%d",  ((epp_ask->msg.srq.parameter >> 2) & 0x01));
        EPP_Debug("\r\n 5 fsk_params.Ncycles:%d", ((epp_ask->msg.srq.parameter >> 3) & 0x03));

        // b0-b1（Depth）
        // b2（Pol）
        // b3-b4（NCycles）
        if((epp_ask->msg.srq.parameter & 0x03) != contract.fsk_params.depth ||   \
           ((epp_ask->msg.srq.parameter >> 2) & 0x01) != contract.fsk_params.pola || \
           ((epp_ask->msg.srq.parameter >> 3) & 0x03) != contract.fsk_params.Ncycles
        )
        {
            BIT_SET(&contract.nego_mask, EPP_SRQ_fsk_03);
            EPP_Debug("\r\n MASK fsk");
        }
        else
        {
            EPP_Debug("\r\n Clear MASK fsk");
            BIT_CLEAR(&contract.nego_mask, EPP_SRQ_fsk_03);
        }
        gd->fsk_cfg.depth = (epp_ask->msg.srq.parameter & 0x03);
        gd->fsk_cfg.polar = ((epp_ask->msg.srq.parameter >> 2) & 0x01);
        gd->fsk_cfg.cycle = (epp_ask->msg.srq.parameter >> 3) & 0x03;
        gd->fsk_cfg.prmbl = FSK_PRMBL_NONE;

        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
    case EPP_SRQ_rp_04: // Reference Power

        if (contract.ref_power != epp_ask->msg.srq.parameter)
        {
            contract.ref_power = epp_ask->msg.srq.parameter;
//            gd->rx_infos.max_power = epp_ask->msg.srq.parameter;
            BIT_SET(&contract.nego_mask, EPP_SRQ_rp_04);
            EPP_Debug("\r\n MASK ref power");
        }
        else
        {
            EPP_Debug("\r\n Clear MASK ref power");
            BIT_CLEAR(&contract.nego_mask, EPP_SRQ_rp_04);
        }
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
    case EPP_SRQ_rep_05: // Re-ping delay
        gd->tx_infos.t_re_ping = (epp_ask->msg.srq.parameter & 0x3F) * 200;
        EPP_Debug("\r\n re_ping_delay:%d", gd->tx_infos.t_re_ping);
        gd->tx_infos.reping_cnt = (epp_ask->msg.srq.parameter & 0x3F) * 200 / 100;
        if(gd->tx_infos.t_re_ping == 0 || gd->tx_infos.t_re_ping >= 12600) //re_ping_delay 0~12600ms
        {
        	gd->tx_infos.reping_cnt = 2;
            BIT_CLEAR(&contract.nego_mask, EPP_SRQ_rep_05);
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
        }
        else
        {
            BIT_SET(&contract.nego_mask, EPP_SRQ_rep_05);
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        }
        break;
    case EPP_SRQ_rcs_06: // Recalibration support
        BIT_SET(&contract.nego_mask, EPP_SRQ_rcs_06);
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
        break;
    default:
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        break;
    }
    // osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
}

void wpc_epp_FOD_pkt_process(struct com_prx_ask_pkt_t *epp_ask)
{
    if (epp_ask == NULL) // if epp_ask is NULL, return directly
    {
        return;
    }
    EPP_Debug("\r\n FOD Pkt");
    if (epp_ask->msg.fod.reserved > 0)
    {
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
    }
    switch (epp_ask->msg.fod.type)
    {
    case FOD_TYPE_qf:
        gd->rx_infos.ref_q = epp_ask->msg.fod.data;
        BIT_SET(&contract.nego_fod_mask, FOD_TYPE_qf);

        if (contract.nego_fod_mask == 0x03)
        {
            if (qfod_nego(gd->rx_infos.ref_q, gd->rx_infos.ref_f) > 0)
            {
            	EPP_FSK_Transmit(EPWM1, T_RESPONSE , _FSK_NAK);
            	gd->tx_infos.fsk_done_event |= 0x20;
            }
            else
            {
            	EPP_FSK_Transmit(EPWM1, T_RESPONSE , _FSK_ACK);
            }
        }
        else
        {
        	EPP_FSK_Transmit(EPWM1, T_RESPONSE , _FSK_ACK);
        }
        break;
    case FOD_TYPE_rf:
        gd->rx_infos.ref_f = epp_ask->msg.fod.data;
        BIT_SET(&contract.nego_fod_mask, FOD_TYPE_rf);

        if (contract.nego_fod_mask == 0x03)
        {
            if (qfod_nego(gd->rx_infos.ref_q, gd->rx_infos.ref_f) > 0)
            {
            	EPP_FSK_Transmit(EPWM1, T_RESPONSE , _FSK_NAK);
            	gd->tx_infos.fsk_done_event |= 0x20;
            }
            else
            {
            	EPP_FSK_Transmit(EPWM1, T_RESPONSE , _FSK_ACK);
            }
        }
        else
        {
        	EPP_FSK_Transmit(EPWM1, T_RESPONSE , _FSK_ACK);
        }
        break;
    default:
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        break;
    }
    
    osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
}

void wpc_epp_RPP_24bit_pkt_process(struct com_prx_ask_pkt_t *epp_ask);
void wpc_epp_xfer_phase_error(void);
void wpc_epp_ADT_pkt_process(struct com_prx_ask_pkt_t *com_ask);
void wpc_epp_DSR_pkt_handler(struct com_prx_ask_pkt_t *com_ask);
void wpc_epp_ADC_pkt_process(struct com_prx_ask_pkt_t *com_ask);
void epp_send_adc_start(uint8_t request, uint8_t param_msb, uint8_t param_lsb);
void wpc_epp_DSR_ack_handler(void);
void epp_send_unknown(void);
void epp_send_adc_end(void);
void epp_send_auth_error(uint8_t error_code, uint8_t error_data);
void epp_setup_adc_packet(uint8_t hdr, uint8_t request, uint8_t param_msb, uint8_t param_lsb);

/// @brief EPP Negotiation phase process
/// @param com_ask
/// @return void
/// @note eg. 07 30  ->  22 01	-> 22 00  -> 20 02	 -> 07 31   ->  20 04  -> 20 01  -> 20 00
//            GRQ/id ->  FOD/rf -> FOD/qf -> SRQ/rpr -> GRQ/cap ->  SRQ/rp -> SRQ/gp -> SRQ/en
//	          ID		 ACK	   ACK		  ACK		  CAP		ACK		  ACK	    ACK
void wpc_epp_nego_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
    if (com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }

    switch (com_ask->hdr)
    {
    case EPP_PRx_PKT_TYP_GRQ: // 0x07
        wpc_epp_GRQ_pkt_process(com_ask);
        break;
//    case EPP_PRx_PKT_TYP_NEGO: // 0x09
//        EPP_Debug("\r\n EPP Negotiation phase process NEGO packet");
//        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
    case EPP_PRx_PKT_TYP_SRQ: // 0x20
        wpc_epp_SRQ_pkt_process(com_ask);
        break;
    case EPP_PRx_PKT_TYP_FOD: // 0x22
        wpc_epp_FOD_pkt_process(com_ask);
        break;
    case EPP_PRx_PKT_TYPE_WPID_msb:
    case EPP_PRx_PKT_TYPE_WPID_lsb:
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        break;
    case EPP_PRx_PKT_TYP_CE:
    	if (gd->nego_flag == 1)
    	{
        	gd->rx_infos.power_profile_mode = BPP;
        	printk("\r\n enter bpp xfer");
    	}
        gd->ptx_protocol_phase = WPC_PHASE_XFER;
        osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
        osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
        osal_stop_timerEx(WPC_NEXT_TIMER);
        break;
    case EPP_PRx_PKT_TYPE_RPP_8bit: // 在nego阶段时如果收到CEP/RP8bit包，则认为Rx NEGO Fail
    	if (gd->nego_flag == 1)
    	{
        	gd->rx_infos.power_profile_mode = BPP;
        	printk("\r\n enter bpp xfer");
            gd->ptx_protocol_phase = WPC_PHASE_XFER;
            osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
            osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
            osal_stop_timerEx(WPC_NEXT_TIMER);
    	}
    	else
    	{
            gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
            wpc_stop_to_idle(ESYS_ERR_CODE_NEG_PHASE_NO_THIS_PKT);
            goto __NEGO_PHASE_ERR__;
    	}
        break;
    case EPP_PRx_PKT_TYPE_RPP_24bit:
    	if (gd->nego_flag == 1)
    	{
            gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
            wpc_stop_to_idle(ESYS_ERR_CODE_NEG_PHASE_NO_THIS_PKT);
            goto __NEGO_PHASE_ERR__;
    	}
    	else //renego and no end
    	{
            wpc_epp_RPP_24bit_pkt_process(com_ask);
            gd->ptx_protocol_phase = WPC_PHASE_XFER;
            osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
            osal_start_timerEx(WPC_RPP_TIMER, EPP_RP1_TIMEOUT, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO); //// Enter RP1 first when entering EPP mode
    	}
        break;
    default:
        if (is_epp_neg_illegal_pkt(com_ask->hdr))
        {
            EPP_Debug("EPP Negotiation phase process illegal packet");
//            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
            gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
            wpc_stop_to_idle(ESYS_ERR_CODE_NEG_PHASE_NO_THIS_PKT);
            goto __NEGO_PHASE_ERR__;
        }
        else if(is_epp_neg_proprietary_pkt(com_ask->hdr))
        {
            EPP_Debug("\r\nEPP Negotiation phase process proprietary packet");
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        }
        else/* if(is_epp_neg_reserved_pkt(com_ask->hdr))*/
        {
            EPP_Debug("\r\nEPP Negotiation phase process reserved packet");
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        }
        break;
    }

    if (gd->ptx_protocol_phase == WPC_PHASE_NEGO)
    {
    	osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
    }
    else
    {
    	osal_stop_timerEx(WPC_NEXT_TIMER);
    }

__NEGO_PHASE_ERR__:
    return;
}


///////////////////////////////////////// EPP Transfer phase START /////////////////////////////////////////////

/// @brief EPP mode CE packet process
/// @param com_ask
/// @return void
/// @note
void wpc_epp_CE_pkt_process(struct com_prx_ask_pkt_t *com_ask)
{
    if (com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }
    osal_start_timerEx(WPC_NEXT_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
}

void wpc_epp_cali_fail_dectct(void)
{
    if (epp_fod.epp_fod_cali_fail_time++ > 2) // Error protection mechanism
    {
        EPP_Debug("\r\n Cali Fail :%d", epp_fod.epp_fod_mode);
        wpc_epp_xfer_phase_error();
    }
}


/// @brief EPP Calibration phase process
/// @param com_ask
/// @return void
/// @note
void wpc_epp_calib_phase_process(struct com_prx_ask_pkt_t *com_ask)
{
    if (com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }
    if (com_ask->msg.rpp.mode == 0x01)
    {
        gd->rx_infos.cali_light = (com_ask->msg.rpp.rp_value >> 8) | (com_ask->msg.rpp.rp_value << 8);
#if EPP_FUNC_ONLINE_CALIB_ENABLE
        if (gd->rx_infos.cali_light > 6553)
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
        else
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
#else
        // It means that it is not the first RP1. IOC: 8.4.xx
        if (epp_fod.epp_fod_mode != 0x00U && epp_fod.epp_fod_mode != 0x02U) 
        {
            wpc_epp_cali_fail_dectct();
            return;
        }
        else
        {
            BIT_SET(&epp_fod.epp_fod_mode,EPP_CALIB_TYPE_01);
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        }
#endif
        osal_start_timerEx(WPC_RPP_TIMER, EPP_RP1_TIMEOUT, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
    }
    else if (com_ask->msg.rpp.mode == 0x02)
    {

        gd->rx_infos.cali_connect = (com_ask->msg.rpp.rp_value >> 8) | (com_ask->msg.rpp.rp_value << 8);

#if EPP_FUNC_ONLINE_CALIB_ENABLE
        if (gd->rx_infos.cali_light > 6553)
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
        else
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
#else
        // This means that RP1 was not sent or another package was sent and then RP2 was sent. IOC: 8.4. 26
        if (epp_fod.epp_fod_mode != 0x02U && epp_fod.epp_fod_mode != 0x06U) 
        {
            wpc_epp_cali_fail_dectct();
            return;
        }
        else
        {
            BIT_SET(&epp_fod.epp_fod_mode,EPP_CALIB_TYPE_02);
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        }
#endif

        osal_start_timerEx(WPC_RPP_TIMER, 21000, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
    }
    else
    {
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
    }
}

/// @brief EPP mode Reference Power packet process and Calibastion phase
/// @param epp_ask
void wpc_epp_RPP_24bit_pkt_process(struct com_prx_ask_pkt_t *epp_ask)
{
    if (epp_ask == NULL) // if epp_ask is NULL, return directly
    {
        return;
    }
    gd->rx_infos.rpp = (epp_ask->msg.rpp.rp_value >> 8) | (epp_ask->msg.rpp.rp_value << 8);
    gd->rx_infos.stand_power = 10 * ((100 * gd->rx_infos.rpp * gd->rx_infos.max_power) >> 16);
    gd->rx_power = gd->rx_infos.stand_power;

//    EPP_Debug("\r\n RPP:0x%X", gd->rx_infos.rpp);
//    EPP_Debug(" Stand Power:%d", gd->rx_infos.stand_power);

    switch (epp_ask->msg.rpp.mode)
    {
    case EPP_RPP_MODE_RP: // 0x00
        if (epp_auth.EPP_DataStream_Tx_mode == TX_DataStream_ATN)
        {
            EPP_Debug("\r\n ATN");
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ATN);
        }
        else
        {
            // It means that it is no send RP1 and RP2. IOC: 8.4.xx
            if(epp_fod.epp_fod_mode != 0x06U && epp_fod.epp_fod_mode != 0x07U) 
            {
                // EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
                wpc_epp_cali_fail_dectct();
                return;
            }
            else
            {
                // gd->rx_power = mpp_ask->msg.report_pla.rcvd_power_msb << 8 | mpp_ask->msg.report_pla.rcvd_power_lsb;
                EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
            }
        }
        BIT_SET(&epp_fod.epp_fod_mode, EPP_RPP_MODE_RP);
//        EPP_Debug("\r\n RPP EPP_auth_Tx_status %d",epp_auth.EPP_DataStream_Tx_mode);
        osal_start_timerEx(WPC_NEXT_TIMER, T_RESPONSE + 2, 0, WPC_TASK, WPC_EVT_PFOD);  //FOD
        osal_start_timerEx(WPC_RPP_TIMER, T_COM_RP_TO, 0, WPC_TASK, WPC_EVT_RPP_TO);
        break;
    case EPP_CALIB_TYPE_01:
    	cali_cep_flag = 1;
    case EPP_CALIB_TYPE_02:
        wpc_epp_calib_phase_process(epp_ask);
        break;
    case EPP_CALIB_TYPE_04:
        gd->rx_infos.rpp_rsp_type = epp_ask->msg.rpp.mode;

        gd->rx_infos.rpp_tick++;

        if (epp_ask->msg.rpp.mode == 0x04)
        {
            osal_set_event(WPC_TASK, WPC_EVT_PFOD); // WPC_EVT_RPP_REPORT
        }
        osal_start_timerEx(WPC_RPP_TIMER, EPP_RP0_TIMEOUT, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
        break;
    default:
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        break;
    }

    return;
}



/// @brief EPP Transfer phase process
/// @param com_ask
/// @return void
/// @note
void wpc_epp_xfer_phase_protocol_process(struct com_prx_ask_pkt_t *com_ask)
{
    if (com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }

    //Only after initialization is complete
    if(contract.wait_update == 1)
    {
        contract.wait_update = 0;
//        gd->rx_infos.max_power = contract.ref_power;
        fml_fsk_param_set(EPWM1, gd->fsk_cfg.polar, gd->fsk_cfg.depth, gd->fsk_cfg.cycle, gd->fsk_cfg.prmbl);
    }
    
    if (is_ADT_valid_packet_type(com_ask->hdr)) // EPP ADT Packet 0x16-0x77
    {
        //epp_ADT_pkt_process(com_ask);
        wpc_epp_ADT_pkt_process(com_ask);
        return;
    }

    switch (com_ask->hdr)
    {
    case EPP_PRx_PKT_TYP_CE:
		gd->rx_infos.cep_val = com_ask->msg.cep.ce_value;

		if (gd->rx_power > gd->rx_infos.max_power * 5000 / 8) //1.2 * max_power
		{
			gd->rx_infos.mpp_restricted_power_limit = 1;
		}
		else if (gd->rx_power < gd->rx_infos.max_power * 4000 / 8)
		{
			gd->rx_infos.mpp_restricted_power_limit = 0;
		}

		if (gd->rx_infos.mpp_restricted_power_limit && gd->rx_infos.cep_val > 0)
		{
			gd->rx_infos.cep_val = 0;
			printk("#");
		}

		if (cali_cep_flag) //for IOC test
		{
			cali_cep_flag = 0;
			gd->rx_infos.cep_val = 0;
		}

		if (gd->rx_infos.cep_val == -60 && gd->rx_infos.pch_t_delay == 0x32)
		{
			printk("\r\n LDSTP_EPP");
			gd->atl_test_ldstp_epp_N60 = 1;
		}

		osal_start_timerEx(WPC_CEP_TIMER, T_COM_CE_TO, 0, WPC_TASK, WPC_EVT_CEP_TO);
		osal_start_timerEx(WPC_NEXT_TIMER, gd->rx_infos.pch_t_delay, 0, WPC_TASK, WPC_EVT_PCH_TO);
        break;
    case EPP_PRx_PKT_TYPE_RPP_24bit: // 0x31
        wpc_epp_RPP_24bit_pkt_process((struct com_prx_ask_pkt_t *)com_ask);
        break;
    case EPP_PRx_PKT_TYPE_RPP_8bit: // 0x04
//        hal_epwm_pwm_stop(EPWM1); //need to < 28ms for IOC test
//        wpc_epp_xfer_phase_error();
//        gd->sys_err_code = ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT;
    	gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
        wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT);
        EPP_Debug("\r\n error: bpp_rp8");
        break;
    case EPP_PRx_PKT_TYP_CHS:
        EPP_Debug("\r\n CHS:%d", com_ask->msg.chs.chs_value);
        break;
    //EPP datastream
    case EPP_PRx_PKT_TYP_DSR_15:
        wpc_epp_DSR_pkt_handler(com_ask);
        break;
    case EPP_PRx_PKT_TYP_ADC_25:
        wpc_epp_ADC_pkt_process(com_ask);
        break;

    case EPP_PRx_PKT_TYP_NEGO:
        // renego phase
        // clear PTC
        // contract.nego_mask = 0;
        // Send ACK
    	EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        osal_start_timerEx(WPC_NEXT_TIMER, T_NEGOTIATE + 2000, 0, WPC_TASK, WPC_EVT_NEGO_NEXT_PKT_TO);
        gd->ptx_protocol_phase = WPC_PHASE_NEGO; // goto nego phase
        gd->nego_flag = 2;
        EPP_Debug("goto nego phase");
        break;
//    case EPP_PRx_PKT_TYP_PCH:
//        // off time == 0
//        if (gd->rx_infos.pch_t_delay == 0)
//            break;
//    case EPP_PRx_PKT_TYP_SRQ:
//        gd->ptx_protocol_phase = WPC_PHASE_NEGO;   // 这么写不好，但是呢，先这样吧
//        wpc_epp_nego_phase_process(com_ask);
//        break;
    case 0x28:
		break;
    case EPP_PRx_PKT_TYP_SIG:
    case EPP_PRx_PKT_TYP_ID:
    case EPP_PRx_PKT_TYP_CFG:
    case EPP_PRx_PKT_TYP_PCH:
    case EPP_PRx_PKT_TYP_SRQ:
        wpc_epp_xfer_phase_error();
    default:
        is_epp_xfer_illegal_pkt(com_ask->hdr);
        break;
    }

    return;
}

void wpc_epp_xfer_phase_error(void)
{
    gd->sys_err_code = ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT;
    wpc_stop_to_idle(ESYS_ERR_CODE_XFER_PHASE_NO_THIS_PKT);
//    gd->ptx_protocol_phase = WPC_PHASE_IDLE;
}

///////////////////////////////////////// EPP Transfer phase END /////////////////////////////////////////////



///////////////////////////////////////// EPP Authentication START /////////////////////////////////////////////
/* Rx Data stream request process
1. RX ADC open trans
2. RX ADT
   ...
3. RX ADC close

Tx Data stream request process
1. TX Send FSK ATN 
2. RX DSR/poll
3. TX ADC open
4. TX ADT, RX DSR/ACK
   ...
5. TX ADC close
*/

void wpc_epp_ADC_pkt_process(struct com_prx_ask_pkt_t *com_ask)
{
    if(com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }
    EPP_Debug("\r\n ---> ADC Request:%d", com_ask->msg.adc.request);

    uint16_t authen_byte = (com_ask->msg.adc.param_msb << 8) | com_ask->msg.adc.params_lsb;


    switch (com_ask->msg.adc.request)
    {
    case ADC_end:

        if(epp_auth.EPP_DataStream_Rx_mode == RX_DataStream_DATA)
        {
            if(epp_auth.EPP_auth_status == EPP_Auth_GET_CHALLENGE)
            {
                EPP_Debug("\r\nChallenge Data: ");
                for (int j = 0; j < epp_auth.rec_challenge_data_len; j++)
                {
                    EPP_Debug("0x%X", rec_challenge_data[j]);
                }
                EPP_Debug("\r\n Data -> FM1230: ");
                extern uint8_t adt_data_recv_buf[18];
                for (int j = 0; j < 18; j++)
                {
                	adt_data_recv_buf[j] = rec_challenge_data[j]; // 除去Header的部分用来给晶片来签证书
                    EPP_Debug("0x%X", adt_data_recv_buf[j]);
                }

                osal_start_timerEx(WPC_AUTH_TIMER, 10, 0, WPC_TASK, WPC_EVT_SE_IC_TBS_AUTH);
                //calculate challenge
                epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_ATN;
            }
        }

        epp_auth.EPP_DataStream_Rx_mode = RX_DataStream_IDLE;
        
        if(epp_auth.EPP_auth_status == EPP_Auth_GET_DIGEST || epp_auth.EPP_auth_status == EPP_Auth_GET_CERTIFICATE)
        {
            EPP_Debug("\r\n ---> ADC_end");
            epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_ATN;
        }


        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
    case ADC_auth:
        // EPP_Authentication
        // 组合数据包params_lsb和param_msb
        epp_auth.EPP_Datastream_RX_len = authen_byte;
        epp_auth.EPP_DataStream_Rx_mode = RX_DataStream_OPEN_HDR;

        EPP_Debug("\r\n ---> ADC authen_byte:%d", authen_byte);
        EPP_Debug("\r\n ---> EPP_DataStream_Rx_mode:%d", epp_auth.EPP_DataStream_Rx_mode);

        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
    case ADC_rst:
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
        break;
//    case ADC_prop0:
//        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_NAK);
//        break;
    default:
    	EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        break;
    }

}

void wpc_epp_ADT_pkt_process(struct com_prx_ask_pkt_t *com_ask)
{
    if(com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }

    uint8_t ADT_data_len = wpc_msg_size_get(com_ask->hdr);       //当前ADT包的长度
    EPP_Debug("\r\n ---> ADT data len: %d", ADT_data_len);
    EPP_Debug("\r\n ---> ADT Request: %X", com_ask->msg.adt.data[0]);

    if(epp_auth.EPP_DataStream_Rx_mode == RX_DataStream_IDLE)
    {
        EPP_Debug("DSR error");
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
        return;
    }


    if(epp_auth.EPP_DataStream_Rx_mode == RX_DataStream_OPEN_HDR)
    {
        epp_auth.EPP_ADC_status = com_ask->msg.adt.data[0] & 0x0F;
        uint8_t Protocol_Version = (com_ask->msg.adt.data[0] & 0xF0) >> 4;

        
        EPP_Debug("\r\n ---> ADC_status: 0x%X ", epp_auth.EPP_ADC_status);
        EPP_Debug("Protocol_Version: 0x%X",Protocol_Version);

        if(Protocol_Version != 0x01)
        {
            EPP_Debug("\r\n ---> Authentication Protocol Version");
            epp_auth.EPP_auth_status = EPP_Auth_ERROR_VERSION;
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
            return;
        }

        switch (epp_auth.EPP_ADC_status) // Authentication Message Header
        {
        case msg_get_digest:

            EPP_Debug("\r\n ---> get_digest"); // 0x19 = digest
            EPP_Debug("\r\n ---> slot:%d", com_ask->msg.adt.data[1] & 0x0F);

            epp_auth.send_digest_slot = com_ask->msg.adt.data[1] & 0x0F;
            printk("\r\n epp_auth.send_digest_slot: %d", epp_auth.send_digest_slot);
            epp_auth.EPP_auth_status = EPP_Auth_GET_DIGEST;

            epp_auth.fsk_adt_is_odd = 1;        //initial the odd bit
            epp_auth.send_auth_data_header = 1;  //auth data header/ADT

            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
            break;
        case msg_get_certtificate:

            EPP_Debug("\r\n ---> get_certtificate  %X", com_ask->msg.adt.data[1]);

            // if(epp_auth.EPP_ADC_len == adc_len)
            // {
            //     EPP_Debug("\r\n ADC Len is notcorrect");
            //     EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_N_D);
            //     break;
            // }
            
            epp_auth.semd_cert_slot = com_ask->msg.adt.data[1] & 0x03;              // (b0-b1)
            epp_auth.EPP_ADC_len = (((com_ask->msg.adt.data[1] >> 2) & 0x07) << 8) | com_ask->msg.adt.data[3];         //(b2-b4) and B3
            epp_auth.EPP_ADC_offset = (((com_ask->msg.adt.data[1] >> 5) & 0x07) << 8) | com_ask->msg.adt.data[2];      //(b5-b7)  and B4

            // EPP_Debug("\r\n ---> 1 slot:%d", epp_auth.semd_cert_slot);
            // EPP_Debug("\r\n ---> 1 ADC_len:%d", epp_auth.EPP_ADC_len);
            // EPP_Debug("\r\n ---> 1 ADC_offset:%X", epp_auth.EPP_ADC_offset);

            if (epp_auth.EPP_ADC_offset >= 0x600)
            {
                epp_auth.EPP_ADC_offset = 2 + 32 + 4 + (cert_chain[36] << 8) + cert_chain[37] + epp_auth.EPP_ADC_offset - 0x600; // cali the read cert offset
                EPP_Debug("\r\n ---> >= 0x600:%X", epp_auth.EPP_ADC_offset);
            }

            // if (epp_auth.EPP_ADC_len == 0)
            // {
            //     epp_auth.EPP_ADC_len = ((cert_chain[0] << 8 | cert_chain[1]) + 1) - epp_auth.EPP_ADC_offset; // cali the read cert length
            // }

            // if ((epp_auth.EPP_ADC_offset + epp_auth.EPP_ADC_len) > CERT_CHAIN_LEN)
            // {
            //     epp_auth.EPP_ADC_len = CERT_CHAIN_LEN - epp_auth.EPP_ADC_offset;
            //     EPP_Debug("\r\n offset + len > CERT_CHAIN_LEN:%X", epp_auth.EPP_ADC_len);
            // }

            //If it is all zeros, then the entire chain of certificates in the cache will be sent, all at once
            epp_auth.send_cert_total_len = (epp_auth.EPP_ADC_len == 0) ? (cert_chain[0] << 8 | cert_chain[1]) : epp_auth.EPP_ADC_len;
            epp_auth.send_cert_offset = (epp_auth.EPP_ADC_offset == 0) ? 0 : epp_auth.EPP_ADC_offset;


            epp_auth.send_auth_data_header = 1;

            epp_auth.fsk_adt_is_odd = 1;        //initial the odd bit

            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);

            EPP_Debug("\r\n ---> cert_chain[1]:%d, cert_chain[0]:%d", cert_chain[1], cert_chain[0]);
            EPP_Debug("\r\n ---> ADC_offset:%d", epp_auth.EPP_ADC_offset);
            EPP_Debug("\r\n ---> ADC_len:%d", epp_auth.EPP_ADC_len);
            EPP_Debug("\r\n ---> send_cert_offset:%d", epp_auth.send_cert_offset);
            EPP_Debug("\r\n ---> send_ADC_len_length:%d", epp_auth.EPP_ADC_len);

            epp_auth.EPP_auth_status = EPP_Auth_GET_CERTIFICATE;
        
            break;
        case msg_get_challenge:
            EPP_Debug("\r\n ---> get_challenge");
            epp_auth.EPP_auth_status = EPP_Auth_GET_CHALLENGE;
            epp_auth.EPP_DataStream_Rx_mode = RX_DataStream_DATA;

            epp_auth.fsk_adt_is_odd = 1;        //initial the odd bit
            epp_auth.rec_challenge_data_offset = 0;
            epp_auth.rec_challenge_data_len = epp_auth.EPP_Datastream_RX_len;

            for(int i = 0; i < ADT_data_len; i ++)
            {
                rec_challenge_data[i] = com_ask->msg.adt.data[i];
                epp_auth.rec_challenge_data_offset ++;
            }
            EPP_Debug("\r\n len:%d", epp_auth.rec_challenge_data_len);
            
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
            break;
        case msg_get_ic_data:
            EPP_Debug("\r\n ---> get_response");
            break;

        default:
            EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);
            break;
        }
    }
    else if(epp_auth.EPP_DataStream_Rx_mode == RX_DataStream_DATA)
    {
        // Rec Rx ADT Data
        EPP_Debug("rec data");
        EPP_Debug("\r\n len:%d", epp_auth.rec_challenge_data_len);
        EPP_FSK_Transmit(EPWM1, T_RESPONSE, _FSK_ACK);

        for (int i = 0; i < ADT_data_len; i++)
        {
            rec_challenge_data[epp_auth.rec_challenge_data_offset ++] = com_ask->msg.adt.data[i];
            
            if(epp_auth.rec_challenge_data_offset >= epp_auth.rec_challenge_data_len)
            {
                EPP_Debug("\r\n---> get_challenge done");
            }
        }
    }

    return;
}


uint8_t ADT_send_buffer[10];

void wpc_epp_DSR_pkt_handler(struct com_prx_ask_pkt_t *com_ask)
{
    if(com_ask == NULL) // if com_ask is NULL, return directly
    {
        return;
    }
    enum
    {
        DSR_nak = 0x00,
        DSR_poll = 0x33,
        DSR_nd = 0x55,
        DSR_ack = 0xFF,
    };

    EPP_Debug("\r\n --->DSR pkt: %X", com_ask->msg.dsr.type);

    switch (com_ask->msg.dsr.type)
    {
    case DSR_nak:
        // busy RX NAK hold
        // pending resend last data packet
        // epp_DSR_poll_handler();
        // data arry --;
        // ADT_send_buffer;
        fml_fsk_data_send(EPWM1, T_RESPONSE, &ADT_send_buffer[0], wpc_msg_size_get(ADT_send_buffer[0]) + 1);
        break;
    case DSR_poll:
        if (epp_auth.EPP_auth_status == EPP_Auth_GET_DIGEST)
        {
            EPP_Debug("\r\n --->GET_DIGEST DSR_poll");
            epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_OPEN;

            if (epp_auth.send_digest_slot & 0x01)
            {
                epp_send_adc_start(0x02, 0x00, 0x22);
            }
            else
            {
                epp_send_adc_start(0x02, 0x00, 0x02); // slot can't support
            }
        } 
        else if (epp_auth.EPP_auth_status == EPP_Auth_GET_CERTIFICATE)
        {
            if(epp_auth.EPP_DataStream_Tx_mode == TX_DataStream_OPEN) //S24U retry
            {
                fml_fsk_data_send(EPWM1, T_RESPONSE, &ADT_send_buffer[0], wpc_msg_size_get(ADT_send_buffer[0]) + 1);
                return;
            }
            EPP_Debug("\r\n --->GET_CERTIFICATE DSR_poll");
            epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_OPEN;

            EPP_Debug("\r\n ---> send_cert_offset:%d", epp_auth.send_cert_offset);
            EPP_Debug("\r\n ---> send_cert_total_len:%d", epp_auth.send_cert_total_len);

            uint16_t send_cert_len;
            if (epp_auth.send_cert_offset > epp_auth.send_cert_total_len)
            {
                EPP_Debug("\r\n ---> Error");
                send_cert_len = 3; // Send 2 bytes error code
                epp_auth.EPP_auth_status = EPP_Auth_ERROR;
            }
            else if(epp_auth.send_cert_offset > 0)
            {
                send_cert_len = epp_auth.send_cert_total_len - epp_auth.send_cert_offset;
                send_cert_len += 1;
            }
            else
            {
                send_cert_len = epp_auth.send_cert_total_len;
                send_cert_len += 1;
            }
            EPP_Debug("\r\n ---> send_cert_len:%d", send_cert_len);

            uint8_t cert_len_msb = (send_cert_len) >> 8;
            uint8_t cert_len_lsb = (send_cert_len & 0x00FF);
            
            epp_send_adc_start(0x02, cert_len_msb, cert_len_lsb);  //authen message header means data length
        }
        else if (epp_auth.EPP_auth_status == EPP_Auth_GET_CHALLENGE)
        {
            EPP_Debug("\r\n --->GET_CHALLENGE DSR_poll");
            epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_OPEN;

            epp_auth.send_auth_data_header = 1;
            epp_send_adc_start(0x02, 0x00, 0x43);
        }
        else if (epp_auth.EPP_auth_status == EPP_Auth_ERROR || epp_auth.EPP_auth_status == EPP_Auth_ERROR_VERSION)
        {
            epp_send_adc_start(0x02, 0x00, 0x03); // slot can't support
        }
        else
        {
            EPP_Debug("\r\n ---> DSR_poll error");
            epp_send_unknown();
        }

        break;
    case DSR_nd:
        // end authentication
        epp_auth.EPP_auth_status = EPP_Auth_IDLE;
        epp_auth.EPP_DataStream_Rx_mode = RX_DataStream_IDLE;
        epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
        epp_send_unknown();

        break;
    case DSR_ack:
        // send data
        // epp_DSR_ack_handler();
        wpc_epp_DSR_ack_handler();
        break;
    default:
        break;
    }
}

/// @brief EPP mode Authentication FSK_ADT Response process
/// @param  
void wpc_epp_DSR_ack_handler(void)
{
    struct epp_ptx_fsk_pkt_t fsk_pkt = {};

    EPP_Debug("\r\n ---> EPP DSR status: %d",epp_auth.EPP_auth_status);
    // 定义FSK ADT报头常量
    uint8_t FSK_ADT_HDR = EPP_AUTH_ADT_HDR;

    epp_auth.fsk_adt_is_odd ^= 1;
    FSK_ADT_HDR += epp_auth.fsk_adt_is_odd;

    fsk_pkt.epp_fsk.ADT_pkt.hdr = FSK_ADT_HDR;

    const uint16_t send_digest_size = DIGEST_LENGTH + 2;
    static uint16_t send_digest_index = 0;

    epp_auth.send_challenge_data_len = 64;

    switch (epp_auth.EPP_auth_status) {
        case EPP_Auth_GET_DIGEST:

            EPP_Debug("\r\n DIGEST:");
            EPP_Debug(" Persent: [%d %%] \r\n", (send_digest_index * 100) / send_digest_size);

            if(epp_auth.send_digest_slot & 0x01)
            {
                // Load 7 bits of data into ADT_pkt. data
                for (uint8_t i = 0; i < 7; i++)
                {
                    if (epp_auth.send_auth_data_header == 1)
                    {
                        epp_auth.send_auth_data_header = 0;
                        fsk_pkt.epp_fsk.ADT_pkt.data[0] = 0X11;//RSP_DIGESTS; // message header
                    }
                    else
                    {
                        fsk_pkt.epp_fsk.ADT_pkt.data[i] = array_digest[send_digest_index];
                        send_digest_index++;
                    }

                    if (send_digest_index >= send_digest_size) // over size
                    {
                        send_digest_index = 0;
                        epp_auth.EPP_auth_status = EPP_Auth_IDLE; // The entire Certificate has been sent.
                        epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;

                        //Subtract the total number of heads from the total number of heads 1 to get the remaining quantity, 
                        //and publish the remaining portion using small heads.
                        fsk_pkt.epp_fsk.ADT_pkt.hdr -= (7 - i) * 0x10U; 

                        fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.epp_fsk.data[0], wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);
                        return;
                    }
                }
            }
            else
            {
                fsk_pkt.epp_fsk.ADT_pkt.hdr = 0x26;             
                fsk_pkt.epp_fsk.ADT_pkt.data[0] = 0x11;//RSP_DIGESTS;
                fsk_pkt.epp_fsk.ADT_pkt.data[1] = 0x11;       //only slot1 have chain
                epp_auth.EPP_auth_status = EPP_Auth_IDLE; // The entire Certificate has been sent.
                epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
                EPP_Debug("DIGEST slot fail");
            }
            fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.epp_fsk.data[0], wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);

            break;
        case EPP_Auth_GET_CERTIFICATE:
            //Load 7 bits of data into ADT_pkt. data
            //I hope to complete this authentication process as soon as possible
            EPP_Debug("\r\n CERTIFICATE:");
            EPP_Debug(" Persent: [%d %%] index[%d] \r\n", (epp_auth.send_cert_offset * 100) / epp_auth.send_cert_total_len, epp_auth.send_cert_offset);
            for(uint8_t i = 0; i < 7; i++)
            {
                if(epp_auth.send_auth_data_header == 1)
                {
                    epp_auth.send_auth_data_header = 0;
                    fsk_pkt.epp_fsk.ADT_pkt.data[0] = 0X12;//RSP_CERTIFICATE; //message header
                }
                else
                {
                    fsk_pkt.epp_fsk.ADT_pkt.data[i] = cert_chain[epp_auth.send_cert_offset];
                    epp_auth.send_cert_offset++;
                }

                if(epp_auth.send_cert_offset > epp_auth.send_cert_total_len) //It's over the total size. we need to send other head.
                {
                    epp_auth.send_cert_offset = 0; //Initialize for next transmission
                    
                    epp_auth.EPP_auth_status  = EPP_Auth_IDLE; //The entire Certificate has been sent.
                    epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
                    
                    //Subtract the total number of heads from the total number of heads 1 to get the remaining quantity,
                    //and publish the remaining portion using small heads.
                    fsk_pkt.epp_fsk.ADT_pkt.hdr -= (7 - i) * 0x10;

                    printk("\r\n send end1: %d %d %d %02x", epp_auth.send_cert_offset, epp_auth.send_cert_total_len, i, fsk_pkt.epp_fsk.ADT_pkt.hdr);

                    // fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.epp_fsk.data[0], wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);
                    break;
                }
//                else if (epp_auth.send_cert_offset == epp_auth.send_cert_total_len)
//                {
//                    epp_auth.send_cert_offset = 0; //Initialize for next transmission
//
//                    epp_auth.EPP_auth_status  = EPP_Auth_IDLE; //The entire Certificate has been sent.
//                    epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
//
//                    fsk_pkt.epp_fsk.ADT_pkt.hdr -= (7 - i) * 0x10;
//
//                    printk("\r\n send end2: %d %d %d %02x", epp_auth.send_cert_offset, epp_auth.send_cert_total_len, i, fsk_pkt.epp_fsk.ADT_pkt.hdr);
//
//                    break;
//                }
            }

            for(uint8_t i = 0; i < 8; i++)
            {
                ADT_send_buffer[i] = fsk_pkt.epp_fsk.data[i];
            }

            fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.epp_fsk.data[0], wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);

			if (epp_auth.send_cert_offset == epp_auth.send_cert_total_len)
			{
				epp_auth.send_cert_offset = 0; //Initialize for next transmission

				epp_auth.EPP_auth_status  = EPP_Auth_IDLE; //The entire Certificate has been sent.
				epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;

//				fsk_pkt.epp_fsk.ADT_pkt.hdr -= (7 - i) * 0x10;

				printk("\r\n send end2");

				break;
			}
            break;
        case EPP_Auth_GET_CHALLENGE:
            EPP_Debug("\r\n CHALLENGE:");
            EPP_Debug(" Persent: [%d %%] \r\n", (epp_auth.send_challenge_data_index * 100) / epp_auth.send_challenge_data_len);
            for(uint8_t i = 0; i < 7; i++)
            {
                if(epp_auth.send_auth_data_header == 1)
                {
                    epp_auth.send_auth_data_header = 0;
                    fsk_pkt.epp_fsk.ADT_pkt.data[0] = 0X13; //RSP_CHALLENGE_AUTH; //message header
                    fsk_pkt.epp_fsk.ADT_pkt.data[1] = 0x11;            
                    fsk_pkt.epp_fsk.ADT_pkt.data[2] = array_digest[32];    

                    i += 2;   
                    EPP_Debug("\r\n send head");
                }
                else
                {
                    fsk_pkt.epp_fsk.ADT_pkt.data[i] = array_chall[epp_auth.send_challenge_data_index];
                    epp_auth.send_challenge_data_index ++;
                }

                if(epp_auth.send_challenge_data_index > epp_auth.send_challenge_data_len) //It's over the total size. we need to send other head.
                {
                    epp_auth.send_challenge_data_index = 0; //Initialize for next transmission
                    
                    epp_auth.EPP_auth_status  = EPP_Auth_IDLE; //The entire Certificate has been sent.
                    epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
                    

                    fsk_pkt.epp_fsk.ADT_pkt.hdr -= (7 - i) * 0x10U;  //The remaining quantity is the head minus the total number   1, and the remaining part is published using the small head.

                    fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.epp_fsk.data[0], wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);
                    return;
                }

            }
            fml_fsk_data_send(EPWM1, T_RESPONSE, &fsk_pkt.epp_fsk.data[0], wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);

            break;
        case EPP_Auth_IDLE:
            if(epp_auth.EPP_DataStream_Tx_mode == TX_DataStream_CLOSE)
            {
                epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_IDLE;
                epp_send_adc_end();
            }
            else
            {
                epp_send_unknown(); // Of course, you can also not post it, but I don't know if it will be wrong
            }
            break;
        case EPP_Auth_ERROR:

            epp_send_auth_error(0x01,0x00); // can't support
            epp_auth.EPP_auth_status = EPP_Auth_IDLE; // The entire Certificate has been sent.
            epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
            break;
        case EPP_Auth_ERROR_VERSION:
            epp_send_auth_error(0x02,0x01); // version error
            epp_auth.EPP_auth_status = EPP_Auth_IDLE; // The entire Certificate has been sent.
            epp_auth.EPP_DataStream_Tx_mode = TX_DataStream_CLOSE;
            break;
        default:
            //NAK
            epp_send_unknown();
            break;
        break;
    }
}

void epp_send_adc_start(uint8_t request, uint8_t param_msb, uint8_t param_lsb)
{
    epp_setup_adc_packet(0x25U, request, param_msb, param_lsb);
}

void epp_send_adc_end(void)
{
    epp_setup_adc_packet(0x25U, 0x00U, 0x00U, 0x00U);
}

void epp_send_unknown(void)
{
    epp_setup_adc_packet(0x00U, 0x00U, 0x00U, 0x00U);
}

void epp_Response_NULL(void)
{
    ;
}

void epp_setup_adc_packet(uint8_t hdr, uint8_t request, uint8_t param_msb, uint8_t param_lsb)
{
    struct epp_ptx_fsk_pkt_t fsk_pkt = {};
    fsk_pkt.epp_fsk.ADC_pkt.hdr_25 = hdr;
    fsk_pkt.epp_fsk.ADC_pkt.request = request;
    fsk_pkt.epp_fsk.ADC_pkt.param_msb = param_msb;
    fsk_pkt.epp_fsk.ADC_pkt.params_lsb = param_lsb;
    fml_fsk_data_send(EPWM1, T_RESPONSE, fsk_pkt.epp_fsk.data, wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);
}

#define ERROR_RESPONSE 0x17
/* 
 * @function    ErrorResponse
 * @brief       Prepare Qi auth error response.
 * @param[in]   ErrCode         slot number of targeted digests.
 * @param[out]  ErrMsg          Returned error response.
 * @return      void
 */
void epp_send_auth_error(uint8_t error_code, uint8_t error_data)
{
    struct epp_ptx_fsk_pkt_t fsk_pkt = {};
    fsk_pkt.epp_fsk.ADT_pkt.hdr = 0x36;
    fsk_pkt.epp_fsk.ADT_pkt.data[0] = ERROR_RESPONSE;
    fsk_pkt.epp_fsk.ADT_pkt.data[1] = error_code;
    fsk_pkt.epp_fsk.ADT_pkt.data[2] = error_data;
    fml_fsk_data_send(EPWM1, T_RESPONSE, fsk_pkt.epp_fsk.data, wpc_msg_size_get(fsk_pkt.epp_fsk.data[0]) + 1);
}


///////////////////////////////////////// EPP Authentication /////////////////////////////////////////////

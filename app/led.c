#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "led.h"
#include"_wpc.h"
#include "tcpm.h"
#include"BMS_FixPoint.h"
#include "port_manager.h"
extern volatile uint16_t sys_ticks;
extern uint16_t key_ui_cnt;
#define LED_DISPLAY

void key_sigle_click_process(void);
void key_double_click_process(void);
void key_long_click_process(void);
volatile uint8_t key_flag = 0;

// following variable will be update to be GB data.
//uint8_t soc_show = 0;// SOC value, for display
static uint8_t flash_flag;//1:charging flashing,2, Error - all flashing
static uint8_t flash_light_on;//flash control, 1 means on state when flash, 0 means off state.
static uint8_t ui_scan_index;// for scan index
#ifdef LED_DISPLAY
static uint8_t soc_show_ram_led = 0;//Temporary variable,represents the display of LED lights, indicating whether each LED needs to be illuminated.
static uint8_t flash_flag_wls;//wireless LED flag, indicating flashing or not

#else
static uint32_t soc_show_ram = 0;//temporary variable, where each bit is used to represent each segment of the 188 digital display.
#endif

#define WAIT_IN_250MS 20

static uint8_t ui_no_timer_scan = 0;

static void drv_IO_control(uint8_t pinx, bool status)
{
	switch (pinx)
	{
	case 6:
		_UI_PIN6_PORT->I_EN.BITS._UI_PIN6_PINx = 0;
		_UI_PIN6_PORT->DOUT.BITS._UI_PIN6_PINx = status;
		_UI_PIN6_PORT-> O_EN.BITS._UI_PIN6_PINx = 1;
		break;
	case 5:
		_UI_PIN5_PORT->I_EN.BITS._UI_PIN5_PINx = 0;
		_UI_PIN5_PORT->DOUT.BITS._UI_PIN5_PINx = status;
		_UI_PIN5_PORT-> O_EN.BITS._UI_PIN5_PINx = 1;
		break;
	case 4:
		_UI_PIN4_PORT->I_EN.BITS._UI_PIN4_PINx = 0;
		_UI_PIN4_PORT->DOUT.BITS._UI_PIN4_PINx = status;
		_UI_PIN4_PORT-> O_EN.BITS._UI_PIN4_PINx = 1;
		break;
	case 3:
		_UI_PIN3_PORT->I_EN.BITS._UI_PIN3_PINx = 0;
		_UI_PIN3_PORT->DOUT.BITS._UI_PIN3_PINx = status;
		_UI_PIN3_PORT-> O_EN.BITS._UI_PIN3_PINx = 1;
		break;
	case 2:
		_UI_PIN2_PORT->I_EN.BITS._UI_PIN2_PINx = 0;
		_UI_PIN2_PORT->DOUT.BITS._UI_PIN2_PINx = status;
		_UI_PIN2_PORT-> O_EN.BITS._UI_PIN2_PINx = 1;
		break;
	case 1:
		_UI_PIN1_PORT->I_EN.BITS._UI_PIN1_PINx = 0;
		_UI_PIN1_PORT->DOUT.BITS._UI_PIN1_PINx = status;
		_UI_PIN1_PORT-> O_EN.BITS._UI_PIN1_PINx = 1;
		break;
    default:
		break;
	}
}

void led_init(void)
{
	_SET_ALL_PINS_IN_PUT();
}
#ifdef LED_DISPLAY

// LED 1~4for battery level , LED 5 for wireless charger.
// LED H for bright,

#define LED_FLOW_0          (0x00)
#define LED_FLOW_1          (0x01)
#define LED_FLOW_2          (0x03)
#define LED_FLOW_3          (0x07)
#define LED_FLOW_4          (0x0F)
//#define LED_FLOW_5          (0x1F)

//set which LED should be on,according to the battery level.
static uint8_t batt_level_table[6]=
{
    LED_FLOW_0,             //!< level 0        //
    LED_FLOW_1,             //!< level 1        //
    LED_FLOW_2,             //!< level 2        //
    LED_FLOW_3,             //!< level 3        //
    LED_FLOW_4,             //!< level 4        //
 //   LED_FLOW_5,           //!< level 4        //
};

// For single light horse race effect in mini current mode
static const uint8_t horse_race_led_table[] = {
	0x00, // All off
	0x01, // LED 1
	0x02, // LED 2
	0x04, // LED 3
	0x08, // LED 4
};

typedef enum
{
    LEVEL_NULL = (uint8_t)0x00,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
 //   LEVEL_5,
}batt_level_t;

//Battery Power Level and Percentage Correspondence
#define BATT_ENERGY_LEVEL1              (25)
#define BATT_ENERGY_LEVEL2              (50)
#define BATT_ENERGY_LEVEL3              (75)
#define BATT_ENERGY_LEVEL4              (100)
uint8_t charge_led_run = 0;
uint8_t charge_led_finish = 0;
uint8_t button_led_run = 0;
static batt_level_t drv_ui_coulomb(void)
{
     uint8_t _batt_energy_table[]=
     {
         BATT_ENERGY_LEVEL1,
         BATT_ENERGY_LEVEL2,
         BATT_ENERGY_LEVEL3,
         BATT_ENERGY_LEVEL4,
     };

     for(uint8_t i= 0; i < sizeof(_batt_energy_table); i++)
     {
         if(gd->real_soc_show < _batt_energy_table[i])
         {
            return (batt_level_t)(i+1);
         }
     }
     return LEVEL_4;
}
static uint8_t disp_map[6]={1,2,3,4,5,6};

static void ui_update_led(void)
{
	static uint8_t charge_cnt = 0;
	static uint8_t charge_index = 0;
	static uint8_t button_cnt = 0;
	static uint8_t flash_light = 0;
	 static uint8_t horse_index = 0;
	 static uint8_t cnt_time = 0;
	 static uint16_t cycle_count = 0;  // 循环计数器，最多 3600 次 (2 h)

//	 if (gd->ptx_protocol_phase >= WPC_PHASE_NEGO || (gd->ptx_idle_phase_status >= WPC_IDLE_STAT_XER_FOD && gd->ptx_idle_phase_status <= WPC_IDLE_STAT_EPT_ERR))
     if(g_port.is_mini_current_mode)
     	{
     		 cnt_time++;
     		 if(cnt_time >= 2) // 2 * 250ms = 500ms
     		 {
     			 cnt_time = 0; // Reset for a consistent 500ms interval
      			 /* 小电流模式：固定循环显示，不依赖电量 */
      			 horse_index++;
      			 if(horse_index > 4)  // 固定4个LED循环 (1,2,3,4)
      			 {
      				 horse_index = 1; // Loop back to the first LED
      				 // 注意：不再限制 cycle_count 到 9，而是一直累加，用于 2h 超时判断
      				 cycle_count++;  // 完成一次完整循环 (约 2 s)
      			 }

      			 // 2h(=7200 s) / 2 s per cycle = 3600 cycles
      			 if(cycle_count >= 3600)
      			 {
     				 /* 2 小时到达：1) 熄灯 2) 退出小电流模式 -> 允许进入休眠 */
     				 soc_show_ram_led = 0;
     				 g_port.is_mini_current_mode = 0;  // 解除小电流模式
     			 }
      		 }

      		 // If we just entered the mode, start the light immediately
      		 if(horse_index == 0)
      		 {
      			 horse_index = 1;
      		 }

      		 if(cycle_count < 3600) // 仅在 2h 超时前显示跑马灯
      		 {
      			 soc_show_ram_led = horse_race_led_table[horse_index];
      			 /* 防止 2h 内被 idle_to_sleep_cnt 触发休眠 */
      			 gd->idle_to_sleep_cnt = 0;
      		 }

     		 /* 2h 内保持唤醒 */
     		 if(cycle_count < 3600)
     		 {
     			 gd->SOC_SleepTime_s = 0;
     		 }

      	 }
      else if(gd->led_fault)
		{
			if(flash_light%2)
			{
				soc_show_ram_led =0x0F;
			}
			else
			{
				soc_show_ram_led = 0;
				flash_flag = 0;
			}
			flash_light++;
		}
		else if(gd->led_fault1)
		{
			if(gd->flash_times<=6)
			{
				if(flash_light%2)
				{
					soc_show_ram_led =0x0F;
				}
				else
				{
					gd->flash_times++;
					soc_show_ram_led = 0;
					flash_flag = 0;
				}
				flash_light++;
			}
			else
			{
				soc_show_ram_led = 0;
			}
		}	
		 else if (charge_led_run)
		{
			if (button_led_run)
			{
				button_led_run = 0;
				button_cnt = 0;
			}
			if (charge_cnt++ >= 2)
			{
				charge_cnt = 0;
				if(charge_index==0)
				{
					soc_show_ram_led = 0;
				}
				if (charge_index < 4)
				{
					soc_show_ram_led |= 1 << charge_index;
					charge_index++;
				}
				else if(charge_index < 5)
				{
					soc_show_ram_led = LED_FLOW_4;
					charge_index++;
				}
				else
				{
					charge_led_run = 0;
					charge_led_finish = 1;
					charge_index = 0;
					charge_cnt = 0;
				}
			}
		}
		else if (button_led_run)
		{
			if (gd->real_soc_show > 5)
			{
				soc_show_ram_led = batt_level_table[drv_ui_coulomb()];
				// 5S后或端口有变化退出
				if ((gd->idle_to_sleep_cnt > 50) || (g_port.port_state[0] != PORT_STATE_NONE) || (g_port.port_state[3] != PORT_STATE_NONE))
				{
					button_led_run = 0;
				}
			}
			else
			{
				if (button_cnt <= 18)
				{
					if(button_cnt == 0)
					{
						soc_show_ram_led = 0;
					}
					if(button_cnt%2==0)
					{
						soc_show_ram_led ^= 1;
					}
					// soc_show_ram_led|=0x10;
					if(button_cnt == 18)
					{
						soc_show_ram_led = 0;
					}
					button_cnt++;
				}
				else
				{
					soc_show_ram_led = 0;
					button_led_run = 0;
					button_cnt = 0;
				}
			}
		}	 
	else
     	 {
			gd->flash_times = 0;
     		cnt_time = 0;
     		horse_index = 0;
     		cycle_count = 0;  // 退出小电流模式时重置循环计数器
     		soc_show_ram_led = batt_level_table[drv_ui_coulomb()];
     		 if (gd->ptx_idle_phase_status >= WPC_IDLE_STAT_XER_FOD && gd->ptx_idle_phase_status <= WPC_IDLE_STAT_EPT_ERR)
     		 {
     		     flash_flag_wls = 1;
     		 }
     		 else
     		 {
     			 flash_flag_wls = 0;
     		 }
     		 if (gd->ptx_protocol_phase >= WPC_PHASE_CNFG || gd->ptx_idle_phase_status == WPC_IDLE_STAT_EPT_REP || gd->ptx_idle_phase_status == WPC_IDLE_STAT_CLOAKING)
     		 {
     			 soc_show_ram_led |= 0x20;// wireless LED is on
     		 }
     		 // if (gd->vpwr>6200 && g_port.port_state[PORT0_INDEX] == PORT_STATE_SOURCE)
     		 // LED5: 快速充电/放电指示灯 - 设备被充电或放电时都点亮
     		 if (gd->vpwr > 6200 && g_port.port_state[PORT0_INDEX] != PORT_STATE_NONE && !buckboost_protection_flag && gd->real_soc_show<100)
     		 {
     			 soc_show_ram_led |= 0x10;// fast LED is on
     		 }
     	     uint8_t _index= 3;// to get the highest bit to blink.
     	     for(; _index> 0; _index--)
     	     {
     	         if((soc_show_ram_led & (1<< _index))!= 0)
     	         {
     	             break;
     	         }
     	     }
     		 if(flash_flag_wls && flash_light_on)//!flash_light_on,to sync with the battery level LED
     		 {
     			 soc_show_ram_led ^= (1 << 5);// for blink-off
     		 }
     	     if(flash_flag == 1 && flash_light_on)
     	     {
     	        uint8_t _index= 3;// to get the highest bit to blink.
     	         for(; _index> 0; _index--)
     	         {
     	             if((soc_show_ram_led & (1<< _index))!= 0)
     	             {
     	                 break;
     	             }
     	         }
     	        soc_show_ram_led ^= (1 << _index);// for blink-off
     	     }
     		 else if(flash_flag == 2)
     	     {
     	    	 if(flash_light_on) soc_show_ram_led = 0x0F;
     	    	 else soc_show_ram_led = 0;
     	     }else if (flash_flag == 4){
     	    	if(flash_light_on) soc_show_ram_led = 0x01;
     	    	 else soc_show_ram_led = 0;
     	     }

     	     else if(flash_flag ==3)
     	     {
     	    	 soc_show_ram_led = 0;
     	     }
     	}
}
#endif

/**********************************************************************/
// ui_display()
// needs to be called with high frequency, to avoid the digital 188 blinking.
// recommend 1 ms period to call, so the function needs to be very simple.
// scan the GPIOs according to the soc_show_ram_X,
/*********************************************************************/
void ui_display (void)
{

	if(ui_no_timer_scan)
	{
		return;
	}

#ifdef LED_DISPLAY
//	_SET_ALL_PINS_IN_PUT();// reserved for multi IO control method
    // led map scan,如果用快速扫描方式
	 if(++ui_scan_index >= (sizeof(disp_map)/ sizeof(disp_map[0])))
	 {
		 ui_scan_index = 0;
	 }
	 bool _sw;
	 _sw = (bool)((soc_show_ram_led >> ui_scan_index) & 0x01); // obtain the bits to show

	 if (_sw == true)
	 {
		 drv_IO_control(disp_map[ui_scan_index], false);
		 //drv_IO_control(disp_map[ui_scan_index][0], true);// reserved for multi IO method
		 //drv_IO_control(disp_map[ui_scan_index][1], false);// reserved for multi IO control
	 }
	 else
	 {
		 drv_IO_control(disp_map[ui_scan_index], true);
	 }
#endif
}

/**********************************************************************/
// ui_update()
// update the UI related parameters.
// can be called by every 250ms, due to blink frequency can be set easier.
/*********************************************************************/

void ui_update(void)
{
	ui_no_timer_scan = 1;

	static uint8_t cnt = 0;
	static uint8_t one_min_cnt = 0;
	static uint8_t prev_woke_mode = 0; // Requirement 6: Track previous mode for unplug detection
 //   if(ui_wait_cnt< WAIT_IN_250MS) ui_wait_cnt++;

	// Requirement 6: Unplug during charge detection
	if (prev_woke_mode == BUCKBOOST_CHAGER_MODE && g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE) {
		key_ui_cnt = 4; // 4 * 250ms = 1s display
	}

	if(key_flag == 1)
	{
		key_sigle_click_process();
		gd->idle_to_sleep_cnt = 0;
		printk("\r\n ----------222------------------//-------key single click");
		// 保持 idle_to_sleep_cnt 计数，用于 2h 休眠判定
	}
	else if(key_flag == 2)
	{
		key_double_click_process();
		gd->idle_to_sleep_cnt = 0;
		// 保持 idle_to_sleep_cnt 计数，用于 2h 休眠判定
	}
	else if(key_flag == 3)
	{
		// 保持 idle_to_sleep_cnt 计数，用于 2h 休眠判定
		gd->idle_to_sleep_cnt = 0;
		key_long_click_process();
		printk("\r\n ----------222------------------//-------key long click");
	}

	key_flag = 0;


	if(gd->real_soc_obtained == 0 )
	{
		if(SOCPack_DisplaySOC_pct >0)
		{
			gd->real_soc_show = SOCPack_DisplaySOC_pct;
			gd->real_soc_obtained = 1;
			printk("update real show soc");
		}
	}
	else
	{
		if(one_min_cnt++>60)//15s
		{
			one_min_cnt =0;
#if(BUCKBOOST_USED_NU6801 == 1)
			if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE && g_buckboost.charging_stat)
#else
			if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
#endif
			{
				if(gd->real_soc_show < SOCPack_DisplaySOC_pct) gd->real_soc_show +=1;
			}
			else if (g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
			{
				if(gd->real_soc_show > SOCPack_DisplaySOC_pct) gd->real_soc_show -=1;
			}
		}

	}
	if(gd->real_soc_show >100)
	{
		gd->real_soc_show = 100;
	}
//	else if ((gd->real_soc_show > SOCPack_DisplaySOC_pct+15 || gd->real_soc_show+15 < SOCPack_DisplaySOC_pct) && SOCPack_DisplaySOC_pct >0)
//	{
//		gd->real_soc_show = SOCPack_DisplaySOC_pct;
//	}
	if(gd->real_soc_show>0) zero_soc_cnt = 0;
    //static uint8_t cnt_2s;
	cnt++;
	if(cnt > 1)//1hz
	{
		cnt = 0;
		flash_light_on ^= 1;  //qu fan
	}
	 if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE) //g_buckboost.charging_stat
    {
        zero_soc_cnt = 0;
		if (gd->real_soc_show >= 100) 
		{
			// Requirement 5: Fully charged, solid lights
        flash_flag = 0;
		} else {
			// Requirement 4: Charging, last LED blinks
			flash_flag = 1;
		}
		// if(bat_charge_ntc_ot_flag)
		// {
		// 	flash_flag = 3;
		// }
    }
   else if((g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE) && (gd->real_soc_show<=5))
    {
		charge_led_finish = 0;
    	flash_flag = 4;// low SOC state, all leds flash
    	if(gd->real_soc_show<=0)
    	{
    		if(zero_soc_cnt< 250) zero_soc_cnt++;
    	}
    }
    else{
		charge_led_finish = 0;
    	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT1_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT2_INDEX] != PORT_STATE_SOURCE && g_port.port_state[PORT3_INDEX] != PORT_STATE_SOURCE)
    	{
    		//extern uint16_t key_ui_cnt;
    		if(key_ui_cnt)
			{
				flash_flag = 0;
				key_ui_cnt--;
			}
    		else flash_flag = 3; // 灭灯
    	}
    	else
    	{
    		// if(buckboost_protection_flag) flash_flag = 2;
    		// else 
			flash_flag = 0;
    	}
		if(gd->ntc_led_off)
		{
			flash_flag = 3;
		}
    }
	// printk("\r\n gd->ntc_led_off %d\r\n",gd->ntc_led_off);
	prev_woke_mode = g_buckboost.woke_mode;
//	printk("flash_flag = %d",flash_flag);
//    printk("\r\n ------------------------real show=%d SOC display=%d  real SOC=%d RAW SOC=%d Ah SOC=%d",gd->real_soc_show, SOCPack_DisplaySOC_pct,SOCPack_RealSOC_pct,gd->SOC_RawSOC_mpct,SOC_AhIntegralSOC_mpct);
//    printk("\r\n SOC_OCVSOC_mpct-> %d  SOC_AhIntegralSOC_mpct-> %d SOC_VirtOCVSOC_mpct-> %d ",
//    		SOC_OCVSOC_mpct,SOC_AhIntegralSOC_mpct,  SOC_VirtOCVSOC_mpct);
//
//    printk("\r\n SOC_OCVUpd_flg-> %d  SOC_CHG_flg-> %d SOCPack_RealSOC_pct--> %d SOCPack_EmptySOC_mpct-> %d  SOCPack_DisplaySOC_pct-> %d",
//    		SOC_OCVUpd_flg,SOC_CHG_flg, SOCPack_RealSOC_pct, SOCPack_EmptySOC_mpct,SOCPack_DisplaySOC_pct);
#ifdef LED_DISPLAY
	ui_update_led();
#else
	ui_update_digital();
#endif

	ui_no_timer_scan = 0;

}

void key_sigle_click_process(void)
{
	if(buckboost_protection_flag) buckboost_fault_restore();

#if(CONFIG_TYPECA_SUPPORT == 1)
	if(gd->tc0_lighting_mode) gd->tc0_lighting_mode = 0;
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
	if(gd->tc1_lighting_mode) gd->tc1_lighting_mode = 0;
#endif

#if(CONFIG_WPC_SUPPORT == 1)
	if(gd->wpc_disable) gd->wpc_disable = 0;
#endif

	g_port.is_mini_current_mode = 0;
	g_port.light0_cnt = 0;
	// if(gd->sigle_clicked)
	// {
	// 	gd->sigle_clicked =0;
	// }
	// else
	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_NONE)
	{
		gd->sigle_clicked =1;
	}
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE && (gd->vpwr >13000)&&gd->sigle_clicked)
	{
		port_manager_set_event(PORT_EVENT_RESET_CHARGE);
	}
	if(g_port.port_state[PORT0_INDEX] == PORT_STATE_NONE && g_port.port_state[PORT3_INDEX] == PORT_STATE_NONE)
	{
		flash_flag = 3; 
		button_led_run = 1;
	}
	gd->ntc_led_off = 0;
}

void key_double_click_process(void)
{
	if(g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE)
	{
#if(CONFIG_TYPECA_SUPPORT == 1)
		if(g_port.port_state[0] == PORT_STATE_SOURCE) gd->tc0_lighting_mode = 0;
#endif

#if(CONFIG_TYPECB_SUPPORT == 1)
		if(g_port.port_state[1] == PORT_STATE_SOURCE) gd->tc1_lighting_mode = 0;
#endif

#if(CONFIG_WPC_SUPPORT == 1)
		 //gd->wpc_disable = 1;
		//tcpm_stop_wpc(WPC_DELAY);
	   //#endif
	  //	g_port.is_mini_current_mode = 0;  //0623
#endif // CONFIG_WPC_SUPPORT


		/* 双击切换小电流模式 */
		//if(g_port.port_state[0] == PORT_STATE_SOURCE)
		/* 双击切换小电流模式：SOURCE 或 NONE 均可 */
		if(g_port.port_state[0] != PORT_STATE_SINK)
		{
			g_port.is_mini_current_mode ^= 1;
			printk("mini_current mode -> %d\n", g_port.is_mini_current_mode);

		}
//		pdlib_restart_typec(PORT0_INDEX);
//		osal_set_event(PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_CONNECT_START);
//		osal_set_event(PORT_MANAGER_TASK, PORT_ENUM_EVT_PORT0_CONNECT_SUCCESS);

		g_port.light0_cnt = 0;
	}
	gd->sigle_clicked =0;
}

void key_long_click_process(void)
{

    {
        printk("\r\n long press power-off - no input detected, wpc_mode=%d\n", wpc_mode);

        if(g_port.port_state[0] == PORT_STATE_SOURCE) gd->tc0_lighting_mode = 1;

        SLP_vNormalToSleep();
        return;
    }
    // ==== 修改 Victor 2024-12-19 end ====
//
//    if(g_port.port_state[0] == PORT_STATE_SOURCE)
//    {
//        if(g_port.is_mini_current_mode)
//            g_port.is_mini_current_mode = 0;
//        else
//        {
//            g_port.is_mini_current_mode = 1;
//            printk("is_mini_current mode\n");
//        }
//
//        g_port.light0_cnt = 0;
//    }
}

//// structure for key information
//typedef struct KeyInfo {
//    uint8_t press_status;  // 0 means release,1 means press down
//    uint8_t is_single_click;
//    uint8_t click_count;  // click count
//    uint8_t is_double_click_pending;
//    uint16_t last_release_time;
//    uint16_t press_start_time;
//}KeyInfo;
//KeyInfo key;
//
//void initKey(void) {
//    key.press_status = 0;
//    key.press_start_time = 0;
//    key.click_count = 0;
//    key.last_release_time = 0;
//    key.is_single_click = 0;
//}

uint16_t key_ui_cnt = 0;
// double click, needs detect single click first.
static uint16_t key_cnt = 0;
static uint16_t key_delay_ms = 0;
static uint8_t key_click_cnt = 0;

void key_handle_10ms()
{
	if(!_KEY_LEVEL)
	{
		key_cnt++;
		if(key_cnt == 300)   // [NEW-VICTOR] 增加长按关机时间 150->300
		{
			key_flag = 3; //long press
			key_click_cnt = 0;
		}
		if(key_cnt == 1200)
		{
			SYS->RST_CTRL.BITS.MCU_RST = 1;
			gd->power_on_magic = 0x00;
		}

		if(key_cnt >= 1800) key_cnt = 1800;
	}
	else
	{
		if(key_cnt >= 3 && key_cnt <= 50)
		{
			if(key_click_cnt == 0)
			{
				key_delay_ms = 50;
				key_click_cnt = 1;
			}
			else
			{
				key_click_cnt = 0;
				key_delay_ms = 0;
				key_flag = 2;
			}
		}
		key_cnt = 0;
	}

	if(key_delay_ms)
	{
		key_delay_ms--;
		if(key_delay_ms == 0)
		{
			if(key_click_cnt)
			{
				key_flag = 1;
				key_click_cnt = 0;
			}
		}
	}
}

//void detectSingleKey() {
//    int currentLevel = _KEY_LEVEL;
//
//    if (currentLevel == 0) {
//        if (key.press_status == 0) {
//            key.press_start_time = sys_ticks;
//            key.press_status = 1;
//            key.click_count++;
//        }
//    } else {
//        if (key.press_status == 1) {
//            uint16_t release_time = sys_ticks;
//            uint16_t press_duration = release_time - key.press_start_time;
//            if (press_duration < LONG_PRESS_TIME_MS) {
//                if (key.click_count == 1) {
//                	 if ((uint16_t)(release_time - key.last_release_time) < DOUBLE_CLICK_TIME_MS){
//                        if (!key.is_single_click) {
//                            key.is_single_click = 1;
//                            key_ui_cnt = 20;
//                            //key_sigle_click_process();
//                            printk("\r\n ----------222------------------//-------key single click");
//                        }
//                        else{
//                        	//key_double_click_process();
//                        key.click_count = 0;
//                        key.is_single_click = 0;
//                        printk("\r\n --------------111------------------//-------key double click");
//                        }
//                    } else {
//                        key.click_count = 0;
//                        key.is_single_click = 1;
//                        //key_sigle_click_process();
//                        key_ui_cnt = 20;
//                        printk("\r\n -------------333--------------------//------key single click");
//                    }
//                }
//            } else {
//                key.click_count = 0;
//                key.is_single_click = 0;
//                printk("\r\n ---------------444---------//----------key long press");
//            }
//            key.last_release_time = release_time;
//        }
//        key.press_status = 0;
//    }
//}







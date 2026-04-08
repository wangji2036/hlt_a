#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "led.h"
#include"_wpc.h"
#include "tcpm.h"
// #include"BMS_FixPoint.h"  // gauge removed — using SOE algorithm
#include "ntc.h"
#include "port_manager.h"
#include "nu6805.h"
#include "buckboost.h"
#include "_fml.h"
#include "sleep.h"
#include "usb_bridge.h"
#include "bat.h"
extern volatile uint16_t sys_ticks;
extern uint16_t key_ui_cnt;
extern uint8_t g_wb7720_awake;
#define LED_DISPLAY

void key_sigle_click_process(void);
void key_double_click_process(void);
void key_long_click_process(void);
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
void key_triple_click_process(void);
void key_quad_click_process(void);
#endif
volatile uint8_t key_flag = 0;
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
static uint8_t comm_feedback_cnt = 0;
#endif
volatile uint8_t charge_read = 0;
volatile uint8_t charge_flag = 0;
// following variable will be update to be GB data.
static uint8_t ui_scan_index;// for scan index
#ifdef LED_DISPLAY
static uint8_t soc_show_ram_led = 0;

#else
static uint8_t flash_flag;
static uint8_t flash_light_on;
static uint32_t soc_show_ram = 0;
#endif

#define WAIT_IN_250MS 20

static uint8_t ui_no_timer_scan = 0;
static uint8_t ui_sleep = 0;        /* 五击睡眠闪烁倒计时 */
static uint8_t ui_key_cnt = 0;      /* 按键触发的电量显示倒计时 */
uint8_t button_led_run = 0;
uint8_t charge_led_run = 0;
uint8_t charge_led_finish = 0;

static void drv_IO_control(uint8_t pinx, bool status)
{
	switch (pinx)
	{
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

static uint8_t disp_map[5]={1,2,3,4,5};
#else

typedef union
{
    uint8_t byte;
    struct
    {
        uint8_t led_0: 1;               //!< led_0 display data //
        uint8_t led_1: 1;               //!< led_1 display data //
        uint8_t led_2: 1;               //!< led_2 display data //
        uint8_t led_3: 1;               //!< led_3 display data //
        uint8_t led_4: 1;               //!< led_4 display data //
        uint8_t led_5: 1;               //!< led_5 display data //
        uint8_t led_6: 1;               //!< led_6 display data //
        uint8_t led_7: 1;               //!< led_7 display data //
    }bit;
}ui_data_t;
typedef enum
{
    LED_HUNDREDS    = (uint8_t)0x00,
    LED_TENS,
    LED_UNITS,
    LED_FAST_CH,
    LED_PRCNT,
    LED_END,
}led_partition_t;


ui_data_t       gram[LED_END];

volatile const uint8_t display_num_tab[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};

uint8_t bit_is_set(uint32_t input,uint8_t bit)
{
	return (uint8_t)((input >> bit) & 0x1);
}
static void ui_update_digital(void)
{
	gram[LED_HUNDREDS].byte = 0;//3;
	gram[LED_TENS].byte = 0;//display_num_tab[0];
	gram[LED_UNITS].byte = 0;//display_num_tab[0];
	gram[LED_PRCNT].byte = 0;
	gram[LED_FAST_CH].byte = 0;
	if(!(!flash_light_on && flash_flag == 2))
	{
		gram[LED_PRCNT].byte = 1;
		gram[LED_FAST_CH].byte = 1;
	}
	if(soc_show < 100)
	{
		uint8_t data_temp = 0;
		gram[LED_HUNDREDS].byte = 0;
		data_temp = soc_show % 10;

		if(!(!flash_light_on && flash_flag!=0)) gram[LED_UNITS].byte = display_num_tab[data_temp];
		if(soc_show < 10)
		{
			gram[LED_TENS].byte = 0;
		}else{
			data_temp = soc_show / 10;
			if(!(!flash_light_on && flash_flag == 2)) gram[LED_TENS].byte = display_num_tab[data_temp];
		}
	}
	else{
		//if(flash_light_on)
		if(!(!flash_light_on && flash_flag == 2))
		{
			gram[LED_HUNDREDS].byte = 3;
			gram[LED_TENS].byte = display_num_tab[0];
			gram[LED_UNITS].byte = display_num_tab[0];
		}
	}
	soc_show_ram = ((gram[LED_PRCNT].byte & 0x01) << 17) | ((gram[LED_FAST_CH].byte & 0x01) << 16) | \
	            ((gram[LED_UNITS].byte & 0x7F) << 9) | ((gram[LED_TENS].byte & 0x7F) << 2) | ((gram[LED_HUNDREDS].byte & 0x03)) ; // Save the GRAM data into a temporary variable
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
    // led map scan
	 if(++ui_scan_index >= (sizeof(disp_map)/ sizeof(disp_map[0])))
	 {
		 ui_scan_index = 0;
	 }
	 bool _sw;
	 _sw = (bool)((soc_show_ram_led >> ui_scan_index) & 0x01); // obtain the bits to show

	 if (_sw == true)
	 {
		 drv_IO_control(disp_map[ui_scan_index], false);
	 }
	 else
	 {
		 drv_IO_control(disp_map[ui_scan_index], true);
	 }

#else
	 _SET_ALL_PINS_IN_PUT();
	 if(ui_scan_index >4) ui_scan_index = 0;
	 switch (ui_scan_index)
	 {
	     case 0:
		     if(bit_is_set(soc_show_ram,10)) drv_IO_control(2,true);
		     if(bit_is_set(soc_show_ram,12)) drv_IO_control(3,true);
		     if(bit_is_set(soc_show_ram,14)) drv_IO_control(4,true);
		     if(bit_is_set(soc_show_ram,15)) drv_IO_control(5,true);
		     drv_IO_control(1,false);
		     break;
	     case 1:
		     if(bit_is_set(soc_show_ram,3)) drv_IO_control(3,true);
		     if(bit_is_set(soc_show_ram,5)) drv_IO_control(4,true);
		     if(bit_is_set(soc_show_ram,6)) drv_IO_control(5,true);
		     if(bit_is_set(soc_show_ram,9)) drv_IO_control(1,true);
		     drv_IO_control(2,false);
		     break;
	     case 2:
		     if(bit_is_set(soc_show_ram,2)) drv_IO_control(2,true);
		     if(bit_is_set(soc_show_ram,4)) drv_IO_control(4,true);
		     if(bit_is_set(soc_show_ram,7)) drv_IO_control(5,true);
		     if(bit_is_set(soc_show_ram,11)) drv_IO_control(1,true);
		     drv_IO_control(3,false);
		     break;
	     case 3:
		     if(bit_is_set(soc_show_ram,0)) drv_IO_control(3,true);
		     if(bit_is_set(soc_show_ram,1)) drv_IO_control(2,true);
		     if(bit_is_set(soc_show_ram,8)) drv_IO_control(5,true);
		     if(bit_is_set(soc_show_ram,13)) drv_IO_control(1,true);
		     drv_IO_control(4,false);
		     break;
	     case 4:
		     if(bit_is_set(soc_show_ram,16)) drv_IO_control(3,true);
		     if(bit_is_set(soc_show_ram,17)) drv_IO_control(2,true);
		     drv_IO_control(5,false);
		     break;
	     default:
    	 break;
		}
	 ui_scan_index++;

#endif
}

/**********************************************************************/
// ui_update()
// update the UI related parameters.
// can be called by every 250ms, due to blink frequency can be set easier.
/*********************************************************************/
uint8_t ui_evt = 0;

uint8_t led_mask = 0;
static uint8_t wpc_state = 0;
uint8_t ui_power_on_cnt = 0;
static uint8_t ui_chg_start_cnt = 0;
static uint8_t ui_state_chg_debounce = 0;

extern uint8_t rx_may_still_be_flag;

void led_ui_event_update(void)
{
	static uint8_t last_state= 0;
	static uint8_t last_state1= 0;
	static uint8_t last_state_wpc= 0;

	/* 有新按键时立即清除上一次按键的LED灯显，当前按键灯显即刻生效 */
	if(key_flag != 0)
	{
		ui_key_cnt = 0;
		ui_sleep = 0;
		ui_power_on_cnt = 0;
		ui_chg_start_cnt = 0;
		ui_evt &= ~UI_EVENT_KEY_CLICK;
	}

#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	/* USB_COM 模式下，任何按键先退出 USB_COM 并消费事件 */
	if (gd->usb_comm_activated && key_flag != 0)
	{
		gd->usb_comm_activated = 0;
		usb_comm_unlock();
		usb_bridge_sleep();
		comm_feedback_cnt = 2;  // 1 flash feedback
		printk("USB comm exit by key %d\n", key_flag);
		key_flag = 0;
	}
#endif

	if(key_flag == 1)
	{
		key_sigle_click_process();
		gd->idle_to_sleep_cnt = 0;
	}
	else if(key_flag == 2)
	{
		key_double_click_process();
		gd->idle_to_sleep_cnt = 0;
	}
	else if(key_flag == 3)
	{
		gd->idle_to_sleep_cnt = 0;
		key_long_click_process();
	}
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	else if(key_flag == 4)
	{
		key_triple_click_process();
		gd->idle_to_sleep_cnt = 0;
	}
	else if(key_flag == 5)
	{
		key_quad_click_process();
		gd->idle_to_sleep_cnt = 0;
	}
#endif

	key_flag = 0;

	if(last_state != g_port.port_state[PORT0_INDEX] ||
	   last_state1 != g_port.port_state[PORT1_INDEX] ||
	   g_port.port_state[WPC_INDEX] != last_state_wpc)
	{
		ui_evt |= UI_EVENT_STATE_CHANGE;
		printk("UI_EVENT_STATE_CHANGE\n");
		g_port.light0_cnt = 0;
		g_port.light1_cnt = 0;
		//g_port.is_mini_current_mode = 0;
	}
	last_state = g_port.port_state[PORT0_INDEX];
	last_state1 = g_port.port_state[PORT1_INDEX];
	last_state_wpc = g_port.port_state[WPC_INDEX];

	static uint8_t delay_cnt = 0;
	if(gd->ptx_idle_phase_status >= WPC_IDLE_STAT_XER_FOD && gd->ptx_idle_phase_status <= WPC_IDLE_STAT_EPT_ERR)
	{
		delay_cnt = 0;
		wpc_state = 2;
	}
	else if (gd->ptx_protocol_phase >= WPC_PHASE_CNFG || gd->ptx_idle_phase_status == WPC_IDLE_STAT_EPT_REP || gd->ptx_idle_phase_status == WPC_IDLE_STAT_CLOAKING || rx_may_still_be_flag)
	{
		delay_cnt = 10;
		wpc_state = 1;
	}
	else
	{
		if(delay_cnt)
		{
			delay_cnt--;
			if(delay_cnt == 0) wpc_state = 0;
		}
		else
			wpc_state = 0;
	}
}
static uint8_t ui_qdt_cali_cnt = 0;
void led_ui_hanlde_250ms(uint8_t bat_cap)
{
	static uint8_t ui_key_long_cnt = 0;
	static uint8_t ui_cnt = 0;
	static uint8_t ui_fault_cnt = 0;
	static uint8_t ui_dischg_cnt = 0;

	static uint8_t ui_qdt_cali_success_cnt = 0;
	//static uint8_t power_flag = 0;
	static uint8_t ui_mode_cnt = 0;

	static uint16_t mini_cnt = 0;
	static uint8_t wpc_fod_blink_cnt = 0;   /* FOD 闪烁次数计数，闪 5 次后关灯 */
	static uint8_t last_wpc_state_led = 0;  /* 上一拍的 wpc_state，用于检测进入 FOD */

#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
	if(comm_feedback_cnt > 0)
	{
		comm_feedback_cnt--;
		soc_show_ram_led = (comm_feedback_cnt & 1) ? 0x00 : 0x0F;
		return;
	}

#if (CONFIG_USB_COMM_LED5_BLINK == 1)
	if(gd->usb_comm_activated)
	{
		static uint8_t comm_blink_cnt = 0;
		comm_blink_cnt++;
		uint8_t led_val = (comm_blink_cnt & 1) ? 0x10 : 0x00;  // LED5 toggle
#if (CONFIG_USB_COMM_LED4_WB_STATE == 1)
		if (g_wb7720_awake)
			led_val |= 0x08;                        // LED4 solid ON
		else
			led_val |= (comm_blink_cnt & 1) ? 0x08 : 0x00;  // LED4 blink
#endif
		soc_show_ram_led = led_val;
		return;
	}
#endif
#endif

	// 拔充电器时自动显示1秒电量
	static uint8_t prev_woke_mode = 0;
	if (prev_woke_mode == BUCKBOOST_CHAGER_MODE && g_buckboost.woke_mode != BUCKBOOST_CHAGER_MODE) {
		key_ui_cnt = 4;
	}
	prev_woke_mode = g_buckboost.woke_mode;

	if(ui_state_chg_debounce) ui_state_chg_debounce--;
    led_printk("FORCE USB=%d\n",gd->force_usb_mode);
	if(g_port.is_mini_current_mode)
	{
		mini_cnt++;
		if(mini_cnt >=28800)  g_port.is_mini_current_mode = 0;
	}
	else
	{
		mini_cnt = 0;
	}

	if(ui_evt & UI_EVENT_QDT_CALI_START)
	{
		ui_evt &= ~UI_EVENT_QDT_CALI_START;
		ui_qdt_cali_cnt = 100;
		ui_key_cnt = 0;
		ui_cnt = 0;
		ui_dischg_cnt = 0;
		ui_mode_cnt = 0;
		ui_chg_start_cnt = 0;
		ui_fault_cnt = 0;
	}
	else if(ui_evt & UI_EVENT_QDT_CALI_SUCCESS)
	{
		ui_evt &= ~UI_EVENT_QDT_CALI_SUCCESS;
		ui_qdt_cali_success_cnt = 17;
		ui_qdt_cali_cnt = 0;
		ui_key_cnt = 0;
		ui_cnt = 0;
		ui_dischg_cnt = 0;
		ui_mode_cnt = 0;
		ui_chg_start_cnt = 0;
		ui_fault_cnt = 0;
	}
	else if(ui_evt & UI_EVENT_STATE_CHANGE)
	{
		ui_evt &= ~UI_EVENT_STATE_CHANGE;
		if(ui_state_chg_debounce == 0)
		{
			if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SINK)
			{
				led_printk("ui_chg_start\n");
				ui_key_cnt = 0;
				ui_cnt = 0;
				ui_chg_start_cnt = 15;
				ui_dischg_cnt = 0;
				ui_mode_cnt = 0;
				gd->force_usb_mode = 0;
			}
			ui_state_chg_debounce = 8;  // 4 * 250ms = 1s debounce skip
		}
	}
	else if(ui_evt & UI_EVENT_KEY_CLICK)
	{
		ui_evt &= ~UI_EVENT_KEY_CLICK;
		{
			led_printk("single click\n");
			ui_key_cnt = 22;  // 22 * 250ms = 5.5秒 (5-6秒范围)
			ui_key_long_cnt = 0;
			ui_cnt = 0;
			ui_mode_cnt = 0;
		}
	}
	else if(ui_evt & UI_EVENT_FAULT)
	{
		led_printk("UI_EVENT_FAULT\n");
		ui_evt &= ~UI_EVENT_FAULT;
		ui_dischg_cnt = 0;
		ui_fault_cnt = 20;
		ui_mode_cnt = 0;
	}

	if(gd->bat_ov_forbid_flag)
	{
		if(ui_cnt % 4 < 2) led_mask = 0x1f;
		else led_mask = 0x00;
	}
	else if(ui_power_on_cnt)
	{
		ui_power_on_cnt--;
		if(ui_power_on_cnt == 19) 		led_mask = 0x00;
		if(ui_power_on_cnt == 16) 		led_mask = 0x01;
		if(ui_power_on_cnt == 12) 		led_mask = 0x02;
		if(ui_power_on_cnt == 8) 		led_mask = 0x04;
		if(ui_power_on_cnt == 4) 		led_mask = 0x08;
		if(ui_power_on_cnt == 0) 		{ led_mask = 0x00; ui_key_cnt = 20;}
	}
	else if(ui_sleep)
	{
		ui_sleep--;
		if(ui_sleep == 7) 		led_mask = 0x00;
		if(ui_sleep == 6) 		led_mask = 0x0f;
		if(ui_sleep == 5) 		led_mask = 0x00;
		if(ui_sleep == 4) 		led_mask = 0x0f;
		if(ui_sleep == 3) 		led_mask = 0x00;
		if(ui_sleep == 2) 		led_mask = 0x0f;
		if(ui_sleep == 1) 		led_mask = 0x00;
		if(ui_sleep == 0) 		led_mask = 0x00;
	}
	else if(gd->force_usb_mode ==1)
	{
		/* LED1+3 / LED2+4 交替闪烁(0.5s周期)，持续闪烁，单击退出 */
		if(ui_cnt % 4 < 2) led_mask = 0x05;  /* LED1+3亮 0.5s */
		else                led_mask = 0x0A;  /* LED2+4亮 0.5s */
	}
	else if(ui_qdt_cali_cnt)
	{
		if(ui_qdt_cali_cnt % 2)led_mask = 0x09;
		else led_mask = 0x00;
		ui_qdt_cali_cnt--;
		if(ui_qdt_cali_cnt == 0) ui_qdt_cali_cnt = 100;
	}
	else if(ui_qdt_cali_success_cnt)
	{
		led_mask = 0x09;
		ui_qdt_cali_success_cnt--;
	}
	else if(ui_chg_start_cnt)
	{
		ui_chg_start_cnt--;
		if(ui_chg_start_cnt > 12) led_mask = 0x01;
		else if(ui_chg_start_cnt > 9) led_mask = 0x03;
		else if(ui_chg_start_cnt > 6) led_mask = 0x07;
		else if(ui_chg_start_cnt > 3) led_mask = 0x0f;
		else led_mask = 0xf;
		ui_cnt = 200;
		if(wpc_state == 1) led_mask |= 0x10;

	}
	else if(ui_key_cnt)
	{
		ui_key_cnt--;
		if(gd->bat_dead_flag)
		{
			{if(ui_cnt % 2 < 1) led_mask = 0x01;else led_mask = 0x00;}
		}
		else
		{
			if(bat_cap < 10) 		{if(ui_cnt % 2 < 1) led_mask = 0x01;else led_mask = 0x00;}
			else if(bat_cap < 25) 	{led_mask = 0x01;}
			else if(bat_cap < 50) 	{led_mask = 0x03;}
			else if(bat_cap < 75) 	{led_mask = 0x07;}
			else {led_mask = 0x0f;}
		}
		if(wpc_state == 1) led_mask |= 0x10;
	}
	else if(ui_fault_cnt)
	{
		if(ui_fault_cnt % 4 < 2)  led_mask = 0x0f;
		else led_mask = 0x00;
		ui_fault_cnt--;
	}
	else if(g_port.port_state[PORT1_INDEX] == PORT_STATE_SINK && !buckboost_protection_flag  && !ntc_stop_chrg_flag && !ntc_lock_flag)
	{
		g_port.is_mini_current_mode = 0;
		if(g_buckboost.bat_full_flag || (g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE && bat_cap >= 100))
		{
		led_mask = 0x0f;
		}
		else
		{
			if(bat_cap <= 25) 		{if(ui_cnt % 4 < 2) led_mask = 0x01;else led_mask = 0x00;}
			else if(bat_cap <= 50) 	{if(ui_cnt % 4 < 2) led_mask = 0x03;else led_mask = 0x01;}
			else if(bat_cap <= 75) 	{if(ui_cnt % 4 < 2) led_mask = 0x07;else led_mask = 0x03;}
			else if(bat_cap < 100) 	{if(ui_cnt % 4 < 2) led_mask = 0x0f;else led_mask = 0x07;}
			else led_mask = 0x00;
		}
		if(wpc_state == 1) led_mask |= 0x10;
	}
	else if(g_port.is_mini_current_mode)
	{
		if(ui_mode_cnt < 2) led_mask = 0x00;
		else if(ui_mode_cnt < 4) led_mask = 0x01;
		else if(ui_mode_cnt < 6) led_mask = 0x02;
		else if(ui_mode_cnt < 8) led_mask = 0x04;
		else if(ui_mode_cnt < 10) led_mask = 0x08;
		else led_mask = 0x00;
		ui_mode_cnt++; if(ui_mode_cnt >=12) ui_mode_cnt = 0;
		if(wpc_state == 1) led_mask |= 0x10;
	}
	else if(((g_port.port_state[PORT1_INDEX] == PORT_STATE_SOURCE && g_port.light1_cnt < 35) || (g_port.port_state[WPC_INDEX] == PORT_STATE_SOURCE || rx_may_still_be_flag) || (wpc_state == 1)) && !buckboost_protection_flag)
	{
		if(bat_cap < 10) 		{if(ui_cnt % 2 < 1) led_mask = 0x01;else led_mask = 0x00;}
		else if(bat_cap < 25) 	{led_mask = 0x01;}
		else if(bat_cap < 50) 	{led_mask = 0x03;}
		else if(bat_cap < 75) 	{led_mask = 0x07;}
		else {led_mask = 0x0f;}
		if(wpc_state == 1) led_mask |= 0x10;
		ui_dischg_cnt--;
	}
	else
	{

		led_mask = 0;
		if(wpc_state == 1) led_mask |= 0x10;
	}

	if(wpc_state == 2)
	{
		/* 刚进入 FOD：重置计数，闪 5 次后关闭无线灯 */
		if(last_wpc_state_led != 2)
			wpc_fod_blink_cnt = 0;
		if(wpc_fod_blink_cnt < 10)  /* 5 次闪烁 = 10 个 250ms 周期 */
		{
			if(ui_cnt % 2 < 1) led_mask |= 0x10;
			else led_mask &= ~0x10;
			wpc_fod_blink_cnt++;
		}
		else
			led_mask &= ~0x10;  /* 闪满 5 次后保持灭 */
	}
	else if(wpc_state == 1)
	{
		//led_mask |= 0x10;
	}
	else
		led_mask &= ~0x10;

	last_wpc_state_led = wpc_state;

	ui_cnt++;if(ui_cnt >= 200) ui_cnt = 0;


	if(g_buckboost.adc_vbat < 2800 && gd->bat_dead_flag)
		soc_show_ram_led = 0;
	else
		soc_show_ram_led = led_mask;

}

void ui_update(void)
{
	ui_no_timer_scan = 1;

	led_ui_event_update();

	gd->real_soc_show = g_bat.bat_level_ui;
	if(gd->real_soc_show > 100)
	{
		gd->real_soc_show = 100;
	}
	if(gd->real_soc_show > 0) zero_soc_cnt = 0;

	led_ui_hanlde_250ms(gd->real_soc_show);

	ui_no_timer_scan = 0;

}
/* 进入休眠前：立刻关闭LED驱动，灯全灭
 * - 停止UI扫描
 * - 将所有LED相关GPIO配置为输入（高阻），物理灯熄灭
 */
void led_all_off_before_sleep(void)
{
	ui_no_timer_scan = 1;      // 不再调用 ui_display 的扫描逻辑
	_SET_ALL_PINS_IN_PUT();    // 直接把所有LED GPIO关掉
}
void key_sigle_click_process(void)
{
	if(buckboost_protection_flag) buckboost_fault_restore();
	gd->typec_scp =0;
	gd->vbus_ovp =0;

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
	if(g_port.port_state[PORT0_INDEX] != PORT_STATE_NONE)
	{
		gd->sigle_clicked =1;
	}
	if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE && (gd->vpwr >13000)&&gd->sigle_clicked)
	{
		port_manager_set_event(PORT_EVENT_RESET_CHARGE);
	}
	gd->ntc_led_off = 0;
	gd->touch_to_weakup = 0;

	if(gd->ptx_idle_phase_status == WPC_IDLE_STAT_QDT_CALI){ gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY; ui_qdt_cali_cnt = 0;}

	if(!g_port.is_mini_current_mode)
	{
		ui_evt |= UI_EVENT_KEY_CLICK;
	}
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
#endif

		/* 双击切换小电流模式：SOURCE 或 NONE 均可 */
		if(g_port.port_state[0] != PORT_STATE_SINK)
		{
			g_port.is_mini_current_mode ^= 1;
			printk("mini_current mode -> %d\n", g_port.is_mini_current_mode);
		}

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
}

#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
void key_triple_click_process(void)
{
	if (buckboost_protection_flag) return;

	gd->usb_comm_activated ^= 1;
	if(gd->usb_comm_activated)
	{
		usb_comm_lock();
		comm_feedback_cnt = 6;  // 3 flashes (on-off-on-off-on-off @ 250ms)
		printk("USB comm activated by triple-click\n");
	}
	else
	{
		usb_comm_unlock();
		usb_bridge_sleep();
		comm_feedback_cnt = 2;  // 1 flash (on-off @ 250ms)
		printk("USB comm deactivated by triple-click\n");
	}
	key_ui_cnt = 0;
}
#endif

void key_quad_click_process(void)
{
	gd->forbid_bypass_flag ^= 1;
	if (gd->forbid_bypass_flag) {
		if (gd->bat_ov_forbid_flag) {
			gd->bat_ov_forbid_flag = 0;
			printk("\r\n[FORBID] OV cleared by quad-click");
		}
		if (gd->bat_uv_forbid_flag) {
			gd->bat_uv_forbid_flag = 0;
			printk("\r\n[FORBID] UV cleared by quad-click");
		}
		printk("\r\n[FORBID] Bypass ENABLED by quad-click");
	} else {
		printk("\r\n[FORBID] Bypass DISABLED by quad-click");
	}
	key_ui_cnt = 0;
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
		if(key_cnt == 300)   // 长按关机时间 300*10ms=3s
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
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
			key_click_cnt++;
			key_delay_ms = 50;
			if(key_click_cnt >= 4)
			{
				key_flag = 5;  // quad click
				key_click_cnt = 0;
				key_delay_ms = 0;
			}
#else
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
#endif
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
#if (CONFIG_TRIPLE_CLICK_COMM_ENABLE == 1)
				if(key_click_cnt == 1)
					key_flag = 1;  // single click
				else if(key_click_cnt == 2)
					key_flag = 2;  // double click
				else if(key_click_cnt == 3)
					key_flag = 4;  // triple click
#else
				key_flag = 1;
#endif
				key_click_cnt = 0;
			}
		}
	}
}

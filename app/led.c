#include "regdef.h"
#include "printk.h"
#include "g_data.h"
#include "delay.h"
#include "led.h"
#include"_wpc.h"
#include"BMS_FixPoint.h"

//#define LED_DISPLAY
#define LED_HIGH_LIGHT
// tese use
//static uint8_t ui_wait_cnt;
//static uint8_t ui_initialed;
//test use end
// following variable will be update to be GB data.
uint8_t soc_show = 0;// SOC的数值，用于最终的显示
static uint8_t flash_flag;//1:充电闪，2：异常闪--全闪。
static uint8_t flash_light_on;//用于闪烁控制，置位的时候，说明闪烁为亮的状态，否则为关闭状态。
static uint8_t ui_scan_index;//用于扫描查询目录。
#ifdef LED_DISPLAY
static uint8_t soc_show_ram_led = 0;//临时变量，用于表征LED灯显，各个LED是否需要点亮。
static uint8_t flash_flag_wls;//无线冲的LED指示，是否闪烁
#else
static uint32_t soc_show_ram = 0;//一个临时变量，各个bit用于表征数码管各个码段，
#endif

#define WAIT_IN_250MS 20
void led_init(void)
{
}

#define _UI_PIN1_PORT     GPA
#define _UI_PIN2_PORT     GPC
#define _UI_PIN3_PORT     GPB
#define _UI_PIN4_PORT     GPB
#define _UI_PIN5_PORT     GPC
#define PORT_GPA          GPA
#define PORT_GPB          GPB

#define _UI_PIN1_PINx     PIN4
#define _UI_PIN2_PINx     PIN5
#define _UI_PIN3_PINx     PIN6
#define _UI_PIN4_PINx     PIN7
#define _UI_PIN5_PINx     PIN7

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

#ifdef LED_DISPLAY

// LED 1~4for battery level , LED 5 for wireless charger.
// LED H for bright,

#define LED_FLOW_0          (0x00)
#define LED_FLOW_1          (0x01)
#define LED_FLOW_2          (0x03)
#define LED_FLOW_3          (0x07)
#define LED_FLOW_4          (0x0F)
//#define LED_FLOW_5          (0x1F)

//set which LED should be on according to the battery level。
static uint8_t batt_level_table[6]=
{
    LED_FLOW_0,             //!< level 0        //
    LED_FLOW_1,             //!< level 1        //
    LED_FLOW_2,             //!< level 2        //
    LED_FLOW_3,             //!< level 3        //
    LED_FLOW_4,             //!< level 4        //
 //   LED_FLOW_5,             //!< level 4        //
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

//电池电量档位、百分比对应关系
#define BATT_ENERGY_LEVEL1              (25)
#define BATT_ENERGY_LEVEL2              (50)
#define BATT_ENERGY_LEVEL3              (75)
#define BATT_ENERGY_LEVEL4              (100)

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
         if(soc_show < _batt_energy_table[i])
         {
            return (batt_level_t)(i+1);
         }
     }
     return LEVEL_4;
}
/*
// reserved for future use,  3 gpios to control more LEDs.
static uint8_t disp_map[7][2]=
{
    // H, L
    {1, 2},   //!< led 0 light
    {2, 1},   //!< led 1 light
    {3, 1},   //!< led 2 light
    {2, 3},   //!< led 3 light
    {1, 3},   //!< led 4 light
    {3, 1},   //!< led 5 light
    {4, 3},   //!< led 6 light//key
};
*/
static uint8_t disp_map[5]={1,2,3,4,5};

static void ui_update_led(void)
{
	 soc_show_ram_led = batt_level_table[drv_ui_coulomb()];


	 gd->ptx_idle_phase_status = WPC_IDLE_STAT_STANDBY;
	 if (gd->ptx_protocol_phase >= WPC_PHASE_NEGO || (gd->ptx_idle_phase_status >= WPC_IDLE_STAT_XER_FOD && gd->ptx_idle_phase_status <= WPC_IDLE_STAT_EPT_ERR))
	 {
	     flash_flag_wls = 1;
	 }
	 else
	 {
		 flash_flag_wls = 0;
	 }
	 if (gd->ptx_protocol_phase >= WPC_PHASE_IDLE)
	 {
		 soc_show_ram_led |= 0x10;// wireless LED is on
	 }


     uint8_t _index= 3;// to get the highest bit to blink.
     for(; _index> 0; _index--)
     {
         if((soc_show_ram_led & (1<< _index))!= 0)
         {
             break;
         }
     }
	 if(flash_flag_wls && !flash_light_on)
	 {
		 soc_show_ram_led ^= (1 << 4);// for blink-off
	 }
     if(flash_flag == 2)
     {
    	 if(flash_light_on) soc_show_ram_led = 0x1F;
    	 else soc_show_ram_led = 0;
     }
     else if (flash_flag == 1)
     {
		 if (!flash_light_on) // if needs blink, and it is blink-off
		 {
			soc_show_ram_led ^= (1 << _index);// // for blink-off
		 }
     }

     printk("\r\n LED ram-> %d SOC %d",soc_show_ram_led,soc_show);
}
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

#define _SET_ALL_PINS_IN_PUT() do{\
        _UI_PIN1_PORT-> O_EN.BITS._UI_PIN1_PINx = 0; _UI_PIN1_PORT->I_EN.BITS._UI_PIN1_PINx = 1;\
		_UI_PIN2_PORT-> O_EN.BITS._UI_PIN2_PINx = 0; _UI_PIN2_PORT->I_EN.BITS._UI_PIN2_PINx = 1;\
		_UI_PIN3_PORT-> O_EN.BITS._UI_PIN3_PINx = 0; _UI_PIN2_PORT->I_EN.BITS._UI_PIN3_PINx = 1;\
		_UI_PIN4_PORT-> O_EN.BITS._UI_PIN4_PINx = 0; _UI_PIN2_PORT->I_EN.BITS._UI_PIN4_PINx = 1;\
		_UI_PIN5_PORT-> O_EN.BITS._UI_PIN5_PINx = 0; _UI_PIN2_PORT->I_EN.BITS._UI_PIN5_PINx = 1;\
} while(0)
volatile const uint8_t display_num_tab[10]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
static const uint8_t disp_map[18][2]=
{
    //H, L hundred bit
    {3, 4},   //!< led 0 light //
    {2, 4},   //!< led 1 light //

    {2, 3},   //!< led 2 light //
    {3, 2},   //!< led 3 light //
    {4, 3},   //!< led 4 light //
    {4, 2},   //!< led 5 light //
    {5, 2},   //!< led 6 light //
    {5, 3},   //!< led 7 light //
    {5, 4},   //!< led 8 light //

    {1, 2},   //!< led 9 light //
    {2, 1},   //!< led 10 light //
    {1, 3},   //!< led 11 light //
    {3, 1},   //!< led 12 light //
    {1, 4},   //!< led 13 light //
    {4, 1},   //!< led 14 light //
    {5, 1},   //!< led 15 light //

    {3, 5},   //!< led 16 light //
    {2, 5},   //!< led 17 light //
};

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
//		else if(flash_flag == 1)
//		{
//			gram[LED_HUNDREDS].byte = 3;
//			gram[LED_TENS].byte = display_num_tab[0];
//		}
	}
	soc_show_ram = ((gram[LED_PRCNT].byte & 0x01) << 17) | ((gram[LED_FAST_CH].byte & 0x01) << 16) | \
	            ((gram[LED_UNITS].byte & 0x7F) << 9) | ((gram[LED_TENS].byte & 0x7F) << 2) | ((gram[LED_HUNDREDS].byte & 0x03)) ; // 将GRAM数据保存到临时变量中
}
#endif

/**********************************************************************/
// ui_display()
// needs to be called with high frequency, to avoid the digital tube blinking.
// recommend 1 ms period to call, so the function needs to be very simple.
// scan the GPIOs according to the soc_show_ram_X,
/*********************************************************************/
void ui_display (void)
{
#ifdef LED_DISPLAY
//	_SET_ALL_PINS_IN_PUT();//如果是多个IO脚高低电平刷新的方式
    // led map scan,如果用快速扫描方式
	 if(++ui_scan_index >= (sizeof(disp_map)/ sizeof(disp_map[0]))) // 增加UI刷新状态
	 {
		 ui_scan_index = 0; // 当刷新状态超过时，将其重置为0
	 }
	 bool _sw;
	 _sw = (bool)((soc_show_ram_led >> ui_scan_index) & 0x01); // 获取对应索引的GRAM数据位的值

	 if (_sw == true) // 如果对应索引的GRAM数据位为1
	 {
		 drv_IO_control(disp_map[ui_scan_index], false);
		 //drv_IO_control(disp_map[ui_scan_index][0], true);//如果是多个IO脚高低电平刷新的方式
		 //drv_IO_control(disp_map[ui_scan_index][1], false);//如果是多个IO脚高低电平刷新的方式
	 }
	 else
	 {
		 drv_IO_control(disp_map[ui_scan_index], true);
	 }

#else
		_SET_ALL_PINS_IN_PUT();
		if(++ui_scan_index >= (sizeof(disp_map)/ sizeof(disp_map[0]))) // 增加UI刷新状态
		{
			ui_scan_index = 0; // 当刷新状态超过时，将其重置为0
		}

		bool _sw;
		_sw = (bool)((soc_show_ram >> ui_scan_index) & 0x0001); // 获取对应索引的GRAM数据位的值

		if (_sw == true) // 如果对应索引的GRAM数据位为1
		{
			drv_IO_control(disp_map[ui_scan_index][0], true);
			drv_IO_control(disp_map[ui_scan_index][1], false);
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
 //   if(ui_wait_cnt< WAIT_IN_250MS) ui_wait_cnt++;
    soc_show = SOCPack_DisplaySOC_pct;
    static uint8_t cnt_2s;
    flash_light_on ^= 1;
    if(g_buckboost.woke_mode == BUCKBOOST_CHAGER_MODE)
    {
    	flash_flag = 1;
    }
    else if((g_buckboost.woke_mode == BUCKBOOST_DISCHG_MODE) && (soc_show<15))
    {
    	flash_flag = 2;
    }
    else{
    	flash_flag = 0;
    }

    printk("\r\n SOC show--------------------> %d  row-------------------------> %d", soc_show,SOCPack_RealSOC_pct);
    printk("\r\n SOC_OCVSOC_mpct-> %d  SOC_AhIntegralSOC_mpct-> %d SOC_RawSOC_mpct--> %d SOC_VirtOCVSOC_mpct-> %d ",
    		SOC_OCVSOC_mpct,SOC_AhIntegralSOC_mpct, SOC_RawSOC_mpct, SOC_VirtOCVSOC_mpct);

    printk("\r\n SOC_OCVUpd_flg-> %d  SOC_CHG_flg-> %d SOCPack_RealSOC_pct--> %d SOCPack_EmptySOC_mpct-> %d  SOCPack_DisplaySOC_pct-> %d",
    		SOC_OCVUpd_flg,SOC_CHG_flg, SOCPack_RealSOC_pct, SOCPack_EmptySOC_mpct,SOCPack_DisplaySOC_pct);
#ifdef LED_DISPLAY
	ui_update_led();
#else
	ui_update_digital();
#endif

}

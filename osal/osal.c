#include "regdef.h"
#include "typdef.h"
#include "printk.h"
#include "usb_pd.h"
#include "osal.h"
#include"led.h"

extern volatile uint16_t sys_ticks;
static volatile uint16_t old_ticks;

static const unsigned char bit_map[] =
{
	0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	   4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

static struct osal_task_t
{
	uint32_t event;
	void (*task_cb)(uint32_t);
} osal_tasks_tbl[MAX_TASK];

enum osal_timer_state_t
{
	OSAL_TIMER_STS_STOPPED = 0,
	OSAL_TIMER_STS_RUNNING = 1,
};

static struct osal_timer_t
{
	uint16_t remain;
	uint16_t period;
	uint32_t event;
	uint16_t state : 8;
	uint16_t task_id : 8;
} osal_timer_tbl[MAX_TIMER];

void osal_init(void)
{
	uint32_t id;

	for (id=0; id<MAX_TASK; id++)
	{
		osal_tasks_tbl[id].event = 0;
		osal_tasks_tbl[id].task_cb = NULL;
	}

	for (id=0; id<MAX_TIMER; id++)
	{
		osal_timer_tbl[id].remain = 0;
		osal_timer_tbl[id].period = 0;
		osal_timer_tbl[id].state = OSAL_TIMER_STS_STOPPED;
		osal_timer_tbl[id].task_id = 0;
		osal_timer_tbl[id].event = 0;
	}
}

void osal_set_event(uint8_t task_id, uint32_t event)
{
	if (task_id >= MAX_TASK)
	{
		printk("\r\n invalid event set");
		return;
	}

	osal_tasks_tbl[task_id].event |= event;
}

void osal_clear_event(uint8_t task_id, uint32_t event)
{
	if (task_id >= MAX_TASK)
	{
		printk("\r\n invalid event clear");
		return;
	}

	osal_tasks_tbl[task_id].event &= ~event;
}

void osal_task_handler_reg(uint8_t task_id, void (*handler)(uint32_t))
{
	if (task_id >= MAX_TASK)
	{
		printk("\r\n invalid event handler register");
		return;
	}

	osal_tasks_tbl[task_id].task_cb = handler;
}

static void osal_event_handle(void)
{
	uint32_t id, event, evt_msk;

	for (id=0; id<MAX_TASK; id++)
	{
		if (osal_tasks_tbl[id].event == 0 || osal_tasks_tbl[id].task_cb == NULL)
		{
			//scan HW protection status, UVLO, OTP
			continue;
		}

		event = osal_tasks_tbl[id].event;
		osal_tasks_tbl[id].event = 0;

		while (event != 0)
		{
            if (event & 0xFF)
            {
            	evt_msk = (1 << ( 0 + bit_map[((event & 0x000000FF) >>  0)]));
            }
            else if (event & 0x0000FF00)
            {
            	evt_msk = (1 << ( 8 + bit_map[((event & 0x0000FF00) >>  8)]));
            }
            else if (event & 0x00FF0000)
            {
            	evt_msk = (1 << (16 + bit_map[((event & 0x00FF0000) >> 16)]));
            }
            else
            {
            	evt_msk = (1 << (24 + bit_map[((event & 0xFF000000) >> 24)]));
            }

			osal_tasks_tbl[id].task_cb(evt_msk);

			event &= ~evt_msk;
		}
	}
}

void osal_start_timerEx(uint8_t timer_id, uint16_t timeout, uint16_t period, uint8_t task_id, uint32_t event)
{
	if (timer_id >= MAX_TIMER)
	{
		printk("\r\n invalid timer start");
		return;
	}

	osal_timer_tbl[timer_id].state = OSAL_TIMER_STS_RUNNING;
	osal_timer_tbl[timer_id].remain = timeout + (uint16_t)(sys_ticks - old_ticks);
	osal_timer_tbl[timer_id].period = period;
	osal_timer_tbl[timer_id].task_id = task_id;
	osal_timer_tbl[timer_id].event = event;
}

void osal_stop_timerEx(uint8_t timer_id)
{
	if (timer_id >= MAX_TIMER)
	{
		printk("\r\n invalid timer stop");
		return;
	}
	osal_timer_tbl[timer_id].event = 0;
	osal_timer_tbl[timer_id].state = OSAL_TIMER_STS_STOPPED;
}

static uint16_t osal_tick_cnt_get(void)
{
	uint16_t interval;

	interval = sys_ticks - old_ticks;
	if (interval > 0)
	{
		old_ticks = sys_ticks;
	}

	return interval;
}

static void osal_timer_update(void)
{
	uint16_t i, elapsed_ticks;

	elapsed_ticks = osal_tick_cnt_get();

	if (elapsed_ticks == 0)
	{
		return;
	}

//	extern void tcpm_state_machine(void);
//	tcpm_state_machine();
//	USBPD_vStateMachine();

//	#include "g_data.h"
//	extern uint8_t tmr2_250ms_int_flag;
//	if (tmr2_250ms_int_flag % 2 == 0)
//	{
//		if (gd->vbus > 10000)
//		{
//			if (usb_pd_9v_flag)
//			{
//				USBPD_vSetVolt(9000);
//			}
//		}
//		else
//		{
//			if (usb_pd_15v_flag)
//			{
//				USBPD_vSetVolt(15000);
//			}
//		}
//	}

	for (i=0; i<MAX_TIMER; i++)
	{
		if (osal_timer_tbl[i].state == OSAL_TIMER_STS_RUNNING)
		{
			if (osal_timer_tbl[i].remain > elapsed_ticks)
			{
				osal_timer_tbl[i].remain -= elapsed_ticks;
			}
			else
			{
				osal_set_event(osal_timer_tbl[i].task_id, osal_timer_tbl[i].event);

				if (osal_timer_tbl[i].period != 0)
				{
					osal_timer_tbl[i].remain = osal_timer_tbl[i].period;
				}
				else
				{
					osal_stop_timerEx(i);
				}
			}
		}
	}
}

void osal_start_system(void)
{
	for (;;)
	{
		hal_wdt_feed();
		osal_timer_update();
		osal_event_handle();
	}
}

uint8_t osal_msg_send(uint8_t task_id, uint8_t *msg_ptr)
{
	return 0;
}

void osal_msg_receive(void)
{

}

void osal_mem_copy(void *dst, const void *src, int len)
{
	__IO uint8_t *tmp_dst = (__IO uint8_t *)dst;
	__IO uint8_t *tmp_src = (__IO uint8_t *)src;

	if (dst == NULL || src == NULL || len == 0)
	{
		return;
	}

	if (tmp_dst <= tmp_src || tmp_dst >= tmp_src + len)
	{
		while (len--)
		{
			*tmp_dst++ = *tmp_src++;
		}
	}
	else
	{
		tmp_dst = tmp_dst + len - 1;
		tmp_src = tmp_src + len - 1;
		while (len--)
		{
			*tmp_dst-- = *tmp_src--;
		}
	}

	return;
}

void osal_mem_set(void *mem, uint8_t val, int len)
{
	__IO uint8_t *pmem = (__IO uint8_t *)mem;

	while (len--)
		*pmem++ = val;
}

void osal_mem_clear(void *mem, int len)
{
	__IO uint8_t *pmem = (__IO uint8_t *)mem;

	while (len--)
		*pmem++ = 0;
}

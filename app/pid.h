#ifndef PID_H_
#define PID_H_

void pid_init(void);
void PID_vDDMEventHandler(void);
void PID_vCtrlAccuracyCheck(int8_t);
void pid_cep_handler(int8_t);

void pid_set_volt_limit(uint16_t, uint16_t, uint16_t);
void pid_set_freq_limit(uint16_t, uint16_t, uint16_t);
void pid_set_duty_limit(uint16_t, uint16_t, uint16_t);
void pid_set_phas_limit(uint16_t, uint16_t, uint16_t);

void ctx_switch(uint8_t ctx_ind);

#define PID_VOLT_LIM_H    (gd->pid_limit.volt_lim_hi)
#define PID_VOLT_LIM_M    (gd->pid_limit.volt_lim_mi)
#define PID_VOLT_LIM_L    (gd->pid_limit.volt_lim_lo)
#define PID_PERD_LIM_H    (gd->pid_limit.perd_lim_hi)
#define PID_PERD_LIM_M    (gd->pid_limit.perd_lim_mi)
#define PID_PERD_LIM_L    (gd->pid_limit.perd_lim_lo)
#define PID_DUTY_LIM_H    (gd->pid_limit.duty_lim_hi)
#define PID_DUTY_LIM_M    (gd->pid_limit.duty_lim_mi)
#define PID_DUTY_LIM_L    (gd->pid_limit.duty_lim_lo)
#define PID_PHAS_LIM_H    (gd->pid_limit.phas_lim_hi)
#define PID_PHAS_LIM_M    (gd->pid_limit.phas_lim_mi)
#define PID_PHAS_LIM_L    (gd->pid_limit.phas_lim_lo)

#endif /* PID_H_ */

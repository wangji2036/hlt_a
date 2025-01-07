#ifndef QDT_H_
#define QDT_H_

/**
 * q_fact: quality factor
 * f_self: self-resonant frequency
 */
void fml_qdt_detect(uint32_t *q_fact, uint32_t *f_self);

enum qdt_state_t
{
	QDT_STA_INITIALIZE = 0,
	QDT_STA_PRECHARGED = 1,
	QDT_STA_DISCHARGED = 2,
};

void fml_qdt_detect_1(enum qdt_state_t state);

#endif /* QDT_H_ */

#ifndef QFOD_H_
#define QFOD_H_

void qfod_qdt_cali_init(void);
void qfod_qdt_cali_process(void);

uint8_t qfod_nego(uint8_t ref_q, uint8_t ref_f);

#endif /* QFOD_H_ */

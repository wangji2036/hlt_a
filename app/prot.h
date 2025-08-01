#ifndef PROT_H_
#define PROT_H_

int16_t fml_ntc_temp_get_typec(void);
int16_t fml_ntc_temp_get_wpc(void);
int16_t fml_die_temp_get(void);
/*--------------------------------------------------------------------------*/
void fml_tntc_otp_init(void);
void fml_tntc_otp_check(int16_t);
void fml_tntc_otp_limit_power(int16_t tntc);
/*--------------------------------------------------------------------------*/
void fml_tntc_utp_init(void);
void fml_tntc_utp_check(int16_t);
/*--------------------------------------------------------------------------*/
void fml_tdie_otp_init(void);
void fml_tdie_otp_check(int16_t);
/*--------------------------------------------------------------------------*/
void fml_tdie_utp_init(void);
void fml_tdie_utp_check(int16_t);
/*--------------------------------------------------------------------------*/
void fml_isns_ocp_init(void);
void fml_isns_ocp_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_icap_ocp_init(void);
void fml_icap_ocp_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_icap_ucp_init(void);
void fml_icap_ucp_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_vbus_ovp_init(void);
void fml_vbus_ovp_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_vbus_uvp_init(void);
void fml_vbus_uvp_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_vbus_dpl_init(void);
void fml_vbus_dpl_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_vpwr_ovp_init(void);
void fml_vpwr_ovp_check(uint16_t);
/*--------------------------------------------------------------------------*/
void fml_pout_opp_init(void);
void fml_pout_opp_check(uint16_t, uint16_t);
/*--------------------------------------------------------------------------*/

#endif /* PROT_H_ */

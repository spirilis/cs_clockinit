/*
 * csinit.h
 *
 *  Created on: Jul 2, 2014
 *      Author: ebrundick
 */

#ifndef CSINIT_H_
#define CSINIT_H_

#include <stdint.h>

// MSP430FR58xx, 59xx, 68xx, 69xx Clock Initialization


/* Frequencies supported (based on MSP430FR58, 59, 68, 69xx User's Guide)
 * 1000000
 * 2670000
 * 3330000
 * 4000000
 * 5330000
 * 6670000
 * 8000000
 * 16000000
 * 21000000
 * 24000000
 *
 * use_xt1 = 0 uses VLOCLK for ACLK
 * use_xt1 = 1 uses LFXT1 for ACLK (oscillator)
 * use_xt2 = 2 uses LFXT1 (CMOS Input on LFXTIN) for ACLK
 */
void cs_clockinit(uint32_t freq, int use_xt1);
void cs_clockdiv(unsigned int mclk_div, unsigned int smclk_div, unsigned int aclk_div);


#endif /* CSINIT_H_ */

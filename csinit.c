/*
 * csinit.c
 *
 *  Created on: Jul 2, 2014
 *      Author: ebrundick
 */

#include <msp430.h>
#include <stdint.h>

static unsigned int _did_clockdiv = 0;

#if defined(__MSP430FR5XX_6XX_FAMILY__) && !defined(__MSP430FR2XX_4XX_FAMILY__)
void cs_clockinit(uint32_t freq, int use_xt1)
{
	unsigned int waits;
	uint32_t mclk_freq;

	SFRIE1 &= ~OFIE;

	CSCTL0_H = CSKEY_H;
	CSCTL1 = DCOFSEL_0;  // Set DCO to 1MHz for the meantime
	if (use_xt1)
		CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
	else
		CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;

	if (!_did_clockdiv) {
		CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;
		_did_clockdiv = 1;
	}

	// Compute FRAM wait states & set
	mclk_freq = freq / (1 << (CSCTL3 & 0x07));
	waits = (unsigned int)(mclk_freq / 8000000);
	if (waits && (mclk_freq % 8000000 == 0))
		waits--;  // Don't need an extra wait state *at* each 8MHz threshold
	FRCTL0 = FRCTLPW | (NWAITS0 * waits);

	// Spin up XT1 (or not)
	if (use_xt1) {
		#if defined(__MSP430FR5969) || defined(__MSP430FR5949) || \
		    defined(__MSP430FR5968) || defined(__MSP430FR5948) || \
		    defined(__MSP430FR5967) || defined(__MSP430FR5947)
		if (use_xt1 > 1) {  // LFXT CMOS-Signal-Input mode
			PJSEL1 = 0x00;
			PJSEL0 = BIT4;
			CSCTL4 &= ~LFXTDRIVE_3;
			CSCTL4 |= LFXTBYPASS | HFXTOFF;
		} else {  // Normal XTAL oscillator mode
			PJSEL1 = 0x00;
			PJSEL0 = BIT4 | BIT5;
			CSCTL4 &= ~LFXTBYPASS;
			CSCTL4 |= LFXTDRIVE_3 | HFXTOFF;
		}
		#endif
		CSCTL4 &= ~LFXTOFF;
		do {
			CSCTL5 &= ~(LFXTOFFG | HFXTOFFG);
			SFRIFG1 &= ~OFIFG;
		} while (SFRIFG1 & OFIFG);
	} else {
		CSCTL4 |= LFXTOFF;
	}

	// Set our DCO frequency
	switch (freq) {
		case 2670000:
			CSCTL1 = DCOFSEL_1;
			break;
		case 3330000:
			CSCTL1 = DCOFSEL_2;
			break;
		case 4000000:
			CSCTL1 = DCOFSEL_3;
			break;
		case 5330000:
			CSCTL1 = DCOFSEL_4;
			break;
		case 6670000:
			CSCTL1 = DCOFSEL_5;
			break;
		case 8000000:
			CSCTL1 = DCOFSEL_6;
			break;
		case 16000000:
			CSCTL1 = DCORSEL | DCOFSEL_4;
			break;
		case 21000000:
			CSCTL1 = DCORSEL | DCOFSEL_5;
			break;
		case 24000000:
			CSCTL1 = DCORSEL | DCOFSEL_6;
			break;
		default:
			CSCTL1 = DCOFSEL_0;  // 1MHz
	}
	CSCTL0_H = 0;
}

void cs_clockdiv(unsigned int mclk_div, unsigned int smclk_div, unsigned int aclk_div)
{
	CSCTL0_H = CSKEY_H;
	CSCTL3 = (conv_div(aclk_div) << 8) |
			 (conv_div(smclk_div) << 4) |
			 conv_div(mclk_div);
	CSCTL0_H = 0;
	_did_clockdiv = 1;
}
#endif

static unsigned int conv_div(unsigned int div)
{
	switch (div) {
		case 0:
			return 0;
		case 1:
			return 0;
		case 2:
			return 1;
		case 4:
			return 2;
		case 8:
			return 3;
		case 16:
			return 4;
		case 32:
			return 5;
	}
	return 0;
}

static unsigned int conv_div_16(unsigned int div)
{
	switch (div) {
		case 0:
		case 1:
			return 0;
		case 16:
			return 1;
		case 32:
			return 2;
		case 128:
			return 3;
		case 256:
			return 4;
		case 384:
			return 5;
		case 512:
			return 6;
		case 768:
			return 7;
		case 1024:
			return 8;
	}
	return 0;
}

static unsigned int conv_div_rev(unsigned int div)
{
	switch (div) {
		case 0:
			return 1;
		case 1:
			return 2;
		case 2:
			return 4;
		case 3:
			return 8;
		case 4:
			return 16;
		case 5:
			return 32;
		case 6:
			return 64;
		case 7:
			return 128;
	}
	return 0;
}

#if defined(__MSP430FR2XX_4XX_FAMILY__)
void cs_clockinit(uint32_t freq, int use_xt1)
{
	unsigned int waits;
	uint32_t mclk_freq;

	SFRIE1 &= ~OFIE;

	if (use_xt1)
		CSCTL3 = SELREF__XT1CLK;
		CSCTL4 = SELA__XT1CLK | SELMS__DCOCLK;
	else
		CSCTL3 = SELREF__REFOCLK;
		CSCTL4 = SELA__REFOCLK | SELMS__DCOCLK;

	if (!_did_clockdiv) {
		CSCTL5 = DIVM__1 | DIVS__1;
		CSCTL6 = (CSCTL6 & 0xF0FF);  // DIVA__1
		_did_clockdiv = 1;
	}

	// Compute FRAM wait states & set
	mclk_freq = freq / conv_div_rev(CSCTL5 & 0x07);
	waits = (unsigned int)(mclk_freq / 8000000);
	if (waits && (mclk_freq % 8000000 == 0))
		waits--;  // Don't need an extra wait state *at* each 8MHz threshold
	FRCTL0 = FRCTLPW | (NWAITS0 * waits);

	// Spin up XT1 (or not)
	if (use_xt1) {
		if (use_xt1 > 1) {  // LFXT CMOS-Signal-Input mode
			P4SEL0 = (P4SEL0 & ~(BIT1 | BIT2)) | BIT1;  // XIN only
			CSCTL6 &= ~(XT1DRIVE_3 | XTS);
			CSCTL6 |= XT1BYPASS;
		} else {  // Normal XTAL oscillator mode
			P4SEL0 |= BIT1 | BIT2;  // XIN and XOUT
			CSCTL6 &= ~(XTS | XT1BYPASS);
			CSCTL6 |= XT1DRIVE_3;
		}
		do {
			CSCTL7 &= ~XT1OFFG;
			SFRIFG1 &= ~OFIFG;
		} while (SFRIFG1 & OFIFG);
	}

	// Set our DCO frequency
	switch (freq) {
		case 1000000:
			CSCTL1 = DCORSEL_0;
			CSCTL2 = FLLD__1 | 30;  // ~1015808Hz
			break;
		case 2000000:
			CSCTL1 = DCORSEL_1;
			CSCTL2 = FLLD__1 | 60;  // ~1998848Hz
			break;
		case 4000000:
			CSCTL1 = DCORSEL_2;
			CSCTL2 = FLLD__1 | 121; // ~3997696Hz
			break;
		case 8000000:
			CSCTL1 = DCORSEL_3;
			CSCTL2 = FLLD__1 | 243; // ~7995392Hz
			break;
		case 12000000:
			CSCTL1 = DCORSEL_4;
			CSCTL2 = FLLD__1 | 365; // ~11993088Hz
			break;
		case 16000000:
			CSCTL1 = DCORSEL_5;
			CSCTL2 = FLLD__1 | 487; // ~15990784Hz
			break;
	}
}

void fr24_cs_clockdiv(unsigned int mclk_div, unsigned int smclk_div, unsigned int aclk_div)
{
	CSCTL5 = (CSCTL5 & 0xFF00) | conv_div(mclk_div) | (conv_div(smclk_div / mclk_div) << 4);
	CSCTL6 = (CSCTL6 & 0x00FF) | (conv_div_16(aclk_div) << 8);
	_did_clockdiv = 1;
}
#endif

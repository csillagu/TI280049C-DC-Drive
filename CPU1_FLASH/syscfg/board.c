/*
 * Copyright (c) 2020 Texas Instruments Incorporated - http://www.ti.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "board.h"

void Board_init()
{
	EALLOW;

	PinMux_init();
	EPWM_init();

	EDIS;
}

void PinMux_init()
{
	//
	// EPWM1 -> myEPWM1 Pinmux
	//
	GPIO_setPinConfig(GPIO_0_EPWM1_A);
	GPIO_setPinConfig(GPIO_1_EPWM1_B);
	//
	// EPWM2 -> myEPWM2 Pinmux
	//
	GPIO_setPinConfig(GPIO_2_EPWM2_A);
	GPIO_setPinConfig(GPIO_3_EPWM2_B);
	//
	// EPWM3 -> myEPWM3 Pinmux
	//
	GPIO_setPinConfig(GPIO_4_EPWM3_A);
	GPIO_setPinConfig(GPIO_5_EPWM3_B);
	//
	// EPWM4 -> myEPWM4 Pinmux
	//
	GPIO_setPinConfig(GPIO_6_EPWM4_A);
	GPIO_setPinConfig(GPIO_7_EPWM4_B);
	//
	// EPWM5 -> myEPWM5 Pinmux
	//
	GPIO_setPinConfig(GPIO_8_EPWM5_A);
	GPIO_setPinConfig(GPIO_9_EPWM5_B);
	//
	// EPWM6 -> myEPWM6 Pinmux
	//
	GPIO_setPinConfig(GPIO_10_EPWM6_A);
	GPIO_setPinConfig(GPIO_11_EPWM6_B);

}

void EPWM_init(){
	//myEPWM1 initialization
	//myEPWM2 initialization
	//myEPWM3 initialization
	//myEPWM4 initialization
	//myEPWM5 initialization
	//myEPWM6 initialization
}

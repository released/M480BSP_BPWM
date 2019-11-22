/*************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * @brief    A project template for M480 MCU.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "NuMicro.h"


void TMR3_IRQHandler(void)
{
	static uint16_t CNT = 0;	
	static uint32_t log = 0;	
	static uint8_t flag = 1;
	static uint32_t freq = 1000;

	
    if(TIMER_GetIntFlag(TIMER3) == 1)
    {
        TIMER_ClearIntFlag(TIMER3);
	
		if (CNT++ > 10)
		{		
			CNT = 0;
			PH0 ^= 1;
			printf("%s : %2d\r\n" , __FUNCTION__ , log++);

			BPWM_ConfigOutputChannel(BPWM0, 0, freq+50, 70);

			// change freq.
			if (flag)
			{
				freq += 100;
				if (freq >= 50000)
				{
					flag = 0;
				}
			}
			else
			{
				freq -= 100;
				if (freq <= 1000)
				{
					flag = 1;
				}		
			}
		}		
    }
}


void BPWM_HW_Init(void)
{
    /* Enable IP module clock */
    CLK_EnableModuleClock(BPWM0_MODULE);

    /* BPWM clock frequency is set double to PCLK: select BPWM module clock source as PLL */
    CLK_SetModuleClock(BPWM0_MODULE, CLK_CLKSEL2_BPWM0SEL_PCLK0, 0);

    /* Set PA.0~5 multi-function pin for BPWM0 channel 0~5 */
    SYS->GPA_MFPL = (SYS->GPA_MFPL & ~SYS_GPA_MFPL_PA0MFP_Msk) | SYS_GPA_MFPL_PA0MFP_BPWM0_CH0;
}

void BPWM_Init(void)	//PA0
{
    BPWM_ConfigOutputChannel(BPWM0, 0, 1200, 50);

    /* Enable output of BPWM0 channel 0~5 */
    BPWM_EnableOutput(BPWM0, BPWM_CH_0_MASK);

//    /* Start BPWM0 counter */
    BPWM_Start(BPWM0, BPWM_CH_0_MASK);
}


void TIMER3_HW_Init(void)
{
    CLK_EnableModuleClock(TMR3_MODULE);
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_HIRC, 0);
}

void TIMER3_Init(void)
{
	GPIO_SetMode(PH, BIT0, GPIO_MODE_OUTPUT);	//monitor
	GPIO_SetMode(PH, BIT1, GPIO_MODE_OUTPUT);	//monitor

	#if 0
    TIMER_Open(TIMER3, TIMER_PERIODIC_MODE, 1000);
	#else
    TIMER_Open(TIMER3, TIMER_PERIODIC_MODE, 2000000);
    TIMER_SET_PRESCALE_VALUE(TIMER3, 5);        //--- 24bit UpCounter Clock = 12M/(5+1) ---//
    TIMER_SET_CMP_VALUE(TIMER3, 2000);          //--- 1ms Trigger ---//	
	#endif
	
    TIMER_EnableInt(TIMER3);
    NVIC_EnableIRQ(TMR3_IRQn);	
    TIMER_Start(TIMER3);
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Set XT1_OUT(PF.2) and XT1_IN(PF.3) to input mode */
    PF->MODE &= ~(GPIO_MODE_MODE2_Msk | GPIO_MODE_MODE3_Msk);

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Set core clock as PLL_CLOCK from PLL */
    CLK_SetCoreClock(FREQ_192MHZ);
    /* Set PCLK0/PCLK1 to HCLK/2 */
    CLK->PCLKDIV = (CLK_PCLKDIV_APB0DIV_DIV2 | CLK_PCLKDIV_APB1DIV_DIV2);

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART clock source from HXT */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HXT, CLK_CLKDIV0_UART0(1));

	TIMER3_HW_Init();

	BPWM_HW_Init();

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /* Set GPB multi-function pins for UART0 RXD and TXD */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk);
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);
	
    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M480 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
	
    SYS_Init();
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());
	
    BPWM_Init();

	TIMER3_Init();

    /* Got no where to go, just loop forever */
    while(1)
    {


    }

}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/

#include "device_driver.h"

void Key_Poll_Init(void)
{
	Macro_Set_Bit(RCC->AHB1ENR, 2); 
	Macro_Write_Block(GPIOC->MODER, 0x3, 0x0, 26);
}

int Key_Get_Pressed(void)
{
	return Macro_Check_Bit_Clear(GPIOC->IDR, 13);	
}

void Key_Wait_Key_Pressed(void)
{
	while(!Macro_Check_Bit_Clear(GPIOC->IDR, 13));
}

void Key_Wait_Key_Released(void)
{
	while(!Macro_Check_Bit_Set(GPIOC->IDR, 13));
}
int Key_PA0_Get_Pressed(void)
{
    return Macro_Check_Bit_Clear(GPIOA->IDR, 0);
}
void Key_PA0_Init(void)
{
    // GPIOA Clock Enable
    Macro_Set_Bit(RCC->AHB1ENR, 0);

    // PA0 = Input mode
    Macro_Write_Block(GPIOA->MODER, 0x3, 0x0, 0);

    // PA0 = Pull-Up
    Macro_Write_Block(GPIOA->PUPDR, 0x3, 0x1, 0);
}
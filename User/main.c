#include "stm32f10x.h" // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "math.h"
#include <stdlib.h>
#include <string.h>
uint8_t KeyNum;

float Target, Actual, Out;
float Kp, Ki, Kd;
float Error0, Error1, Error2,ErrorInt;
uint8_t loc=0;


int main(void)
{
	OLED_Init();
	Key_Init();
	Motor_Init();
	Encoder_Init();
	Serial_Init();
	
	Timer_Init();
	
	
	OLED_Update();
	
	while (1)
	{
		KeyNum = Key_GetNum();
		if (KeyNum == 1){
			loc=!loc;
			Target=0, Actual=0, Out=0;
		}
		if(loc==0){
			OLED_Printf(0, 0, OLED_8X16, "Speed Control      ");
			int speed;
			Kp=0.8, Ki=0.01, Kd=0;
			if(Serial_RxFlag==1){
				if (sscanf(Serial_RxPacket, "speed%%%d", &speed)==1) {
					Target = speed ;
				}
				Serial_RxFlag=0;
			}
		}
		else if(loc==1){
			Kp=0.6, Ki=0.01, Kd=0;
			
			OLED_Printf(0, 0, OLED_8X16, "Location Control   ");

		}
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Kp);
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Ki);
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Kd);
		
		OLED_Printf(64, 16, OLED_8X16, "Tar:%+04.0f", Target);
		OLED_Printf(64, 32, OLED_8X16, "Act:%+04.0f", Actual);
		OLED_Printf(64, 48, OLED_8X16, "Out:%+04.0f", Out);
		
		OLED_Update();
		
		Serial_Printf("%f,%f,%f\r\n", Target, Actual, Out);
	}
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count ++;
		if(loc==0){
			if (Count >= 20)
			{
				Count = 0;
				
				Actual = Encoder1_Get();
				
				Error2 = Error1;
				Error1 = Error0;
				Error0 = Target - Actual;
				
				Out += Kp * (Error0 - Error1) + Ki * Error0
						+ Kd * (Error0 - 2 * Error1 + Error2);
				
				if (Out > 100) {Out = 100;}
				if (Out < -100) {Out = -100;}
				
				Motor1_SetPWM(Out);


			}
		}
		else if(loc==1){
			if (Count >= 20)
			{
				Count = 0;
				
				Target += Encoder1_Get();
				Actual += Encoder2_Get();
				
				Error1 = Error0;
				Error0 = Target - Actual;
				
				float C = 1 / (0.2 * fabs(Error0) + 1);
				
				ErrorInt += C * Error0;
				
				Out = Kp * Error0 + Ki * ErrorInt + Kd * (Error0 - Error1);
				
				if (Out > 100) {Out = 100;}
				if (Out < -100) {Out = -100;}
				
				Motor2_SetPWM(Out);
			}
		
		}
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}

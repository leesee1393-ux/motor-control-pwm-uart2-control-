#include "device_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void Sys_Init(int baud) 
{
	SCB->CPACR |= (0x3 << 10*2)|(0x3 << 11*2); 
	Clock_Init();
	Uart2_Init(baud);
	setvbuf(stdout, NULL, _IONBF, 0);
	LED_Init();
}

#define BASE  (500) //msec

#if 0
static void Buzzer_Beep(unsigned char tone, int duration)
{
	const static unsigned short tone_value[] = {261,277,293,311,329,349,369,391,415,440,466,493,523,554,587,622,659,698,739,783,830,880,932,987};

	TIM3_Out_Freq_Generation(tone_value[tone]);
	TIM2_Delay(duration);
	TIM3_Out_Stop();
}

void Main(void)
{
	Sys_Init(115200);
	printf("Buzzer Test!!\n");

	int i;
	enum key{C1, C1_, D1, D1_, E1, F1, F1_, G1, G1_, A1, A1_, B1, C2, C2_, D2, D2_, E2, F2, F2_, G2, G2_, A2, A2_, B2};
	enum note{N16=BASE/4, N8=BASE/2, N4=BASE, N2=BASE*2, N1=BASE*4};
	const int song1[][2] = {{G1,N4},{G1,N4},{E1,N8},{F1,N8},{G1,N4},{A1,N4},{A1,N4},{G1,N2},{G1,N4},{C2,N4},{E2,N4},{D2,N8},{C2,N8},{D2,N2}};
	const char * note_name[] = {"C1", "C1#", "D1", "D1#", "E1", "F1", "F1#", "G1", "G1#", "A1", "A1#", "B1", "C2", "C2#", "D2", "D2#", "E2", "F2", "F2#", "G2", "G2#", "A2", "A2#", "B2"};

	TIM3_Out_Init();

	printf("%s ", note_name[C1]);
	Buzzer_Beep(C1,N4);
	printf("%s ", note_name[D1]);
	Buzzer_Beep(D1,N4);
	printf("%s ", note_name[E1]);
	Buzzer_Beep(E1,N4);
	printf("%s ", note_name[F1]);
	Buzzer_Beep(F1,N4);
	printf("%s ", note_name[G1]);
	Buzzer_Beep(G1,N4);
	printf("%s ", note_name[A1]);
	Buzzer_Beep(A1,N4);
	printf("%s ", note_name[B1]);
	Buzzer_Beep(B1,N4);
	printf("%s ", note_name[C2]);
	Buzzer_Beep(C2,N4);

	printf("\nSong Play\n");

	for(i=0; i<(sizeof(song1)/sizeof(song1[0])); i++)
	{
		printf("%s ", note_name[song1[i][0]]);
		Buzzer_Beep(song1[i][0], song1[i][1]);
	}
}

#endif 

#if 0
void Main(void)
{
    Sys_Init(115200);

    char ch;

    printf("UART2 RX Test\n");

    for(;;)
    {
        ch = Uart2_Get_Pressed();

        if(ch != 0)
        {
            printf("RX = %c, ASCII = %d\n", ch, ch);
        }
    }
}
#endif 
#if 1
#include "device_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


// ============================================================
// 전역 변수
// ============================================================

static int motor_enable = 0;

// 0 = Forward
// 1 = Reverse
static int motor_direction = 0;

// 기본 PWM Duty
static int pwm_duty = 50;


// ============================================================
// Motor GPIO Init
//
// PA1 -> L298N IN1
// PA4 -> L298N IN2
// PB0 -> L298N ENA (TIM3_CH3 PWM)
// ============================================================

void Motor_Init(void)
{
    // GPIOA Clock Enable
    Macro_Set_Bit(RCC->AHB1ENR, 0);

    // ----------------------------------------
    // PA1 = General Purpose Output
    //
    // MODER1 = bit 3:2
    // 01 = Output
    // ----------------------------------------

    Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 2);


    // ----------------------------------------
    // PA4 = General Purpose Output
    //
    // MODER4 = bit 9:8
    // 01 = Output
    // ----------------------------------------

    Macro_Write_Block(GPIOA->MODER, 0x3, 0x1, 8);


    // Push Pull
    Macro_Clear_Bit(GPIOA->OTYPER, 1);
    Macro_Clear_Bit(GPIOA->OTYPER, 4);


    // 초기 모터 정지

    Macro_Clear_Bit(GPIOA->ODR, 1);    // IN1 = 0
    Macro_Clear_Bit(GPIOA->ODR, 4);    // IN2 = 0
}


// ============================================================
// Motor Forward
//
// IN1 = 1
// IN2 = 0
// ============================================================

void Motor_Forward(void)
{
    Macro_Set_Bit(GPIOA->ODR, 1);
    Macro_Clear_Bit(GPIOA->ODR, 4);
}


// ============================================================
// Motor Reverse
//
// IN1 = 0
// IN2 = 1
// ============================================================

void Motor_Reverse(void)
{
    Macro_Clear_Bit(GPIOA->ODR, 1);
    Macro_Set_Bit(GPIOA->ODR, 4);
}


// ============================================================
// Motor Stop
//
// IN1 = 0
// IN2 = 0
// ============================================================

void Motor_Stop(void)
{
    Macro_Clear_Bit(GPIOA->ODR, 1);
    Macro_Clear_Bit(GPIOA->ODR, 4);
}


// ============================================================
// Motor PWM Handler
//
// motor_enable = 1
//      -> 현재 pwm_duty 출력
//
// motor_enable = 0
//      -> Duty 0%
// ============================================================

void Motor_PWM_Handler(void)
{
    if(motor_enable == 1)
    {
        TIM3_Out_Duty_Set(pwm_duty);
    }
    else
    {
        TIM3_Out_Duty_Set(0);
    }
}


// ============================================================
// Motor Power Handler
// ============================================================

void Motor_Power_Handler(int enable)
{
    // ========================================================
    // Motor ON
    // ========================================================

    if(enable == 1)
    {
        motor_enable = 1;

        // LED ON
        LED_On();


        // 현재 방향에 맞게 설정

        if(motor_direction == 0)
        {
            Motor_Forward();
        }
        else
        {
            Motor_Reverse();
        }


        // PWM 출력
        Motor_PWM_Handler();
    }


    // ========================================================
    // Motor OFF
    // ========================================================

    else
    {
        motor_enable = 0;

        // PWM 먼저 OFF
        TIM3_Out_Duty_Set(0);

        // Motor Stop
        Motor_Stop();

        // LED OFF
        LED_Off();
    }
}


// ============================================================
// Motor Direction Handler
// ============================================================

void Motor_Direction_Handler(void)
{
    // --------------------------------------------------------
    // 모터가 돌고 있는 상태이면
    // 바로 역회전하지 않고
    //
    // PWM OFF
    // ↓
    // 100ms
    // ↓
    // 방향 변경
    // ↓
    // PWM 복원
    // --------------------------------------------------------

    if(motor_enable == 1)
    {
        TIM3_Out_Duty_Set(0);

        TIM2_Delay(100);


        // Forward

        if(motor_direction == 0)
        {
            Motor_Forward();
        }


        // Reverse

        else
        {
            Motor_Reverse();
        }


        // 기존 Duty 복원

        TIM3_Out_Duty_Set(pwm_duty);
    }
}


// ============================================================
// Qt로 현재 상태 전송
// ============================================================

void Send_Status(void)
{
    // Motor 상태

    if(motor_enable == 1)
    {
        printf("MOTOR:ON\n");
    }
    else
    {
        printf("MOTOR:OFF\n");
    }


    // 방향 상태

    if(motor_direction == 0)
    {
        printf("DIR:FORWARD\n");
    }
    else
    {
        printf("DIR:REVERSE\n");
    }


    // 현재 Duty

    printf("SPEED:%d\n", pwm_duty);
}


// ============================================================
// PA0 Button Handler
//
// 모터 OFF 상태
// 버튼 누름
//      ↓
// LED ON
// Forward
//
// 모터 ON 상태
// 버튼 누름
//      ↓
// Forward ↔ Reverse
//
// 그리고 상태를 UART로 Qt에 전송
// ============================================================

void Button_Handler(void)
{
    static int prev = 0;

    int curr;


    curr = Key_PA0_Get_Pressed();


    // ========================================================
    // Rising Edge
    //
    // 0 -> 1
    // ========================================================

    if((prev == 0) && (curr == 1))
    {
        // 채터링 방지
        TIM2_Delay(20);


        // 20ms 후에도 눌려있는지 확인

        if(Key_PA0_Get_Pressed())
        {
            // =================================================
            // 모터 OFF
            // =================================================

            if(motor_enable == 0)
            {
                // 최초 방향은 Forward

                motor_direction = 0;


                // Motor ON

                Motor_Power_Handler(1);


                printf("BUTTON:PRESS\n");

                Send_Status();
            }


            // =================================================
            // 모터가 이미 ON
            // =================================================

            else
            {
                // Forward <-> Reverse

                motor_direction ^= 1;


                // 실제 방향 변경

                Motor_Direction_Handler();


                // Qt에 알려줌

                printf("BUTTON:PRESS\n");

                Send_Status();
            }
        }
    }


    // 현재 상태 저장

    prev = curr;
}


// ============================================================
// 숫자로만 되어 있는지 검사
//
// 예:
// "50"  -> 1
// "100" -> 1
// "abc" -> 0
// ============================================================

int Is_Number_String(char *str)
{
    int i = 0;


    if(str[0] == '\0')
    {
        return 0;
    }


    while(str[i] != '\0')
    {
        if((str[i] < '0') || (str[i] > '9'))
        {
            return 0;
        }

        i++;
    }


    return 1;
}


// ============================================================
// Duty 변경 함수
// ============================================================

void Motor_Duty_Command(int duty)
{
    // 0 ~ 100 검사

    if((duty >= 0) && (duty <= 100))
    {
        pwm_duty = duty;


        // 모터 동작 중이면 즉시 변경

        if(motor_enable == 1)
        {
            TIM3_Out_Duty_Set(pwm_duty);
        }


        printf("SPEED:%d\n", pwm_duty);
    }

    else
    {
        printf("ERROR:SPEED\n");
    }
}


// ============================================================
// UART Command 처리
//
// 기존 숫자 입력도 유지
//
// 70
//
// 그리고 Qt 명령 추가
//
// ON
// OFF
// SPEED:70
// FORWARD
// REVERSE
// STATUS
// ============================================================

void UART_Command_Handler(char *msg)
{
    int duty;


    // ========================================================
    // ON
    // ========================================================

    if(strcmp(msg, "ON") == 0)
    {
        Motor_Power_Handler(1);

        Send_Status();

        return;
    }


    // ========================================================
    // OFF
    // ========================================================

    if(strcmp(msg, "OFF") == 0)
    {
        Motor_Power_Handler(0);

        Send_Status();

        return;
    }


    // ========================================================
    // FORWARD
    // ========================================================

    if(strcmp(msg, "FORWARD") == 0)
    {
        motor_direction = 0;

        Motor_Direction_Handler();

        Send_Status();

        return;
    }


    // ========================================================
    // REVERSE
    // ========================================================

    if(strcmp(msg, "REVERSE") == 0)
    {
        motor_direction = 1;

        Motor_Direction_Handler();

        Send_Status();

        return;
    }


    // ========================================================
    // STATUS
    // ========================================================

    if(strcmp(msg, "STATUS") == 0)
    {
        Send_Status();

        return;
    }


    // ========================================================
    // SPEED:70
    // ========================================================

    if(strncmp(msg, "SPEED:", 6) == 0)
    {
        duty = atoi(&msg[6]);

        Motor_Duty_Command(duty);

        return;
    }


    // ========================================================
    // 기존 방식도 그대로 지원
    //
    // TeraTerm
    //
    // 70 Enter
    // ========================================================

    if(Is_Number_String(msg))
    {
        duty = atoi(msg);

        Motor_Duty_Command(duty);

        return;
    }


    // ========================================================
    // 잘못된 명령
    // ========================================================

    printf("ERROR:COMMAND\n");
}


// ============================================================
// UART Handler
// ============================================================

void UART_Handler(void)
{
    static char msg[20];

    static int index = 0;

    char ch;


    ch = Uart2_Get_Pressed();


    // 수신 데이터 없음

    if(ch == 0)
    {
        return;
    }


    // ========================================================
    // Enter
    // ========================================================

    if((ch == '\r') || (ch == '\n'))
    {
        // 문자열 끝

        msg[index] = '\0';


        // 빈 문자열 제외

        if(index > 0)
        {
            UART_Command_Handler(msg);
        }


        // 다음 명령 받을 준비

        index = 0;
    }


    // ========================================================
    // 일반 문자
    // ========================================================

    else
    {
        if(index < 19)
        {
            msg[index] = ch;

            index++;
        }
    }
}


// ============================================================
// Main
// ============================================================

void Main(void)
{
    // ========================================================
    // System Init
    //
    // 기존 Sys_Init 그대로 사용
    // ========================================================

    Sys_Init(115200);


    // ========================================================
    // PA0 Button
    // ========================================================

    Key_PA0_Init();


    // ========================================================
    // Motor GPIO
    //
    // PA1 -> IN1
    // PA4 -> IN2
    // ========================================================

    Motor_Init();


    // ========================================================
    // PWM
    //
    // PB0 -> TIM3_CH3 -> L298N ENA
    // ========================================================

    TIM3_Out_Init();


    // PWM Frequency = 1kHz

    TIM3_Out_Freq_Generation(1000);


    // ========================================================
    // 초기 상태
    // ========================================================

    TIM3_Out_Duty_Set(0);

    Motor_Stop();

    LED_Off();


    // ========================================================
    // 시작 메시지
    // ========================================================

    printf("\n");

    printf("READY\n");

    Send_Status();


    // ========================================================
    // Main Loop
    // ========================================================

    for(;;)
    {
        // 실제 버튼 검사

        Button_Handler();


        // Qt / TeraTerm 명령 검사

        UART_Handler();
    }
}

#endif
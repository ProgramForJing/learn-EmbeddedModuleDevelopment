#ifndef KEY_H
#define KEY_H

/*================================ 头文件包含 =================================*/
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*================================== 宏定义 ===================================*/
#ifndef Key_InputMode_Enable
#define Key_InputMode_Enable	1
#endif

/*============================== 结构体联合体定义 =============================*/
typedef enum
{
	KEY_STATE_UP,	/**< 按键松开状态 */
	KEY_STATE_DOWN,	/**< 按键按下状态 */
	KEY_STATE_INVALID,	/**< 按键无效输入 */
	KEY_STATE_DISABLE	/**< 按键失能状态 */
}KEY_STATE;

typedef enum{
	PRESS_TYPE_SHORT = 0x01,	/**< 按键按下类型为短按 */
	PRESS_TYPE_LONG = 0x00,		/**< 按键按下类型为长按 */
	PRESS_TYPE_IDLE				/**< 按键无按下，空闲状态 */
}PRESS_TYPE;

typedef enum{
	INPUT_MODE_NORMAL,		/**< 普通输入模式 */
#if (Key_InputMode_Enable == 1)
	INPUT_MODE_COMMAND		/**< 命令输入模式，命令输入模式下根据短按和长按的组合来执行对应的命令 */
#endif
}INPUT_MODE;

#if (Key_InputMode_Enable == 1)
typedef enum{
	COMMAND_EXECUTION_STATE_RUNNING,	/**< 命令正在执行中 */
	COMMAND_EXECUTION_STATE_WAITING		/**< 等待命令输入中 */
}COMMAND_EXECUTION_STATE;
#endif

//命令队列的最大长度，例如commandMaxLength=3，只要按下按键然后松开，在松开后commandIntervalThreshold时间之内再次按下按键
//就视作输入命令，commandMaxLength=3，表示最多可将连续三次按下视作一个命令，例如短按短按可视作一个命令，短按短按长按可视作一个命令
//如果短按短按长按之后再次输入一个按下（短按或长按），那么这次按下将视为一次普通的短按或长按（因为命令队列已达到最大长度）
//如果短按短按之后commandIntervalThreshold之内没有检测到按下，则之后再次按下将视作一次普通的短按或长按

typedef struct
{
	char * keyName;		/**< 按键名称 */
	FunctionalState status;		/**< 按键使能状态，ENABLE使能，DISABLE失能 */
	KEY_STATE keyState;		/**< 按键状态 */
	PRESS_TYPE pressType;	/**< 按键按下类型 */
	
    /*!
	 *  短按判断阈值时间
	 *  设按键松开时刻为t，按键按下时刻为x
	 *  当 x < t <= x + shortPressThreshold 时，认为此次按键按下为短按
     */
	TickType_t shortPressThreshold;
	
#if (Key_InputMode_Enable == 1)
    /*!
	 *  命令输入判断阈值时间
	 *  设前一次按键按下松开时刻为x，当前按键按下开始时刻为t
	 *  当 x < t <= x + commandIntervalThreshold 时，认为此次输入需要记录到命令队列中
     */
	TickType_t commandIntervalThreshold;
	
	uint8_t commandMaxLength;		/**< 命令队列最大长度 */
	uint8_t commandCurrentLength;	/**< 命令队列现有长度 */
	
    /*!
	 *  命令值
     *  命令使用一个uint16_t类型的数来表示
	 *  用“输入”表示一次长按或短按
	 *  command = 0x80 即 command = 0b1000 0000， 此时表示命令队列中没有记录任何输入
	 *  当每次有输入时，对command左移一位，然后进行或运算
	 *  短按为 0x01，长按为 0x00
	 *  例如第一次输入为短按
	 *  command = 0b1000 0000 -> 左移一位，command = 0b0000 0000 -> 
	 *  或运算 command |= 0x01 -> command = 0b000 0001 ，同时 commandCurrentLength++
	 *  第二次输入为长按，command左移一位与0x00或运算， command = 0b0000 0010，同时commandCurrentLength++
	 *  第三次输入为短按，command左移一位与0x01或运算， command = 0b0000 0101，同时commandCurrentLength++
	 *  这样就可根据 command 的值和 commandCurrentLength 来判断输入了什么命令
     */
	uint8_t command;
#endif

	uint16_t pressCount;		/**< 统计按键按下次数，可以因某些原因清零 */
	uint16_t pressTotalCount;	/**< 统计按键按下总次数 */
	
	TickType_t pressEntryPoint;		/**< 按键按下开始时刻 */
	TickType_t pressExitPoint;		/**< 按键按下结束时刻 */
	TickType_t lastPressExitPoint;	/**< 上一次按键按下结束时刻 */

    /*!
	 *  长按异常判断阈值时间
	 *  设按键按下开始时刻为x，按键按下松开时刻为t
	 *  当 x < t <= x + shortPressThreshold 时，认为此次按键按下为一次短按
	 *  当 x + shortPressThreshold < t <= x + longPressErrorThreshold 时， 认为此次按键按下是一次正常的长按
	 *  当 t > x + longPressErrorThreshold 时，认为此次按键按下时间过长，出现了异常
     */
	TickType_t longPressErrorThreshold;	
#if (Key_InputMode_Enable == 1)
    /*!
     *  频繁按下按键异常判断阈值时间，这里的频繁指相邻按键按下的间隔非常短
	 *  认为人手按下按键，上一次按键按下松开和下一次按键按下开始的间隔不应小于quickPressErrorThreshold
	 *  如果连续多次出现间隔小于quickPressErrorThreshold的情况，就认为按键出现了问题
     */
	TickType_t quickPressErrorThreshold;	/**<  */
	uint8_t quickPressErrorCount;		/**< 频繁按下按键次数统计 */
	uint8_t quickPressErrorMaxCount;	/**< 频繁按下按键次数统计上限，当超过上限时，认为因为外界因素导致按键出现了问题 */
#endif
	
	GPIO_TypeDef* GPIO_Port;	/**< 按键对应的GPIOx */
	uint16_t GPIO_Pin;			/**< 按键对应的GPIO_Pin */
	GPIOMode_TypeDef GPIOMode;	/**< 按键输入模式，默认为上拉输入 */
	GPIOSpeed_TypeDef GPIOSpeed;	/**< 按键输入对应的GPIOSpeed,默认为2MHz */
	
	KEY_STATE prevState;		/**< 按键的上个状态，用于消抖 */
	KEY_STATE prevprevState;	/**< 按键上上个状态，用于消抖*/
	
#if (Key_InputMode_Enable == 1)
	TimerHandle_t commandExecutionTimer;	/**< 按键命令输入模式下的软件定时器 */
	TickType_t waitingTimeForCommandExecution;	/**< 当完成一次按键按下过多久后自动执行命令 */
#endif
	
    /*!
     *  按键输入模式，分为普通输入模式和命令输入模式
	 *  普通输入模式：这个模式下可以分长按和短按，也可以不分长按和短按，希望按键松开后立刻行动或按键处于按下状态时行动
	 *  命令输入模式：分为短按或长按，这时按键松开后需要过一段时间才执行相应的命令
     */
	INPUT_MODE inputMode;	/**<  */

#if (Key_InputMode_Enable == 1)
	/*!
	 * 假设 commandMaxLength = 3， waitingTimeForCommandExecution = 1.2s
	 * 用S表示短按，L表示长按，用“输入”表示一次长按或短按，每次输入后长按或短按会进入命令队列中
	 * command(X)表示命令队列为X时要执行的命令，那么可能的命令组合有
	 * 只输入一次就执行命令：	 command(S)		command(L)
	 * 连续输入两次后执行命令：	 command(SS)	command(SL)
	 *							 command(LS)    command(LL)
	 * 连续输入三次后执行命令：	 command(SSS)	command(SSL)	command(SLS)	command(SLL)
	 *							 command(LSS)	command(LSL)	command(LLS)	command(LLL)
	 * 在命令输入模式下，每输入一次，输入完成后都会开启一个一次性定时器，将lastCommandExecutionState 置为 RUNNING
	 * 等到定时器到时间后就会执行命令队列中对应的命令，等命令执行完后将lastCommandExecutionState 置为 WAITING。
	 * 第一次输入后，有两种可能的去向
	 * 1.在第一次输入结束后的commandIntervalThreshold时间内开始第二次输入。
	 * 	 这会停止一次性定时器的定时，并将lastCommandExecutionState 置为 WAITING，表示命令队列有新的输入，需要重新解析命令
	 *	 等第二次输入结束后再重新开启一次性定时器，将lastCommandExecutionState置为 RUNNING
	 * 2.在第一次输入结束后的commandIntervalThreshold时间内没有第二次输入，这样第一次输入开启的一次性定时器不会停止，
	 *   该定时器到时间后就会执行第一次输入对应的命令：command(S)或command(L)，在定时器到时间前，任何的输入都是无效的
	 *   需要等待命令执行完成后才能开始新的输入。
	 * 当commandCurrentLength = commandMaxLength 时，即第三次输入后，只有一种去向，即等待命令执行完，命令执行完前任何输入都是无效的
	 */
	COMMAND_EXECUTION_STATE lastCommandExecutionState;	/**< 用来表示上个命令的执行状态 */
	
	void (*commandFunc[14])(void);	/**< 不同的命令执行不同的函数，可以在函数中创建任务，由用户自己定义 */
#endif

}Key_Structure, KEY;

/*================================= 函数声明 ==================================*/

void Key_Init(Key_Structure * keyStructure, GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin, char * keyName);
void Key_StructInit(Key_Structure * keyStructure);
void Key_DisableInput(Key_Structure * keyStructure);
void Key_InvalidateInput(Key_Structure * keyStructure);
KEY_STATE Key_GetStatus(Key_Structure * keyStructure);
void Key_vTask(void * pvParameters);

#if (Key_InputMode_Enable == 1)
void Key_vCommandExecutionTimerCallBack(TimerHandle_t xTimer);
#endif

#endif

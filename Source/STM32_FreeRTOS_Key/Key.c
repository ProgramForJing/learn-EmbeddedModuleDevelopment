#include "Key.h"

 /*******************************************************************************
  * @brief    初始化Key_Structure结构体
  *           
  * @param    keyStructure	指向Key_Structure类型的指针
  *           GPIOx			按键对应的STM32硬件的GPIO端口
  *			  GPIO_Pin		按键对应的STM32硬件的GPIO引脚
  *			  keyName		按键名称，用来辅助创建命令输入模式下的一次性定时器
  * @retval   无
  ******************************************************************************/
void Key_Init(Key_Structure * keyStructure, GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin, char * keyName)
{
	keyStructure->GPIO_Port = GPIOx;
	keyStructure->GPIO_Pin = GPIO_Pin;
	keyStructure->keyName = keyName;
	
    /*!
     * 用于STM32硬件初始化按键输入引脚 
     */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = keyStructure->GPIOMode;
	GPIO_InitStructure.GPIO_Pin = keyStructure->GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = keyStructure->GPIOSpeed;
	GPIO_Init(keyStructure->GPIO_Port, &GPIO_InitStructure);
	
#if (Key_InputMode_Enable == 1)
    /*!
     * 创建命令输入模式下的一次性软件定时器 
     */
	keyStructure->commandExecutionTimer = xTimerCreate(keyStructure->keyName, 
													   keyStructure->waitingTimeForCommandExecution,
													   pdFALSE,
													   (void *)keyStructure,
													   Key_vCommandExecutionTimerCallBack);
#endif
													   
}

 /*******************************************************************************
  * @brief    Key_Structure成员较多，该函数用来设置各成员初始值
  *
  * @param    keyStructure	指向Key_Structure类型的指针
  *           
  * @retval   无
  ******************************************************************************/
void Key_StructInit(Key_Structure * keyStructure)
{
	keyStructure->status = ENABLE;		/**< 默认按键为使能状态 */

	keyStructure->keyState = KEY_STATE_UP;		/**< 默认按键为松开状态 */
	keyStructure->pressType = PRESS_TYPE_IDLE;	/**< 默认按键无按下为空闲状态 */
	
	keyStructure->shortPressThreshold = pdMS_TO_TICKS(800);		/**< 默认短按判断阈值时间为800ms */

#if (Key_InputMode_Enable == 1)
	keyStructure->commandIntervalThreshold = pdMS_TO_TICKS(1000);		/**< 默认命令输入判断阈值时间为1000ms */
	keyStructure->commandMaxLength = 3;		/**< 默认命令队列的最大长度为3 */
	keyStructure->commandCurrentLength = 0;		/**< 默认命令队列当前长度为0 */
	
	keyStructure->command = 0x80;		/**< 默认命令值为0x80 */
#endif
	
	keyStructure->pressCount = 0;		/**< 默认pressCount = 0 */
	keyStructure->pressTotalCount = 0;	/**< 默认pressTotalCount = 0 */
	
	keyStructure->pressEntryPoint = 0;	/**< 默认按键按下开始时刻为0 */
	keyStructure->pressExitPoint = 0;	/**< 默认按键按下松开时刻为0 */
	keyStructure->lastPressExitPoint = 0;	/**< 默认上一次按键按下松开时刻为0 */
	
	keyStructure->longPressErrorThreshold = pdMS_TO_TICKS(6000);	/**< 默认长按异常判断阈值时间为6000ms */

#if (Key_InputMode_Enable == 1)
	keyStructure->quickPressErrorThreshold = pdMS_TO_TICKS(100);	/**< 默认频繁按下按键异常判断阈值时间为100ms */
	keyStructure->quickPressErrorCount = 0;			/**< 频繁按下按键次数统计，默认值为0 */
	keyStructure->quickPressErrorMaxCount = 5;		/**< 频繁按下按键次数统计上限，默认为5 */
#endif
	
    /*!
     *  STM32 GPIO口初始化参数
	 *  默认上拉输入， A0引脚, 2MHz
     */
	keyStructure->GPIOMode = GPIO_Mode_IPU;
	keyStructure->GPIOSpeed = GPIO_Speed_2MHz;
	keyStructure->GPIO_Pin = GPIO_Pin_0;
	keyStructure->GPIO_Port = GPIOA;
	
	keyStructure->prevprevState = KEY_STATE_UP;		/**< 按键的上个状态，默认为KEY_STATE_UP */
	keyStructure->prevState = KEY_STATE_UP;			/**< 按键上上个状态，默认为KEY_STATE_UP */
	keyStructure->inputMode = INPUT_MODE_NORMAL;		/**< 按键输入模式，默认为NORMAL（普通输入模式） */
	
#if (Key_InputMode_Enable == 1)
	keyStructure->commandExecutionTimer = NULL;		/**< 命令输入模式下软件定时器句柄，默认为NULL */
	keyStructure->waitingTimeForCommandExecution = pdMS_TO_TICKS(1200);		/**< 当完成一次按键按下过多久后自动执行命令，默认为1200ms */
	keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;		/**< 用来表示上个命令的执行状态，默认为WAITING */
	
	for(int i = 0; i < 14; i++)
	{
		keyStructure->commandFunc[i] = NULL;
	}
#endif
}

 /*******************************************************************************
  * @brief    失能按键
  *
  * @param    keyStructure	指向Key_Structure类型的指针
  *           
  * @retval   无
  ******************************************************************************/
void Key_DisableInput(Key_Structure * keyStructure)
{
	keyStructure->status = DISABLE;
	keyStructure->keyState = KEY_STATE_DISABLE;
	keyStructure->pressType = PRESS_TYPE_IDLE;
	keyStructure->pressCount = 0;
	keyStructure->pressEntryPoint = 0;
	keyStructure->pressExitPoint = 0;
	keyStructure->lastPressExitPoint = 0;

#if (Key_InputMode_Enable == 1)
	keyStructure->commandCurrentLength = 0;
	keyStructure->command = 0x80;
	keyStructure->quickPressErrorCount = 0;
	keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
	if(keyStructure->commandExecutionTimer != NULL)
	{
		xTimerStop(keyStructure->commandExecutionTimer, pdMS_TO_TICKS(200));
	}
#endif
}


 /*******************************************************************************
  * @brief    使按键输入无效
  *
  * @param    keyStructure	指向Key_Structure类型的指针
  *           
  * @retval   无
  ******************************************************************************/
void Key_InvalidateInput(Key_Structure * keyStructure)
{
	keyStructure->keyState = KEY_STATE_INVALID;
	keyStructure->pressType = PRESS_TYPE_IDLE;

	keyStructure->pressEntryPoint = 0;
	keyStructure->pressExitPoint = 0;
	keyStructure->lastPressExitPoint = 0;

#if (Key_InputMode_Enable == 1)
	keyStructure->quickPressErrorCount = 0;
#endif
}

 /*******************************************************************************
  * @brief    使按键输入无效
  *
  * @param    keyStructure	指向Key_Structure类型的指针
  *           
  * @retval   KEY_STATE_DOWN	表示按键处于按下状态
  * 		  KEY_STATE_UP		表示按键处于松开状态
  *    		  KEY_STATE_DISABLE	表示按键处于失能状态
  *           KEY_STATE_INVALID	表示按键输入无效
  ******************************************************************************/
KEY_STATE Key_GetStatus(Key_Structure * keyStructure)
{
	/**< 如果按键失能直接返回 */
	if(keyStructure->status == DISABLE)
	{
		keyStructure->keyState = KEY_STATE_DISABLE;
		return keyStructure->keyState;
	}
	else
	{
		if(GPIO_ReadInputDataBit(keyStructure->GPIO_Port, keyStructure->GPIO_Pin) == 0)
		{
		    /*!
             * 如果硬件读取引脚输入为0，且按键上上次状态和上次状态为KEY_STATE_UP
			 * 表明按键开始按下
             */
			if(keyStructure->prevprevState == KEY_STATE_UP && keyStructure->prevState == KEY_STATE_UP)
			{
				keyStructure->keyState = KEY_STATE_DOWN;
				keyStructure->pressEntryPoint = xTaskGetTickCount();	/**< 记录按键按下的时刻 */
				
				#if (Key_InputMode_Enable == 1)
			    /*!
                 *  如果 inputMode 为命令输入模式，只在按键松开时判断输入是短按还是长按
                 */
				if(keyStructure->inputMode == INPUT_MODE_COMMAND)
				{
					/**< 如果这不是第一次按键按下 */
					if(keyStructure->lastPressExitPoint != 0)
					{
						/**< 计算这次按键按下时刻和上次按键松开时刻的间隔 */
						TickType_t duration = keyStructure->pressEntryPoint - keyStructure->lastPressExitPoint;
						
						/**< 如果间隔小于quickPressErrorThreshold，意味着两次按键按下的间隔非常短，可能出现了异常 */
						if( duration <= keyStructure->quickPressErrorThreshold)
						{
						    /*!
                             *  统计频繁按下按键次数，如果连续超过一定数量，就认为按键出现了异常
							 *  如果没达到一定数量且commandCurrentLength < commandMaxLength，就这次按键按下认为是一次命令输入
                             */
							keyStructure->quickPressErrorCount++;
							if(keyStructure->quickPressErrorCount >= keyStructure->quickPressErrorMaxCount)
							{
								Key_DisableInput(keyStructure);
								return keyStructure->keyState;
							}
							/**< 命令队列还未满，这时需要重新判断命令 */
							if(keyStructure->commandCurrentLength < keyStructure->commandMaxLength)
							{
								xTimerStop(keyStructure->commandExecutionTimer, pdMS_TO_TICKS(200));
								keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
							}
						}
						/**< 如果两次按键按下之间的间隔小于commandIntervalThreshold，就认为这次按键按下是一次命令输入 */
						else if( duration > keyStructure->quickPressErrorThreshold &&
								duration <= keyStructure->commandIntervalThreshold)
						{			
							keyStructure->quickPressErrorCount--;
							keyStructure->quickPressErrorCount = keyStructure->quickPressErrorCount > 0 ? keyStructure->quickPressErrorCount : 0;
						    /*!
                             *  如果现有的命令输入数量未达到最大数量，则暂停正在运行的定时器
							 *  重新判定命令，lastCommandExecutionState用来标志命令队列中的命令是否正在执行
							 *  认为如果commandExecutionTimer开始就视为命令正在执行，直到命令对应的操作完成后
							 *  WAITING表示等待执行，RUNNING表示正在执行
							 *  这里不判断现有命令数量超过最大数量的情况，因为这会在按键松开那部分判断
                             */
							if(keyStructure->commandCurrentLength < keyStructure->commandMaxLength)
							{
								xTimerStop(keyStructure->commandExecutionTimer, pdMS_TO_TICKS(200));
								keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
							}
						}
					    /*!
                         *  按键按下超过了命令输入时限，但其是否有效还要看命令对应的操作是否执行完成
						 *  如果按键松开时，命令对应的操作正在执行，此时应该忽略输入
						 *  如果按键松开时，命令对应的操作已经执行完毕，那么这次按键按下应该视为新的命令输入开始
						 *  上面说的功能会在按键松开部分实现
                         */
						else if( duration > keyStructure->commandIntervalThreshold)
						{
							keyStructure->quickPressErrorCount--;
							keyStructure->quickPressErrorCount = keyStructure->quickPressErrorCount > 0 ? keyStructure->quickPressErrorCount : 0;
						}
					}
					else
					{
						/**< 是第一次输入，要记入命令队列中 */
						keyStructure->commandCurrentLength = 0;
						keyStructure->command = 0x80;
					}
				}
				#endif
				
			}
			else if(keyStructure->prevprevState == KEY_STATE_DOWN && keyStructure->prevState == KEY_STATE_DOWN)
			{
				/**< 这里表示按键处于按下状态 */
				keyStructure->keyState = KEY_STATE_DOWN;
			}			
			/**< 更新prevprevState 和 prevState */
			keyStructure->prevprevState = keyStructure->prevState;
			keyStructure->prevState = KEY_STATE_DOWN;	
		}
		else if(GPIO_ReadInputDataBit(keyStructure->GPIO_Port, keyStructure->GPIO_Pin) == 1)
		{
			/*!
             * 如果硬件读取引脚输入为1，且按键上上次状态和上次状态为KEY_STATE_DOWN
			 * 表明按键开始松开
             */
			if(keyStructure->prevprevState == KEY_STATE_DOWN && keyStructure->prevState == KEY_STATE_DOWN)
			{
				keyStructure->keyState = KEY_STATE_UP;
				keyStructure->pressExitPoint = xTaskGetTickCount();		/**< 记录按键松开时刻 */
				keyStructure->pressCount++;
				keyStructure->pressTotalCount++;
				
				#if (Key_InputMode_Enable == 1)
				if(keyStructure->inputMode == INPUT_MODE_COMMAND)
				{
					keyStructure->lastPressExitPoint = keyStructure->pressExitPoint;	/**< 更新lastPressExitPoint */
					
					/**< 计算按键松开时刻和按键按下时刻的间隔，即按键按下的时间长短 */
					TickType_t duration = keyStructure->pressExitPoint - keyStructure->pressEntryPoint;
					
					/**< 如果按键按下时间小于等于shortPressThreshold，则认为是一次短按 */
					if( duration <= keyStructure->shortPressThreshold)
					{
						keyStructure->pressType = PRESS_TYPE_SHORT;
					}
					/**< 如果按键按下时间大于shortPressThreshold且小于等于longPressErrorThreshold，则认为是一次长按 */
					else if(duration > keyStructure->shortPressThreshold && 
							duration <= keyStructure->longPressErrorThreshold)
					{
						keyStructure->pressType = PRESS_TYPE_LONG;
					}
				    /*!
                     *  如果按键按下时间大于longPressErrorThreshold，则认为按键按下的时间过长，按键出现问题
					 *  此时使按键失能并返回
                     */
					else if(duration > keyStructure->longPressErrorThreshold)
					{
						Key_DisableInput(keyStructure);
						return keyStructure->keyState;
					}
					
					/**< 如果上个命令对应的操作还没完成，此时不应该接收任何按键输入 */
					if(keyStructure->lastCommandExecutionState == COMMAND_EXECUTION_STATE_RUNNING)
					{
						/**< 所以这里此次输入无效直接返回 */
						Key_InvalidateInput(keyStructure);
						keyStructure->pressCount--;
						return keyStructure->keyState;
					}
				    /*!
                     * 这里有两种可能
					 * 1.commandCurrentLegnth 达到最大，对应的命令执行完了，这次输入是新的命令输入开始
					 * 2.commandCurrentLength 未达到最大，其输入开始时刻在commandIntervalThreshold之内，那么应该重新判断命令
                     */
					else
					{
						/**< 根据输入类型更新command值和commandCurrentLength  */
						keyStructure->command = keyStructure->command << 1;
						keyStructure->command |= (uint8_t)keyStructure->pressType;
						keyStructure->commandCurrentLength++;
						
					    /*!
                         * 启动一次性软件定时器，超过一段时间后自动执行对应的命令并清空命令队列 
						 * 如果定时器复位失败，则清空命令队列，重新开始下一次命令输入
                         */
						if(xTimerIsTimerActive(keyStructure->commandExecutionTimer) == pdFALSE)
						{
							if(xTimerReset(keyStructure->commandExecutionTimer, pdMS_TO_TICKS(400)) == pdFALSE)
							{
								keyStructure->commandCurrentLength = 0;
								keyStructure->command = 0x80;
								keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
							}
							else
							{
								keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_RUNNING;
							}
						}
					}
				}
				#endif
			}
			else if(keyStructure->prevprevState == KEY_STATE_UP && keyStructure->prevState == KEY_STATE_UP)
			{
				/**< 这里表示按键处于松开状态 */
				keyStructure->keyState = KEY_STATE_UP;
			}			
			keyStructure->prevprevState = keyStructure->prevState;
			keyStructure->prevState = KEY_STATE_UP;
		}
		return keyStructure->keyState;
	}
}

#if (Key_InputMode_Enable == 1)
 /*******************************************************************************
  * @brief    软件定时器回调函数，用来执行命令,'->'表示映射的索引，L表示长按，S表示短按
  *  		  注意输入顺序： LSS 表示先长按后短按再短按
  *           一次输入： L(0->0)	S(1->1)
  *  		  两次输入： LL(00->2)	LS(01->3)
  *			  			 SL(10->4)	SS(11->5)
  *			  三次输入： LLL(000->6)	LLS(001->7)		LSL(010->8)		LSS(011->9)
  *  		  			 SLL(100->10)	SLS(101->11)	SSL(110->12)	SSS(111->13)
  *
  * @param    xTimer	软件定时器句柄
  *           
  * @retval   无
  ******************************************************************************/
void Key_vCommandExecutionTimerCallBack(TimerHandle_t xTimer)
{
	Key_Structure * keyStructure = (Key_Structure*)pvTimerGetTimerID(xTimer);
	
	if(keyStructure->command == 0x80 && keyStructure->commandCurrentLength == 0)
	{
		return;
	}
	else
	{
		uint8_t offset =  0;
		if(keyStructure->commandCurrentLength == 1)
		{
			offset = 0;
		}
		else if(keyStructure->commandCurrentLength == 2)
		{
			offset = 2;
		}
		else if(keyStructure->commandCurrentLength == 3)
		{
			offset = 6;
		}
	    /*!
         *  用函数指针进行命令处理
		 *  如果只在函数中实现对应的操作,命令对应的操作应该简短，不应执行耗时的操作
		 *  如果需要创建任务来实现对应的操作，应对创建的任务进行管理，
		 *  防止多次输入同一个命令重复创建任务。
         */
		if(keyStructure->commandFunc[keyStructure->command + offset] != NULL)
		{
			keyStructure->commandFunc[keyStructure->command + offset]();
		}	
	}
	
	//完成命令处理后
	keyStructure->commandCurrentLength = 0;
	keyStructure->command = 0x80;
	keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
}
#endif

//这里提供一个模板
//如果使用硬件定时器，需要在对应的中断服务例程中给出信号量
//然后将中断处理放到该任务中来
//该任务用来集中检测按键输入
//void Key_vKeyTaskFromTIMx(void * pvParameters)
//{
//	for(;;)
//	{
//		if(xSemaphoreTake(xBinSemphr_TIMx, portMAX_DELAY) == pdPASS)
//		{
//			Key_Func_key1(&key1);
//			Key_Func_key2(&key2);
//		}

//	}
//}

//这里只提供一个模板
//实际使用时还要根据不同的按键加入对应的功能并修改函数的名字
//如有两个按键，key1,key2
//那么就需要根据模板创建两个函数，函数名自定
//Key_Func_key1(Key_Structure * keyStructure)
//Key_Func_key2(Key_Structure * keyStructure)
//然后根据模板的提示加入想要的功能
//void Key_Func_keyX(Key_Structure * keyStructure)
//{
//	KEY_STATE ks = Key_GetStatus(keyStructure);
//	if(ks == KEY_STATE_DISABLE)
//	{
//		//按键失能
//		//执行相应操作
//		//...
//	}
//	else if(ks == KEY_STATE_INVALID)
//	{
//		//输入无效
//		//执行相应操作
//		//...
//	}
//	if(keyStructure->inputMode == INPUT_MODE_NORMAL)
//	{
//		if(ks == KEY_STATE_DOWN)
//		{
//			//这里执行按键按下时应该执行的动作
//			//...

//			TickType_t currentTime = xTaskGetTickCount();
//			if(currentTime - keyStructure->pressEntryPoint > keyStructure->shortPressThreshold &&
//				currentTime - keyStructure->pressEntryPoint < keyStructure->longPressErrorThreshold)
//			{
//				//执行长按时应该执行的动作
//				//...
//			}
//			else if(currentTime - keyStructure->pressEntryPoint >= keyStructure->longPressErrorThreshold)
//			{
//				//长按时间过长时要执行的动作
//				//...
//			}
//			
//		}
//		else if(ks == KEY_STATE_UP)
//		{
//			if(keyStructure->prevprevState == KEY_STATE_DOWN)
//			{
//				//这里执行按键按下一次后应该执行的动作，不论长按还是短按
//				//...
//			}
//		}
//	}
//	else
//	{
//		//命令输入模式下会通过软件定时器一段时间后自动执行命令
//	}
//}

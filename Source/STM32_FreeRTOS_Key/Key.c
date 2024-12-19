#include "Key.h"

 /*******************************************************************************
  * @brief    ��ʼ��Key_Structure�ṹ��
  *           
  * @param    keyStructure	ָ��Key_Structure���͵�ָ��
  *           GPIOx			������Ӧ��STM32Ӳ����GPIO�˿�
  *			  GPIO_Pin		������Ӧ��STM32Ӳ����GPIO����
  *			  keyName		�������ƣ���������������������ģʽ�µ�һ���Զ�ʱ��
  * @retval   ��
  ******************************************************************************/
void Key_Init(Key_Structure * keyStructure, GPIO_TypeDef * GPIOx, uint16_t GPIO_Pin, char * keyName)
{
	keyStructure->GPIO_Port = GPIOx;
	keyStructure->GPIO_Pin = GPIO_Pin;
	keyStructure->keyName = keyName;
	
    /*!
     * ����STM32Ӳ����ʼ�������������� 
     */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = keyStructure->GPIOMode;
	GPIO_InitStructure.GPIO_Pin = keyStructure->GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = keyStructure->GPIOSpeed;
	GPIO_Init(keyStructure->GPIO_Port, &GPIO_InitStructure);
	
#if (Key_InputMode_Enable == 1)
    /*!
     * ������������ģʽ�µ�һ���������ʱ�� 
     */
	keyStructure->commandExecutionTimer = xTimerCreate(keyStructure->keyName, 
													   keyStructure->waitingTimeForCommandExecution,
													   pdFALSE,
													   (void *)keyStructure,
													   Key_vCommandExecutionTimerCallBack);
#endif
													   
}

 /*******************************************************************************
  * @brief    Key_Structure��Ա�϶࣬�ú����������ø���Ա��ʼֵ
  *
  * @param    keyStructure	ָ��Key_Structure���͵�ָ��
  *           
  * @retval   ��
  ******************************************************************************/
void Key_StructInit(Key_Structure * keyStructure)
{
	keyStructure->status = ENABLE;		/**< Ĭ�ϰ���Ϊʹ��״̬ */

	keyStructure->keyState = KEY_STATE_UP;		/**< Ĭ�ϰ���Ϊ�ɿ�״̬ */
	keyStructure->pressType = PRESS_TYPE_IDLE;	/**< Ĭ�ϰ����ް���Ϊ����״̬ */
	
	keyStructure->shortPressThreshold = pdMS_TO_TICKS(800);		/**< Ĭ�϶̰��ж���ֵʱ��Ϊ800ms */

#if (Key_InputMode_Enable == 1)
	keyStructure->commandIntervalThreshold = pdMS_TO_TICKS(1000);		/**< Ĭ�����������ж���ֵʱ��Ϊ1000ms */
	keyStructure->commandMaxLength = 3;		/**< Ĭ��������е���󳤶�Ϊ3 */
	keyStructure->commandCurrentLength = 0;		/**< Ĭ��������е�ǰ����Ϊ0 */
	
	keyStructure->command = 0x80;		/**< Ĭ������ֵΪ0x80 */
#endif
	
	keyStructure->pressCount = 0;		/**< Ĭ��pressCount = 0 */
	keyStructure->pressTotalCount = 0;	/**< Ĭ��pressTotalCount = 0 */
	
	keyStructure->pressEntryPoint = 0;	/**< Ĭ�ϰ������¿�ʼʱ��Ϊ0 */
	keyStructure->pressExitPoint = 0;	/**< Ĭ�ϰ��������ɿ�ʱ��Ϊ0 */
	keyStructure->lastPressExitPoint = 0;	/**< Ĭ����һ�ΰ��������ɿ�ʱ��Ϊ0 */
	
	keyStructure->longPressErrorThreshold = pdMS_TO_TICKS(6000);	/**< Ĭ�ϳ����쳣�ж���ֵʱ��Ϊ6000ms */

#if (Key_InputMode_Enable == 1)
	keyStructure->quickPressErrorThreshold = pdMS_TO_TICKS(100);	/**< Ĭ��Ƶ�����°����쳣�ж���ֵʱ��Ϊ100ms */
	keyStructure->quickPressErrorCount = 0;			/**< Ƶ�����°�������ͳ�ƣ�Ĭ��ֵΪ0 */
	keyStructure->quickPressErrorMaxCount = 5;		/**< Ƶ�����°�������ͳ�����ޣ�Ĭ��Ϊ5 */
#endif
	
    /*!
     *  STM32 GPIO�ڳ�ʼ������
	 *  Ĭ���������룬 A0����, 2MHz
     */
	keyStructure->GPIOMode = GPIO_Mode_IPU;
	keyStructure->GPIOSpeed = GPIO_Speed_2MHz;
	keyStructure->GPIO_Pin = GPIO_Pin_0;
	keyStructure->GPIO_Port = GPIOA;
	
	keyStructure->prevprevState = KEY_STATE_UP;		/**< �������ϸ�״̬��Ĭ��ΪKEY_STATE_UP */
	keyStructure->prevState = KEY_STATE_UP;			/**< �������ϸ�״̬��Ĭ��ΪKEY_STATE_UP */
	keyStructure->inputMode = INPUT_MODE_NORMAL;		/**< ��������ģʽ��Ĭ��ΪNORMAL����ͨ����ģʽ�� */
	
#if (Key_InputMode_Enable == 1)
	keyStructure->commandExecutionTimer = NULL;		/**< ��������ģʽ�������ʱ�������Ĭ��ΪNULL */
	keyStructure->waitingTimeForCommandExecution = pdMS_TO_TICKS(1200);		/**< �����һ�ΰ������¹���ú��Զ�ִ�����Ĭ��Ϊ1200ms */
	keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;		/**< ������ʾ�ϸ������ִ��״̬��Ĭ��ΪWAITING */
	
	for(int i = 0; i < 14; i++)
	{
		keyStructure->commandFunc[i] = NULL;
	}
#endif
}

 /*******************************************************************************
  * @brief    ʧ�ܰ���
  *
  * @param    keyStructure	ָ��Key_Structure���͵�ָ��
  *           
  * @retval   ��
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
  * @brief    ʹ����������Ч
  *
  * @param    keyStructure	ָ��Key_Structure���͵�ָ��
  *           
  * @retval   ��
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
  * @brief    ʹ����������Ч
  *
  * @param    keyStructure	ָ��Key_Structure���͵�ָ��
  *           
  * @retval   KEY_STATE_DOWN	��ʾ�������ڰ���״̬
  * 		  KEY_STATE_UP		��ʾ���������ɿ�״̬
  *    		  KEY_STATE_DISABLE	��ʾ��������ʧ��״̬
  *           KEY_STATE_INVALID	��ʾ����������Ч
  ******************************************************************************/
KEY_STATE Key_GetStatus(Key_Structure * keyStructure)
{
	/**< �������ʧ��ֱ�ӷ��� */
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
             * ���Ӳ����ȡ��������Ϊ0���Ұ������ϴ�״̬���ϴ�״̬ΪKEY_STATE_UP
			 * ����������ʼ����
             */
			if(keyStructure->prevprevState == KEY_STATE_UP && keyStructure->prevState == KEY_STATE_UP)
			{
				keyStructure->keyState = KEY_STATE_DOWN;
				keyStructure->pressEntryPoint = xTaskGetTickCount();	/**< ��¼�������µ�ʱ�� */
				
				#if (Key_InputMode_Enable == 1)
			    /*!
                 *  ��� inputMode Ϊ��������ģʽ��ֻ�ڰ����ɿ�ʱ�ж������Ƕ̰����ǳ���
                 */
				if(keyStructure->inputMode == INPUT_MODE_COMMAND)
				{
					/**< ����ⲻ�ǵ�һ�ΰ������� */
					if(keyStructure->lastPressExitPoint != 0)
					{
						/**< ������ΰ�������ʱ�̺��ϴΰ����ɿ�ʱ�̵ļ�� */
						TickType_t duration = keyStructure->pressEntryPoint - keyStructure->lastPressExitPoint;
						
						/**< ������С��quickPressErrorThreshold����ζ�����ΰ������µļ���ǳ��̣����ܳ������쳣 */
						if( duration <= keyStructure->quickPressErrorThreshold)
						{
						    /*!
                             *  ͳ��Ƶ�����°��������������������һ������������Ϊ�����������쳣
							 *  ���û�ﵽһ��������commandCurrentLength < commandMaxLength������ΰ���������Ϊ��һ����������
                             */
							keyStructure->quickPressErrorCount++;
							if(keyStructure->quickPressErrorCount >= keyStructure->quickPressErrorMaxCount)
							{
								Key_DisableInput(keyStructure);
								return keyStructure->keyState;
							}
							/**< ������л�δ������ʱ��Ҫ�����ж����� */
							if(keyStructure->commandCurrentLength < keyStructure->commandMaxLength)
							{
								xTimerStop(keyStructure->commandExecutionTimer, pdMS_TO_TICKS(200));
								keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
							}
						}
						/**< ������ΰ�������֮��ļ��С��commandIntervalThreshold������Ϊ��ΰ���������һ���������� */
						else if( duration > keyStructure->quickPressErrorThreshold &&
								duration <= keyStructure->commandIntervalThreshold)
						{			
							keyStructure->quickPressErrorCount--;
							keyStructure->quickPressErrorCount = keyStructure->quickPressErrorCount > 0 ? keyStructure->quickPressErrorCount : 0;
						    /*!
                             *  ������е�������������δ�ﵽ�������������ͣ�������еĶ�ʱ��
							 *  �����ж����lastCommandExecutionState������־��������е������Ƿ�����ִ��
							 *  ��Ϊ���commandExecutionTimer��ʼ����Ϊ��������ִ�У�ֱ�������Ӧ�Ĳ�����ɺ�
							 *  WAITING��ʾ�ȴ�ִ�У�RUNNING��ʾ����ִ��
							 *  ���ﲻ�ж���������������������������������Ϊ����ڰ����ɿ��ǲ����ж�
                             */
							if(keyStructure->commandCurrentLength < keyStructure->commandMaxLength)
							{
								xTimerStop(keyStructure->commandExecutionTimer, pdMS_TO_TICKS(200));
								keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
							}
						}
					    /*!
                         *  �������³�������������ʱ�ޣ������Ƿ���Ч��Ҫ�������Ӧ�Ĳ����Ƿ�ִ�����
						 *  ��������ɿ�ʱ�������Ӧ�Ĳ�������ִ�У���ʱӦ�ú�������
						 *  ��������ɿ�ʱ�������Ӧ�Ĳ����Ѿ�ִ����ϣ���ô��ΰ�������Ӧ����Ϊ�µ��������뿪ʼ
						 *  ����˵�Ĺ��ܻ��ڰ����ɿ�����ʵ��
                         */
						else if( duration > keyStructure->commandIntervalThreshold)
						{
							keyStructure->quickPressErrorCount--;
							keyStructure->quickPressErrorCount = keyStructure->quickPressErrorCount > 0 ? keyStructure->quickPressErrorCount : 0;
						}
					}
					else
					{
						/**< �ǵ�һ�����룬Ҫ������������� */
						keyStructure->commandCurrentLength = 0;
						keyStructure->command = 0x80;
					}
				}
				#endif
				
			}
			else if(keyStructure->prevprevState == KEY_STATE_DOWN && keyStructure->prevState == KEY_STATE_DOWN)
			{
				/**< �����ʾ�������ڰ���״̬ */
				keyStructure->keyState = KEY_STATE_DOWN;
			}			
			/**< ����prevprevState �� prevState */
			keyStructure->prevprevState = keyStructure->prevState;
			keyStructure->prevState = KEY_STATE_DOWN;	
		}
		else if(GPIO_ReadInputDataBit(keyStructure->GPIO_Port, keyStructure->GPIO_Pin) == 1)
		{
			/*!
             * ���Ӳ����ȡ��������Ϊ1���Ұ������ϴ�״̬���ϴ�״̬ΪKEY_STATE_DOWN
			 * ����������ʼ�ɿ�
             */
			if(keyStructure->prevprevState == KEY_STATE_DOWN && keyStructure->prevState == KEY_STATE_DOWN)
			{
				keyStructure->keyState = KEY_STATE_UP;
				keyStructure->pressExitPoint = xTaskGetTickCount();		/**< ��¼�����ɿ�ʱ�� */
				keyStructure->pressCount++;
				keyStructure->pressTotalCount++;
				
				#if (Key_InputMode_Enable == 1)
				if(keyStructure->inputMode == INPUT_MODE_COMMAND)
				{
					keyStructure->lastPressExitPoint = keyStructure->pressExitPoint;	/**< ����lastPressExitPoint */
					
					/**< ���㰴���ɿ�ʱ�̺Ͱ�������ʱ�̵ļ�������������µ�ʱ�䳤�� */
					TickType_t duration = keyStructure->pressExitPoint - keyStructure->pressEntryPoint;
					
					/**< �����������ʱ��С�ڵ���shortPressThreshold������Ϊ��һ�ζ̰� */
					if( duration <= keyStructure->shortPressThreshold)
					{
						keyStructure->pressType = PRESS_TYPE_SHORT;
					}
					/**< �����������ʱ�����shortPressThreshold��С�ڵ���longPressErrorThreshold������Ϊ��һ�γ��� */
					else if(duration > keyStructure->shortPressThreshold && 
							duration <= keyStructure->longPressErrorThreshold)
					{
						keyStructure->pressType = PRESS_TYPE_LONG;
					}
				    /*!
                     *  �����������ʱ�����longPressErrorThreshold������Ϊ�������µ�ʱ�������������������
					 *  ��ʱʹ����ʧ�ܲ�����
                     */
					else if(duration > keyStructure->longPressErrorThreshold)
					{
						Key_DisableInput(keyStructure);
						return keyStructure->keyState;
					}
					
					/**< ����ϸ������Ӧ�Ĳ�����û��ɣ���ʱ��Ӧ�ý����κΰ������� */
					if(keyStructure->lastCommandExecutionState == COMMAND_EXECUTION_STATE_RUNNING)
					{
						/**< ��������˴�������Чֱ�ӷ��� */
						Key_InvalidateInput(keyStructure);
						keyStructure->pressCount--;
						return keyStructure->keyState;
					}
				    /*!
                     * ���������ֿ���
					 * 1.commandCurrentLegnth �ﵽ��󣬶�Ӧ������ִ�����ˣ�����������µ��������뿪ʼ
					 * 2.commandCurrentLength δ�ﵽ��������뿪ʼʱ����commandIntervalThreshold֮�ڣ���ôӦ�������ж�����
                     */
					else
					{
						/**< �����������͸���commandֵ��commandCurrentLength  */
						keyStructure->command = keyStructure->command << 1;
						keyStructure->command |= (uint8_t)keyStructure->pressType;
						keyStructure->commandCurrentLength++;
						
					    /*!
                         * ����һ���������ʱ��������һ��ʱ����Զ�ִ�ж�Ӧ��������������� 
						 * �����ʱ����λʧ�ܣ������������У����¿�ʼ��һ����������
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
				/**< �����ʾ���������ɿ�״̬ */
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
  * @brief    �����ʱ���ص�����������ִ������,'->'��ʾӳ���������L��ʾ������S��ʾ�̰�
  *  		  ע������˳�� LSS ��ʾ�ȳ�����̰��ٶ̰�
  *           һ�����룺 L(0->0)	S(1->1)
  *  		  �������룺 LL(00->2)	LS(01->3)
  *			  			 SL(10->4)	SS(11->5)
  *			  �������룺 LLL(000->6)	LLS(001->7)		LSL(010->8)		LSS(011->9)
  *  		  			 SLL(100->10)	SLS(101->11)	SSL(110->12)	SSS(111->13)
  *
  * @param    xTimer	�����ʱ�����
  *           
  * @retval   ��
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
         *  �ú���ָ����������
		 *  ���ֻ�ں�����ʵ�ֶ�Ӧ�Ĳ���,�����Ӧ�Ĳ���Ӧ�ü�̣���Ӧִ�к�ʱ�Ĳ���
		 *  �����Ҫ����������ʵ�ֶ�Ӧ�Ĳ�����Ӧ�Դ�����������й���
		 *  ��ֹ�������ͬһ�������ظ���������
         */
		if(keyStructure->commandFunc[keyStructure->command + offset] != NULL)
		{
			keyStructure->commandFunc[keyStructure->command + offset]();
		}	
	}
	
	//���������
	keyStructure->commandCurrentLength = 0;
	keyStructure->command = 0x80;
	keyStructure->lastCommandExecutionState = COMMAND_EXECUTION_STATE_WAITING;
}
#endif

//�����ṩһ��ģ��
//���ʹ��Ӳ����ʱ������Ҫ�ڶ�Ӧ���жϷ��������и����ź���
//Ȼ���жϴ���ŵ�����������
//�������������м�ⰴ������
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

//����ֻ�ṩһ��ģ��
//ʵ��ʹ��ʱ��Ҫ���ݲ�ͬ�İ��������Ӧ�Ĺ��ܲ��޸ĺ���������
//��������������key1,key2
//��ô����Ҫ����ģ�崴�������������������Զ�
//Key_Func_key1(Key_Structure * keyStructure)
//Key_Func_key2(Key_Structure * keyStructure)
//Ȼ�����ģ�����ʾ������Ҫ�Ĺ���
//void Key_Func_keyX(Key_Structure * keyStructure)
//{
//	KEY_STATE ks = Key_GetStatus(keyStructure);
//	if(ks == KEY_STATE_DISABLE)
//	{
//		//����ʧ��
//		//ִ����Ӧ����
//		//...
//	}
//	else if(ks == KEY_STATE_INVALID)
//	{
//		//������Ч
//		//ִ����Ӧ����
//		//...
//	}
//	if(keyStructure->inputMode == INPUT_MODE_NORMAL)
//	{
//		if(ks == KEY_STATE_DOWN)
//		{
//			//����ִ�а�������ʱӦ��ִ�еĶ���
//			//...

//			TickType_t currentTime = xTaskGetTickCount();
//			if(currentTime - keyStructure->pressEntryPoint > keyStructure->shortPressThreshold &&
//				currentTime - keyStructure->pressEntryPoint < keyStructure->longPressErrorThreshold)
//			{
//				//ִ�г���ʱӦ��ִ�еĶ���
//				//...
//			}
//			else if(currentTime - keyStructure->pressEntryPoint >= keyStructure->longPressErrorThreshold)
//			{
//				//����ʱ�����ʱҪִ�еĶ���
//				//...
//			}
//			
//		}
//		else if(ks == KEY_STATE_UP)
//		{
//			if(keyStructure->prevprevState == KEY_STATE_DOWN)
//			{
//				//����ִ�а�������һ�κ�Ӧ��ִ�еĶ��������۳������Ƕ̰�
//				//...
//			}
//		}
//	}
//	else
//	{
//		//��������ģʽ�»�ͨ�������ʱ��һ��ʱ����Զ�ִ������
//	}
//}

#ifndef KEY_H
#define KEY_H

/*================================ ͷ�ļ����� =================================*/
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/*================================== �궨�� ===================================*/
#ifndef Key_InputMode_Enable
#define Key_InputMode_Enable	1
#endif

/*============================== �ṹ�������嶨�� =============================*/
typedef enum
{
	KEY_STATE_UP,	/**< �����ɿ�״̬ */
	KEY_STATE_DOWN,	/**< ��������״̬ */
	KEY_STATE_INVALID,	/**< ������Ч���� */
	KEY_STATE_DISABLE	/**< ����ʧ��״̬ */
}KEY_STATE;

typedef enum{
	PRESS_TYPE_SHORT = 0x01,	/**< ������������Ϊ�̰� */
	PRESS_TYPE_LONG = 0x00,		/**< ������������Ϊ���� */
	PRESS_TYPE_IDLE				/**< �����ް��£�����״̬ */
}PRESS_TYPE;

typedef enum{
	INPUT_MODE_NORMAL,		/**< ��ͨ����ģʽ */
#if (Key_InputMode_Enable == 1)
	INPUT_MODE_COMMAND		/**< ��������ģʽ����������ģʽ�¸��ݶ̰��ͳ����������ִ�ж�Ӧ������ */
#endif
}INPUT_MODE;

#if (Key_InputMode_Enable == 1)
typedef enum{
	COMMAND_EXECUTION_STATE_RUNNING,	/**< ��������ִ���� */
	COMMAND_EXECUTION_STATE_WAITING		/**< �ȴ����������� */
}COMMAND_EXECUTION_STATE;
#endif

//������е���󳤶ȣ�����commandMaxLength=3��ֻҪ���°���Ȼ���ɿ������ɿ���commandIntervalThresholdʱ��֮���ٴΰ��°���
//�������������commandMaxLength=3����ʾ���ɽ��������ΰ�������һ���������̰��̰�������һ������̰��̰�����������һ������
//����̰��̰�����֮���ٴ�����һ�����£��̰��򳤰�������ô��ΰ��½���Ϊһ����ͨ�Ķ̰��򳤰�����Ϊ��������Ѵﵽ��󳤶ȣ�
//����̰��̰�֮��commandIntervalThreshold֮��û�м�⵽���£���֮���ٴΰ��½�����һ����ͨ�Ķ̰��򳤰�

typedef struct
{
	char * keyName;		/**< �������� */
	FunctionalState status;		/**< ����ʹ��״̬��ENABLEʹ�ܣ�DISABLEʧ�� */
	KEY_STATE keyState;		/**< ����״̬ */
	PRESS_TYPE pressType;	/**< ������������ */
	
    /*!
	 *  �̰��ж���ֵʱ��
	 *  �谴���ɿ�ʱ��Ϊt����������ʱ��Ϊx
	 *  �� x < t <= x + shortPressThreshold ʱ����Ϊ�˴ΰ�������Ϊ�̰�
     */
	TickType_t shortPressThreshold;
	
#if (Key_InputMode_Enable == 1)
    /*!
	 *  ���������ж���ֵʱ��
	 *  ��ǰһ�ΰ��������ɿ�ʱ��Ϊx����ǰ�������¿�ʼʱ��Ϊt
	 *  �� x < t <= x + commandIntervalThreshold ʱ����Ϊ�˴�������Ҫ��¼�����������
     */
	TickType_t commandIntervalThreshold;
	
	uint8_t commandMaxLength;		/**< ���������󳤶� */
	uint8_t commandCurrentLength;	/**< ����������г��� */
	
    /*!
	 *  ����ֵ
     *  ����ʹ��һ��uint16_t���͵�������ʾ
	 *  �á����롱��ʾһ�γ�����̰�
	 *  command = 0x80 �� command = 0b1000 0000�� ��ʱ��ʾ���������û�м�¼�κ�����
	 *  ��ÿ��������ʱ����command����һλ��Ȼ����л�����
	 *  �̰�Ϊ 0x01������Ϊ 0x00
	 *  �����һ������Ϊ�̰�
	 *  command = 0b1000 0000 -> ����һλ��command = 0b0000 0000 -> 
	 *  ������ command |= 0x01 -> command = 0b000 0001 ��ͬʱ commandCurrentLength++
	 *  �ڶ�������Ϊ������command����һλ��0x00�����㣬 command = 0b0000 0010��ͬʱcommandCurrentLength++
	 *  ����������Ϊ�̰���command����һλ��0x01�����㣬 command = 0b0000 0101��ͬʱcommandCurrentLength++
	 *  �����Ϳɸ��� command ��ֵ�� commandCurrentLength ���ж�������ʲô����
     */
	uint8_t command;
#endif

	uint16_t pressCount;		/**< ͳ�ư������´�����������ĳЩԭ������ */
	uint16_t pressTotalCount;	/**< ͳ�ư��������ܴ��� */
	
	TickType_t pressEntryPoint;		/**< �������¿�ʼʱ�� */
	TickType_t pressExitPoint;		/**< �������½���ʱ�� */
	TickType_t lastPressExitPoint;	/**< ��һ�ΰ������½���ʱ�� */

    /*!
	 *  �����쳣�ж���ֵʱ��
	 *  �谴�����¿�ʼʱ��Ϊx�����������ɿ�ʱ��Ϊt
	 *  �� x < t <= x + shortPressThreshold ʱ����Ϊ�˴ΰ�������Ϊһ�ζ̰�
	 *  �� x + shortPressThreshold < t <= x + longPressErrorThreshold ʱ�� ��Ϊ�˴ΰ���������һ�������ĳ���
	 *  �� t > x + longPressErrorThreshold ʱ����Ϊ�˴ΰ�������ʱ��������������쳣
     */
	TickType_t longPressErrorThreshold;	
#if (Key_InputMode_Enable == 1)
    /*!
     *  Ƶ�����°����쳣�ж���ֵʱ�䣬�����Ƶ��ָ���ڰ������µļ���ǳ���
	 *  ��Ϊ���ְ��°�������һ�ΰ��������ɿ�����һ�ΰ������¿�ʼ�ļ����ӦС��quickPressErrorThreshold
	 *  ���������γ��ּ��С��quickPressErrorThreshold�����������Ϊ��������������
     */
	TickType_t quickPressErrorThreshold;	/**<  */
	uint8_t quickPressErrorCount;		/**< Ƶ�����°�������ͳ�� */
	uint8_t quickPressErrorMaxCount;	/**< Ƶ�����°�������ͳ�����ޣ�����������ʱ����Ϊ��Ϊ������ص��°������������� */
#endif
	
	GPIO_TypeDef* GPIO_Port;	/**< ������Ӧ��GPIOx */
	uint16_t GPIO_Pin;			/**< ������Ӧ��GPIO_Pin */
	GPIOMode_TypeDef GPIOMode;	/**< ��������ģʽ��Ĭ��Ϊ�������� */
	GPIOSpeed_TypeDef GPIOSpeed;	/**< ���������Ӧ��GPIOSpeed,Ĭ��Ϊ2MHz */
	
	KEY_STATE prevState;		/**< �������ϸ�״̬���������� */
	KEY_STATE prevprevState;	/**< �������ϸ�״̬����������*/
	
#if (Key_InputMode_Enable == 1)
	TimerHandle_t commandExecutionTimer;	/**< ������������ģʽ�µ������ʱ�� */
	TickType_t waitingTimeForCommandExecution;	/**< �����һ�ΰ������¹���ú��Զ�ִ������ */
#endif
	
    /*!
     *  ��������ģʽ����Ϊ��ͨ����ģʽ����������ģʽ
	 *  ��ͨ����ģʽ�����ģʽ�¿��Էֳ����Ͷ̰���Ҳ���Բ��ֳ����Ͷ̰���ϣ�������ɿ��������ж��򰴼����ڰ���״̬ʱ�ж�
	 *  ��������ģʽ����Ϊ�̰��򳤰�����ʱ�����ɿ�����Ҫ��һ��ʱ���ִ����Ӧ������
     */
	INPUT_MODE inputMode;	/**<  */

#if (Key_InputMode_Enable == 1)
	/*!
	 * ���� commandMaxLength = 3�� waitingTimeForCommandExecution = 1.2s
	 * ��S��ʾ�̰���L��ʾ�������á����롱��ʾһ�γ�����̰���ÿ������󳤰���̰���������������
	 * command(X)��ʾ�������ΪXʱҪִ�е������ô���ܵ����������
	 * ֻ����һ�ξ�ִ�����	 command(S)		command(L)
	 * �����������κ�ִ�����	 command(SS)	command(SL)
	 *							 command(LS)    command(LL)
	 * �����������κ�ִ�����	 command(SSS)	command(SSL)	command(SLS)	command(SLL)
	 *							 command(LSS)	command(LSL)	command(LLS)	command(LLL)
	 * ����������ģʽ�£�ÿ����һ�Σ�������ɺ󶼻Ὺ��һ��һ���Զ�ʱ������lastCommandExecutionState ��Ϊ RUNNING
	 * �ȵ���ʱ����ʱ���ͻ�ִ����������ж�Ӧ�����������ִ�����lastCommandExecutionState ��Ϊ WAITING��
	 * ��һ������������ֿ��ܵ�ȥ��
	 * 1.�ڵ�һ������������commandIntervalThresholdʱ���ڿ�ʼ�ڶ������롣
	 * 	 ���ֹͣһ���Զ�ʱ���Ķ�ʱ������lastCommandExecutionState ��Ϊ WAITING����ʾ����������µ����룬��Ҫ���½�������
	 *	 �ȵڶ�����������������¿���һ���Զ�ʱ������lastCommandExecutionState��Ϊ RUNNING
	 * 2.�ڵ�һ������������commandIntervalThresholdʱ����û�еڶ������룬������һ�����뿪����һ���Զ�ʱ������ֹͣ��
	 *   �ö�ʱ����ʱ���ͻ�ִ�е�һ�������Ӧ�����command(S)��command(L)���ڶ�ʱ����ʱ��ǰ���κε����붼����Ч��
	 *   ��Ҫ�ȴ�����ִ����ɺ���ܿ�ʼ�µ����롣
	 * ��commandCurrentLength = commandMaxLength ʱ���������������ֻ��һ��ȥ�򣬼��ȴ�����ִ���꣬����ִ����ǰ�κ����붼����Ч��
	 */
	COMMAND_EXECUTION_STATE lastCommandExecutionState;	/**< ������ʾ�ϸ������ִ��״̬ */
	
	void (*commandFunc[14])(void);	/**< ��ͬ������ִ�в�ͬ�ĺ����������ں����д����������û��Լ����� */
#endif

}Key_Structure, KEY;

/*================================= �������� ==================================*/

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

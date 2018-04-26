/**
* \defgroup �ṹ���Ͷ���
* AVD�㷨���ò�������
* @author �������ټ���
* @version 2.0
* @data 2009-2011
*/
/*@{*/
#ifndef _IMP_AVD_PARA_H_
#define _IMP_AVD_PARA_H_

#include "imp_algo_type.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MODE_CNT    2		// ����ģʽ����
#define LEVEL_CNT	5		// �����ȼ���������Զ��弶��Ϊ0�����������ڴ�
#define THRSHLD_CNT 2		// �÷ֱ仯��ֵ����
#define PARA_CNT	1		// ��⹦�ܵ��ڲ�������

typedef enum impIR_STATE_E
{
	NORMAL_MODE=0,
	IR_MODE
}IR_STATE_E;			// ����״̬ö��

typedef enum impIR_SWITCH_E
{
	EXT_IR_CHG=0,
	AUTO_IR_CHG
}IR_SWITCH_E;			// �������л�������״̬ö��

typedef enum impSCN_CHG_SIGNAL_E
{
	NO_SCN_CHANGE=0,
	SCN_CHANGE
}SCN_CHG_SIGNAL_E;		// �������л��������ź�ö��

typedef enum impSCN_CHG_SWITCH_E
{
	EXT_SCN_CHG=0,
	AUTO_SCN_CHG,
	BOTH_SCN_CHG
}SCN_CHG_SWITCH_E;		// �������л�������״̬ö��
/*
typedef enum impTRANS_THRSHLDS_E
{
	TH1=0,
	TH2
}impTRANS_THRSHLDS_E;	// ת����ֵö��

typedef enum impTRANS_PARAS_E
{
	PARA1=0,
}TRANS_PARAS_E;			// ת������ö��

typedef enum impSENSITY_E
{
	USER_LEVEL=0,
	LEVEL1,
	LEVEL2,
	LEVEL3,
	LEVEL4,
	LEVEL5
}SENSITY_E;			    // ���жȼ���ö��
*/
typedef struct impAVD_BRIGHT_ABMNL_PARA_S
{
	IMP_U32 u32Enable;			// Brightness abormal detection Enable. 0 - disable, 1 - enable. Default 1
	IMP_U32 u32AlarmLevel;		///< �����쳣��⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< �����쳣����ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
	IMP_U32 u32BHUsrNmLTP1;	///< �������ɼ���ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32BHUsrNmLTP2;	///< �������ɼ���ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
	IMP_U32 u32BHUsrIrLTP1;		///< ����������ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32BHUsrIrLTP2;		///< ����������ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
	IMP_U32 u32BLUsrNmLTP1;	///< �������ɼ���ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32BLUsrNmLTP2;	///< �������ɼ���ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
	IMP_U32 u32BLUsrIrLTP1;	///< ����������ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32BLUsrIrLTP2;	///< ����������ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
}IMP_AVD_BRIGHT_ABMNL_PARA_S;

typedef struct impAVD_CLARITY_PARA_S
{
	IMP_U32 u32Enable;			// Clarity abormal detection Enable. 0 - disable, 1 - enable. Default 1
	IMP_U32 u32AlarmLevel;		///< �����ȼ�⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< �����ȼ�ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
	IMP_U32 u32UsrNmLTP1;		///< �����ȼ��ɼ���ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32UsrNmLTP2;		///< �����ȼ��ɼ���ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
	IMP_U32 u32UsrIrLTP1;		///< �����ȼ�����ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32UsrIrLTP2;		///< �����ȼ�����ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
}IMP_AVD_CLARITY_PARA_S;

typedef struct impAVD_NOISE_PARA_S
{
	IMP_U32 u32Enable;			// Noise abormal detection Enable. 0 - disable, 1 - enable. Default 1
	IMP_U32 u32AlarmLevel;		///< ������⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< ������ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
	IMP_U32 u32UsrNmLTP1;		///< �������ɼ���ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32UsrNmLTP2;		///< �������ɼ���ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
	IMP_U32 u32UsrIrLTP1;		///< ����������ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32UsrIrLTP2;		///< ����������ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60
}IMP_AVD_NOISE_PARA_S;

typedef struct impAVD_COLOR_PARA_S
{
	IMP_U32 u32Enable;			///< ƫɫ��⹦�ܿ��أ�0Ϊ�رգ�1Ϊ������Ĭ��Ϊ1
	IMP_U32 u32AlarmLevel;		///< ƫɫ��⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< ƫɫ��ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
	IMP_U32 u32UsrNmLTP1;		///< ƫɫ���ɼ���ģʽ���û��Զ���˫��ֵ��1��ֵ��Ĭ��Ϊ30
	IMP_U32 u32UsrNmLTP2;		///< ƫɫ���ɼ���ģʽ���û��Զ���˫��ֵ��2��ֵ��Ĭ��Ϊ60

}IMP_AVD_COLOR_PARA_S;

typedef struct impAVD_FREEZE_PARA_S
{
	IMP_U32 u32Enable;			///< ���涳���⹦�ܿ��أ�0Ϊ�رգ�1Ϊ������Ĭ��Ϊ1
	IMP_U32 u32AlarmLevel;		///< ���涳���⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< ���涳���ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
}IMP_AVD_FREEZE_PARA_S;

typedef struct impAVD_NOSIGNAL_PARA_S
{
	IMP_U32 u32Enable;			///< �ź�ȱʧ��⹦�ܿ��أ�0Ϊ�رգ�1Ϊ������Ĭ��Ϊ1
	IMP_U32 u32AlarmLevel;		///< �ź�ȱʧ��⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< �ź�ȱʧ��ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
}IMP_AVD_NOSIGNAL_PARA_S;

typedef struct impAVD_CHANGE_PARA_S
{
	IMP_U32 u32Enable;			///< �����任��⹦�ܿ��أ�0Ϊ�رգ�1Ϊ������Ĭ��Ϊ1
	IMP_U32 u32AlarmLevel;		///< �����任��⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
}IMP_AVD_CHANGE_PARA_S;

typedef struct impAVD_INTERFERE_PARA_S
{
	IMP_U32 u32Enable;			///< ��Ϊ���ż�⹦�ܿ��أ�0Ϊ�رգ�1Ϊ������Ĭ��Ϊ1
	IMP_U32 u32AlarmLevel;		///< ��Ϊ���ż�⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< ��Ϊ���ż�ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
}IMP_AVD_INTERFERE_PARA_S;

typedef struct impAVD_PTZ_LOSE_CTL_PARA_S
{
	IMP_U32 u32Enable;			///< PTZʧ�ؼ�⹦�ܿ��أ�0Ϊ�رգ�1Ϊ������Ĭ��Ϊ1
	IMP_U32 u32AlarmLevel;		///< PTZʧ�ؼ�⹦�������ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ�������0��Ϊ�û��Զ���ģʽ��Ĭ�ϵ�λΪ3
	IMP_U32 u32AlarmTime;		///< PTZʧ�ؼ�ⱨ��ʱ�䣬��0-20�룩����ֵԽ����Ҫ�ı���ʱ��Խ����Ĭ��Ϊ5��
}IMP_AVD_PTZ_LOSE_CTL_PARA_S;

typedef struct impAVD_TRIG_MODE_PARA_S
{
	IMP_U32 u32IrTrigMode;		///< �����л�������ʽ��0Ϊ�ֶ�������ʽ��1Ϊ�Զ�������ʽ��Ĭ��Ϊ1
	IMP_U32 u32ScnTrigMode;		///< �����л�������ʽ��0Ϊ�ⲿ������ʽ��1Ϊ�ڲ��Զ�������ʽ��2Ϊ��ͬ������ʽ��Ĭ��Ϊ2
}IMP_AVD_TRIG_MODE_PARA_S;

typedef struct impAVD_IR_MODE_PARA_S
{
	IMP_U32 u32BalanceLevel;	///< �ɼ���-�����Զ��л�ƽ�⼶��1-5������1Ϊ�����ڱ��ж�Ϊ�ɼ���ģʽ��5Ϊ�����ڱ��ж�Ϊ����ģʽ��Ĭ��Ϊ3
}IMP_AVD_IR_MODE_PARA_S;

typedef struct impEXT_TRIGGERS_S
{
	IMP_U32 u32ExtScnChg;		///< �ⲿ�����л�������0Ϊ�������л���1Ϊ�����л�
	IMP_U32 u32ExtIrState;		///< �ⲿ�����л�������0Ϊ�л�Ϊ�ɼ���״̬��1Ϊ�л�Ϊ����״̬
}EXT_TRIGGERS_S;

 
typedef struct impAVD_PARA_S
{
	IMP_AVD_TRIG_MODE_PARA_S stTrigModePara;			///< ������ʽ����
	IMP_AVD_IR_MODE_PARA_S stIrModePara;				///< ����ģʽ������
	IMP_AVD_BRIGHT_ABMNL_PARA_S stBrightAbnmlPara;		///< �����쳣������
	IMP_AVD_CLARITY_PARA_S stClarityPara;				///< �����ȼ�����
	IMP_AVD_NOISE_PARA_S stNoisePara;					///< ����������
	IMP_AVD_COLOR_PARA_S stColorPara;					///< ƫɫ������
	IMP_AVD_FREEZE_PARA_S stFreezePara;					///< ���涳�������
	IMP_AVD_NOSIGNAL_PARA_S stNosignalPara;				///< �ź�ȱʧ������
	IMP_AVD_CHANGE_PARA_S stChangePara;					///< �����任������
	IMP_AVD_INTERFERE_PARA_S stInterferePara;			///< ��Ϊ���ż�����
	IMP_AVD_PTZ_LOSE_CTL_PARA_S stPtzLoseCtlPara;		///< PTZʧ�ؼ�����
}IMP_AVD_PARA_S;


typedef struct tagAVD_RULE_S
{
	int iLevel;						     /// �����ȵ�λ��1-5������ֵԽ��Խ����Խ�ױ���
	int itBrightAbnmlEnable;				///< �����쳣ʹ��
	int iClarityEnable;					///< �����ȼ��ʹ��
	int iNoiseEnable;					///< �������ʹ��
	int iColorEnable;					///< ƫɫ���ʹ��
	int iFreezeEnable;					///< ���涳����ʹ��
	int iNosignalEnable;					///< �ź�ȱʧ���ʹ��
	int iChangeEnable;					///< �����任���ʹ��
	int iInterfereEnable;					///< ��Ϊ���ż��ʹ�� 
	int iPtzLoseCtlEnable;					///< PTZʧ�ؼ��ʹ��
	
}AVD_RULE_S;


#ifdef __cplusplus
}
#endif

#endif /*_IMP_AVD_PARA_H_*/

/*@}*/

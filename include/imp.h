//
//  "$Id: Imp.h 4 2009-08-17 14:09:00Z liwj $"
//
//  Copyright (c)2008-2008, ZheJiang JuFeng Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef __PAL_FLASH_H__
#define __PAL_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif


/// \defgroup FLASH API
/// Flash�豸���ʽӿڣ��������������ڴ˹���
///	\n ��������ͼ
/// \code
///    ===========================
///                |
///            ImpInit
///                |
///            
///                |
///            ImpSetConfig
///                |
///            ImpGetConfig
///                |
///            ImpStart
///                |
///	           ImpPEARead
///                |
///            ImpStop
///                |
///    ===========================
/// \endcode
/// @{

// ----------------------------------------------------------------------
// from IMP header: �������ݽṹ
typedef unsigned char IMP_U8;
typedef unsigned char IMP_UCHAR;
typedef unsigned short IMP_U16;
typedef unsigned int IMP_U32;
typedef char IMP_S8;
typedef short IMP_S16;
typedef int IMP_S32;

// ----------------------------------------------------------------------
// from IMP header: �ܽ����ݽṹ
/** �������� */
typedef enum impURP_SCENE_TYPE_E
{
	IMP_URP_INDOOR = 0,				/**< ���ڳ��� */
	IMP_URP_OUTDOOR					/**< ���ⳡ�� */
} URP_SCENE_TYPE_E;

//�������
typedef enum impURP_CAMERA_TYPE_E
{
	IMP_URP_COMMON_CAMERA = 0,
	IMP_URP_PINHOLE_CAMERA
} URP_CAMERA_TYPE_E;

/** �ܽ���ģʽ */
typedef enum impURP_PERIMETER_MODE_E
{
	IMP_URP_PMODE_INTRUSION = 0,			/**< ���� */
	IMP_URP_PMODE_ENTER,				/**< ���� */
	IMP_URP_PMODE_EXIT				/**< �뿪 */
} URP_PERIMETER_MODE_E;

/** ���߽���� */
#define IMP_MAX_BOUNDARY_POINT_NUM 8

//Added by mzhang 2011-11-14 14:02:48
#define IMP_MAX_POINT_NUM 4
#define IMP_NAME_BUF_SIZE 16

/** ����� */
typedef struct impURP_IMP_POINT_S
{
	IMP_S16 s16X;					/**< x */
	IMP_S16 s16Y;					/**< y */
}URP_IMP_POINT_S;

/** �߽ṹ */
typedef struct impURP_LINE_S
{
	URP_IMP_POINT_S stStartPt;			/**< �߶���� */
	URP_IMP_POINT_S stEndPt;			/**< �߶��յ� */
}URP_LINE_S;

//added by mzhang 2011-11-14 14:00:14
typedef struct impURP_POLYGON_REGION_S
{
	IMP_S32 s32Valid;
	IMP_S32 s32PointNum;
	URP_IMP_POINT_S astPoint[IMP_MAX_BOUNDARY_POINT_NUM];
} URP_POLYGON_REGION_S;

/** URP�ܽ���߽�� */
typedef struct impURP_PERIMETER_LIMIT_BOUNDARY_S
{
	IMP_S32       s32BoundaryPtNum;					/**< �߽����Ŀ */
	URP_IMP_POINT_S   astBoundaryPts[IMP_MAX_BOUNDARY_POINT_NUM];	/**< �߽�㼯�� */
}URP_PERIMETER_LIMIT_BOUNDARY_S;

/** URP�ܽ������Ʋ��� */
typedef struct impURP_PERIMETER_LIMIT_PARA_S
{
	IMP_S32 s32MinDist;						/**< ��С���ؾ��� */
	IMP_S32 s32MinTime;						/**< ���ʱ�� */
	IMP_S32	s32DirectionLimit;					/**< �Ƿ����������� */
	IMP_S32 s32ForbiddenDirection;					/**< ��ֹ����Ƕ�(��λ: �Ƕ�) */
	URP_PERIMETER_LIMIT_BOUNDARY_S stBoundary;			/**< �ܽ�߽����� */
}URP_PERIMETER_LIMIT_PARA_S;

/** URP�ܽ��������� */
typedef struct impURP_PERIMETER_RULE_PARA_S
{
	IMP_S32 s32TypeLimit;						/**< Ŀ���������� */
	IMP_S32 s32TypeHuman;						/**< Ŀ�������(�޶�����ʱʹ��) */
	IMP_S32 s32TypeVehicle;						/**< Ŀ����𣺳� */
	IMP_S32 s32Mode;						/**< �ܽ�ģʽ */
	URP_PERIMETER_LIMIT_PARA_S stLimitPara;				/**< �ܽ����Ʋ��� */
} URP_PERIMETER_RULE_PARA_S;

// ----------------------------------------------------------------------
// from IMP header: �������ݽṹ
/** URP������������� */
#define IMP_URP_MAX_TRIPWIRE_CNT 8

/** URP�����߽ṹ */
typedef struct impURP_TRIPWIRE_S
{
	IMP_S32     s32Valid;						/**< �Ƿ���Ч */
	IMP_S32     s32IsDoubleDirection;				/**< ��ʾ�ð����Ƿ�Ϊ˫�����(0: ��, 1: ��) */
	IMP_S32     s32ForbiddenDirection;				/**< �����߽�ֹ����Ƕ�(��λ: ��) */
	URP_LINE_S	stLine;						/**< ������λ�� */
}URP_TRIPWIRE_S;

/** URP���������Ʋ��� */
typedef struct impURP_TRIPWIRE_LIMIT_PARA_S
{
	IMP_S32 s32MinDist;						/**< ��С���ؾ��� */
	IMP_S32 s32MinTime;						/**< ���ʱ�� */
}URP_TRIPWIRE_LIMIT_PARA_S;

/** URP�����߹������ */
typedef struct impURP_TRIPWIRE_RULE_PARA_S
{
	IMP_S32 s32TypeLimit;						/**< Ŀ���������� */
	IMP_S32 s32TypeHuman;						/**< Ŀ�������(�޶�����ʱʹ��) */
	IMP_S32 s32TypeVehicle;						/**< Ŀ����𣺳� */
	URP_TRIPWIRE_S astLines[IMP_URP_MAX_TRIPWIRE_CNT];		/**< ������ */
	URP_TRIPWIRE_LIMIT_PARA_S stLimitPara;				/**< ���������Ʋ��� */
} URP_TRIPWIRE_RULE_PARA_S;


/** URPĿ����������ȼ��� */
typedef enum impSENSITIVITY_LEVEL_E
{
	IMP_HIGH_LEVEL_SENSITIVITY,         /**< �������� */
	IMP_MIDDLE_LEVEL_SENSITIVITY,		/**< �������� */
	IMP_LOW_LEVEL_SENSITIVITY			/**< �������� */
}SENSITIVITY_LEVEL_E;

/** �㷨ģ�� */
typedef enum impALGO_MODULE_E
{
	IMP_NONE_AGLO_MODULE= 0x00000000,		/**< �㷨ģ�� */
	IMP_PEA_AGLO_MODULE = 0x00000001,		/**< PEA�㷨ģ�� */
	IMP_AVD_AGLO_MODULE = 0x00000002,		/**< AVD�㷨ģ�� */
	IMP_VFD_AGLO_MODULE = 0x00000004,		/**< VFD�㷨ģ�� */
	IMP_AAI_AGLO_MODULE = 0x00000008,		/**< AAI�㷨ģ�� */
	IMP_OSC_AGLO_MODULE = 0x00000010,		/**< osc�㷨ģ�� */
	IMP_CPC_AGLO_MODULE = 0x00000012		/**< cpc�㷨ģ�� */
}ALGO_MODULE_E;

// ----------------------------------------------------------------------
// ���濪ʼ���Զ���ṹ
#define MAX_IMP_CHANNEL			1			// ����ͨ����

typedef struct tagPEA_RULE_S
{
	int iShowTrack;						/// �Ƿ���ʾ�켣
	int iShowRule;						/// �Ƿ���ʾ����
	int iLevel;						/// ���伶��0���Ǿ�����Ĭ��ֵ����1���ͼ���2���м���3: �߼�
	int iPerimeterEnable;					/// �ܽ����ʹ��
	//PERIMETER_RULE_S  stPerimeter;			/// �ܽ����
	URP_PERIMETER_RULE_PARA_S	stPerimeterRulePara;	/**< �ܽ��� */
	int iTripWireEnable;					/// �����߹���ʹ��
	//TRIPWIRE_RULE_PARA_S  stTripWire;			/// �����߹���
	URP_TRIPWIRE_RULE_PARA_S	stTripwireRulePara;	/**< �����߼�� */
}PEA_RULE_S;

/// IMP�������ṹ
typedef struct IMP_CAPS					/// ��1��ʾ��ͨ��֧�ֶ�Ӧ�������㷨����0��ʾ��֧�֡�
{
	int dwPEA;
	int dwAVD;
	int dwVFD;
	int dwAAI;
	int dwOSC;
	int dwCPC;
	int iResv[2];
} IMP_CAPS;

typedef struct tag_STATUS_S
{
	unsigned int dwImpStatus;	//Imp״̬���룬��pea��osc�б�ʾ��Ӧ����ͨ���ţ���avd�б�ʾ��Ӧ��������.
								
	int iResv[4];

}STATUS_S;

typedef struct impCPC_RESULT_NUM
{
	int inNum;
	int outNum;
	int sumNum;
} CPC_RESULT_NUM;





/// ��ʼ��IMP���㷨����
/// \param [in] nChannel ͨ����
/// \param [in] ImpModule �㷨���͡�
/// \return 0 ��ʼ���ɹ�
/// \return < 0��ʼ��ʧ��
int ImpInit(int nChannel, ALGO_MODULE_E ImpModule);

/// ����IMP���㷨
/// \return 0 ��ʼ���ɹ�
int ImpDestroy();

/// ��ʼ��IMP���㷨����
/// \param [in] nChannel ͨ����
/// \param [in] ImpModule �㷨���͡�
/// \return 0 ��ʼ���ɹ�
/// \return < 0��ʼ��ʧ��

int ImpGetCaps(IMP_CAPS *pCaps);

/// ����OSC�Ĺ���
/// \param [in] nChannel ͨ����
/// \param [in] PEA_RULE_S PEA����
/// \retval 0  �ɹ�
/// \retval <0  ʧ��
//int	ImpSetOSCConfig(int nChannel, OSC_RULE_S *pRule);

/// ����AVD�Ĺ���
/// \param [in] nChannel ͨ����
/// \param [in] PEA_RULE_S PEA����
/// \retval 0  �ɹ�
/// \retval <0  ʧ��
//int	ImpSetAVDConfig(int nChannel, AVD_RULE_S *pRule);

/// ����PEA�Ĺ���
/// \param [in] nChannel ͨ����
/// \param [in] PEA_RULE_S PEA����
/// \retval 0  �ɹ�
/// \retval <0  ʧ��
int	ImpSetPEAConfig(int nChannel, PEA_RULE_S *pRule);



/// ��ȡPEA�Ĺ���
/// \param [in] nChannel ͨ����
/// \param [in] PEA_RULE_S PEA����
/// \retval 0  �ɹ�
/// \retval <0  ʧ��
int	ImpGetPEAConfig(int nChannel, PEA_RULE_S *pRule);

/// ��������ʶ��
/// \param [in] nChannel ͨ����
/// \retval 0  �ɹ�
/// \retval <0 ʧ��
int	ImpStart(int nChannel);

/// �ر�����ʶ��
/// \param [in] nChannel ͨ����
/// \retval 0  �ɹ�
/// \retval <0 ʧ��
int ImpStop(int nChannel);

/// ������ʶ�������״̬
/// \param [out] pPEAStatus PEA�ı���״̬
/// \retval =0 �ɹ�
/// \retval <0 ��ʧ��

//int ImpRead(STATUS_S *pStatus,int nChannel);

int ImpCpcRead(int nChannel,CPC_RESULT_NUM *pStatus);


/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //__FLASH_API_H__


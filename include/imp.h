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
/// Flash设备访问接口，升级管理器基于此构建
///	\n 调用流程图
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
// from IMP header: 基础数据结构
typedef unsigned char IMP_U8;
typedef unsigned char IMP_UCHAR;
typedef unsigned short IMP_U16;
typedef unsigned int IMP_U32;
typedef char IMP_S8;
typedef short IMP_S16;
typedef int IMP_S32;

// ----------------------------------------------------------------------
// from IMP header: 周界数据结构
/** 场景类型 */
typedef enum impURP_SCENE_TYPE_E
{
	IMP_URP_INDOOR = 0,				/**< 室内场景 */
	IMP_URP_OUTDOOR					/**< 室外场景 */
} URP_SCENE_TYPE_E;

//相机类型
typedef enum impURP_CAMERA_TYPE_E
{
	IMP_URP_COMMON_CAMERA = 0,
	IMP_URP_PINHOLE_CAMERA
} URP_CAMERA_TYPE_E;

/** 周界检测模式 */
typedef enum impURP_PERIMETER_MODE_E
{
	IMP_URP_PMODE_INTRUSION = 0,			/**< 入侵 */
	IMP_URP_PMODE_ENTER,				/**< 进入 */
	IMP_URP_PMODE_EXIT				/**< 离开 */
} URP_PERIMETER_MODE_E;

/** 最大边界点数 */
#define IMP_MAX_BOUNDARY_POINT_NUM 8

//Added by mzhang 2011-11-14 14:02:48
#define IMP_MAX_POINT_NUM 4
#define IMP_NAME_BUF_SIZE 16

/** 坐标点 */
typedef struct impURP_IMP_POINT_S
{
	IMP_S16 s16X;					/**< x */
	IMP_S16 s16Y;					/**< y */
}URP_IMP_POINT_S;

/** 线结构 */
typedef struct impURP_LINE_S
{
	URP_IMP_POINT_S stStartPt;			/**< 线段起点 */
	URP_IMP_POINT_S stEndPt;			/**< 线段终点 */
}URP_LINE_S;

//added by mzhang 2011-11-14 14:00:14
typedef struct impURP_POLYGON_REGION_S
{
	IMP_S32 s32Valid;
	IMP_S32 s32PointNum;
	URP_IMP_POINT_S astPoint[IMP_MAX_BOUNDARY_POINT_NUM];
} URP_POLYGON_REGION_S;

/** URP周界检测边界点 */
typedef struct impURP_PERIMETER_LIMIT_BOUNDARY_S
{
	IMP_S32       s32BoundaryPtNum;					/**< 边界点数目 */
	URP_IMP_POINT_S   astBoundaryPts[IMP_MAX_BOUNDARY_POINT_NUM];	/**< 边界点集合 */
}URP_PERIMETER_LIMIT_BOUNDARY_S;

/** URP周界检测限制参数 */
typedef struct impURP_PERIMETER_LIMIT_PARA_S
{
	IMP_S32 s32MinDist;						/**< 最小像素距离 */
	IMP_S32 s32MinTime;						/**< 最短时间 */
	IMP_S32	s32DirectionLimit;					/**< 是否做方向限制 */
	IMP_S32 s32ForbiddenDirection;					/**< 禁止方向角度(单位: 角度) */
	URP_PERIMETER_LIMIT_BOUNDARY_S stBoundary;			/**< 周界边界区域 */
}URP_PERIMETER_LIMIT_PARA_S;

/** URP周界检测规则参数 */
typedef struct impURP_PERIMETER_RULE_PARA_S
{
	IMP_S32 s32TypeLimit;						/**< 目标类型限制 */
	IMP_S32 s32TypeHuman;						/**< 目标类别：人(限定类型时使用) */
	IMP_S32 s32TypeVehicle;						/**< 目标类别：车 */
	IMP_S32 s32Mode;						/**< 周界模式 */
	URP_PERIMETER_LIMIT_PARA_S stLimitPara;				/**< 周界限制参数 */
} URP_PERIMETER_RULE_PARA_S;

// ----------------------------------------------------------------------
// from IMP header: 绊线数据结构
/** URP单绊线最多条数 */
#define IMP_URP_MAX_TRIPWIRE_CNT 8

/** URP单绊线结构 */
typedef struct impURP_TRIPWIRE_S
{
	IMP_S32     s32Valid;						/**< 是否有效 */
	IMP_S32     s32IsDoubleDirection;				/**< 表示该绊线是否为双向绊线(0: 否, 1: 是) */
	IMP_S32     s32ForbiddenDirection;				/**< 单绊线禁止方向角度(单位: 度) */
	URP_LINE_S	stLine;						/**< 单绊线位置 */
}URP_TRIPWIRE_S;

/** URP单绊线限制参数 */
typedef struct impURP_TRIPWIRE_LIMIT_PARA_S
{
	IMP_S32 s32MinDist;						/**< 最小像素距离 */
	IMP_S32 s32MinTime;						/**< 最短时间 */
}URP_TRIPWIRE_LIMIT_PARA_S;

/** URP单绊线规则参数 */
typedef struct impURP_TRIPWIRE_RULE_PARA_S
{
	IMP_S32 s32TypeLimit;						/**< 目标类型限制 */
	IMP_S32 s32TypeHuman;						/**< 目标类别：人(限定类型时使用) */
	IMP_S32 s32TypeVehicle;						/**< 目标类别：车 */
	URP_TRIPWIRE_S astLines[IMP_URP_MAX_TRIPWIRE_CNT];		/**< 单绊线 */
	URP_TRIPWIRE_LIMIT_PARA_S stLimitPara;				/**< 单绊线限制参数 */
} URP_TRIPWIRE_RULE_PARA_S;


/** URP目标输出灵敏度级别 */
typedef enum impSENSITIVITY_LEVEL_E
{
	IMP_HIGH_LEVEL_SENSITIVITY,         /**< 高灵敏度 */
	IMP_MIDDLE_LEVEL_SENSITIVITY,		/**< 中灵敏度 */
	IMP_LOW_LEVEL_SENSITIVITY			/**< 低灵敏度 */
}SENSITIVITY_LEVEL_E;

/** 算法模块 */
typedef enum impALGO_MODULE_E
{
	IMP_NONE_AGLO_MODULE= 0x00000000,		/**< 算法模块 */
	IMP_PEA_AGLO_MODULE = 0x00000001,		/**< PEA算法模块 */
	IMP_AVD_AGLO_MODULE = 0x00000002,		/**< AVD算法模块 */
	IMP_VFD_AGLO_MODULE = 0x00000004,		/**< VFD算法模块 */
	IMP_AAI_AGLO_MODULE = 0x00000008,		/**< AAI算法模块 */
	IMP_OSC_AGLO_MODULE = 0x00000010,		/**< osc算法模块 */
	IMP_CPC_AGLO_MODULE = 0x00000012		/**< cpc算法模块 */
}ALGO_MODULE_E;

// ----------------------------------------------------------------------
// 下面开始是自定义结构
#define MAX_IMP_CHANNEL			1			// 智能通道数

typedef struct tagPEA_RULE_S
{
	int iShowTrack;						/// 是否显示轨迹
	int iShowRule;						/// 是否显示规则
	int iLevel;						/// 警戒级别0：非警报（默认值）；1：低级；2：中级；3: 高级
	int iPerimeterEnable;					/// 周界规则使能
	//PERIMETER_RULE_S  stPerimeter;			/// 周界规则
	URP_PERIMETER_RULE_PARA_S	stPerimeterRulePara;	/**< 周界检测 */
	int iTripWireEnable;					/// 单绊线规则使能
	//TRIPWIRE_RULE_PARA_S  stTripWire;			/// 单绊线规则
	URP_TRIPWIRE_RULE_PARA_S	stTripwireRulePara;	/**< 单绊线检测 */
}PEA_RULE_S;

/// IMP的能力结构
typedef struct IMP_CAPS					/// 置1表示该通道支持对应的智能算法，置0表示不支持。
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
	unsigned int dwImpStatus;	//Imp状态掩码，在pea和osc中表示对应报警通道号，在avd中表示对应报警类型.
								
	int iResv[4];

}STATUS_S;

typedef struct impCPC_RESULT_NUM
{
	int inNum;
	int outNum;
	int sumNum;
} CPC_RESULT_NUM;





/// 初始化IMP的算法类型
/// \param [in] nChannel 通道号
/// \param [in] ImpModule 算法类型。
/// \return 0 初始化成功
/// \return < 0初始化失败
int ImpInit(int nChannel, ALGO_MODULE_E ImpModule);

/// 销毁IMP的算法
/// \return 0 初始化成功
int ImpDestroy();

/// 初始化IMP的算法类型
/// \param [in] nChannel 通道号
/// \param [in] ImpModule 算法类型。
/// \return 0 初始化成功
/// \return < 0初始化失败

int ImpGetCaps(IMP_CAPS *pCaps);

/// 设置OSC的规则
/// \param [in] nChannel 通道号
/// \param [in] PEA_RULE_S PEA规则
/// \retval 0  成功
/// \retval <0  失败
//int	ImpSetOSCConfig(int nChannel, OSC_RULE_S *pRule);

/// 设置AVD的规则
/// \param [in] nChannel 通道号
/// \param [in] PEA_RULE_S PEA规则
/// \retval 0  成功
/// \retval <0  失败
//int	ImpSetAVDConfig(int nChannel, AVD_RULE_S *pRule);

/// 设置PEA的规则
/// \param [in] nChannel 通道号
/// \param [in] PEA_RULE_S PEA规则
/// \retval 0  成功
/// \retval <0  失败
int	ImpSetPEAConfig(int nChannel, PEA_RULE_S *pRule);



/// 获取PEA的规则
/// \param [in] nChannel 通道号
/// \param [in] PEA_RULE_S PEA规则
/// \retval 0  成功
/// \retval <0  失败
int	ImpGetPEAConfig(int nChannel, PEA_RULE_S *pRule);

/// 开启智能识别
/// \param [in] nChannel 通道号
/// \retval 0  成功
/// \retval <0 失败
int	ImpStart(int nChannel);

/// 关闭智能识别
/// \param [in] nChannel 通道号
/// \retval 0  成功
/// \retval <0 失败
int ImpStop(int nChannel);

/// 读智能识别的输入状态
/// \param [out] pPEAStatus PEA的报警状态
/// \retval =0 成功
/// \retval <0 读失败

//int ImpRead(STATUS_S *pStatus,int nChannel);

int ImpCpcRead(int nChannel,CPC_RESULT_NUM *pStatus);


/// @} end of group

#ifdef __cplusplus
}
#endif

#endif //__FLASH_API_H__


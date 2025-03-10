#ifndef _libzkfp_h
#define _libzkfp_h
/**
*	@file		libzkfp.h
*	@brief		�ӿڶ���
*	@author		scar chen
*	@date		2016-04-12
*	@version	5.0
*	@par	��Ȩ��
*				ZKTeco
*	@par	��ʷ�汾			
*
*	@note
*
*/


#include "libzkfptype.h"
#include "zkinterface.h"


typedef enum LOG_LEVEL {
   LOG_VERBOSE  = 2,
   LOG_DEBUG    = 3,
   LOG_INFO     = 4,
   LOG_WARN     = 5,
   LOG_ERROR    = 6,
   LOG_ASSERT   = 7,
} Log_Level;
typedef enum LOG_TYPE{
    LOG_TYPE_CONSOLE	=	1,	//console(android logcat)
    LOG_TYPE_FILE       =	2,
    LOG_TYPE_DBGVIEW	=	3,	//windows only
}Log_Type;


#ifdef __cplusplus
extern "C"
{
#endif

#define ZKFP_DLOPEN
#ifdef ZKFP_DLOPEN

typedef int (*T_ZKFPM_Init)();
typedef int (*T_ZKFPM_Terminate)();
typedef int (*T_ZKFPM_GetDeviceCount)();
typedef HANDLE (*T_ZKFPM_OpenDevice)(int index);
typedef int (*T_ZKFPM_CloseDevice)(HANDLE hDevice);
typedef int (*T_ZKFPM_SetParameters)(HANDLE hDevice, int nParamCode, unsigned char* paramValue, unsigned int cbParamValue);
typedef int (*T_ZKFPM_GetParameters)(HANDLE hDevice, int nParamCode, unsigned char* paramValue, unsigned int* cbParamValue);
typedef int (*T_ZKFPM_AcquireFingerprint)(HANDLE hDevice, unsigned char* fpImage, unsigned int cbFPImage, unsigned char* fpTemplate, unsigned int* cbTemplate);
typedef int (*T_ZKFPM_AcquireFingerprintImage)(HANDLE hDevice, unsigned char* fpImage, unsigned int cbFPImage);



typedef HANDLE (*T_ZKFPM_DBInit)();
typedef int (*T_ZKFPM_DBFree)(HANDLE hDBCache);
typedef int (*T_ZKFPM_DBMerge)(HANDLE hDBCache, unsigned char* temp1, unsigned char* temp2, unsigned char* temp3, unsigned char* regTemp, unsigned int* cbRegTemp);
typedef int (*T_ZKFPM_DBAdd)(HANDLE hDBCache, unsigned int fid, unsigned char* fpTemplate, unsigned int cbTemplate);
typedef int (*T_ZKFPM_DBDel)(HANDLE hDBCache, unsigned int fid);
typedef int (*T_ZKFPM_DBClear)(HANDLE hDBCache);
typedef int (*T_ZKFPM_DBCount)(HANDLE hDBCache, unsigned int* fpCount);
typedef int (*T_ZKFPM_DBIdentify)(HANDLE hDBCache, unsigned char* fpTemplate, unsigned int cbTemplate, unsigned int* FID, unsigned int* score);
typedef int (*T_ZKFPM_DBMatch)(HANDLE hDBCache, unsigned char* template1, unsigned int cbTemplate1, unsigned char* template2, unsigned int cbTemplate2);
typedef int (*T_ZKFPM_ExtractFromImage)(HANDLE hDBCache, const char* lpFilePathName, unsigned int DPI, unsigned char* fpTemplate, unsigned int *cbTemplate);

typedef void (*T_ZKFPM_SetLogLevel)(int nLevel);
typedef void (*T_ZKFPM_ConfigLog)(int nLevel, int nType, char *fileName);


EXTERN_SDK_FUN T_ZKFPM_Init ZKFPM_Init;
EXTERN_SDK_FUN T_ZKFPM_Terminate ZKFPM_Terminate;
EXTERN_SDK_FUN T_ZKFPM_GetDeviceCount ZKFPM_GetDeviceCount;
EXTERN_SDK_FUN T_ZKFPM_OpenDevice ZKFPM_OpenDevice;
EXTERN_SDK_FUN T_ZKFPM_CloseDevice ZKFPM_CloseDevice;
EXTERN_SDK_FUN T_ZKFPM_SetParameters ZKFPM_SetParameters;
EXTERN_SDK_FUN T_ZKFPM_GetParameters ZKFPM_GetParameters;
EXTERN_SDK_FUN T_ZKFPM_AcquireFingerprint ZKFPM_AcquireFingerprint;
EXTERN_SDK_FUN T_ZKFPM_AcquireFingerprintImage ZKFPM_AcquireFingerprintImage;

EXTERN_SDK_FUN T_ZKFPM_DBInit ZKFPM_DBInit;
EXTERN_SDK_FUN T_ZKFPM_DBFree ZKFPM_DBFree;
EXTERN_SDK_FUN T_ZKFPM_DBMerge ZKFPM_DBMerge;
EXTERN_SDK_FUN T_ZKFPM_DBAdd ZKFPM_DBAdd;
EXTERN_SDK_FUN T_ZKFPM_DBDel ZKFPM_DBDel;
EXTERN_SDK_FUN T_ZKFPM_DBClear ZKFPM_DBClear;
EXTERN_SDK_FUN T_ZKFPM_DBCount ZKFPM_DBCount;
EXTERN_SDK_FUN T_ZKFPM_DBIdentify ZKFPM_DBIdentify;
EXTERN_SDK_FUN T_ZKFPM_DBMatch ZKFPM_DBMatch;
EXTERN_SDK_FUN T_ZKFPM_ExtractFromImage ZKFPM_ExtractFromImage;

EXTERN_SDK_FUN T_ZKFPM_SetLogLevel ZKFPM_SetLogLevel;
EXTERN_SDK_FUN T_ZKFPM_ConfigLog ZKFPM_ConfigLog;

#else
/**
	*	@brief	ʼ
	*	@param	:
	*		��
	*	@return
	*	����ֵ˵�����£�
	*	value			|	type		|	description of value
	*	----------------|---------------|-------------------------------
	*	0				|	int			|	�ɹ�
	*	����			|	int			|	ʧ��
	*	@note 
*/
ZKINTERFACE int APICALL ZKFPM_Init();

/**
	*	@brief	�ͷſ�
	*	@param	:
	*		��
	*	@return
	*	����ֵ˵�����£�
	*	value			|	type		|	description of value
	*	----------------|---------------|-------------------------------
	*	0				|	int			|	�ɹ�
	*	����			|	int			|	ʧ��
	*	@note 
*/
ZKINTERFACE int APICALL ZKFPM_Terminate();

/**
	*	@brief	��ȡ�豸��
	*	@param	:
	*		��
	*	@return
	*	����ֵ˵�����£�
	*		��ǰ����ָ�������豸��
	*	@note 
*/
ZKINTERFACE int APICALL ZKFPM_GetDeviceCount();

/**
		*	@brief	���豸
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	index			|	int			  |	[in]				|	�豸����
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	NULL			|	HANDLE		|	ʧ��
		*	����			|	HANDLE		|	�ɹ�
		*	@note 
	*/
	ZKINTERFACE HANDLE APICALL ZKFPM_OpenDevice(int index);

	/**
		*	@brief	�ر��豸
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDevice			|	HANDLE		  |	[in]				|	�豸����ʵ��ָ��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_CloseDevice(HANDLE hDevice);

	/**
		*	@brief	���ò���
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDevice			|	HANDLE		  |	[in]				|	�豸����ʵ��ָ��
		*	nParamCode		|	int			  |	[in]				|	��������
		*	paramValue		|	unsigned char*|	[in]				|	����ֵ
		*	cbParamValue	|	unsigned int  |	[in]				|	�������ݳ���
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_SetParameters(HANDLE hDevice, int nParamCode, unsigned char* paramValue, unsigned int cbParamValue);

	/**
		*	@brief	��ȡ����
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDevice			|	HANDLE		  |	[in]				|	�豸����ʵ��ָ��
		*	nParamCode		|	int			  |	[in]				|	��������
		*	paramValue		|	unsigned char*|	[out]				|	����ֵ
		*	cbParamValue	|	unsigned int* |	[out]				|	�������ݳ���
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_GetParameters(HANDLE hDevice, int nParamCode, unsigned char* paramValue, unsigned int* cbParamValue);
	
	/**
		*	@brief	��ȡָ��(ͼ��/ģ�壩
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDevice			|	HANDLE		  |	[in]				|	�豸����ʵ��ָ��
		*	fpImage			|	unsigned char*|	[out]				|	ָ��ͼ��
		*	cbFPImage		|	unsigned int  |	[in]				|	fpImage�ڴ��С
		*	fpTemplate		|	unsigned char*|	[out]				|	ָ��ģ��
		*	cbTemplate		|	unsigned int* |	[in/out]			|	ָ��ģ�峤��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_AcquireFingerprint(HANDLE hDevice, unsigned char* fpImage, unsigned int cbFPImage, unsigned char* fpTemplate, unsigned int* cbTemplate);
		
	/**
		*	@brief	��ȡָ��ͼ��
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDevice			|	HANDLE		  |	[in]				|	�豸����ʵ��ָ��
		*	fpImage			|	unsigned char*|	[out]				|	ָ��ͼ��
		*	cbFPImage		|	unsigned int  |	[in]				|	fpImage�ڴ��С
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_AcquireFingerprintImage(HANDLE hDevice, unsigned char* fpImage, unsigned int cbFPImage);

	/**
		*	@brief	�����㷨����ʵ��
		*	@param	:
		*	��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	NULL			|	HANDLE		|	ʧ��
		*	����			|	HANDLE		|	�ɹ�
		*	@note 
	*/
	ZKINTERFACE HANDLE APICALL ZKFPM_DBInit();	

	/**
		*	@brief	�ͷ��㷨����ʵ��
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBFree(HANDLE hDBCache); 

	ZKINTERFACE int APICALL ZKFPM_DBSetParameter(HANDLE hDBCache, int nParamCode, unsigned char* paramValue, unsigned int cbParamValue);
	ZKINTERFACE int APICALL ZKFPM_DBGetParameter(HANDLE hDBCache, int nParamCode, unsigned char* paramValue, unsigned int cbParamValue);


	/**
		*	@brief	��3��ָ��ģ��ϳɵǼ�ģ��
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	temp1			|	unsigned char*|	[in]				|	ָ��ģ��1
		*	temp2			|	unsigned char*|	[in]				|	ָ��ģ��2
		*	temp3			|	unsigned char*|	[in]				|	ָ��ģ��3
		*	regTemp			|	unsigned char*|	[out]				|	�Ǽ�ģ��
		*	cbRegTemp		|	unsigned int* |	[in/out]			|	�Ǽ�ģ�峤��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBMerge(HANDLE hDBCache, unsigned char* temp1, unsigned char* temp2, unsigned char* temp3, unsigned char* regTemp, unsigned int* cbRegTemp);	

	/**
		*	@brief	����ָ��ģ�嵽����
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	fid				|	unsigned int  |	[in]				|	��ָID
		*	fpTemplate		|	unsigned char*|	[in]				|	ָ��ģ��
		*	cbTemplate		|	unsigned int  |	[in]				|	ָ��ģ�峤��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBAdd(HANDLE hDBCache, unsigned int fid, unsigned char* fpTemplate, unsigned int cbTemplate);

	/**
		*	@brief	�ӻ���ɾ��ָ��ģ��
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	fid				|	unsigned int  |	[in]				|	��ָID
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBDel(HANDLE hDBCache, unsigned int fid);

	/**
		*	@brief	����㷨����
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBClear(HANDLE hDBCache);	

	/**
		*	@brief	��ȡ����ģ����
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	fpCount			|	unsigned int* |	[out]				|	ָ��ģ����
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBCount(HANDLE hDBCache, unsigned int* fpCount);	

	/**
		*	@brief	ָ��ʶ��(1:N)
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	fpTemplate		|	unsigned char*|	[in]				|	ָ��ģ��
		*	cbTemplate		|	unsigned int  | [in]				|	ָ��ģ���С
		*	FID				|	unsigned int* |	[out]				|	ָ����ID
		*	score			|	unsigned int* |	[out]				|	����
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	0				|	int			|	�ɹ�
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBIdentify(HANDLE hDBCache, unsigned char* fpTemplate, unsigned int cbTemplate, unsigned int* FID, unsigned int* score);


	/**
		*	@brief	�ȶ���öָ��
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	template1		|	unsigned char*|	[in]				|	ָ��ģ��1
		*	cbTemplate1		|	unsigned int  | [in]				|	ָ��ģ��1��С
		*	template2		|	unsigned char*|	[in]				|	ָ��ģ��2
		*	cbTemplate2		|	unsigned int  | [in]				|	ָ��ģ��2��С
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	>0				|	int			|	����
		*	����			|	int			|	ʧ��
		*	@note 
	*/
	ZKINTERFACE int APICALL ZKFPM_DBMatch(HANDLE hDBCache, unsigned char* template1, unsigned int cbTemplate1, unsigned char* template2, unsigned int cbTemplate2);

	/**
		*	@brief	��Bitmap�ļ���ȡָ��ģ��
		*	@param	:
		*	����˵�����±�
		*	name			|	type		  |	param direction		|	description of param
		*	----------------|-----------------|---------------------|------------------------
		*	hDBCache		|	HANDLE		  |	[in]				|	�㷨����ʵ��ָ��
		*	lpFilePathName	|	const char*   |	[in]				|	BMPͼƬ·��
		*	DPI				|	unsigned int  | [in]				|	BMPͼƬDPI
		*	fpTemplate		|	unsigned char*|	[out]				|	ָ��ģ��
		*	cbTemplate		|	unsigned int* |	[in/out]			|	ģ�峤��
		*	@return
		*	����ֵ˵�����£�
		*	value			|	type		|	description of value
		*	----------------|---------------|-------------------------------
		*	>0				|	int			|	����
		*	����			|	int			|	ʧ��
		*	@note 
	*/
        ZKINTERFACE int APICALL ZKFPM_ExtractFromImage(HANDLE hDBCache, const char* lpFilePathName, unsigned int DPI, unsigned char* fpTemplate, unsigned int *cbTemplate);
	

        ZKINTERFACE void APICALL ZKFPM_SetLogLevel(int nLevel);
        ZKINTERFACE void APICALL ZKFPM_ConfigLog(int nLevel, int nType, char *fileName);

#endif	

#ifdef __cplusplus
};
#endif


#endif	//_libzkfp_h

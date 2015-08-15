#include "CameraParam.h"
#include "FileOpr.h"

extern T_ServerParam SerParam;

CamParam::CamParam(uint32 ID,SingleCamera *sincam)
{
  	CameraID = ID;
  	SinCam = sincam;
  	memset(CameraIP,0,IP_LEN_16);
	frameRate = 0;
	memset(CamUrl,0,SINGLE_URL_LEN_128);
	memset(RtspUrl,0,SINGLE_URL_LEN_128);
	CameraFunc = 0;
	AnalyzeNUM = 0;
	AnalyzeType = 0;
	AnalyzeType1 = 0;
	AnalyzeType2 = 0;
}

CamParam::~CamParam()
{

}
string CamParam::get_camera_rtsp_url()
{
	string url;
	url = RtspUrl;
	return url;
}

int CamParam::generate_url()
{
	char rtsp[128] ={0};
	uint32 port =558;
	if(strlen(SerParam.Serverip) == 0) return -1;
	sprintf(rtsp,"%s%s:%d/%d","rtsp://",SerParam.Serverip,port,CameraID);
	memcpy(RtspUrl,rtspL,SINGLE_URL_LEN_128);
	return 0;
}





int CamParam::set(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	int iRet = -1; 
	memset(CameraIP,addCam.ip,IP_LEN_16);
	memcpy(CamUrl,addCam.CameUrL,SINGLE_URL_LEN_128);
	frameRate 	= addCam.frameRate;
	CameraFunc 	= addCam.CameraFunc;
	AnalyzeNUM    = addCam.AnalyzeNUM;
	AnalyzeType   = addCam.AnalyzeType;
	generate_url();
	switch (AnalyzeNUM){
		case 1:
			AnalyzeType1 =check_analyzetype(AnalyzeType);
			if(AnalyzeType1 == 0) AnalyzeNUM = 0;
			break;
		case 2:
			parse_type(AnalyzeType);
			break;
		default :
			dbgprint("%s(%d),wrong AnalyzeNUM and AnalyzeNUM is %d!\n", DEBUGARGS,AnalyzeNUM);
		 	break;
	}
	//start thread
	T_CAM_PARAM t_cam_param;
	memcpy(t_cam_param.url,addCam.CameUrL,SINGLE_URL_LEN_128);
	
	t_cam_param.CameraFunc =   CameraFunc; 
	t_cam_param.AnalyzeNUM  =   AnalyzeNUM;

	t_cam_param.AnalyzeType1 = AnalyzeType1;
	t_cam_param.AnalyzeType2 = AnalyzeType2;
	
	iRet = SinCam->Ana->start_all_thread(&t_cam_param);  /*start with stop*/
	if(iRet < 0 ){
		dbgprint("%s(%d),start cam thread failure!\n", DEBUGARGS);
		return -1;	
	}
	return 0;
}

void CamParam::reset(ST_VDCS_VIDEO_PUSH_CAM & addCam)
{
	memset(CameraIP,0,IP_LEN_16);
	frameRate = 0;
	memset(CamUrl,0,SINGLE_URL_LEN_128);
	memset(RtspUrl,0,SINGLE_URL_LEN_128);
	CameraFunc = 0;
	AnalyzeNUM = 0;
	AnalyzeType = 0;
	AnalyzeType1 = 0;
	AnalyzeType2 = 0;


	memset(CameraIP,addCam.ip,IP_LEN_16);
	memcpy(CamUrl,addCam.CameUrL,SINGLE_URL_LEN_128);
	frameRate 	= addCam.frameRate;
	CameraFunc 	= addCam.CameraFunc;
	AnalyzeNUM    = addCam.AnalyzeNUM;
	AnalyzeType   = addCam.AnalyzeType;
	generate_url();
	switch (AnalyzeNUM){
		case 1:
			AnalyzeType1 =check_analyzetype(AnalyzeType);
			if(AnalyzeType1 == 0) AnalyzeNUM = 0;
			break;
		case 2:
			parse_type(AnalyzeType);
			break;
		default :
			dbgprint("%s(%d),wrong AnalyzeNUM and AnalyzeNUM is %d!\n", DEBUGARGS,AnalyzeNUM);
		 	break;
	}
}



#include "CamAnaThread.h"

CamAnaThread::CamAnaThread()
{
  m_AnaFlag   = true;
  m_Status      = false;
  AnalyzeType      = 0;
  AnaIndex  	= 0;
  alarm         = 0;
  AnalyzeEn = 0;

  frame = 0;
  startflag         = 0;
  stopflag          = 0;
  intervalflag      = 0;
  alarmStartframe   = 0;
  alarmStopframe    = 0;

  //mut   = PTHREAD_MUTEX_INITIALIZER;
  //cond  = PTHREAD_COND_INITIALIZER;
  pthread_mutex_init (&mut,NULL);
  pthread_cond_init(&cond, NULL);


  region = new CRegion(CameraID);

  videoHandler = NULL;
  fire = new CFire(CameraID ,&videoHandler);

  smoke = new CSmoke(CameraID);

  human = new CHuman(CameraID);

}

CamAnaThread::~CamAnaThread()
{
    pthread_mutex_destroy(&mut);
    pthread_cond_destroy(&cond);

    delete region;
    region = NULL;

    videoHandler = NULL;
    delete fire;
    fire = NULL;

    delete smoke;
    smoke = NULL;

    delete human;
    human = NULL;
}


void CamAnaThread::set_video_draw( vector <VIDEO_DRAW> &DrawPkg)
{
	uint8 i =0;
	pkg.clear();
	for(i = 0; i <DrawPkg.size(); i++)
	{
		VIDEO_DRAW  tmp;
		tmp = DrawPkg[i];
		pkg.push_back(tmp);
	}
}

int CamAnaThread::alarmStrategy()
{
  //alarm strategy 5 frame alarm  and 4s later start another
  if( 1 == alarm && !startflag ){
      alarmStartframe++;
  }else{
    alarmStartframe = 0;
  }

  if(0 == alarm && stopflag){
      alarmStopframe++;
  }else{
      alarmStopframe = 0;
  }

  if(alarmStartframe > START_FRAME_INDEX)
  {
    startflag = 1;
    intervalflag = 1;
    alarmStartframe = 0;
    return alarmOn;
  }

  if(1 == intervalflag)
  {
      frame++;
  }

  if (frame > INTERVAL_FRAME_INDEX)
  {
     stopflag  = 1;
     intervalflag = 0;
     frame = 0;
  }

  if(alarmStopframe >  STOP_FRAME_INDEX)
  {
    stopflag  = 0;
    startflag = 0;
    alarmStopframe = 0;
    return alarmStop;
  }

  return 0;
}

int CamAnaThread::human_detect(Mat &frame)
{
  int iRet = -1;

  if(!frame.empty())
  {
      human->HumanDetectRun(frame);
      alarm = human->alarm ;
      usleep(20*1000);
      /*
      humanALL  	= human->humanstatis.numAll;
			humanIN		= MAX(human->humanstatis.doorin[0],human->humanstatis.doorin[1]);//human->humanstatis.inAll;
			humanOUT	= MAX(human->humanstatis.doorout[0],human->humanstatis.doorout[1]);//human->humanstatis.outAll;
			for(int i=0; i<LINENUM;i++){
				doorIN[i]		= human->humanstatis.doorin[i];
				doorOUT[i]	= human->humanstatis.doorout[i];
			}*/
			//imshow(HumanAlarmWindow,HumandispalyFrame);
  }
  else
  {
      alarm = 0;
      usleep(40*1000);
  }
  iRet =  alarmStrategy();

  if(iRet == alarmOn)
  {
    //TODO: notify huamn alarm start and push rtsp
  }

  if(iRet == alarmStop)
  {
    //TODO: notify huamn alarm stop
  }
  return 0;
}

int  CamAnaThread::region_detect(Mat &frame)
{
  int iRet = -1;

  if(!frame.empty())
  {
  		region->RegionDetectRun(frame);
      		alarm = region->alarm ;
  		usleep(20*1000);
  }
  else
  {
      alarm = 0;
      usleep(40*1000);
  }
  iRet =  alarmStrategy();

  if(iRet == alarmOn)
  {
    //TODO: notify region alarm start and push rtsp
  }

  if(iRet == alarmStop)
  {
    //TODO: notify region alarm stop
  }
  return 0;
}

int  CamAnaThread::fire_detect(Mat &frame)
{
  int iRet = -1;

  if(!frame.empty())
  {
      fire->FireDetectRun(frame,(void *)videoHandler);
      alarm = fire->alarm ;
      usleep(20*1000);
  }
  else
  {
      alarm = 0;
      usleep(40*1000);
  }
  iRet =  alarmStrategy();

  if(iRet == alarmOn)
  {
    //TODO: notify fire alarm start and push rtsp
  }

  if(iRet == alarmStop)
  {
    //TODO: notify fire alarm stop
  }
  return 0;
}

int  CamAnaThread::smoke_detect(Mat &frame)
{
  int iRet = -1;

  if(!frame.empty())
  {
      smoke->SmokeDetectRun(frame);
      alarm = smoke->alarm ;
      usleep(20*1000);
  }
  else
  {
      alarm = 0;
      usleep(40*1000);
  }
  iRet =  alarmStrategy();

  if(iRet == alarmOn)
  {
    //TODO: notify smoke alarm start and push rtsp
  }

  if(iRet == alarmStop)
  {
    //TODO: notify smoke alarm stop
  }
  return 0;
}


int CamAnaThread::alarm_run(Mat &frame ,uint8 iType)
{
  switch (iType) {
    case HumanDetect:
          human_detect(frame);
          break;
    case SmokeDetect:
          smoke_detect(frame);
          break;
    case RegionDetect:
          region_detect(frame);
          break;
    case FixedObjDetect:
          //fixobj_detect(frame);
          break;
    case FireDetect:
          fire_detect(frame);
          break;
    case ResidueDetect:
          //residue_detect(frame);
          break;
    default:
          dbgprint("%s(%d),cam %d alarmindex  %d wrong AnalyzeType !\n",
                                  DEBUGARGS,CameraID,iType);
          usleep(40*1000);
          break;
  }
  return 0;
}

void CamAnaThread::resource_release()
{
  alarm = 0;

  frame = 0;
  startflag         = 0;
  stopflag          = 0;
  intervalflag      = 0;
  alarmStartframe   = 0;
  alarmStopframe    = 0;

  //release region resource
  region->frameindex = 0;
  region->alarm      = 0;
}

void CamAnaThread::run()
{  
	while(m_AnaFlag){
		pthread_mutex_lock(&mut);
		while (!m_Status)
		{
			  pthread_cond_wait(&cond, &mut);
			  //release_resource()
			  //vector_analyze()
		}
		pthread_mutex_unlock(&mut);

		if(AnalyzeEn)
		{
			  switch (AnaIndex) {
			    case 1:
			          //m_AlarmCamera->Alarmthead1Frame.copyTo(frame1);
			          alarm_run(frame1,AnalyzeType);  //alarm_run(Mat & frame);
			          break;
			    case 2:
			        //  m_AlarmCamera->Alarmthead2Frame.copyTo(frame2);
			          alarm_run(frame2,AnalyzeType);
			          break;
			    default:
			      usleep(40*1000);
			      break;
			  }
		}
		else{
			usleep(40*1000);
		}
	}

	dbgprint("%s(%d),%d CAlarmThread exit!\n",DEBUGARGS,CameraID);
	pthread_exit(NULL);
}

int CamAnaThread::CreateAnaThread()
{
	int iRet = -1;
	pthread_t AnaThread;
	iRet = pthread_create(&AnaThread,NULL,RunAnaThread,this);
	if(iRet != 0)
  {
		 dbgprint("%s(%d),cam %d create %d AlarmThread failed!\n",DEBUGARGS,CameraID,AnaIndex);
		 return -1;
	}
	pthread_detach(AnaThread);
	return 0;
}

void CamAnaThread::resume()
{
    if (m_Status == false)
    {
        pthread_mutex_lock(&mut);
        m_Status = true;
        pthread_cond_signal(&cond);
        dbgprint("%s(%d), cam %d alarm %d pthread run!\n",DEBUGARGS,CameraID,AnaIndex);
        pthread_mutex_unlock(&mut);
    }
    else
    {
        dbgprint("%s(%d), cam %d alarm %d pthread run already!\n",DEBUGARGS,CameraID,AnaIndex);
    }
}

void CamAnaThread::pause()
{
    if (m_Status == true)
    {
        pthread_mutex_lock(&mut);
        m_Status = false;
        dbgprint("%s(%d), cam %d alarm %d pthread stop!\n",DEBUGARGS,CameraID,AnaIndex);
        pthread_mutex_unlock(&mut);
    }
    else
    {
        dbgprint("%s(%d), cam %d alarm %d pthread stop already!\n",DEBUGARGS,CameraID,AnaIndex);
    }
}

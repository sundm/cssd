#include "stdafx.h"
#include "desktopreader.h"
#include<Windows.h> 
#include <tchar.h> 

#ifdef _DEBUG 

#define DP0(fmt) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt));OutputDebugString(sOut);} 
#define DP1(fmt,var) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt),var);OutputDebugString(sOut);} 
#define DP2(fmt,var1,var2) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt),var1,var2);OutputDebugString(sOut);} 
#define DP3(fmt,var1,var2,var3) {TCHAR sOut[256];_stprintf_s(sOut,_T(fmt),var1,var2,var3);OutputDebugString(sOut);} 

#endif 

#ifndef _DEBUG 

#define DP0(fmt) ; 
#define DP1(fmt, var) ; 
#define DP2(fmt,var1,var2) ; 
#define DP3(fmt,var1,var2,var3) ; 

#endif 

//void hex_str(unsigned char *inchar, unsigned int len, char *outtxt)
//{
//	unsigned char hbit, lbit;
//	unsigned int i;
//	for (i = 0; i < len; i++)
//	{
//		hbit = (*(inchar + i) & 0xf0) >> 4;
//		lbit = *(inchar + i) & 0x0f;
//		if (hbit > 9) outtxt[2 * i] = 'A' + hbit - 10;
//		else outtxt[2 * i] = '0' + hbit;
//		if (lbit > 9) outtxt[2 * i + 1] = 'A' + lbit - 10;
//		else    outtxt[2 * i + 1] = '0' + lbit;
//	}
//	outtxt[2 * i] = 0;
//}

DesktopReader* DesktopReader::_instance = NULL;

static void onTagFound(int hreader, TAGINFO *tag, void *pdata)
{
	char out[MAXEPCBYTESCNT];
	memset(out, 0, MAXEPCBYTESCNT);
	Hex2Str(tag->EpcId, tag->Epclen, out);
	//DP1("onTagFound, len = %d\n", tag->Epclen);
	//DP1("onTagFound, code = %hs\n", out);
	DesktopReader::getInstance()->broadcast(CodeType::Transponder, out);
}

static void onReaderError(int hreader, READER_ERR err, void *pdata)
{

}

DesktopReader::DesktopReader()
{
	_address.clear();
	_antenna = 0;
}

DesktopReader *DesktopReader::getInstance() {
	if (NULL == _instance) {
		_instance = new DesktopReader();
	}

	return _instance;
}

DesktopReader::~DesktopReader()
{

}

bool DesktopReader::connect()
{
	_connected = false;
	_err = MT_OK_ERR;

	if (_address.empty() || _antenna < 1)
	{
		_err = MT_IO_ERR;
		return false;
	}
	
	DP1("connect:%d\n", _err);

	if (_hreader != -1)
	{
		disconnect();
	}

	//初始化设备
	_err = InitReader_Notype(&_hreader, (char*)_address.c_str(), _antenna);

	DP1("init:%d\n", _err);

	if (_err != MT_OK_ERR) return _connected;

	//获取读写器最大发射功率 
	AntPowerConf pwrs;
	pwrs.antcnt = _antenna;
	int maxpower = 0;

	_err = ParamGet(_hreader, MTR_PARAM_RF_MAXPOWER, &maxpower);

	DP1("get power:%d\n", _err);
	
	if (_err != MT_OK_ERR)
		return _connected;

	DP1("MaxPower:%d\n", maxpower);

	for (int i = 0; i < _antenna; ++i)
	{
		pwrs.Powers[i].antid = i + 1;
		pwrs.Powers[i].readPower = maxpower;
		pwrs.Powers[i].writePower = maxpower;
	}

	_err = ParamSet(_hreader, MTR_PARAM_RF_ANTPOWER, &pwrs);

	DP1("set power:%d\n", _err);

	if (_err != MT_OK_ERR)
		return _connected;

	//设置读写器执行GEN2空中接口协议                                                            
	Inv_Potls_ST potls;                                                       
	potls.potlcnt = 1;                                                        
	potls.potls[0].potl = SL_TAG_PROTOCOL_GEN2;                               
	potls.potls[0].weight = 30;                                               
	//不用检查返回值，不会失败                                                 
	ParamSet(_hreader, MTR_PARAM_TAG_INVPOTL, &potls);                      
																			 
	//设置盘存到标签时的回调处理函数                                           
	OnTagReadBlock trblk;                                                   
	trblk.handler = onTagFound;
	trblk.cookie = this;                                                    
	SetTagReadHandler(_hreader, trblk);                                    
	//设置读写器发生错误时的回调处理函数                                       
	OnReadingErrorBlock reblk;                                              
	reblk.handler = onReaderError;
	reblk.cookie = this;                                                       
	SetReadingErrorHandler(_hreader, reblk);                                  

	_connected = true;

	return _connected;
}

void DesktopReader::disconnect()
{
	CloseReader(_hreader);
	_hreader = -1;
	_connected = false;
}

bool DesktopReader::isConnected()
{
	return _connected;
}

READER_ERR DesktopReader::setStart()
{
	int ants[4];
	int antcnt;
	ants[0] = 1;
	ants[1] = 2;
	ants[2] = 3;
	ants[3] = 4;
	antcnt = 4;

	//初始化结BackReadOption构体,设置BackReadOption前必须先初始化为全0
	memset(&m_BROption, 0, sizeof(BackReadOption));

	//采用非高速模式盘存标签
	m_BROption.IsFastRead = 0;

	//非高速盘存模式下必须要设置的参数<盘存周期>单位为ms.
	m_BROption.ReadDuration = 200 * antcnt; 

	//盘存周期间的设备不工作时间,单位为ms,一般设置为0
	m_BROption.ReadInterval = 200;

	return StartReading(_hreader, ants, antcnt, &m_BROption);
}

READER_ERR DesktopReader::setStop()
{
	return StopReading(_hreader);
}
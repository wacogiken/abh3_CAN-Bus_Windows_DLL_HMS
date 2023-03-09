/*
 ******************************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2021-2023, Waco Giken Co., Ltd.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/o2r other materials provided
 *     with the distribution.
 *   * Neither the name of the Waco Giken nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************
*/

//概要
//	本DLLでエクスポートする関数を格納しているソースコード
#include "pch.h"

//特定の警告を無視
#pragma warning(disable : 4996)

//#define _NO_CANABH3API_EXPORT	//定義なしでエクスポート、定義有りでインポート
#include "CANabh3.h"

//安全な削除
template <class T> void SafeDelete(T** ppT)
	{
	if (*ppT)
		{
		delete *ppT;
		*ppT = NULL;
		}
	}

//ABH3制御クラスとインターフェースクラスへのポインタ（グローバル）
static CAbh3*		g_pAbh3 = NULL;
static CIxxatV2*	g_pIF = NULL;

//================================================================================
//ユーザー用関数(Windows固有系)
//================================================================================

//DLL使用開始時に呼び出す必要が有ります
CANABH3API void InitInstance(int32_t nIFnum)
	{
	//概要
	//	DLL使用開始時に呼び出す必要が有ります
	//パラメータ
	//	nIFnum		使用インターフェース（0...Ixxat USB-to-CAN V2）
	//戻り値
	//	無し
	//注意点
	//	本製品では、Ixxat USB-to-CAN V2のみサポートされます

	//
	ExitInstance();
	//
	g_pIF = new CIxxatV2();
	g_pAbh3 = new CAbh3(g_pIF);
	}

//DLL使用終了時に呼び出す必要が有ります
CANABH3API void ExitInstance()
	{
	//概要
	//	DLL使用終了時に呼び出す必要が有ります
	//パラメータ
	//	無し
	//戻り値
	//	無し
	//注意点
	//	DLL開放寸前で呼び出して下さい

	SafeDelete(&g_pAbh3);
	SafeDelete(&g_pIF);
	}

//使用可能なCANインターフェース数を取得
CANABH3API int32_t GetInterfaceCount()
	{
	return(g_pAbh3->GetInterfaceCount());
	}

//インターフェースを指定する
CANABH3API int32_t SetInterface(int32_t nDeviceNum)
	{
	return(g_pAbh3->SetInterface(nDeviceNum));
	}

//現在開いているインターフェース番号を取得
CANABH3API int32_t GetCurrentInterface()
	{
	return(g_pAbh3->GetCurrentInterface());
	}

//現在インターフェースを開いているか？
CANABH3API int32_t IsOpenInterface()
	{
	return(g_pAbh3->IsOpenInterface());
	}

//インターフェースを開くタイムアウト時間を設定
CANABH3API void SetOpenTimeout(uint32_t nTimeoutMS)
	{
	g_pAbh3->SetOpenTimeout(nTimeoutMS);
	}

//送信タイムアウト時間を設定
CANABH3API void SetSendTimeout(uint32_t nTimeoutMS)
	{
	g_pAbh3->SetSendTimeout(nTimeoutMS);
	}

//受信タイムアウト時間を設定
CANABH3API void SetRecvTimeout(uint32_t nTimeoutMS)
	{
	g_pAbh3->SetRecvTimeout(nTimeoutMS);
	}

//通信ホストのアドレスを設定
CANABH3API void SetHostID(uint8_t nAdrs)
	{
	g_pAbh3->SetHostID(nAdrs);
	}

//通信ホストのアドレスを取得
CANABH3API uint8_t GetHostID(void)
	{
	return(g_pAbh3->GetHostID());
	}

//通信速度を指定
CANABH3API void SetBaudrate(uint32_t nBaudrateKbps)
	{
	g_pAbh3->SetBaudrate(nBaudrateKbps);
	}

//設定した通信速度を取得
CANABH3API uint32_t GetBaudrate()
	{
	return(g_pAbh3->GetBaudrate());
	}

//PCが起動してからの時間を[ms]単位で取得
CANABH3API uint32_t GetTm()
	{
	return(g_pAbh3->GetTm());
	}

//1秒間に受信したデータ量を取得します
CANABH3API uint32_t GetCounter()
	{
	return(g_pAbh3->GetCounter());
	}

//CANバスにエラーが発生しているか調べます
CANABH3API uint32_t GetCANerror()
	{
	return(g_pAbh3->GetCANerror());
	}

//CANバスのエラー解除を行います
CANABH3API uint32_t ResetCANerror()
	{
	return(g_pAbh3->ResetCANerror());
	}

//================================================================================
//ユーザー用関数(通信系)
//================================================================================

//指令の初期化＋CANポートを開く
CANABH3API int32_t abh3_can_init(uint8_t nTargetID,pCANABH3_RESULT pPtr)
	{
	//概要
	//	指令の初期化＋CANポートを開く
	//詳細
	//	以下の順で処理を行う
	//	(1)CANポートを開く
	//	(2)A/Y指令、B/X指令を0に設定し、接続先にシングルパケットDP0を送信
	//	(3)(受信バッファ指定時のみ)シングルパケットDP0の受信
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_init(nTargetID,pPtr));
	}

//CANポートを開く
CANABH3API int32_t abh3_can_port_init(void)
	{
	//概要
	//	CANポートを開く
	//詳細
	//	CANポートを開く
	//パラメータ
	//	無し
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_port_init());
	}

//指令の初期化
CANABH3API int32_t abh3_cmd_init(uint8_t nTargetID,pCANABH3_RESULT pPtr)
	{
	//概要
	//	指令の初期化
	//詳細
	//	以下の順で処理を行う
	//	(1)A/Y指令、B/X指令を0に設定し、接続先にシングルパケットDP0を送信
	//	(2)(受信バッファ指定時のみ)シングルパケットDP0の受信
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_cmd_init(nTargetID,pPtr));
	}

//現在の受信データ状況を一括取得
CANABH3API void abh3_can_copylastdata(uint8_t nTargetID,pCANABH3_LASTRECV pPtr)
	{
	//概要
	//	現在の通信先から受信したデータ領域の複製を取得する
	//詳細
	//	現在の通信先から受信したデータを反映させた構造体を取得
	//パラメータ
	//	pPtr		データ領域の複製格納先
	//戻り値
	//	無し
	//注意点
	//	取得した領域の内、更新フラグが成立している領域のみ利用する事
	//	更新フラグの解除は、abh3_can_resetlastdata関数で行える

	g_pAbh3->abh3_can_copylastdata(nTargetID,pPtr);
	}

//現在の受信データ領域の更新フラグを解除する
CANABH3API void abh3_can_resetlastdata(uint8_t nTargetID,int32_t nAdrs)
	{
	//概要
	//	現在の受信データ領域の更新フラグを解除する
	//詳細
	//	受信データ領域に含まれる更新フラグの内、指定箇所をクリアする
	//パラメータ
	//	nAdrs		更新フラグを解除するアドレス、又は -1(全領域)を指定
	//戻り値
	//	無し
	//注意点
	//	abh3_can_copylastdata関数を利用してから、本関数を利用する迄に
	//	1つ以上のパケットを受信可能な場合、パラメータに全領域を指定して
	//	実行すると、未処理の更新フラグがクリアされてしまう事がある事に注意。
	//	基本的には、abh3_can_copylastdataで取得した領域を順次処理する時に、
	//	都度本関数にアドレスを指定して解除要求する事が望ましい

	g_pAbh3->abh3_can_resetlastdata(nTargetID,nAdrs);
	}

//受信対象を指定して受信
CANABH3API int32_t abh3_can_read(uint8_t nTargetID,pCANABH3_RESULT pPtr,PACKETTYPE nType,uint8_t nAdrs)
	{
	//概要
	//	指定条件に合うCANパケットを受信する
	//パラメータ
	//	nTargetID	受信対象を指定する発信元ID
	//	pPtr		受信バッファ
	//	nCode		受信対象を指定するコード
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//詳細
	//	既に受信バッファに条件が合う物が有れば、それを取得して戻る
	//	無い場合は、受信タイムアウト時間迄、受信を待つ
	//注意点
	//	無し
		
	return(g_pAbh3->abh3_can_read(nTargetID,pPtr,nType,nAdrs));
	}

//受信バッファの内容を受信データ状況に反映させる
CANABH3API int32_t abh3_can_flush()
	{
	//概要
	//	受信バッファの内容を受信データ状況に反映させる
	//パラメータ
	//	無し
	//戻り値
	//	処理件数
	//詳細
	//	受信バッファにあるデータ全て（要求無しで送ってくる場合等を想定）を
	//	取得し、「受信データ状況」に反映させる
	//注意点
	//	要求無しではホストへ何も送って来ない場合、この処理は不要となる
	//	要求無しでホストへ送ってくるデータが有る場合は、一定周期毎に
	//	この関数を使用して「受信データ状況」にデータを更新させる事で、
	//	値が反映させる。（受信データの値が必要な場合は、「受信データ状況」を利用する事）
		
	return(g_pAbh3->abh3_can_flush());
	}

//指令の送信（軸別）
CANABH3API int32_t abh3_can_cmdAY(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_cmdAY(nTargetID,cmd,pPtr));
	}
CANABH3API int32_t abh3_can_cmdBX(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_cmdBX(nTargetID,cmd,pPtr));
	}

//指令の送信（同時）
CANABH3API int32_t abh3_can_cmd(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_cmd(nTargetID,cmdAY,cmdBX,pPtr));
	}

//入力の送信（一括）
CANABH3API int32_t abh3_can_opSet(uint8_t nTargetID,int32_t data,int32_t mask,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_opSet(nTargetID,data,mask,pPtr));
	}

//入力の送信（ビット）
CANABH3API int32_t abh3_can_opBitSet(uint8_t nTargetID,int8_t num,int8_t data,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_opBitSet(nTargetID,num,data,pPtr));
	}

//指令と入力の送信（同時）
CANABH3API int32_t abh3_can_cmdAndopSet(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,int32_t data,int32_t mask,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_cmdAndopSet(nTargetID,cmdAY,cmdBX,data,mask,pPtr));
	}



//ブロードキャストパケットのリクエスト
CANABH3API int32_t abh3_can_reqBRD(uint8_t nTargetID,uint8_t num,uint8_t nBroadcast,pCANABH3_RESULT pPtr)
	{
	//概要
	//パラメータ
	//	pPtr		受信バッファ（NULLの場合は受信しない）
	//戻り値
	//	0			正常終了
	//	上記以外	異常発生
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_reqBRD(nTargetID,num,nBroadcast,pPtr));
	}

//マルチパケットによるTelABH3パケットの送受信
CANABH3API int32_t abh3_can_trans(uint8_t nTargetID,char* sbuf,char* rbuf,uint32_t* rbuflen)
	{
	//注意点
	//	アプリとDLLのHEAPが同一にならないような使い方（例：MFCアプリで使用）を
	//	した場合、DLL側で確保したバッファをアプリ側で開放出来ない(ASSERTされる)為、
	//	ここでアプリ側のバッファにDLL側のバッファ内容をコピーする
	//
	//パラメータ
	//	sbuf		送信データ(ANSI)
	//	rbuf		受信データ格納領域（アプリ側で確保する）
	//	rbuflen		受信データ格納領域のサイズ（アプリ側で設定し、本関数で格納サイズが入る）
	//戻り値
	//	0			正常
	//	-1			異常
	//	-2			バッファサイズが足りない
	
	//
	char* pTmp = NULL;
	size_t nTmpLen = 0;
	int32_t nResult = g_pAbh3->abh3_can_trans(nTargetID,sbuf,pTmp,nTmpLen);
	//正常？
	if(nResult == 0)
		{
		//最低限必要なサイズを設定
		*rbuflen = (uint32_t)nTmpLen;

		//サイズが足りる？
		if(*rbuflen >= nTmpLen)
			{
			::CopyMemory(rbuf,pTmp,nTmpLen);
			}
		else
			{
			//足りない
			nResult = -2;		//領域サイズが足りない扱い
			}
		//中継している為、開放が必要
		SafeDelete(&pTmp);
		}
	//
	return(nResult);
	}

//回線を閉じる
CANABH3API int32_t abh3_can_finish(void)
	{
	//概要
	//パラメータ
	//戻り値
	//	無し
	//注意点
	//	無し

	return(g_pAbh3->abh3_can_finish());
	}

//================================================================================
//ユーザー用関数(単位変換系)
//================================================================================

//速度
CANABH3API int16_t cnvVel2CAN(float vel)
	{
	return(g_pAbh3->cnvVel2CAN(vel));
	}
CANABH3API float cnvCAN2Vel(int16_t vel)
	{
	return(g_pAbh3->cnvCAN2Vel(vel));
	}

//電流値
CANABH3API int16_t cnvCur2CAN(float cur)
	{
	return(g_pAbh3->cnvCur2CAN(cur));
	}
CANABH3API float cnvCAN2Cur(int16_t cur)
	{
	return(g_pAbh3->cnvCAN2Cur(cur));
	}

//負荷率
CANABH3API float cnvCAN2Load(int16_t load)
	{
	return(g_pAbh3->cnvCAN2Load(load));
	}

//アナログ入力
CANABH3API float cnvCAN2Analog(int16_t analog)
	{
	return(g_pAbh3->cnvCAN2Analog(analog));
	}

//電源電圧
CANABH3API float cnvCAN2Volt(int16_t volt)
	{
	return(g_pAbh3->cnvCAN2Volt(volt));
	}

//実数
CANABH3API PACK_FLOAT flt_to_pkflt(float flt)
	{
	CPackFloat pf(flt);
	PACK_FLOAT result;
	result.u.nDirectData = pf.u.nDirectData;
	return(result);
	}
CANABH3API float pkflt_to_flt(PACK_FLOAT okflt)
	{
	CPackFloat pf(okflt);
	float result = 0.0f;
	pf.GetValue(result);
	return(result);
	}
CANABH3API int str_to_pkfltA(const char* str,PACK_FLOAT* p_pkflt)
	{
	CPackFloat pf((char*)str);
	p_pkflt->u.nDirectData = pf.u.nDirectData;
	return(0);
	}
CANABH3API int str_to_pkfltW(const wchar_t* str,PACK_FLOAT* p_pkflt)
	{
	CPackFloat pf((wchar_t*)str);
	p_pkflt->u.nDirectData = pf.u.nDirectData;
	return(0);
	}
CANABH3API char* pkflt_to_strA(PACK_FLOAT pkflt,char* str)
	{
	//注意・格納先領域のサイズを行わない為、十分なバッファを渡す必要が有る
	CPackFloat pf(pkflt);
	char* pTmp = new char[128];
	pf.GetValueA(pTmp,128);
	strcpy(str,pTmp);
	delete [] pTmp;
	return(str);
	}
CANABH3API wchar_t* pkflt_to_strW(PACK_FLOAT pkflt,wchar_t* str)
	{
	//注意・格納先領域のサイズを行わない為、十分なバッファを渡す必要が有る
	CPackFloat pf(pkflt);
	wchar_t* pTmp = new wchar_t[128];
	pf.GetValueW(pTmp,128);
	wcscpy(str,pTmp);
	delete [] pTmp;
	return(str);
	}

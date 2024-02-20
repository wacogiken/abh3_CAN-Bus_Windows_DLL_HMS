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

//多重インクルード防止
#ifndef _ABH3_H_INCLUDED_
#define _ABH3_H_INCLUDED_

//プラットフォーム依存
#include <Windows.h>
#include <stdint.h>
#include <timeapi.h>
#include <stdio.h>

//CAN関連
#include "Can1939.h"
#include "CanIF.h"

//型定義
#include "typedef.h"

//クラス定義
class CAbh3
{
//================================================================================
//ユーザーが利用可能な型
//================================================================================
public:

//================================================================================
//内部で使用される型
//================================================================================
protected:

//================================================================================
//ユーザーが利用可能な関数（Windows固有）
//================================================================================
public:
	//使用可能なCANインターフェース数を取得
	int32_t GetInterfaceCount(void);
	//開くインターフェースを指定
	int32_t SetInterface(int32_t nDeviceNum);
	//インターフェースを開く
	int32_t OpenInterface(void);
	//開いたインターフェースを閉じる
	void CloseInterface(void);
	//現在開いているインターフェース番号を取得
	int32_t GetCurrentInterface(void);
	//現在インターフェースを開いているか？
	int32_t IsOpenInterface(void);
	//インターフェースを開くタイムアウト時間を設定
	void SetOpenTimeout(uint32_t nTimeoutMS);
	//送信タイムアウト時間を設定
	void SetSendTimeout(uint32_t nTimeoutMS);
	//受信タイムアウト時間を設定
	void SetRecvTimeout(uint32_t nTimeoutMS);
	//通信ホストのアドレスを設定
	void SetHostID(uint8_t nAdrs);
	//通信ホストのアドレスを取得
	uint8_t GetHostID(void);
	//通信速度を指定
	void SetBaudrate(uint32_t nBaudrateKbps);
	//設定した通信速度を取得
	uint32_t GetBaudrate(void);
	//PCが起動してからの時間を[ms]単位で取得
	uint32_t GetTm(void)
		{
		uint32_t nResult = 0;
		if(m_pVar)
			{
			if(IsValidDevice())
				nResult = m_pVar->pDeviceClass->GetTm();
			}
		return(nResult);
		}	

	//1秒間に送受信したデータ量を取得します
	uint32_t GetCounter(void)
		{
		uint32_t nResult = 0;
		if(m_pVar)
			{
			if(IsValidDevice())
				nResult = m_pVar->pDeviceClass->GetCounter();
			}
		return(nResult);
		}

	//送受信カウンタを取得します
	uint32_t GetTransmitCounter(uint8_t nRecv)
		{
		if(m_pVar)
			{
			if(IsValidDevice())
				return(m_pVar->pDeviceClass->GetTransmitCounter(nRecv));
			}
		return(0);
		}

	//CANバスにエラーが発生しているか調べます
	uint32_t GetCANerror(void);
	//CANバスのエラー解除を行います
	 uint32_t ResetCANerror(void);

	//シングルパケットの送信PDUコードを設定
	void SetSinglePacketCode(uint8_t nTargetID,uint8_t nCode)
		{
		if(m_pVar)
			m_pVar->config.target[nTargetID].nPDU_single = nCode;
		}
	//シングルパケットの送信PDUコードを取得
	uint8_t GetSinglePacketCode(uint8_t nTargetID)
		{
		uint8_t nResult = 0;
		if(m_pVar)
			nResult = m_pVar->config.target[nTargetID].nPDU_single;
		if(nResult == 0)
			nResult = 0xEF;
		return(nResult);
		}
	//ブロードキャストパケットの送信PDUコードを設定
	void SetBroadcastPacketCode(uint8_t nTargetID,uint8_t nCode)
		{
		if(m_pVar)
			m_pVar->config.target[nTargetID].nPDU_broadcast = nCode;
		}
	//ブロードキャストパケットの送信PDUコードを取得
	uint8_t GetBroadcastPacketCode(uint8_t nTargetID)
		{
		uint8_t nResult = 0;
		if(m_pVar)
			nResult = m_pVar->config.target[nTargetID].nPDU_broadcast;
		if(nResult == 0)
			nResult = 0xEA;
		return(nResult);
		}

//================================================================================
//ユーザーが利用可能な関数（ABH3固有）
//================================================================================
public:
	//指令の初期化（回線制御付き）
	int32_t abh3_can_init(uint8_t nTargetID,pCANABH3_RESULT pResult);
	int32_t abh3_can_port_init(void);
	int32_t abh3_can_cmd_init(uint8_t nTargetID,pCANABH3_RESULT pResult);
	//指令の送信（軸別）
	int32_t abh3_can_cmdAY(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr);
	int32_t abh3_can_cmdBX(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr);
	//指令の送信（同時）
	int32_t abh3_can_cmd(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,pCANABH3_RESULT pPtr);
	//入力の送信（一括）
	int32_t abh3_can_opSet(uint8_t nTargetID,int32_t data,int32_t mask,pCANABH3_RESULT pPtr);
	//入力の送信（ビット）
	int32_t abh3_can_opBitSet(uint8_t nTargetID,int8_t num,int8_t data,pCANABH3_RESULT pPtr);
	//指令と入力の送信（同時）
	int32_t abh3_can_cmdAndopSet(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,int32_t data,int32_t mask,pCANABH3_RESULT pPtr);
	//ブロードキャストパケットのリクエスト
	int32_t abh3_can_reqBRD(uint8_t nTargetID,uint8_t num,uint8_t nBroadcast,pCANABH3_RESULT pPtr);
	//マルチパケットによるTelABH3パケットの送受信
	int32_t abh3_can_trans(uint8_t nTargetID,char* sbuf,char*& rbuf,size_t& rbuflen);
	//現在の受信データ状況を一括取得
	void abh3_can_copylastdata(uint8_t nTargetID,pCANABH3_LASTDATA pPtr);
	//受信データ状況の指定箇所の更新フラグを解除
	void abh3_can_resetlastdata(uint8_t nTargetID,int32_t nAdrs);
	//受信(外部用・排他制御あり)
	int32_t abh3_can_read(uint8_t nTargetID,pCANABH3_RESULT pPtr,PACKETTYPE nType,uint8_t nAdrs);
	//受信(外部用・排他制御あり)
	int32_t abh3_can_flush(void);
	//回線を閉じる
	int32_t abh3_can_finish(void);

//================================================================================
//ユーザー用関数(単位変換系)
//================================================================================
public:
	//速度
	int16_t	cnvVel2CAN(float vel)
		{
		int16_t nResult = int16_t(vel / 0.2f);
		return(nResult);
		}
	float	cnvCAN2Vel(int16_t vel)
		{
		float nResult = float(vel) * 0.2f;
		return(nResult);
		}

	//負荷率
	float	cnvCAN2Load(int16_t load)
		{
		float nResult = float(load) * 1.0f;
		return(nResult);
		}

	//アナログ入力
	float	cnvCAN2Analog(int16_t analog)
		{
		float nResult = float(analog) * 0.01f;
		return(nResult);
		}

	//電源電圧
	float	cnvCAN2Volt(int16_t volt)
		{
		float nResult = float(volt) * 0.1f;
		return(nResult);
		}

	//電流
	float	cnvCAN2Cur(int16_t cur)
		{
		float nResult = float(cur) * 0.01f;
		return(nResult);
		}
	int16_t cnvCur2CAN(float cur)
		{
		int16_t nResult = int16_t(cur / 0.01f);
		return(nResult);
		}


//================================================================================
//内部用
//================================================================================
protected:


	//設定
	typedef struct _CANABH3_CONFIG
		{
		//共通項目
		uint8_t			nHostAdrs;		//ホスト側（PC）アドレス
		uint8_t			nSingleCode;	//EFh	シングルパケットの識別コード
		//接続先毎に異なる要素
		struct _CONFIG_TARGET
			{
			uint8_t		nPDU_single;		//シングルパケット時のPDU（未指定で0xEF）
			uint8_t		nPDU_broadcast;		//ブロードキャストパケットのPDU（未指定で0xEA）
			} target[256];
		} CANABH3_CONFIG,*pCANABH3_CONFIG;

	//内部用変数
	typedef struct _CANABH3_VAR
		{
		CCanIF*				pDeviceClass;	//出力先のクラス
		int32_t				nReqDevice;		//開く予定のデバイス番号
		int32_t				nOpenDevice;	//0..開いていない  1以上..開いたインターフェース番号+1が入る
		HANDLE				hTerm;			//通信排他用のセマフォ
		CANABH3_CONFIG		config;			//設定
		CANABH3_LASTDATA	lastdata[256];	//最終データ関連（送信・受信）
		} CANABH3_VAR,*pCANABH3_VAR;

	//内部変数（動的確保して使う）
	pCANABH3_VAR m_pVar;

	//受信IDから識別に必要な要素を抽出
	PACKETTYPE recvid2any(uint32_t nCANID,uint8_t& nSenderID,uint8_t& nTargetID,uint8_t& nGroup,uint8_t& nAdrs,uint8_t* pRecvData);

	//最終受信データ領域を取得）
	pCANABH3_LASTDATA GetLastData(uint8_t nID)
		{
		pCANABH3_LASTDATA pResult = NULL;
		if(m_pVar)
			pResult = &m_pVar->lastdata[nID];
		return(pResult);
		}

	//最終送信データ領域を取得（CANに送信されるデータの領域もある）
	pCANABH3_LASTSEND GetLastSendData(uint8_t nID)
		{
		pCANABH3_LASTDATA pResult = GetLastData(nID);
		if(pResult)
			return(&pResult->send);
		return(NULL);
		}

	//最終受信データ領域を取得（CANから受信したデータと同じ）
	pCANABH3_LASTRECV GetLastRecvData(uint8_t nID)
		{
		pCANABH3_LASTDATA pResult = GetLastData(nID);
		if(pResult)
			return(&pResult->recv);
		return(NULL);
		}

	//デバイスクラスが登録されているか？
	bool IsValidDevice(void)
		{
		if(m_pVar == NULL)
			return(false);
		//登録されていればtrueが戻る
		return((bool)(m_pVar->pDeviceClass != 0));
		}

	//CANIDを構築
	uint32_t CreateSinglePacketID(uint8_t nTargetID)
		{
		//下位16bitに送信先と送信元のIDが入る

		//シングルパケットの識別コード
		uint8_t nCode = GetSinglePacketCode(nTargetID);

		//CAN-ID構築（0x00XXYYZZ）
		//	XX..シングルパケットの識別コード
		//	YY..送信先ID
		//	ZZ..ホストID
		uint32_t nResult = ((uint32_t)nCode) << 16 | ((uint32_t)nTargetID) << 8 | GetHostID();
		return(nResult);
		}

	//CANIDを構築
	uint32_t CreateBroadcastPacketID(uint8_t nTargetID,uint8_t num,uint8_t nAdrs,uint8_t nBroadcast = 0)
		{
		//パラメータ
		//	nTargetID		送信先ID
		//	num				ブロードキャスト番号（上位5bitがグループ番号、下位3bitがアドレス）
		//	nGroup			送信先グループ番号
		//	nAdrs			送信先アドレス
		//	nBroadcast		0以外で全体送信（送信先が0xFFとして扱われる）

		//全体送信？
		if(nBroadcast)
			nTargetID = 0xff;

		//ブロードキャストパケットの識別コード
		uint8_t nCode = GetBroadcastPacketCode(nTargetID);

		//CAN-ID構築（0x00XXYYZZ）
		//	XX..ブロードキャストパケットの識別コード
		//	YY..送信先ID
		//	ZZ..ホストID
		uint32_t nResult = ((uint32_t)nCode) << 16 | ((uint32_t)nTargetID) << 8 | GetHostID();
		return(nResult);
		}

	//ブロードキャストのシグネチャ（データの先頭3バイトに入る物）を構築
	uint32_t CreateBrdID(uint8_t nBrdNum)
		{
		//下位8bitにブロードキャストのグループとアドレスから算出される番号が入る
		uint32_t nResult = 0x00ff00 | nBrdNum;
		return(nResult);
		}

	//排他制御要求
	int32_t Lock(void)
		{
		//内部変数の確認
		if(m_pVar == NULL)
			return(-1);	//内部変数が用意出来てない
		//デバイスクラスの確認
		else if(!IsValidDevice())
			return(-1);	//デバイスクラスが登録されていない
		//送信タイムアウト時間を取得
		uint32_t nTimeoutMS = m_pVar->pDeviceClass->GetTimeout_Send();

		//ロックする時は送信許容時間を利用する
		if(::WaitForSingleObject(m_pVar->hTerm,nTimeoutMS) != WAIT_OBJECT_0)
			return(-1);	//資源占有失敗
		//資源占有成功
		return(0);
		}

	//排他制御開放
	void Unlock(void)
		{
		if(m_pVar == NULL)
			return;
		//注意点
		//	確認しないで開放する為、2重開放の類に注意
		::ReleaseSemaphore(m_pVar->hTerm,1,NULL);
		}

	//受信バッファクリア
	void ClearRecvBuffer(void)
		{
		if(m_pVar == NULL)
			return;
		else if(IsValidDevice())
			m_pVar->pDeviceClass->OnCanRecvClear();
		}

	//受信(内部用・排他制御無し)
	int32_t abh3_can_recv(uint8_t nTargetID,pCANABH3_RESULT pPtr,PACKETTYPE nType,uint8_t nAdrs = 0);

	//CAN送信
	int32_t CanSend8(uint32_t nSendID,uint8_t* pSendData,uint8_t nLength);

	//CAN受信
	int32_t CanRecv8(uint32_t* pRecvID,uint8_t* pRecvData); 

	//マルチパケット送信
	int32_t CanTermSendMulti(uint8_t nTargetID,uint8_t* pSendData,uint32_t nSendDataSize,uint8_t*& pRecvData,uint32_t& nRecvDataSize);

	//受信したデータを最終データとして格納
	void StockLastRecvData(uint32_t nRecvID,uint8_t* pRecvData);

	//送信するパケットデータを最終データとして格納
	void StockLastSendData(uint8_t nSendID,uint8_t* pSendData,uint8_t nLength,bool bSinglePacket);


	//CANIDから発信元をを取得
	uint8_t canid2fromid(uint32_t nID)
		{
		//発信元はbit[0-7]
		return((uint8_t)(nID));
		}

	//CANIDから送信先を取得
	uint8_t canid2toid(uint32_t nID)
		{
		//送信先はbit[8-15]
		return((uint8_t)(nID >> 8));
		}

public:
	//コンストラクタ
	CAbh3(CCanIF* pDeviceClass = NULL);
	//デストラクタ
	virtual ~CAbh3();
};

#endif

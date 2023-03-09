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

#pragma once

//
#include <Windows.h>
#include <stdint.h>

//
class CCanIF
{
public:
//================================================================================
//使用CANインターフェース毎に実装が必須の仮想関数
//================================================================================
public:

	//利用可能なCANインターフェース数を取得する場合に呼び出されます
	virtual int32_t OnGetInterfaceCount(void);

	//指定CANインターフェースを開く場合に呼び出されます
	virtual int32_t OnOpenInterface(int32_t nDeviceNum);

	//開いたCANインターフェースを閉じる場合に呼び出されます
	virtual void OnCloseInterface(void);

	//CANインターフェースから受信する場合に呼び出されます
	virtual int32_t OnCanRecv(uint32_t* pCanID,uint8_t* pData8,bool bNoWait = false);

	//CANインターフェースに送信する場合に呼び出されます
	virtual int32_t OnCanSend(uint32_t nCanID,uint8_t* pData8,uint8_t nLength);

	//CANインターフェースにエラーが有るか調べる場合に呼び出されます
	virtual int32_t OnCanGetError(void);

	//CANインターフェースのエラーを解除する場合に呼び出されます
	virtual int32_t OnCanClearError(void);

	//受信バッファをクリアする場合に呼び出されます
	virtual int32_t OnCanRecvClear(void);

//================================================================================
//設定用関数
//================================================================================
public:

	//CANインターフェースのタイムアウトを設定
	void SetTimeout(uint32_t nOpenMS,uint32_t nSendMS,uint32_t nRecvMS);
	void SetTimeout_Open(uint32_t nTimeoutMS);
	void SetTimeout_Send(uint32_t nTimeoutMS);
	void SetTimeout_Recv(uint32_t nTimeoutMS);

	//現在のCANインターフェースタイムアウト設定を取得
	void GetTimeout(uint32_t& nOpenMS,uint32_t& nSendMS,uint32_t& nRecvMS);
	uint32_t GetTimeout_Open(void);
	uint32_t GetTimeout_Send(void);
	uint32_t GetTimeout_Recv(void);

	//CANインターフェースのボーレートを設定
	void SetBaudrate(uint32_t nBaudrate);

	//現在のCANインターフェースのボーレートを取得
	uint32_t GetBaudrate(void);

	//無視する発信元IDを指定
	void SetIgnoreID(uint8_t nID);

	//無視する発信元IDを取得
	uint8_t GetIgnoreID(uint8_t nID);

//================================================================================
//上位含めて利用可能な関数
//================================================================================
public:

	//現在時刻を[ms]単位で取得
	uint32_t GetTm(void);

	//送受信サイズカウンタを取得
	uint32_t GetCounter(void);

//================================================================================
//継承先のみ利用可能な関数
//================================================================================
protected:

	//指定IDが無視する対象か調べます
	bool IsIgnoreID(uint32_t nID)
		{
		//概要
		//	事前設定された「無視するID」と比較します
		//パラメータ
		//	nID		比較するID（通常は受信したCANパケットの発信元を指定する）
		//戻り値
		//	true	無視する対象と判断
		//	false	無視しない対象と判断
		//注意点
		//	基本的には、無視する対象のIDとして、事前にホスト側(PC)のIDを指定しておく
		//　これは回線を開いた時に自動設定される
		//	CANパケットを受信したら、そのパケットの発信元をこの関数でチェックする
		//	これは「PC側から送信されたパケット」が受信される為の措置となる。
		//

		//指定ID自体が0か？（発信元IDが0はあり得ない）
		if(nID == 0)
			return(true);		//正常なパケットではないので無視する

		//特定発信元を無視する設定か？
		if(m_var.nIgnoreID)
			{
			//指定IDの発信元が一致するか？
			if(m_var.nIgnoreID == uint8_t(nID))
				return(true);	//一致しているので無視する
			}
		
		//条件に合わないので無視しない
		return(false);
		}

	//受信サイズカウンタに値を加算
	void AddCounter(uint32_t nDLCsize)
		{
		//実際のバス占有サイズは、ヘッダ54bits + データbits数
		uint32_t nBitSize = 54 + (nDLCsize * 8);
		m_var.counter.nBitSize += nBitSize;
		}

//================================================================================
//内部用
//================================================================================
private:

	//内部変数
	typedef struct _CANIF_VAR
		{
		uint32_t	nBaudrate;
		uint8_t		nIgnoreID;			//0以外で同じ値の発信元(IDの下位8bit)を無視する
		//uint32_t	nOnlyID;			//0以外でこのIDのみ受信する（nIgnoreIDより優先）
		struct _TMO
			{
			uint32_t	nOpen;
			uint32_t	nSend;
			uint32_t	nRecv;
			} timeout;
		struct _COUNTER
			{
			uint32_t	nOldBitSize;//1つ前の基準時間で送受信したビットサイズ
			uint32_t	nBitSize;	//現在の基準時間内に送受信したビットサイズ
			} counter;


		} CANIF_VAR,*pCANIF_VAR;
	CANIF_VAR m_var;

public:
	//コンストラクタ
	CCanIF()
		{
		::memset(&m_var,0,sizeof(CANIF_VAR));
		}
	//デストラクタ
	virtual ~CCanIF()
		{
		}
};


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

//標準的なインクルード
#include <Windows.h>
#include <stdint.h>

//継承元クラス
#include "CanIF.h"

//本クラスで使用するデバイス用のインクルード
//(USB-to-CAN v2 のフォルダをプロジェクトに追加して下さい
#define INITGUID
#include "vcisdk.h"
#include "vcitype.h"

//本クラス定義
class CIxxatV2 : public CCanIF
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
//内部用（スレッドから利用為にpublic扱い）
//================================================================================
public:
	//
	typedef struct _IXXATV2_STATUS
		{
		bool	bCanStart;	//false..stopped or reseted  true..started
		union
			{
			DWORD nRaw;
			struct _errbit_info
				{
				DWORD	nStuff:1;
				DWORD	nForm:1;
				DWORD	nACK:1;
				DWORD	nBit:1;
				DWORD	nCRC:1;
				DWORD	nOther:1;
				DWORD	nDummy:26;
				} info;
			} err;
		} IXXATV2_STATUS,*pIXXATV2_STATUS;
	//
	IXXATV2_STATUS	m_status;
	IBalObject*		m_pBalObject;		// bus access object
	LONG			m_lSocketNo;		// socket number
	ICanControl*	m_pCanControl;		// control interface
	ICanChannel*	m_pCanChn;			// channel interface
	LONG			m_lMustQuit;		// quit flag for the receive thread
	HANDLE			m_hEventReader;		//
	PFIFOREADER		m_pReader;			//
	PFIFOWRITER		m_pWriter;			//
	HANDLE			m_hSema;			//受信バッファ排他制御用セマフォ
	HANDLE			m_hReadThread;		//受信スレッドハンドル
	PCANMSG			m_pCanMsg;
	uint32_t		m_nCanReadPt;
	uint32_t		m_nCanWritePt;

	//非同期受信スレッド
	static void ReceiveThread(void* Param);

	//受信バッファ排他制御要求
	bool LockCanMsg(DWORD nTimeoutMS = INFINITE);

	//受信バッファ排他制御解除
	void UnlockCanMsg(void);

	//受信データの登録処理
	uint32_t AddCanMsg(PCANMSG pMsg,int nCount = 1);

	//登録されたCANメッセージを1つ取得
	uint32_t GetCanMsg(PCANMSG pMsg);

//================================================================================
//内部用
//================================================================================
protected:

public:
	CIxxatV2();
	virtual ~CIxxatV2();
};

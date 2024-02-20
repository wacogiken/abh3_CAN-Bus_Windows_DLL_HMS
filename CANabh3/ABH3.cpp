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
//	ABH3用CAN制御クラス
//
//履歴
//	2023/03/31	yo0043		1th release
//							前のCAbh3クラスから大幅変更の為、別物として扱う
//							受信バッファを指定しない通信要求は、送信のみ行う
//							インターフェースのオープンクローズにも排他制御を入れる

#include "pch.h"
#include <memory.h>
#include "Abh3.h"

//安全な削除
template <class T> void SafeDelete(T** ppT)
	{
	if (*ppT)
		{
		delete *ppT;
		*ppT = NULL;
		}
	}

//コンストラクタ
CAbh3::CAbh3(CCanIF* pDeviceClass /* NULL */)
	{
	//内部変数を初期化状態で確保
	m_pVar = new CANABH3_VAR();

	//通信制御用セマフォ構築
	m_pVar->hTerm = ::CreateSemaphore(NULL,1,1,NULL);

	//出力段のクラスを保存
	m_pVar->pDeviceClass = pDeviceClass;
	}

//デストラクタ
CAbh3::~CAbh3()
	{
	//インターフェースを閉じる（開いている場合が有る為）
	CloseInterface();


	//通信制御用セマフォの開放
	::CloseHandle(m_pVar->hTerm);
	m_pVar->hTerm = NULL;

	//内部変数解放
	SafeDelete(&m_pVar);
	}

//================================================================================
//ユーザー用関数(Windows固有系)
//================================================================================

//使用可能なCANインターフェース数を取得
int32_t CAbh3::GetInterfaceCount(void)
	{
	//デバイスが未登録？
	if(!IsValidDevice())
		return(0);	//デバイス無し扱い
	return(m_pVar->pDeviceClass->OnGetInterfaceCount());
	}

//インターフェースを指定する
int32_t CAbh3::SetInterface(int32_t nDeviceNum)
	{
	m_pVar->nReqDevice = nDeviceNum;
	return(0);
	}

//指定インターフェースを開く
int32_t CAbh3::OpenInterface()
	{
	//開く予定のデバイス番号を取得
	int32_t nDeviceNum = m_pVar->nReqDevice;

	//CANインターフェースを開いている場合が有るので閉じる
	CloseInterface();

	//デバイスが未登録？
	if(!IsValidDevice())
		return(-1);	//エラー

	//CANインターフェースを開く
	int32_t nResult = m_pVar->pDeviceClass->OnOpenInterface(nDeviceNum);

	//異常終了？
	if(nResult)
		return(nResult);

	//開いたCANインターフェース情報を保存
	m_pVar->nOpenDevice = nDeviceNum + 1;

	//デバイスにホストのIDを渡して、無視する対象とさせる（送信が受信に乗る為）
	m_pVar->pDeviceClass->SetIgnoreID(GetHostID());

	//
	return(0);
	} 

//開いたインターフェースを閉じる
void CAbh3::CloseInterface()
	{
	//開いているCANインターフェース情報を解除
	m_pVar->nOpenDevice = 0;

	//デバイス登録有り？
	if(IsValidDevice())
		{
		//CANインターフェースを閉じる
		m_pVar->pDeviceClass->OnCloseInterface();
		}
	}

//現在開いているインターフェース番号を取得
int32_t CAbh3::GetCurrentInterface()
	{
	return(m_pVar->nOpenDevice - 1);
	}

//現在インターフェースを開いているか？
int32_t CAbh3::IsOpenInterface()
	{
	if(GetCurrentInterface() >= 0)
		return(1);
	return(0);
	}

//インターフェースを開くタイムアウト時間を設定
void CAbh3::SetOpenTimeout(uint32_t nTimeoutMS)
	{
	if(IsValidDevice())
		m_pVar->pDeviceClass->SetTimeout_Open(nTimeoutMS);
	}

//送信タイムアウト時間を設定
void CAbh3::SetSendTimeout(uint32_t nTimeoutMS)
	{
	if(IsValidDevice())
		m_pVar->pDeviceClass->SetTimeout_Send(nTimeoutMS);
	}

//受信タイムアウト時間を設定
void CAbh3::SetRecvTimeout(uint32_t nTimeoutMS)
	{
	if(IsValidDevice())
		m_pVar->pDeviceClass->SetTimeout_Recv(nTimeoutMS);
	}

//通信ホストのアドレスを設定
void CAbh3::SetHostID(uint8_t nAdrs)
	{
	m_pVar->config.nHostAdrs = nAdrs;
	}

//通信ホストのアドレスを取得
uint8_t CAbh3::GetHostID()
	{
	return(m_pVar->config.nHostAdrs);
	}

//通信速度を指定
void CAbh3::SetBaudrate(uint32_t nBaudrateKbps)
	{
	//概要
	//	通信ボーレート指定
	//パラメータ
	//	nBaudrateKbps	ボーレートを[kbps]単位で指定（デバイス依存なので注意）

	//デバイスが登録済みならボーレート設定
	if(IsValidDevice())
		m_pVar->pDeviceClass->SetBaudrate(nBaudrateKbps);
	}

//設定した通信速度を取得
uint32_t CAbh3::GetBaudrate(void)
	{
	//デバイスが未登録？
	if(!IsValidDevice())
		return(0);
	return(m_pVar->pDeviceClass->GetBaudrate());
	}

//CANバスにエラーが発生しているか調べます
uint32_t CAbh3::GetCANerror()
	{
	//概要
	//	CANバスにエラーが発生しているか調べます
	//パラメータ
	//	無し
	//戻り値
	//	0			エラー無し
	//	2			インターフェースのアクセス自体に問題が有る
	//	上記以外	エラー有り

	if(!IsValidDevice())
		return(2);
	return((uint32_t)m_pVar->pDeviceClass->OnCanGetError());
	}

//CANバスのエラー解除を行います
 uint32_t CAbh3::ResetCANerror()
	{
	//概要
	//	CANバスのエラー解除を行います
	//パラメータ
	//	無し
	//戻り値
	//	0			正常終了
	//	2			インターフェースのアクセス自体に問題が有る
	//	上記以外	エラー解除に失敗
	//注意点
	//	本関数ではエラーから復帰出来ない場合が有る事に注意
	//	この場合は、インターフェースを一度綴じ、再度開き直す
	//	手順で大抵の場合は復帰する

	if(!IsValidDevice())
		return(2);
	return((uint32_t)m_pVar->pDeviceClass->OnCanClearError());
	}

//================================================================================
//ユーザー用関数(通信系)
//================================================================================

//
int32_t CAbh3::abh3_can_init(uint8_t nTargetID,pCANABH3_RESULT pPtr)
	{
	int32_t nResult = this->abh3_can_port_init();
	if(nResult == 0)
		nResult = this->abh3_can_cmd_init(nTargetID,pPtr);
	return(nResult);
	}

//
int32_t CAbh3::abh3_can_port_init()
	{
	int32_t nResult = 0;

	//通信排他制御用のセマフォを取得
	if(Lock() == 0)
		{
		nResult = OpenInterface();
		
		Unlock();
		}
	else
		nResult = -1;

	return(nResult);
	}

//指令の初期化
int32_t CAbh3::abh3_can_cmd_init(uint8_t nTargetID,pCANABH3_RESULT pPtr)
	{
	//実行
	return(
		abh3_can_cmdAndopSet(
			nTargetID,
			0,
			0,
			0,
			0,
			pPtr
			)
		);
	}

//指令の送信（軸別）
int32_t CAbh3::abh3_can_cmdAY(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr)
	{
	//接続先別の最終送信データ領域を取得
	pCANABH3_LASTSEND pLastsend = GetLastSendData(nTargetID);
	//実行
	return(
		abh3_can_cmdAndopSet(
			nTargetID,
			cmd,
			pLastsend->nOrderBX,
			pLastsend->nInputBit,
			0,
			pPtr
			)
		);
	}
int32_t CAbh3::abh3_can_cmdBX(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr)
	{
	//接続先別の最終送信データ領域を取得
	pCANABH3_LASTSEND pLastsend = GetLastSendData(nTargetID);
	//実行
	return(
		abh3_can_cmdAndopSet(
			nTargetID,
			pLastsend->nOrderAY,
			cmd,
			pLastsend->nInputBit,
			0,
			pPtr
			)
		);
	}

//指令の送信（同時）
int32_t CAbh3::abh3_can_cmd(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,pCANABH3_RESULT pPtr)
	{
	//接続先別の最終送信データ領域を取得
	pCANABH3_LASTSEND pLastsend = GetLastSendData(nTargetID);
	//実行
	return(
		abh3_can_cmdAndopSet(
			nTargetID,
			cmdAY,
			cmdBX,
			pLastsend->nInputBit,
			0,
			pPtr
			)
		);
	}

//入力の送信（一括）
int32_t CAbh3::abh3_can_opSet(uint8_t nTargetID,int32_t data,int32_t mask,pCANABH3_RESULT pPtr)
	{
	//接続先別の最終送信データ領域を取得
	pCANABH3_LASTSEND pLastsend = GetLastSendData(nTargetID);

	return(
		abh3_can_cmdAndopSet(
			nTargetID,
			pLastsend->nOrderAY,
			pLastsend->nOrderBX,
			data,
			mask,
			pPtr
			)
		);
	}

//入力の送信（ビット）
int32_t CAbh3::abh3_can_opBitSet(uint8_t nTargetID,int8_t num,int8_t data,pCANABH3_RESULT pPtr)
	{
	//接続先別の最終送信データ領域を取得
	pCANABH3_LASTSEND pLastsend = GetLastSendData(nTargetID);

	//
	int32_t nData = pLastsend->nInputBit;
	if(data)
		nData |= (1 << num);
	else
		nData &= ~(1 << num);

	//
	return(
		abh3_can_cmdAndopSet(
			nTargetID,
			pLastsend->nOrderAY,
			pLastsend->nOrderBX,
			nData,
			0,
			pPtr
			)
		);
	}

//指令と入力の送信（同時）
int32_t CAbh3::abh3_can_cmdAndopSet(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,int32_t data,int32_t mask,pCANABH3_RESULT pPtr)
	{
	//戻り値
	int32_t nResult = -1;

	//通信排他制御用のセマフォを取得
	if(Lock() == 0)
		{
		//インターフェースを開いている？
		if(IsOpenInterface())
			{
			//接続先別の最終送信データ領域を取得
			pCANABH3_LASTSEND pLastsend = GetLastSendData(nTargetID);

			//設定値を格納
			pLastsend->nOrderAY = cmdAY;
			pLastsend->nOrderBX = cmdBX;
			pLastsend->nInputBit = (data & ~mask) | (data & mask);

			//シングルパケット(DP0)を構築
			uint8_t* pPacket = CCan1939::CreateSGL0(
				 pLastsend->nOrderAY
				,pLastsend->nOrderBX
				,pLastsend->nInputBit
				);

			//送信先IDを作成
			uint32_t nSendID = CreateSinglePacketID(nTargetID);

			//送信パケットを保存
			StockLastSendData(nTargetID,pPacket,8,true);

			//固定長で送信
			nResult = CanSend8(nSendID,pPacket,8);

			//受信バッファ指定有りでここ迄エラー無しならシングルパケットDP0を受信
			if(pPtr && (nResult == 0))
				nResult = abh3_can_recv(nTargetID,pPtr,PACKETTYPE::SINGLE_PACKET);

			//開放
			CCan1939::FreeBuffer(pPacket);
			}
		//通信排他制御用のセマフォを開放
		Unlock();
		}

	//完了
	return(nResult);
	}


//ブロードキャストパケット送信
int32_t CAbh3::abh3_can_reqBRD(uint8_t nTargetID,uint8_t num,uint8_t nBroadcast,pCANABH3_RESULT pPtr)
	{
	//概要
	//	ブロードキャストパケットの送信
	//パラメータ
	//	nTargetID	送信先ID
	//	num			グループ番号とアドレスから計算される値（例：グループ5でアドレス0なら0x28）
	//	nBoradcast	0..現在の接続先が送信先  0以外..全ABH3が送信先
	//	pPtr		受信用領域、NULL指定で受信しない
	//戻り値
	//	0			正常終了
	//	-1			CAN回線が使用中
	//	上記以外	その他のエラー
	//

	//戻り値
	int32_t nResult = -1;

	//通信排他制御用のセマフォを取得
	if(Lock() == 0)
		{
		//インターフェースを開いている？
		if(IsOpenInterface())
			{
			//データ部先頭3バイトのシグネチャを構築
			uint32_t nSign = CreateBrdID(num);

			//ブロードキャストパケット(num)を構築
			uint8_t* pPacket = CCan1939::CreateBRD(nSign);

			//送信先CANIDを作成
			uint32_t nSendID = CreateBroadcastPacketID(nTargetID,num,nBroadcast);

			//送信パケットを保存
			StockLastSendData(nTargetID,pPacket,3,false);

			//固定長で送信
			nResult = CanSend8(nSendID,pPacket,3);

			//受信バッファ指定有りでここ迄エラー無しならブロードキャストパケットを受信
			if(pPtr && (nResult == 0))
				{
				int nAdrs = num & 0x07;	//下位3ビットがアドレス
				nResult = abh3_can_recv(nTargetID,pPtr,PACKETTYPE::BROADCAST_PACKET,nAdrs);
				}

			//開放
			CCan1939::FreeBuffer(pPacket);
			}
		//通信排他制御用のセマフォを開放
		Unlock();
		}

	//完了
	return(nResult);
	}

//マルチパケットによるTelABH3パケットの送受信
int32_t CAbh3::abh3_can_trans(uint8_t nTargetID,char* sbuf,char*& rbuf,size_t& rbuflen)
	{
	//概要
	//	sbuf		送信データが格納されたポインタ
	//	rbuf		受信データを格納するポインタ変数への参照
	//	rbuflen		受信データバッファ長
	//戻り値
	//	0			正常
	//	0以外		異常
	//注意点
	//	rbufは呼び出し元で開放(delete)する必要があります
	//	通信が成功又は失敗するまで処理が戻りません
	//使用例
	//	char* sbuf = "CP000095";
	//	char* rbuf = NULL;
	//	size_t rbuflen = 0;
	//	if(abh3_can_trans(sbuf,rbuf,rbuflen) == 0)
	//		{
	//		//正常な場合の処理
	//		printf("%s\n",rbuf);
	//		}
	//	else
	//		{
	//		//異常な場合の処理
	//		printf("Error\n");
	//		}
	//	if(rbuf)
	//		delete rbuf;

	//戻り値
	int32_t nResult = -1;

	//通信排他制御用のセマフォを取得
	if(Lock() == 0)
		{
		//インターフェースを開いている？
		if(IsOpenInterface())
			{
			//一時バッファ構築
			//2020.10.09 yo0043 以下仕様に変更
			//	データ部分が最低9バイトになる様に1つ以上のSYNを追加

			//文字列のサイズを元に、バッファサイズを算出
			size_t nSrcLen = ::strlen(sbuf);
			size_t nBufLen = 0;
			if(nSrcLen < 9)
				nBufLen = 9;
			else
				nBufLen = nSrcLen + 1;

			//終端分を考慮してバッファを構築
			char* pTmp = new char[nBufLen + 1]();	//初期化付きで確保（終端の為）

			//終端を除いてSYN(0x16)で埋める
			::memset(pTmp,0x16,nBufLen);

			//元文字列を先頭から文字数分だけコピーする
			::memcpy(pTmp,sbuf,nSrcLen);

			//型変換
			uint8_t* pSrc = (uint8_t*)pTmp;
			uint8_t* pDst = 0;
			uint32_t nDstSize = 0;

			//送受信
			nResult = CanTermSendMulti(nTargetID,pSrc,uint32_t(nBufLen),pDst,nDstSize);
			if(nResult == 0)
				{
				rbuf = (char*)pDst;
				rbuflen = (size_t)nDstSize;
				}
			//一時バッファ開放
			delete [] pTmp;
			}
		//通信排他制御用のセマフォを開放
		Unlock();
		}

	//完了
	return(nResult);
	}

//現在の受信データ状況を一括取得
void CAbh3::abh3_can_copylastdata(uint8_t nTargetID,pCANABH3_LASTDATA pPtr)
	{
	//
	pCANABH3_LASTDATA pSrc = GetLastData(nTargetID);
	if(pSrc && pPtr)
		::CopyMemory(pPtr,pSrc,sizeof(CANABH3_LASTDATA));
	}

//受信データ状況の指定箇所の更新フラグを解除
void CAbh3::abh3_can_resetlastdata(uint8_t nTargetID,int32_t nAdrs)
	{
	if(nAdrs < 0)
		{
		for(nAdrs = 0;nAdrs < 8;nAdrs++)
			abh3_can_resetlastdata(nTargetID,nAdrs);
		}
	else
		{
		pCANABH3_LASTRECV pSrc = GetLastRecvData(nTargetID);
		if(pSrc)
			pSrc->update[nAdrs & 0x7].nUpdate = 0;
		}
	}

//受信(外部用・排他制御あり)
int32_t CAbh3::abh3_can_read(uint8_t nTargetID,pCANABH3_RESULT pPtr,PACKETTYPE nType,uint8_t nAdrs)
	{
	//戻り値
	int32_t nResult = -1;

	//通信排他制御用のセマフォを取得
	if(Lock() == 0)
		{
		nResult =  abh3_can_recv(nTargetID,pPtr,nType,nAdrs);
		Unlock();
		}
	//
	return(nResult);
	}

//受信(外部用・排他制御あり)
int32_t CAbh3::abh3_can_flush()
	{
	//概要
	//	受信バッファにある内容を全て取得し、最終受信データに反映させる
	//パラメータ
	//	無し
	//戻り値
	//	受信バッファから取得して処理した数
	//

	//戻り値
	int32_t nCount = 0;

	//ダミーリード用バッファ
	uint32_t nCanID;
	uint8_t data8[8];

	//デバイスクラスが登録済みなら受信
	if(m_pVar->pDeviceClass)
		{
		//下位層のバッファから読み出す（バッファが空で失敗する迄繰り返す）
		int32_t nResult = 0;
		while(nResult == 0)
			{
			//1つ読み出す
			nResult = m_pVar->pDeviceClass->OnCanRecv(&nCanID,data8,true);
			if(nResult == 0)
				{
				//最終受信データへ反映させる
				StockLastRecvData(nCanID,data8);
				++nCount;
				}
			}
		}

	//完了
	return(nCount);
	}

//回線を閉じる
int32_t CAbh3::abh3_can_finish()
	{
	//通信排他制御用のセマフォを取得
	if(Lock() == 0)
		{
		CloseInterface();
		Unlock();
		}

	return(0);
	}

//================================================================================
//内部用
//================================================================================

//受信のみ（内部用）
int32_t CAbh3::abh3_can_recv(uint8_t nTargetID,pCANABH3_RESULT pPtr,PACKETTYPE nType,uint8_t nAdrs /* 0 */)
	{
	//概要
	//	CANインターフェースからデータを受信
	//パラメータ
	//	nTargetID		この発信元から来たパケットを対象とする
	//	pPtr			受信データ格納先へのポインタ
	//	nType			受信を終了するパケット種類
	//					PACKETTYPE::SINGLE_PACLET 又は PACKETTYPE::BROADCAST_PACKET を指定する
	//	nAdrs			機器アドレスを指定（ブロードキャストパケット指定時のみ利用される）
	//
	//戻り値
	//	0			正常終了
	//	0以外		異常終了
	//
	//終了判定条件
	//	nType == PACKETTYPE::ANY_PACKET
	//		何か受信したら戻る。（nTargetID,nAdrsは無視される）
	//	nType == PACKETTYPE::SINGLE_PACKET
	//		nTargetIDから発信されたシングルパケットを受信したら戻る。（nAdrsは無視される）
	//	nType == PACKETTYPE::BROADCAST_PACKET
	//		nTargetIDから発信されたブロードキャストパケットで、nAdrsのアドレスが一致したら戻る。
	//		グループ番号は無視される
	//
	//本関数が前提としている条件
	//	todo
	//
	//注意点
	//	この関数は他の通信関数から呼び出される為、ロック禁止
	//	指定した種類のパケットが来ない状態で、タイムアウトにならない周期毎に
	//	パケットを受信する状態になると処理から抜けられない場合が有る事に注意。
	//

	//受信
	int32_t nResult = 0;
	while(nResult == 0)
		{
		//受信
		nResult = CanRecv8(&pPtr->nID,pPtr->u.raw);

		//無条件？
		if(nType == PACKETTYPE::ANY_PACKET)
			break;

		//正常受信？
		else if(nResult == 0)
			{
			//パケット識別
			uint8_t nPacketSenderID = 0;
			uint8_t nPacketTargetID = 0;
			uint8_t nPacketGroup = 0;
			uint8_t nPacketAdrs = 0;
			PACKETTYPE nPacketType = recvid2any(pPtr->nID,nPacketSenderID,nPacketTargetID,nPacketGroup,nPacketAdrs,NULL);

			//判定

			//シングルパケット指定？
			if((nType == PACKETTYPE::SINGLE_PACKET) && (nPacketType == PACKETTYPE::SINGLE_PACKET))
				{
				//発信元が指定した所？
				if(nTargetID == nPacketSenderID)
					break;	//該当
				}
			//ブロードキャストパケット指定？
			else if(nType == PACKETTYPE::BROADCAST_PACKET)
				{
				if((nPacketType == PACKETTYPE::BROADCAST_PACKET) && (nAdrs == nPacketAdrs))
					{
					//発信元が指定した所？
					if(nTargetID == nPacketSenderID)
						break;	//該当
					}
				}
			}
		else
			{
			//受信自体が失敗
			}
		}
	//
	return(nResult);
	}

//8[bytes]送受信
int32_t CAbh3::CanSend8(uint32_t nSendID,uint8_t* pSendData,uint8_t nLength)
	{
	//概要
	//	CANインターフェースに対してIDを指定してデータを送信
	//パラメータ
	//	nSendID		CAN-ID(29bit)
	//	pSendData	送信データへのポインタ
	//戻り値
	//	0			正常終了
	//	0以外		異常終了
	//注意点
	//	排他制御はこの関数の呼び出し元が行う事

	//戻り値
	int32_t nResult = -1;

	//デバイスクラスが登録済みなら送信
	if(m_pVar->pDeviceClass)
		{
		nResult = m_pVar->pDeviceClass->OnCanSend(nSendID,pSendData,nLength);
		}

	//完了
	return(nResult);
	}

//受信
int32_t CAbh3::CanRecv8(uint32_t* pRecvID,uint8_t* pRecvData)
	{
	//概要
	//	CANインターフェースからデータを受信
	//パラメータ
	//	pRecvID		受信したCAN-ID(29bit)格納先へのポインタ
	//	pRecvData	受信データ格納先へのポインタ
	//戻り値
	//	0			正常終了
	//	0以外		異常終了
	//注意点
	//	排他制御はこの関数の呼び出し元が行う事

	//戻り値
	int32_t nResult = -1;

	//デバイスクラスが登録済みなら受信
	if(m_pVar->pDeviceClass)
		{
		//下位層のバッファから1つ持ってくる
		nResult = m_pVar->pDeviceClass->OnCanRecv(pRecvID,pRecvData);

		//受信成功？
		if(nResult == 0)
			{
			//最終データ格納処理に渡す（格納されるかは中身次第）
			StockLastRecvData(*pRecvID,pRecvData);
			}
		}

	//完了
	return(nResult);
	}


//受信IDから識別に必要な要素を抽出
PACKETTYPE CAbh3::recvid2any(uint32_t nCANID,uint8_t& nSenderID,uint8_t& nTargetID,uint8_t& nGroup,uint8_t& nAdrs,uint8_t* pRecvData)
	{
	//パラメータ
	//	nCANID		受信したパケットのCANID（送信パケットのは使えないので注意）
	//	nSenderID	パケットを送信した機器のIDの格納先
	//	nTargetID	受信先に設定されているIDの格納先
	//	nGroup		ブロードキャストパケットの場合、グループ番号の格納先
	//	nAdrs		ブロードキャストパケットの場合、機器アドレスの格納先
	//戻り値
	//	PACKETTYPE::SINGLE_PACKET		シングルパケット
	//	PACKETTYPE::BROADCAST_PACKET	ブロードキャストパケット
	//	PACKETTYPE::UNKNOWN_PACKET		判定不可
	//

	//戻り値
	PACKETTYPE nResult = PACKETTYPE::UNKNOWN_PACKET;

	uint8_t nD2 = uint8_t(nCANID >> 16);	//xxXXxxxx
	uint8_t nD1 = uint8_t(nCANID >> 8);		//xxxxXXxx
	uint8_t nD0 = uint8_t(nCANID);			//xxxxxxXX


	//(確定)送信元
	nSenderID = nD0;

	//ブロードキャストパケットの返答？(xxFFxxxx)
	if(nD2 == 0xff)
		{
		//(確定)ブロードキャストパケット
		nResult = PACKETTYPE::BROADCAST_PACKET;
		//(確定)グループ番号とアドレス
		nAdrs = nD1 & 0x07;				//(bit:xxxxxXXX)
		nGroup = (nD1 >> 3) & 0x1f;		//(bit:XXXXXxxx)
		//受信先は含まれない為、ホストIDを設定する
		nTargetID = GetHostID();
		}
	//シングルパケットの返答又は要求？(xxEFxxxx)
	else if(nD2 == GetSinglePacketCode(nSenderID))
		{
		//(確定)シングルパケット
		nResult = PACKETTYPE::SINGLE_PACKET;
		//(確定)受信先
		nTargetID = nD1;
		//グループとアドレスは含まれない為、0を設定する
		nGroup = 0;
		nAdrs = 0;
		}
	//受信データ指定有り？（ブロードキャスト要求を判断する場合は指定必須）
	else if(pRecvData)
		{
		//ブロードキャストパケットの要求？(xxEAxxxx)
		if(nD2 == GetBroadcastPacketCode(nSenderID))
			{
			//(確定)ブロードキャストパケット
			nResult = PACKETTYPE::BROADCAST_PACKET;
			//(確定)グループ番号とアドレス
			nAdrs = pRecvData[0] & 0x07;	//下位3bit
			nGroup = (pRecvData[0] >> 3) & 0x1f;		//上位5bit
			//(確定)受信先
			nTargetID = nD1;
			}
		}

	//
	return(nResult);
	}

//送信するパケットデータを最終データとして格納
void CAbh3::StockLastSendData(uint8_t nSendID,uint8_t* pSendData,uint8_t nLength,bool bSinglePacket)
	{
	//
	pCANABH3_LASTSEND pLastdata = GetLastSendData(nSendID);
	if(pLastdata == NULL)
		return;	//準備前
	//シングルパケット？
	if(bSinglePacket)
		{
		//
		CopyMemory(pLastdata->DP0R.packet,pSendData,nLength);
		pLastdata->DP0R.nLength = nLength;
		}
	else if((pSendData[0] == 0x00) && (pSendData[1] == 0xff))
		{
		//ブロードキャストパケット
		uint8_t nGroup = (pSendData[2] >> 3) & 0xf;
		uint8_t nAdrs = (pSendData[2] & 0x7);
		CopyMemory(pLastdata->BR[nAdrs].packet,pSendData,nLength);
		pLastdata->BR[nAdrs].nLength = nLength;
		}
	}

//受信したデータを最終データとして格納
void CAbh3::StockLastRecvData(uint32_t nRecvID,uint8_t* pRecvData)
	{
	//注意点
	//	判別で送信元が無条件で確定する
	//	ブロードキャストパケットの場合、グループ番号は格納に使わない
	//	（送信元IDとアドレスで格納先が決定する為）
	//

	//パケット判別
	uint8_t nSenderID = 0;
	uint8_t nTargetID = 0;
	uint8_t nGroup = 0;
	uint8_t nAdrs = 0;
	PACKETTYPE nType = recvid2any(nRecvID,nSenderID,nTargetID,nGroup,nAdrs,pRecvData);

	//格納先
	pCANABH3_LASTRECV pLastdata = GetLastRecvData(nSenderID);

	//シングルパケット？
	if(nType == PACKETTYPE::SINGLE_PACKET)
		{
		//格納先にコピー
		::CopyMemory(&pLastdata->DP0R,pRecvData,8);
		//更新フラグ
		pLastdata->update[0].nUpdate = 1;
		}
	//ブロードキャストパケット？
	else if(nType == PACKETTYPE::BROADCAST_PACKET)
		{
		//各アドレスに保存
		if(nAdrs == 0)
			::CopyMemory(&pLastdata->BR0,pRecvData,8);
		else if(nAdrs == 1)
			::CopyMemory(&pLastdata->BR1,pRecvData,8);
		else if(nAdrs == 2)
			::CopyMemory(&pLastdata->BR2,pRecvData,8);
		else if(nAdrs == 3)
			::CopyMemory(&pLastdata->BR3,pRecvData,8);
		else if(nAdrs == 4)
			::CopyMemory(&pLastdata->BR4,pRecvData,8);
		else if(nAdrs == 5)
			::CopyMemory(&pLastdata->BR5,pRecvData,8);
		else if(nAdrs == 6)
			::CopyMemory(&pLastdata->BR6,pRecvData,8);
		//更新フラグ
		pLastdata->update[nAdrs + 1].nUpdate = 1;
		}
	else
		{
		//不明なパケットを受信
		}
	}

//マルチパケット送信
int32_t CAbh3::CanTermSendMulti(uint8_t nTargetID,uint8_t* pSendData,uint32_t nSendDataSize,uint8_t*& pRecvData,uint32_t& nRecvDataSize)
	{
	//概要
	//	マルチパケット送信（PC側を起点とする送受信）を行う
	//パラメータ
	//	pSendData		送信データへのポインタ
	//	nSendDataSize	送信データ長[bytes]
	//	pRecvData		受信データを扱うポインタへの参照
	//	nRecvDataSize	受信データ長を格納する変数の参照
	//戻り値
	//	0				正常終了
	//	-11				通信先から中止要求(ABORT)を受信
	//	-12				通信先からセッション重複要求（複数回のRTS）を受信
	//	-13				PC側で送信失敗
	//	-14				PC側で受信失敗（タイムアウト）
	//
	//注意点
	//	関数が正常に終了した場合、pRecvDataには受信データが格納された領域へのポインタが入りますが、
	//	このポインタは呼び出し元で開放する必要が有ります
	//	pSendData末尾にSYN(0x16)を追加してから呼び出す事
	//	(abh3_can_trans関数内で追加している）

	//戻り値
	int32_t nExitCode = 0;

	//
	uint32_t nStage = 0;	//状態遷移処理用ステージ番号
	int32_t nResult = 0;	//汎用の戻り値
	uint8_t nAbort = 0;		//Abort返答時の理由

	//ホスト側から見た送信用IDと受信判定用ID
	uint32_t nSendID		= 0x00ec0000 | (nTargetID << 8) | m_pVar->config.nHostAdrs;
	uint32_t nSendDTID		= 0x00eb0000 | (nTargetID << 8) | m_pVar->config.nHostAdrs;
	uint32_t nRecvID		= 0x00ec0000 | (m_pVar->config.nHostAdrs << 8) | nTargetID;
	uint32_t nRecvDTID		= 0x00eb0000 | (m_pVar->config.nHostAdrs << 8) | nTargetID;
	uint32_t nID			= 0;	//
	uint8_t nMaxPacket		= 0;	//送信許可パケット数
	uint8_t nPacketNum		= 0;	//パケット番号（1-255)
	uint8_t nTotalPacket	= 0;	//受信時の総パケット数
	//

	//受信バッファクリア
	ClearRecvBuffer();

	while(-1)
		{
		//RTSを送るステージ
		//	PCが送信ノード・ABH3が受信ノードで送信要求を送る
		if(nStage == 0)
			{
			//送信許可パケット数を初期化（ホストの最大値）
			nMaxPacket = 0xff;

			//CM_RTSパケット構築
			uint8_t* pPacket = CCan1939::CreateCMRTS(nSendDataSize,nMaxPacket);

			//送信パケットを保存
			StockLastSendData(nTargetID,pPacket,8,false);

			//送信
			nResult = CanSend8(nSendID,pPacket,8);

			//パケットの開放
			CCan1939::FreeBuffer(pPacket);

			//送信正常？
			if(nResult == 0)
				nStage = 1;
			else
				{
				//送信失敗
				nExitCode = -13;
				nStage = 99;	//RTS送信時に失敗、何も送信してないのでそのまま終了
				}
			}

		//CTS/EOMA/ABORTを受け取るステージ
		//	PCが送信ノード・ABH3が受信ノードで送信要求(RTS)又はデータ送信(DT)の返答が戻る
		else if(nStage == 1)
			{
			//受信
			uint8_t* pPacket = CCan1939::CreateBuffer();
			nResult = CanRecv8(&nID,pPacket);

			if(nResult == 0)
				{
				//受信は正常

				//何のパケット？
				uint8_t nType = CCan1939::IsPacket(pPacket);

				//送信要求？
				if(nType == CCan1939::DEF_CTS)
					{
					//ターゲット側から指定された送信許可パケット数を保存
					nMaxPacket = pPacket[1];

					//次に送信するパケット番号を保存
					nPacketNum = pPacket[2];

					//DTを送るステージに遷移
					nStage = 2;
					}
				//完了通知？
				else if(nType == CCan1939::DEF_EOMA)
					{
					//受信完了扱い
					nStage = 3;
					}
				//中止要求？
				else if(nType == CCan1939::DEF_ABORT)
					{
					//中止扱い
					nExitCode = -11;
					nStage = 99;
					}
				//新規要求？
				else if(nType == CCan1939::DEF_RTS)
					{
					//セッション重複扱いでエラー
					nExitCode = -12;
					nAbort = 1;
					nStage = 10;
					}
				//上記以外？
				else
					{
					//上記以外は見なかった事にして再送を待つ
					Sleep(0);
					}
				}
			else
				{
				//受信処理に失敗（タイムアウト）
				nExitCode = -14;
				nAbort = 3;	//Timeout
				nStage = 10;
				}

			//パケットの開放
			CCan1939::FreeBuffer(pPacket);
			}

		//DTを送るステージ
		//	PCが送信ノード・ABH3が受信ノードで1つ前の返答(CTS)で指定されたデータを送る
		//	送る位置(nPacketNum)と個数(nMaxPacket)は、1つ前の返答(CTS)で指定されている
		//	データの送信試行回数は「通信先が指定した回数」であり、それよりデータが短い場合は
		//	少ない回数となる
		else if(nStage == 2)
			{
			//要求数が0?
			if(nMaxPacket < 1)
				{
				//最低1つは送る
				nMaxPacket = 1;
				}
			//
			for(uint8_t nLoop = 0;nLoop < nMaxPacket;nLoop++)
				{
				//CM_DTパケットを作成（データオーバーランする場合はNULLが戻る）
				uint8_t* pPacket = CCan1939::CreateCMDT(pSendData,nSendDataSize,nPacketNum);
				//送信対象のデータが有る？
				if(pPacket)
					{
					//送信パケットを保存
					StockLastSendData(nTargetID,pPacket,8,false);

					//送信
					nResult = CanSend8(nSendDTID,pPacket,8);	//CM_DT送信

					//パケットの開放
					CCan1939::FreeBuffer(pPacket);

					//送る位置を更新
					++nPacketNum;
					}

				//送信失敗？
				if(nResult)
					break;	//途中で抜ける
				}

			//CM_DTパケットの送信に失敗？
			if(nResult)
				{
				//送信失敗
				nExitCode = -13;
				nAbort = 2;	//resource不足
				nStage = 10;
				}
			else
				nStage = 1;
			}

		//EOMAを受け取った後のRTS受信ステージ
		//	PCが受信ノード・ABH3が送信ノードに役割変更し、返答(CM_RTS)が届くはずなのでそれを受信
		else if(nStage == 3)
			{
			//受信領域構築
			uint8_t* pPacket = CCan1939::CreateBuffer();
			nResult = CanRecv8(&nID,pPacket);

			//受信は正常？
			if(nResult == 0)
				{
				//何のパケット？
				uint8_t nType = CCan1939::IsPacket(pPacket);
				//CM_RTS?
				if(nType == CCan1939::DEF_RTS)
					{
					//メッセージサイズを取得し、バッファを構築
					nRecvDataSize = CCan1939::Get16L(pPacket,1);
					pRecvData = new uint8_t[nRecvDataSize + 1]();	//終端用に+1して初期化済みで確保する

					//ターゲット側から指定された総パケット数を保存
					nTotalPacket = CCan1939::Get8L(pPacket,3);

					//ターゲット側から指定された送信許可パケット数を保存
					nMaxPacket = pPacket[4];

					//パケット番号を初期化
					nPacketNum = 1;

					//CTSを返答するステージに遷移
					nStage = 4;
					}
				//CM_EOMA?
				else if(nType == CCan1939::DEF_EOMA)
					{
					nStage = 6;
					}
				//CM_ABORT?
				else if(nType == CCan1939::DEF_ABORT)
					{
					//中止扱い
					nExitCode = -11;
					nStage = 99;
					}
				//上記以外？
				else
					{
					//上記以外は見なかった事にして再送を待つ
					Sleep(0);
					}
				}
			else
				{
				//受信処理時に失敗、ABORT送信
				nExitCode = -14;
				nAbort = 3;	//Timeout
				nStage = 10;	
				}

			//パケットの開放
			CCan1939::FreeBuffer(pPacket);
			}
		//CTS返答ステージ
		//	PCが受信ノード・ABH3が送信ノードで、受信要求(CM_RTS)又はデータ(CM_DT)の返答(CM_CTS)が必要なのでそれを送信
		else if(nStage == 4)
			{
			//送信許可パケットはターゲット側が指定した値をそのまま利用

			//CM_CTSパケットを作って送信
			uint8_t nRemainPacket = nMaxPacket - nPacketNum + 1;				//残りのパケット数
			uint8_t* pPacket = CCan1939::CreateCMCTS(nRemainPacket,nPacketNum);

			//送信パケットを保存
			StockLastSendData(nTargetID,pPacket,8,false);

			//送信
			nResult = CanSend8(nSendID,pPacket,8);

			//パケットの開放
			CCan1939::FreeBuffer(pPacket);

			//CM_CTS送信正常？
			if(nResult == 0)
				nStage = 5;
			else
				{
				//送信失敗
				nExitCode = -13;
				nAbort = 2;	//resource不足
				nStage = 10;
				}
			}

		//DT受信ステージ
		//	PCが受信ノード・ABH3が送信ノードで、要求(CM_CTS)又はデータ(CM_DT)の返答(CM_RTS)が必要なのでそれを送信
		else if(nStage == 5)
			{
			//
			if(nMaxPacket < 1)
				{
				nMaxPacket = 1;
				}

			//受信領域構築
			bool bRetry = false;
			uint8_t* pPacket = CCan1939::CreateBuffer();

			//送信されるデータ個数（予定数）の数だけ受信処理を行う
			for(uint8_t nLoop = 0;nLoop < nMaxPacket;nLoop++)
				{
				//受信
				nResult = CanRecv8(&nID,pPacket);

				//受信は正常？
				if(nResult == 0)
					{
					//シーケンス番号が異なる？
					if(pPacket[0] != nPacketNum)
						{
						//要求したのと異なる物が届いたので、再度要求する
						bRetry = true;
						break;
						}
					
					//データ格納（7バイト）
					CCan1939::SetData(pRecvData,nRecvDataSize,CCan1939::packetnum2datapt(nPacketNum),&pPacket[1],7);

					//最終パケット番号？
					if(nPacketNum == nTotalPacket)
						break;	//完了

					//要求したパケット番号で届いたので、次のパケット番号に更新
					++nPacketNum;
					}
				else
					{
					//受信処理に失敗
					nExitCode = -14;
					nAbort = 3;	//Timeout
					nStage = 10;
					break;
					}
				}

			//パケットの開放
			CCan1939::FreeBuffer(pPacket);

			//リトライが必要？
			if(bRetry)
				{
				//パケット番号(nPacketNum)が正しくない箇所から再度データ(CM_DT)を要求する
				nStage = 4;
				}

			//正常？
			else if(nResult == 0)
				{
				if(nPacketNum == nTotalPacket)
					nStage = 6;	//EOMA送信ステージに遷移
				else
					nStage = 4;	//CTS返答に戻る
				}

			else
				{
				//受信失敗、受信ルーチン内で移行先が設定されている為、何もしない
				Sleep(0);
				}
			}

		//EOMA返答ステージ
		else if(nStage == 6)
			{
			//パケットを作って送信
			uint8_t* pPacket = CCan1939::CreateCMEOMA(nTotalPacket * 8,nTotalPacket);

			//送信パケットを保存
			StockLastSendData(nTargetID,pPacket,8,false);

			//
			nResult = CanSend8(nSendID,pPacket,8);
	
			//パケットの開放
			CCan1939::FreeBuffer(pPacket);

			//送信正常？
			if(nResult == 0)
				{
				//完了ステージに移行
				nStage = 7;
				}
			else
				{
				//送信失敗
				nExitCode = -13;
				nAbort = 2;
				nStage = 10;
				}
			}

		//完了ステージ
		else if(nStage == 7)
			{
			//正常完了
			break;
			}

		//ホスト側から中止要求ステージ
		//	CM_ABORTを送る、nAbortに理由を入れておく(2...リソース不足  3..タイムアウト)
		else if(nStage == 10)
			{
			//ABORTパケットを作って送信
			uint8_t* pPacket = CCan1939::CreateCMABORT(nAbort);

			//送信パケットを保存
			StockLastSendData(nTargetID,pPacket,8,false);

			nResult = CanSend8(nSendID,pPacket,8);

			//パケットの開放
			CCan1939::FreeBuffer(pPacket);

			//エラーステージへ遷移
			nStage = 99;
			}

		//上記以外（エラーステージ）
		else
			{
			//異常終了
			break;
			}
		}

	//終了
	return(nExitCode);
	}

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
//========================================
//概要
//========================================
//	Ixxat V2のデバイスを使う為のクラス
//
//========================================
//履歴
//========================================
//2023/02/24	yo0043		OnCanRecvに待ち時間なし機能追加
//
//========================================
//実装に必要な前処理
//========================================
//	IxxatV2用のSDKをインストール
//	プロジェクト設定で、追加インクルードフォルダに以下を追加
//		"C:\Program Files\HMS\Ixxat VCI\sdk\vci\inc"
//	プロジェクト設定で、追加ライブラリフォルダに以下を追加
//		32bit "C:\Program Files\HMS\Ixxat VCI\sdk\vci\lib\x32\release"
//
//========================================
//使用方法
//========================================
//	基本的な使い方は、サンプルソースを見て下さい
//
//========================================
///デバイス固有
//========================================
//エラー発生時
//	基本的に関数でエラーが発生したら、回線を切断し、接続する所から
//	やり直しを推奨します。（リセットで全要素が消えるので、結局繋ぎ直し処理必須となる為）
//
//	ボーレートには以下の値のみが設定可能です
//		値		設定			値		設定	
//	---------------------------------------------			
//		10	->	10Kbps			20	->	20Kbps
//		50	->	50Kbps			100	->	100Kbps
//		125	->	125Kbps			250	->	250Kbps
//		500	->	500Kbps			800	->	800Kbps
//		1000->	1000Kbps
//	---------------------------------------------			
//
//OnOpenInterfaceに渡すデバイスの番号
//	何本目のインターフェースか(0開始)を渡して下さい
//
//========================================

//
#include "pch.h"
#include "IxxatV2.h"
#include <process.h>

//
#pragma warning(disable : 6385)

//本クラスで扱うデバイスに必須のライブラリ
#pragma comment(lib,"vciapi.lib")


//安全な削除
template <class T> void SafeDelete(T** ppT)
	{
	if (*ppT)
		{
		delete *ppT;
		*ppT = NULL;
		}
	}

//安全な解放
template <class T> void SafeRelease(T **ppT)
	{
	if (*ppT)
		{
		(*ppT)->Release();
		*ppT = NULL;
		}
	}

//コンストラクタ
CIxxatV2::CIxxatV2()
	{
	//
	::ZeroMemory(&m_status,sizeof(IXXATV2_STATUS));
	m_pBalObject	= 0;	// bus access object
	m_lSocketNo		= 0;	// socket number
	m_pCanControl	= 0;	// control interface
	m_pCanChn		= 0;	// channel interface
	m_lMustQuit		= 0;	// quit flag for the receive thread
	m_hEventReader	= 0;
	m_pReader		= 0;
	m_pWriter		= 0;
	m_hSema			= NULL;
	m_hReadThread	= 0;
	m_nCanReadPt	= 0;
	m_nCanWritePt	= 0;
	m_pCanMsg		= NULL;

	//受信データ格納用領域の初期化
	m_pCanMsg = new CANMSG[256]();
	}

//デストラクタ
CIxxatV2::~CIxxatV2()
	{
	//回線を開いている場合が有るので閉じる処理を行う
	OnCloseInterface();

	//
	SafeDelete(&m_pCanMsg);

	}

//================================================================================
//使用CANインターフェース毎に実装が必須の仮想関数
//================================================================================

//利用可能なCANインターフェース数を取得する場合に呼び出されます
int32_t CIxxatV2::OnGetInterfaceCount()
	{
	//概要
	//	指定CANインターフェースの利用可能数を取得する場合に呼び出されます
	//パラメータ
	//	無し
	//戻り値
	//	0			利用可能インターフェースがありません
	//	-1			調査時にエラーが発生しました
	//	上記以外	利用可能なCANインターフェース数
	//

	//todo:CANインターフェースの数を戻す処理を実装して下さい

	//デバイスマネージャを開く
	IVciDeviceManager*  pDevMgr = 0;    // device manager
	HRESULT hResult = VciGetDeviceManager(&pDevMgr);
	if (hResult != VCI_OK)
		{
		//デバイスマネージャでエラー
		return(-1);
		}

	//接続されたインターフェースデバイスを列挙して数を調べる
	VCIDEVICEINFO sInfo;
	int32_t nCount = 0;
	IVciEnumDevice* pEnum = NULL;
	hResult = pDevMgr->EnumDevices(&pEnum);
	while (hResult == VCI_OK)
		{
		//情報取得
		hResult = pEnum->Next(1,&sInfo,NULL);
		//取得成功？
		if (hResult == VCI_OK)
			++nCount;	//デバイス数加算
		}
	SafeRelease(&pEnum);

	//完了
	return(nCount);
	}

//指定CANインターフェースを開く場合に呼び出されます
int32_t CIxxatV2::OnOpenInterface(int32_t nDeviceNum)
	{
	//概要
	//	指定CANインターフェースを開く場合に呼び出されます
	//パラメータ
	//	nDeviceNum		0開始のインターフェース番号
	//戻り値
	//	0				正常終了
	//	0以外			異常終了

	//todo:CANインターフェースを開く処理を実装して下さい

	//
	if(nDeviceNum >= BAL_MAX_SOCKETS)
		return(-2);	//デバイス番号の指定が正しくない

	//設定から回線を開く為の許容時間を取得
	uint32_t nTimeoutMS = GetTimeout_Open();

	//1つ以上のインターフェースが有るか？
	if (OnGetInterfaceCount() < 1)
		{
		return(-1);
		}

	//デバイスマネージャを取得する
	IVciDeviceManager*  pDevMgr = 0;
	HRESULT hResult = VciGetDeviceManager(&pDevMgr);

	//デバイスマネージャ取得に失敗？
	if (hResult != VCI_OK)
		{
		return(-1);
		}

	//デバイスマネージャを使用してデバイス列挙インターフェースを取得する
	IVciEnumDevice* pEnum = NULL;
	hResult = pDevMgr->EnumDevices(&pEnum);

	//デバイス列挙インターフェースを使用してデバイス数を取得する
	int32_t nCount = 0;	//
	VCIDEVICEINFO sInfo;
	while (hResult == VCI_OK)
		{
		//デバイス列挙インターフェースを使用して、デバイス情報を(先頭から)順に取得する
		hResult = pEnum->Next(1,&sInfo,NULL);
		if (hResult == VCI_OK)
			{
			//デバイス情報を正常に取得したので、デバイス数を更新
			++nCount;
			}
		}

	//指定インターフェース番号のデバイスは存在してない？
	if (nDeviceNum >= nCount)
		{
		SafeRelease(&pEnum);
		SafeRelease(&pDevMgr);
		return(-1);
		}

	//デバイスマネージャを使用してデバイスを取得する
	IVciDevice* pDevice = 0;
	hResult = pDevMgr->OpenDevice(sInfo.VciObjectId,&pDevice);
	if (hResult != VCI_OK)
		{
		//デバイスの取得に失敗した
		SafeRelease(&pEnum);
		SafeRelease(&pDevMgr);
		return(-1);
		}

	//デバイスを利用して、バスアクセスレイヤーを取得する
	hResult = pDevice->OpenComponent(CLSID_VCIBAL,IID_IBalObject,(void**)&m_pBalObject);

	//不要となったデバイスを開放
	SafeRelease(&pDevice);

	//不要となったデバイス列挙を開放
	SafeRelease(&pEnum);

	//不要となったデバイスマネージャを開放
	SafeRelease(&pDevMgr);

	//================================================================================
	//この時点で、未開放の物はバスアクセスレイヤーのみ
	//================================================================================

	//バスアクセスレイヤーが無効か？
	if (m_pBalObject == NULL)
		return(-1);

	//バスアクセスレイヤーを利用して、機能を取得
	BALFEATURES features = { 0 };
	hResult = m_pBalObject->GetFeatures(&features);
	if (hResult != VCI_OK)
		{
		//機能情報の取得に失敗
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//指定したデバイス番号が正しくない？
	if (nDeviceNum >= features.BusSocketCount)
		{
		//指定したデバイス番号はデバイス数を超過している
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//指定したデバイス番号のデバイスが利用可能なCANデバイスでは無い？
	if (VCI_BUS_TYPE(features.BusSocketType[nDeviceNum]) != VCI_BUS_CAN)
		{
		//指定したデバイス番号のデバイスは利用不可
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//バスアクセスレイヤーを使用してCAN用ソケットを開く
	ICanSocket* pCanSocket = 0;
	hResult = m_pBalObject->OpenSocket(nDeviceNum,IID_ICanSocket,(void**)&pCanSocket);
	if (hResult == VCI_OK)
		{
		//CAN用ソケットを使用して互換性を取得
		CANCAPABILITIES capabilities = { 0 };
		hResult = pCanSocket->GetCapabilities(&capabilities);
		if (VCI_OK == hResult)
			{
			//互換性情報は取得出来た
			//このCAN用ソケットでは、標準ID(11bits)と拡張ID(29bits)が使えるか？
			//（29bitsだけの判定方法が無い事に注意）
			if (capabilities.dwFeatures & CAN_FEATURE_STDANDEXT)
				{
				//両方のIDが使える（つまり拡張ID(29bits)が利用可能）

				//メッセージ用チャンネルを構築
				m_pCanChn = NULL;
				hResult = pCanSocket->CreateChannel(FALSE,&m_pCanChn);
				if(hResult != VCI_OK)
					{
					}
				}
			else
				{
				//使えない（多分標準ID(11bits)しか使えないと思われる）
				hResult = -1;
				}
			}
		else
			{
			//互換性情報の取得に失敗した
			}
		}

	//CAN用ソケットを開放
	SafeRelease(&pCanSocket);

	//エラー有り？
	if (VCI_OK != hResult)
		{
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//================================================================================
	//この時点で、未開放の物は
	//	バスアクセスレイヤー(m_pBalObject)
	//	メッセージチャンネル(m_pCanChn)
	//================================================================================

	//メッセージチャンネルを初期化
	UINT16 wRxFifoSize	= 1024;
	UINT16 wRxThreshold	= 1;		//1つ以上の受信で通知
	UINT16 wTxFifoSize	= 128;
	UINT16 wTxThreshold	= 1;		//送信残りが1つ未満で通知
	hResult = m_pCanChn->Initialize(wRxFifoSize,wTxFifoSize);
	if (hResult != VCI_OK)
		{
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//メッセージチャンネルを利用して、リーダーを取得する
	hResult = m_pCanChn->GetReader(&m_pReader);
	if (hResult == VCI_OK)
		{
		//リーダーが取得出来たので、通知レベル（通知される受信数）を設定する
		m_pReader->SetThreshold(wRxThreshold);

		//受信用イベントを構築
		m_hEventReader = CreateEvent(NULL,FALSE,FALSE,NULL);

		//リーダーが使うイベントとして登録
		m_pReader->AssignEvent(m_hEventReader);
		}
	else
		{
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//ライターを取得する
	hResult = m_pCanChn->GetWriter(&m_pWriter);
	if (hResult == VCI_OK)
		{
		//ライターが取得出来たので、通知レベル（通知される送信数）を設定する
		m_pWriter->SetThreshold(wTxThreshold);
		}
	else
		{
		::CloseHandle(m_hEventReader);
		m_hEventReader = NULL;
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//メッセージ用チャンネルを有効化する
	hResult = m_pCanChn->Activate();

	//エラー有り？
	if(hResult != VCI_OK)
		{
		//受信用イベント開放
		::CloseHandle(m_hEventReader);
		m_hEventReader = NULL;
		//メッセージチャンネル開放
		SafeRelease(&m_pCanChn);
		//バスアクセスレイヤー開放
		SafeRelease(&m_pBalObject);
		//失敗扱いで戻る
		return(-1);
		}

	//================================================================================
	//この時点で、未開放の物は
	//	バスアクセスレイヤー(m_pBalObject)
	//	メッセージチャンネル(m_pCanChn)
	//================================================================================

	//CANコントロールの取得
	hResult = m_pBalObject->OpenSocket(nDeviceNum,IID_ICanControl,(void**)&m_pCanControl);
	if (hResult == VCI_OK)
		{
		//ボーレートを取得し、設定用の値に変換
		uint32_t nBaudrateKbps = GetBaudrate();
		uint8_t nB0 = 0,nB1 = 0;
		switch(nBaudrateKbps)
			{
			case 10:
				nB0 = CAN_BT0_10KB;
				nB1 = CAN_BT1_10KB;
				break;
			case 20:
				nB0 = CAN_BT0_20KB;
				nB1 = CAN_BT1_20KB;
				break;
			case 50:
				nB0 = CAN_BT0_50KB;
				nB1 = CAN_BT1_50KB;
				break;
			case 100:
				nB0 = CAN_BT0_100KB;
				nB1 = CAN_BT1_100KB;
				break;
			case 125:
				nB0 = CAN_BT0_125KB;
				nB1 = CAN_BT1_125KB;
				break;
			case 250:
				nB0 = CAN_BT0_250KB;
				nB1 = CAN_BT1_250KB;
				break;
			case 500:
				nB0 = CAN_BT0_500KB;
				nB1 = CAN_BT1_500KB;
				break;
			case 800:
				nB0 = CAN_BT0_800KB;
				nB1 = CAN_BT1_800KB;
				break;
			case 1000:
				nB0 = CAN_BT0_1000KB;
				nB1 = CAN_BT1_1000KB;
				break;
			default:
				nB0 = CAN_BT0_50KB;
				nB1 = CAN_BT1_50KB;
				break;
			}

		//CANコントロールを利用して、CANラインを指定ボーレート設定で初期化する
		CANINITLINE init =
			{
			 CAN_OPMODE_STANDARD | CAN_OPMODE_EXTENDED | CAN_OPMODE_ERRFRAME		// opmode
			,0																		// bReserved
			,nB0																	// bt0のボーレート
			,nB1																	// bt1のボーレート
			};
		hResult = m_pCanControl->InitLine(&init);
		if (hResult != VCI_OK)
			{
			//CANラインの初期化に失敗
			}

		//此処迄にエラーが無いか？
		if (hResult == VCI_OK)
			{
			//CANコントロールを利用して、アクセスタンスフィルタに標準CANID(11bits)を指定してみる
			//dwCode = 評価IDを指定、ID未指定時はCAN_ACC_CODE_NONEを指定するか、dwMaskで全ビット無効にする
			//dwMask = 評価IDの「有効なbit」を指定（0..無効 1..有効）
			//	#define CAN_ACC_MASK_ALL          0x00000000
			//	#define CAN_ACC_CODE_ALL          0x00000000
			//	#define CAN_ACC_MASK_NONE         0xFFFFFFFF
			//	#define CAN_ACC_CODE_NONE         0x80000000

			hResult = m_pCanControl->SetAccFilter(CAN_FILTER_STD,CAN_ACC_CODE_ALL,CAN_ACC_MASK_ALL);
			if (hResult == VCI_OK)
				{
				//CANコントロールを利用して、アクセスタンスフィルタに拡張(29bits)を指定してみる
				hResult = m_pCanControl->SetAccFilter(CAN_FILTER_EXT,CAN_ACC_CODE_ALL,CAN_ACC_MASK_ALL);
				}
			//どちらかのフィルタ指定が失敗した？
			if (VCI_OK != hResult)
				{
				}
			}

		//此処迄にエラーが無いか？
		if (hResult == VCI_OK)
			{
			//CANラインの制御を開始する
			hResult = m_pCanControl->StartLine();
			if (hResult != VCI_OK)
				{
				//CANラインの制御開始を失敗した
				}
			}
		}
	else
		{
		//CANコントロールの取得に失敗した
		}
	
	//此処迄にエラーが有るか？
	if (hResult != VCI_OK)
		return(-1);

	//ステータスの初期化
	m_status.err.nRaw = 0;

	//受信データ排他制御用セマフォ
	m_hSema = ::CreateSemaphore(NULL,1,1,NULL);

	//受信スレッド開始
	_beginthread(ReceiveThread,0,this);

	//規定時間迄にCAN startedメッセージが来るか調べる
	//（通知は非同期）
	bool bReady = false;
	uint32_t nStartTime = GetTm();
	uint32_t nEndTime = nStartTime + nTimeoutMS;
	do
		{
		//CANスタート（制御開始）状態？
		if(m_status.bCanStart)
			{
			bReady = true;
			break;
			}

		//待ち時間（固定）
		Sleep(100);

		} while(GetTm() < nEndTime);
	if(!bReady)
		{
		//失敗の為、インターフェースを閉じて後始末させる
		OnCloseInterface();
		return(-1);	//失敗扱いで戻る
		}

	//処理完了
	return(0);
	}

//開いたCANインターフェースを閉じる場合に呼び出されます
void CIxxatV2::OnCloseInterface()
	{
	//概要
	//	開いたCANインターフェースを閉じる場合に呼び出されます
	//パラメータ
	//	無し
	//戻り値
	//	無し

	//todo:CANインターフェースを開いている場合に閉じる処理を実装して下さい

	//受信スレッド動作中？
	if (m_hReadThread)
		{
		//受信スレッド停止
		m_lMustQuit = 1;
		::WaitForSingleObject(m_hReadThread,INFINITE);
		::CloseHandle(m_hReadThread);
		m_hReadThread = NULL;
		}

	//リーダー開放
	SafeRelease(&m_pReader);

	//ライター開放
	SafeRelease(&m_pWriter);

	//チャンネルインターフェース開放
	SafeRelease(&m_pCanChn);

	//CANコントローラ使用中？
	if (m_pCanControl)
		{
		//CANコントローラを停止させる
		HRESULT hResult = m_pCanControl->StopLine();
		if (hResult != VCI_OK)
			{
			//停止に失敗（失敗は既に停止済み？）
			}

		//CANコントローラとフィルタを初期化
		hResult = m_pCanControl->ResetLine();
		if (hResult != VCI_OK)
			{
			//初期化に失敗（既に停止済み？）
			}

		//CANコントローラを開放
		SafeRelease(&m_pCanControl);
		}

	//バスアクセスレイヤーを開放する
	SafeRelease(&m_pBalObject);

	//受信バッファ排他制御用セマフォ開放
	if(m_hSema)
		{
		::CloseHandle(m_hSema);
		m_hSema = NULL;
		}
	}

//CANインターフェースから受信する場合に呼び出されます
int32_t CIxxatV2::OnCanRecv(uint32_t* pCanID,uint8_t* pData8,bool bNoWait /* false */)
	{
	//概要
	//	CANインターフェースから1つ分のパケットを受信します
	//パラメータ
	//	pCanID			受信パケットのIDを格納する領域へのポインタ
	//	pData8			受信パケットを格納する8[bytes]領域へのポインタ
	//	bNoWait			trueの場合、既に受信済みパケットのみ対象として受信待ちしません
	//					falseの場合、受信済みパケットが無い場合は、GetTimeoutRecvで取得した時間迄受信を待ちます
	//戻り値
	//	0				正常終了
	//	0以外			異常終了

	//todo:CANインターフェースから1回分の受信を行う処理を実装して下さい

	//戻り値
	int32_t nResult = -1;

	//受信タイムアウト絶対時間を算出
	uint32_t nLimitTime = GetTm() + GetTimeout_Recv();

	//制限時間内のみ処理する（但し最低1回は処理する）
	do
		{
		CANMSG msg;
		if (GetCanMsg(&msg) == 0)
			{
			//正常取得
			*pCanID = uint32_t(msg.dwMsgId);
			::memcpy(pData8,msg.abData,sizeof(char) * 8);
			nResult = 0;
			break;
			}
		//待ち時間無し？
		if(bNoWait)
			break;
		//時間待ち
		Sleep(1);
		} while(GetTm() < nLimitTime);

	//
	return(nResult);
	}

//CANインターフェースに送信する場合に呼び出されます
int32_t CIxxatV2::OnCanSend(uint32_t nCanID,uint8_t* pData8,uint8_t nLength)
	{
	//概要
	//	CANインターフェースに1つ分のパケットを送信します
	//パラメータ
	//	nCanID			送信パケットのID
	//	pData8			送信パケットが格納されている8[bytes]領域へのポインタ
	//	nTimeoutMS		送信許容時間[ms]
	//戻り値
	//	0				正常終了
	//	0以外			異常終了

	//todo:CANインターフェースに送信する処理を実装して下さい

	//データ長
	UINT payloadLen = nLength;

	//送信用FIFOにアクセスして、データを設定する
	PCANMSG pMsg;
	UINT16 nCount = 0;
	HRESULT hResult = m_pWriter->AcquireWrite((void**)&pMsg,&nCount);
	if (hResult == VCI_OK)
		{
		// number of written messages needed for ReleaseWrite
		UINT16 written = 0;

		if (nCount > 0)
			{
			pMsg->dwTime = 0;
			pMsg->dwMsgId = nCanID;
			pMsg->uMsgInfo.Bits.type	= CAN_MSGTYPE_DATA;
			pMsg->uMsgInfo.Bits.ssm		= 1;					//シングルショートメッセージ
			pMsg->uMsgInfo.Bits.hpm		= 0;					//ハイプライオリティを指定しない
			pMsg->uMsgInfo.Bits.edl		= 0;					//拡張データを付けない
			pMsg->uMsgInfo.Bits.fdr		= 0;					//High bit rate
			pMsg->uMsgInfo.Bits.esi		= 0;					//(out)Error state indicator
			pMsg->uMsgInfo.Bits.res		= 0;					//予約エリア(0指定)
			pMsg->uMsgInfo.Bits.dlc		= payloadLen;			//パケット長[byte(s)]
			pMsg->uMsgInfo.Bits.ovr		= 0;					//(out)Data overrun
			pMsg->uMsgInfo.Bits.srr		= 1;					//Self reception request.
			pMsg->uMsgInfo.Bits.rtr		= 0;					//Remote transmission request
			pMsg->uMsgInfo.Bits.ext		= 1;					//拡張ID(29bits)
			pMsg->uMsgInfo.Bits.afc		= 0;					//(out)アクセプタンスフィルタコード

			//データ格納
			for (UINT nLoop = 0;nLoop < payloadLen;nLoop++)
				pMsg->abData[nLoop] = pData8[nLoop];

			//送信が必要
			written = 1;
			}

		//ステータスを初期化する（この送信で発生するエラーを判別する為）
		m_status.err.nRaw = 0;	//@@暫定

		//送信用FIFOを動作させ、送信させる
		hResult = m_pWriter->ReleaseWrite(written);
		if (hResult != VCI_OK)
			{
			//送信失敗
			return(-1);
			}

		//カウンタの値に送信サイズ(dlc)を加算
		AddCounter((uint32_t)payloadLen);
		}
	else
		{
		//送信用FIFOへのアクセスに失敗
		return(-1);
		}

	return(0);
	}

//CANインターフェースにエラーが有るか調べる場合に呼び出されます
int32_t CIxxatV2::OnCanGetError()
	{
	//概要
	//	CANインターフェースにエラーが有るか調べます
	//パラメータ
	//	無し
	//戻り値
	//	0				正常
	//	0以外			異常
	//注意点
	//	このデバイスでは発生したエラーが全て受信データとして通知される

	//todo:CANインターフェースにエラーが有るか調べる処理を実装して下さい

	//内部に格納したエラーが有るか？
	if(m_status.err.nRaw)
		return(1);

	return(0);
	}

//CANインターフェースのエラーを解除する場合に呼び出されます
int32_t CIxxatV2::OnCanClearError()
	{
	//概要
	//	CANインターフェースのエラーを解除します
	//パラメータ
	//	無し
	//戻り値
	//	0				正常
	//	0以外			異常

	//todo:CANインターフェースのエラーを解除する処理を実装して下さい

	//排他制御成功？
	if(LockCanMsg())
		{
		//内部に格納したエラー値を解除
		m_status.err.nRaw = 0;
		//排他制御解除
		UnlockCanMsg();
		}
	//デバイスのエラーを解除
	m_pCanControl->ResetLine();

	return(0);
	}

//受信バッファをクリアする場合に呼び出されます
int32_t CIxxatV2::OnCanRecvClear()
	{
	//概要
	//	受信バッファをクリアする場合に呼び出されます
	//パラメータ
	//	無し
	//戻り値
	//	0				正常
	//	0以外			異常

	//todo:受信バッファをクリアする処理を実装して下さい

	//排他制御成功？
	if(LockCanMsg())
		{
		m_nCanReadPt = 0;
		m_nCanWritePt = 0;
		//排他制御解除
		UnlockCanMsg();
		}
	return(0);
	}

//================================================================================
//本クラス専用の要素
//================================================================================

//非同期受信スレッド
void CIxxatV2::ReceiveThread(void* Param)
	{
	//スレッド構築元のクラス
	CIxxatV2* pClass = (CIxxatV2*)Param;

	//
	DWORD		nWaitTimeoutMS = 10;	//受信待ち時間[ms]
	uint32_t	nStage = 0;				//受信待ちステージから開始
	PCANMSG		pCanMsg = 0;
	UINT16		wCount = 0;

	//
	while (pClass->m_lMustQuit == 0)
		{
		//受信待ち
		if (nStage == 0)
			{
			//イベント待ち（何かあると受信扱いで情報が入ってくる）
			if (WaitForSingleObject(pClass->m_hEventReader,10) == WAIT_OBJECT_0)
				nStage = 100;
			}
		//データ受信
		else if (nStage == 100)
			{
			//受信制御
			pCanMsg = NULL;
			HRESULT hResult = pClass->m_pReader->AcquireRead((PVOID*)&pCanMsg,&wCount);
			if (hResult == VCI_OK)
				{
				//取得した「受信パケット」は、全て登録に回す
				//（登録側で中身を判断する）

				//登録
				pClass->AddCanMsg(pCanMsg,int(wCount));
				//データ受信に戻る
				nStage = 100;
				}
			else if (hResult == VCI_E_RXQUEUE_EMPTY)
				{
				//受信バッファが空だったので、受信待ちに戻る
				nStage = 0;
				}
			else
				{
				//何かイレギュラーな状態になったので、受信待ちに戻る
				nStage = 0;
				}
			//受信制御開放
			pClass->m_pReader->ReleaseRead(wCount);
			}
		}
	//
	_endthread();
	}

//受信バッファ排他制御要求
bool CIxxatV2::LockCanMsg(DWORD nTimeoutMS /* INFINITE */)
	{
	//資源確保に失敗？
	if(::WaitForSingleObject(m_hSema,nTimeoutMS) != WAIT_OBJECT_0)
		return(false);	//確保失敗
	//確保成功
	return(true);
	}

//受信バッファ排他制御解除
void CIxxatV2::UnlockCanMsg()
	{
	//確保した資源を開放
	::ReleaseSemaphore(m_hSema,1,NULL);
	}

//受信データの登録処理
uint32_t CIxxatV2::AddCanMsg(PCANMSG pMsg,int nCount /* 1 */)
	{
	//資源占有
	if(!LockCanMsg())
		return(0);	//資源占有失敗時は、登録数0として戻る

	//
	uint32_t nResult = 0;
	while(nCount)
		{
		//カウンタの値に受信サイズ(dlc)を加算
		//ステータス等、多少余計な分が入るがそこまでの精度は求めていない
		AddCounter((uint32_t)pMsg->uMsgInfo.Bits.dlc);

		//登録要素はデータパケット？
		if (pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA)
			{
			//このパケットは無視しない対象か？
			if(!IsIgnoreID(pMsg->dwMsgId))
				{
				//バッファFULLでは無い？
				if(m_nCanWritePt < 255)
					{
					//格納
					::CopyMemory(&m_pCanMsg[m_nCanWritePt],pMsg,sizeof(CANMSG));
					//格納先更新
					++m_nCanWritePt;
					//格納数更新
					++nResult;
					}
				else
					{
					//バッファが一杯
					Sleep(0);
					}
				}
			else
				{
				//無視する対象
				Sleep(0);
				}
			}

		//登録要素は情報パケット？
		else if (pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_INFO)
			{
			//情報別処理の判別開始
			switch (pMsg->abData[0])
				{
				//CAN開始？
				case CAN_INFO_START:
					m_status.bCanStart = true;
					break;
				//CAN停止？
				case CAN_INFO_STOP:
					m_status.bCanStart = false;
					break;
				//CANリセット？
				case CAN_INFO_RESET:
					m_status.bCanStart = false;
					break;
				}
			}

		//登録要素はエラーパケット？
		else if (pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_ERROR)
			{
			//エラー別処理の判別開始
			switch (pMsg->abData[0])
				{
				case CAN_ERROR_STUFF:
					m_status.err.info.nStuff = 1;
					break; 
				case CAN_ERROR_FORM:
					m_status.err.info.nForm = 1;
					break; 
				case CAN_ERROR_ACK:
					m_status.err.info.nACK = 1;
					break;
				case CAN_ERROR_BIT:
					m_status.err.info.nBit = 1;
					break; 
				case CAN_ERROR_CRC:
					m_status.err.info.nCRC = 1;
					break; 
				case CAN_ERROR_OTHER:
				default:
					m_status.err.info.nOther = 1;
					break;
				}
			}

		//登録要素はステータス情報？
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_STATUS)
			{
			//何が入っているのか不明
			Sleep(0);
			}
		//？
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_WAKEUP)
			{
			//詳細不明
			Sleep(0);
			}
		//登録要素はタイマーオーバーラン情報？
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_TIMEOVR)
			{
			//発生条件不明
			Sleep(0);
			}
		//？
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_TIMERST)
			{
			//詳細不明
			Sleep(0);
			}

		//登録要素の次位置
		++pMsg;

		//登録要素の残数を更新
		--nCount;
		}
	//資源開放
	UnlockCanMsg();
	//
	return(nResult);
	}

//登録されたCANメッセージを1つ取得
uint32_t CIxxatV2::GetCanMsg(PCANMSG pMsg)
	{
	//
	uint32_t nResult = 0;
	//資源占有
	if(!LockCanMsg())
		return(2);	//資源占有失敗時は、その他エラーとして戻る
	//未読み出しの要素が有る？
	else if(m_nCanReadPt < m_nCanWritePt)
		{
		//該当箇所から取得
		::CopyMemory(pMsg,&m_pCanMsg[m_nCanReadPt],sizeof(CANMSG));
		//読み出し位置の更新
		++m_nCanReadPt;
		//読み出し残りが無い？
		if(m_nCanReadPt >= m_nCanWritePt)
			{
			//初期化（全クリア扱いでバッファを先頭に戻すが、排他制御済みなのでClearCanMsgを使わない事）
			m_nCanReadPt = 0;
			m_nCanWritePt = 0;
			}
		nResult = 0;
		}
	else
		nResult = 1;

	//資源開放
	UnlockCanMsg();
	//
	return(nResult);
	}

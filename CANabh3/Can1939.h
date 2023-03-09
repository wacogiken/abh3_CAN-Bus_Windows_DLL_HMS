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

#include <windows.h>
#include <stdint.h>

//概要
//	CANパケットの中身(SAE1939仕様)を構築する為のクラス
//	IDについては一切考慮しない
//
//更新履歴
//	2020.xx.xx	yo0043	1st release
//	2021.02.15	yo0043	2nd release
//						コメント追加等（コードに変更無し）
//


class CCan1939
{
public:

	//列挙子
	typedef enum
		{
		//マルチパケット関連定義
		 DEF_RTS			= 0x10			//CM.RTSの先頭バイトに入る値
		,DEF_CTS			= 0x11			//CM.CTSの先頭バイトに入る値
		,DEF_EOMA			= 0x13			//CM.EOMAの先頭バイトに入る値
		,DEF_BAM			= 0x20			//CM_BAMの先頭バイトに入る値
		,DEF_ABORT			= 0xff			//CM_ABORTの先頭バイトに入る値
		,DEF_SIGN			= 0x00ef00		//CM_*で使う当社専用シグネチャ(6,7,8バイト目に入る)
		,DEF_UNKNOWN		= 0x00			//マルチパケット受信判別時、不明な結果が出た場合の値
		} ENUM;

public:

	//CANバッファの構築
	static uint8_t* CreateBuffer(uint8_t nInitValue = 0x00)
		{
		//概要
		//	CANで使用する8[bytes]のバッファを構築
		//パラメータ
		//	nInitValue		バッファの初期値
		//戻り値
		//	確保したバッファへのポインタ
		//注意点
		//	確保したバッファは、呼び出し元で開放(delete)するか、
		//	FreeBufferメンバ関数を使用して開放する必要が有ります

		uint8_t* pResult = new uint8_t[8];
		::FillMemory(pResult,8,nInitValue);
		return(pResult);
		}

	//CANバッファの開放
	static void FreeBuffer(uint8_t* pBuffer)
		{
		//概要
		//	CreateBufferメンバ関数で構築したバッファの開放
		//パラメータ
		//	pBuffer			バッファへのポインタ
		//戻り値
		//	無し
		//注意点
		//	pBufferが無効(NULL指定)の場合は、何もしません

		if(pBuffer)
			delete pBuffer;
		}

	//適切な位置にデータを格納
	static void SetData(uint8_t* pDst,uint32_t nDstSize,uint32_t nDstPt,uint8_t* pSrc,uint32_t nSrcSize)
		{
		//概要
		//	適切な位置にデータを格納
		//パラメータ
		//	pDst		書き込み先データの先頭位置
		//	nDstSize	書き込み先データの領域サイズ
		//	nDstPt		書き込み先の要素番号
		//	pSrc		書き込みデータの先頭位置
		//	nSrcSize	書き込みデータサイズ
		//戻り値
		//	無し
		//注意点
		//	格納先のバッファは必ず確保済みである事

		//書き込み可能サイズ
		uint32_t nRemain = nDstSize - nDstPt;
		if(nRemain < nSrcSize)
			nSrcSize = nRemain;

		//書き込み
		::memcpy(&pDst[nDstPt],pSrc,size_t(nSrcSize));
		}

	//概要
	//	値の設定(8,16,24,32bit指定）
	//パラメータ
	//	pBuffer		格納先配列の先頭要素
	//	nPt			格納先配列の要素番号
	//	nValue		格納データ
	//戻り値
	//	無し
	//備考
	//	バッファの先頭側がLSB側に相当する
	static void Set8L(uint8_t* pBuffer,int32_t nPt,uint8_t nValue)
		{
		pBuffer[nPt] = nValue;
		}
	static void Set16L(uint8_t* pBuffer,int32_t nPt,uint16_t nValue)
		{
		Set8L(pBuffer,nPt,uint8_t(nValue));
		Set8L(pBuffer,nPt + 1,uint8_t(nValue >> 8));
		}
	static void Set24L(uint8_t* pBuffer,int32_t nPt,uint32_t nValue)
		{
		Set8L(pBuffer,nPt,uint8_t(nValue));
		Set8L(pBuffer,nPt + 1,uint8_t(nValue >> 8));
		Set8L(pBuffer,nPt + 2,uint8_t(nValue >> 16));
		}
	static void Set32L(uint8_t* pBuffer,int32_t nPt,uint32_t nValue)
		{
		Set16L(pBuffer,nPt,uint16_t(nValue));
		Set16L(pBuffer,nPt + 2,uint16_t(nValue >> 16));
		}

	//概要
	//	値の取得(8,16,24,32bit指定)
	//パラメータ
	//	pBuffer		格納先配列の先頭要素
	//	nPt			格納先配列の要素番号
	//戻り値
	//	指定した要素番号の値が戻ります
	static uint8_t Get8L(uint8_t* pBuffer,int32_t nPt)
		{
		return(pBuffer[nPt]);
		}
	static uint16_t Get16L(uint8_t* pBuffer,int32_t nPt)
		{
		uint16_t nResult = Get8L(pBuffer,nPt + 0) | (Get8L(pBuffer,nPt + 1) << 8);
		return(nResult);
		}
	static uint32_t Get24L(uint8_t* pBuffer,int32_t nPt)
		{
		uint32_t nResult = Get8L(pBuffer,nPt + 0) | (Get8L(pBuffer,nPt + 1) << 8) | (Get8L(pBuffer,nPt + 2) << 16);
		return(nResult);
		}
	static uint32_t Get32L(uint8_t* pBuffer,int32_t nPt)
		{
		uint32_t nResult = Get16L(pBuffer,nPt + 0) | (Get16L(pBuffer,nPt + 2) << 16);
		return(nResult);
		}

	//概要
	//	シングルパケット(DP0)の構築
	//パラメータ
	//	nAYcmd		A/Y指令
	//	nBXcmd		B/X指令
	//	nBITcmd		入力(bit対応)
	//戻り値
	//	構築されたシングルパケット(DP0)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateSGL0(uint16_t nAYcmd,uint16_t nBXcmd,uint32_t nBITcmd)
		{
		uint8_t* pBuffer = CreateBuffer(0x00);
		Set16L(pBuffer,0,nAYcmd);
		Set16L(pBuffer,2,nBXcmd);
		Set32L(pBuffer,4,nBITcmd);
		return(pBuffer);
		}

	//概要
	//	シングルパケット(DP1)の構築
	//パラメータ
	//	無し
	//戻り値
	//	構築されたシングルパケット(DP1)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateSGL1(void)
		{
		uint8_t* pBuffer = CreateBuffer(0x00);
		return(pBuffer);
		}

	//概要
	//	ブロードキャストパケットの構築
	//パラメータ
	//	nSign		データ先頭3バイトに入る固定コード(0x00ffxx)
	//戻り値
	//	構築されたブロードキャストパケットへのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateBRD(uint32_t nSign)
		{
		uint8_t* pBuffer = CreateBuffer(0x00);
		Set24L(pBuffer,0,nSign);
		return(pBuffer);
		}

	//概要
	//	マルチパケット(CM.RTS)の構築
	//パラメータ
	//	nDataSize		データサイズ
	//	nMaxResponce	最大返答長
	//戻り値
	//	構築されたマルチパケット(CM_RTS)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateCMRTS(uint32_t nDataSize,uint8_t nMaxResponce = 0x08)
		{
		uint8_t* pBuffer = CreateBuffer(0xff);
		uint8_t nMsgCount = datasize2msgcount(nDataSize);
		uint16_t nMsgSize = nMsgCount * 8;
		Set8L(pBuffer,0,DEF_RTS);
		Set16L(pBuffer,1,nDataSize);
		Set8L(pBuffer,3,nMsgCount);
		Set8L(pBuffer,4,nMaxResponce);
		Set24L(pBuffer,5,DEF_SIGN);
		return(pBuffer);
		}

	//概要
	//	マルチパケット(CM_CTS)の構築
	//パラメータ
	//	nMaxResponce	最大返答長
	//	nNextPacketNum	次パケット番号
	//戻り値
	//	構築されたマルチパケット(CM_CTS)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateCMCTS(uint8_t nMaxResponce,uint8_t nNextPacketNum)
		{
		uint8_t* pBuffer = CreateBuffer(0xff);
		Set8L(pBuffer,0,DEF_CTS);
		Set8L(pBuffer,1,nMaxResponce);
		Set8L(pBuffer,2,nNextPacketNum);
		Set24L(pBuffer,5,DEF_SIGN);
		return(pBuffer);
		}

	//概要
	//	マルチパケット(CM_DT)の構築
	//パラメータ
	//	pData			送信データの先頭
	//	nDataSize		データサイズ
	//	nPacketNum		パケット番号
	//戻り値
	//	構築されたマルチパケット(CM_DT)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateCMDT(uint8_t* pData,uint32_t nDataSize,uint8_t nPacketNum)
		{
		//データの送信対象位置を算出し、データ範囲内かチェックする
		uint32_t nDataPt = (nPacketNum - 1) * 7;
		if(nDataPt + 1 > nDataSize)
			return(NULL);	//オーバーラン
		//
		uint8_t* pBuffer = CreateBuffer(0xff);
		Set8L(pBuffer,0,nPacketNum);
		for(uint8_t nLoop = 0;nLoop < 7;nLoop++)
			{
			if(nDataPt + nLoop >= nDataSize)
				break;								//終端超過
			else
				Set8L(pBuffer,1 + nLoop,pData[nDataPt + nLoop]);
			}
		return(pBuffer);
		}

	//概要
	//	マルチパケット(CM_EOMA)の構築
	//パラメータ
	//	nMsgSize		メッセージサイズ
	//	nPacketCount	パケット数
	//戻り値
	//	構築されたマルチパケット(CM_EOMA)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateCMEOMA(uint16_t nMsgSize,uint8_t nPacketCount)
		{
		uint8_t* pBuffer = CreateBuffer(0xff);
		Set8L(pBuffer,0,DEF_EOMA);
		Set16L(pBuffer,1,nMsgSize);
		Set8L(pBuffer,3,nPacketCount);
		Set24L(pBuffer,5,DEF_SIGN);
		return(pBuffer);
		}

	//概要
	//	マルチパケット(CM_ABORT)の構築
	//パラメータ
	//	nCause			中止理由
	//戻り値
	//	構築されたマルチパケット(CM_ABORT)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateCMABORT(uint8_t nCause)
		{
		uint8_t* pBuffer = CreateBuffer(0xff);
		Set8L(pBuffer,0,DEF_ABORT);
		Set8L(pBuffer,1,nCause);
		Set24L(pBuffer,5,DEF_SIGN);
		return(pBuffer);
		}

	//概要
	//	マルチパケット(CM_BAM)の構築
	//パラメータ
	//	nMsgSize		メッセージサイズ
	//	nPacketCount	パケット数
	//戻り値
	//	構築されたマルチパケット(CM_BAM)へのポインタが戻ります
	//注意点
	//	戻り値は呼び出し元で開放する必要が有ります
	static uint8_t* CreateCMBAM(uint16_t nMsgSize,uint8_t nPacketCount)
		{
		uint8_t* pBuffer = CreateBuffer(0xff);
		Set8L(pBuffer,0,DEF_BAM);
		Set16L(pBuffer,1,nMsgSize);
		Set8L(pBuffer,3,nPacketCount);
		Set24L(pBuffer,5,DEF_SIGN);
		return(pBuffer);
		}

	//概要
	//	データサイズから送信回数に変換
	//パラメータ
	//	nDatasize		データサイズ
	//戻り値
	//	データを送信するのに必要なパケット数
	static uint8_t datasize2msgcount(uint32_t nDatasize)
		{
		if(nDatasize > 7 * 0xff)
			return(0);	//1[byte]に入らない送信回数
		uint8_t nResult = nDatasize / 7;
		if(nDatasize % 7)
			++nResult;
		return(nResult);
		}

	//概要
	//	パケット番号からデータ格納位置を算出
	//パラメータ
	//	nPacket			パケット番号
	//戻り値
	//	このパケットに対応するデータ位置
	static uint32_t packetnum2datapt(uint8_t nPacket)
		{
		uint32_t nResult = (nPacket - 1) * 7;
		return(nResult);
		}

	//概要
	//	パケット種別を取得
	//パラメータ
	//	pData			パケット先頭へのポインタ
	//戻り値
	//	DEF_UNKNOWN		不明なパケット
	//	DEF_*			該当するパケット種別
	static uint8_t IsPacket(uint8_t* pData)
		{
		uint8_t nResult = pData[0];
		switch(nResult)
			{
			case DEF_RTS:
			case DEF_CTS:
			case DEF_EOMA:
			case DEF_ABORT:
				break;
			default:
				nResult = DEF_UNKNOWN;
				break;
			}
		return(nResult);
		}

public:
	CCan1939()
		{}
	virtual ~CCan1939()
		{}
};

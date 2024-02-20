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
#ifndef _TYPEDEF_H_INCLUDED_
#define _TYPEDEF_H_INCLUDED_

//プラットフォーム依存
#include <Windows.h>
#include <stdint.h>

//================================================================================
//ユーザーが利用可能な型
//================================================================================

//パケット種類の指定用
typedef enum
	{
	UNKNOWN_PACKET		= 0,	//内部用（ユーザーでは利用しない）
	SINGLE_PACKET		= 1,	//返答：シングルパケット
	BROADCAST_PACKET	= 2,	//返答：ブロードキャストパケット
	//SINGLE_PACKET_W		= 3,	//要求：シングルパケット
	//BROADCAST_PACKET_W	= 4,	//要求：ブロードキャストパケット
	ANY_PACKET			= 0,	//パケット種類を問わない
	} PACKETTYPE;

//受信に使用する構造体（受信情報＋受信データ8バイト）
typedef struct _CANABH3_RESULT
	{
	//受信データのCAN-ID
	uint32_t	nID;

	//共有体
	union
		{
		//内部用
		uint8_t		raw[8];

		//シングルパケット(DP0)
		struct _DP0R
			{
			int16_t		nBackAY;			//	A/Y帰還
			int16_t		nBackBX;			//	B/X帰還
			uint32_t	nCtrlBit;			//	制御フラグ
			} DP0R;
		//ブロードキャストパケット(0)
		struct _BR0
			{
			uint32_t	nErrorBit;			//	異常フラグ
			uint32_t	nWarnBit;			//	警告フラグ
			} BR0;
		//ブロードキャストパケット(1)
		struct _BR1
			{
			uint32_t	nIoBit;				//	IOフラグ
			uint32_t	nCtrlBit;			//	入力フラグ
			} BR1;
		//ブロードキャストパケット(2)
		struct _BR2
			{
			int16_t		nOrderSpeedAY;		//	A/Y速度指令
			int16_t		nOrderSpeedBX;		//	B/X速度指令
			int16_t		nBackSpeedAY;		//	A/Y速度帰還
			int16_t		nBackSpeedBX;		//	B/X速度帰還
			} BR2;
		//ブロードキャストパケット(3)
		struct _BR3
			{	
			int16_t		nOrderCurrentAY;	//	A/Y電流指令
			int16_t		nOrderCurrentBX;	//	B/X電流指令
			int16_t		nLoadA;				//	A負荷率
			int16_t		nLoadB;				//	B負荷率
			} BR3;
		//ブロードキャストパケット(4)
		struct _BR4
			{
			int32_t		nCountPulseA;		//	Aパルス積算値
			int32_t		nCountPulseB;		//	Bパルス積算値
			} BR4;
		//ブロードキャストパケット(5)
		struct _BR5
			{
			int16_t		nAnalog0;			//	アナログ入力0
			int16_t		nAnalog1;			//	アナログ入力1
			int16_t		nPowerMain;			//	主電源電圧
			int16_t		nPowerCtrl;			//	制御電源電圧
			} BR5;
		//ブロードキャストパケット(6)
		struct _BR6
			{
			float		nMonitor0;			//	モニタ0データ
			float		nMonitor1;			//	モニタ1データ
			} BR6;
		} u;
	} CANABH3_RESULT,*pCANABH3_RESULT;

//最終送信データ用構造体
typedef struct _CANABH3_LASTSEND
	{
	//指令類
	int16_t		nOrderAY;
	int16_t		nOrderBX;
	uint32_t	nInputBit;

	//送信パケット類

	//シングルパケット(DP0)
	struct _DP0R
		{
		uint8_t		packet[8];				//　パケットデータ
		uint8_t		nLength;				//	パケット長
		} DP0R;
	//ブロードキャストパケット(0-7)
	struct _BR
		{
		uint8_t		packet[8];				//　パケットデータ
		uint8_t		nLength;				//	パケット長
		} BR[8];
	} CANABH3_LASTSEND,*pCANABH3_LASTSEND;

//最終受信データ用構造体（最終送信データを含む）
typedef struct _CANABH3_LASTRECV
	{
	//シングルパケット(DP0)
	struct _DP0R
		{
		int16_t		nBackAY;				//	A/Y帰還
		int16_t		nBackBX;				//	B/X帰還
		uint32_t	nCtrlBit;				//	制御フラグ
		} DP0R;
	//ブロードキャストパケット(0)
	struct _BR0
		{
		uint32_t	nErrorBit;				//	異常フラグ
		uint32_t	nWarnBit;				//	警告フラグ
		} BR0;
	//ブロードキャストパケット(1)
	struct _BR1
		{
		uint32_t	nIoBit;					//	IOフラグ
		uint32_t	nCtrlBit;				//	入力フラグ
		} BR1;
	//ブロードキャストパケット(2)
	struct _BR2
		{
		int16_t		nOrderSpeedAY;			//	A/Y速度指令
		int16_t		nOrderSpeedBX;			//	B/X速度指令
		int16_t		nBackSpeedAY;			//	A/Y速度帰還
		int16_t		nBackSpeedBX;			//	B/X速度帰還
		} BR2;
	//ブロードキャストパケット(3)
	struct _BR3
		{	
		int16_t		nOrderCurrentAY;		//	A/Y電流指令
		int16_t		nOrderCurrentBX;		//	B/X電流指令
		int16_t		nLoadA;					//	A負荷率
		int16_t		nLoadB;					//	B負荷率
		} BR3;
	//ブロードキャストパケット(4)
	struct _BR4
		{
		int32_t		nCountPulseA;			//	Aパルス積算値
		int32_t		nCountPulseB;			//	Bパルス積算値
		} BR4;
	//ブロードキャストパケット(5)
	struct _BR5
		{
		int16_t		nAnalog0;				//	アナログ入力0
		int16_t		nAnalog1;				//	アナログ入力1
		int16_t		nPowerMain;				//	主電源電圧
		int16_t		nPowerCtrl;				//	制御電源電圧
		} BR5;
	//ブロードキャストパケット(6)
	struct _BR6
		{
		float		nMonitor0;				//	モニタ0データ
		float		nMonitor1;				//	モニタ1データ
		} BR6;
	//各格納場所の更新フラグ
	struct _UPDATE
		{
		uint8_t		nUpdate;				//0以外で値が更新されている（受信している）
		} update[8];						//0..DP0R  1..BR0  2..BR1  ....  7..BR6

	} CANABH3_LASTRECV,*pCANABH3_LASTRECV;

//最終データ関連（送信・受信）
typedef struct _CANABH3_LASTDATA
	{
	//過去に把握したデータ（受信・送信）
	CANABH3_LASTRECV recv;
	CANABH3_LASTSEND send;
	} CANABH3_LASTDATA,*pCANABH3_LASTDATA;

//PACK_FLOAT構造体
typedef struct _PACK_FLOAT
	{
	union
		{
		struct _PACK_FLOAT_INFO
			{
			//LSB側から記載(処理系に注意)
			int32_t kasuu:21;			// 仮数部(-999999～0～99999)
			int32_t shousuutenn:4;		// 少数点(-8～0～7)
			int32_t shisuu:7;			// 指数部(-64～0～63)
			//こちらがMSB側
			} info;
		int32_t	nDirectData;			// 直接アクセス用
		} u;
	} *pPACK_FLOAT,PACK_FLOAT;

#endif	//多重インクルード防止

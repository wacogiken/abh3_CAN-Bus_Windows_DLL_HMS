/*
 ******************************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2021-2023,2022 Waco Giken Co., Ltd.
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

//このファイルは、ユーザー側でのみ使用します
//（DLL構築時には不要）

#include <Windows.h>
#include <stdint.h>
#include "typedef.h"

//============================================================
//エクスポートする関数を簡単に扱う為の要素
//============================================================

//----------------------------------------
//Windows固有
//----------------------------------------

//使用開始時に呼び出す必要が有ります
typedef void(*CA3_InitInstance)(int32_t);
//使用終了時に呼び出す必要が有ります
typedef void (*CA3_ExitInstance)(void);
//使用可能なCANインターフェース数を取得
typedef int32_t(*CA3_GetInterfaceCount)(void);
//利用するインターフェースを指定
typedef int32_t(*CA3_SetInterface)(int32_t);
//現在開いているインターフェース番号を取得
typedef int32_t(*CA3_GetCurrentInterface)(void);
//現在インターフェースを開いているか？
typedef int32_t(*CA3_IsOpenInterface)(void);
//インターフェースを開くタイムアウト時間を設定
typedef void(*CA3_SetOpenTimeout)(uint32_t);
//送信タイムアウト時間を設定
typedef void(*CA3_SetSendTimeout)(uint32_t);
//受信タイムアウト時間を設定
typedef void(*CA3_SetRecvTimeout)(uint32_t);
//通信ホストのアドレスを設定
typedef void(*CA3_SetHostID)(uint8_t);
//通信ホストのアドレスを取得
typedef uint8_t(*CA3_GetHostID)(void);
//通信速度を指定
typedef void(*CA3_SetBaudrate)(uint32_t);
//設定した通信速度を取得
typedef uint32_t(*CA3_GetBaudrate)(void);
//PCが起動してからの時間を[ms]単位で取得
typedef uint32_t(*CA3_GetTm)(void);
//バス占有カウンタを取得します
typedef uint32_t(*CA3_GetCounter)(void);
//送受信カウンタを取得します
typedef uint32_t(*CA3_GetTransmitCounter)(uint8_t);
//CANバスにエラーが発生しているか調べます
typedef uint32_t(*CA3_GetCANerror)(void);
//CANバスのエラー解除を行います
typedef uint32_t(*CA3_ResetCANerror)(void);

//----------------------------------------
//ABH3固有
//----------------------------------------

//指令の初期化＋CANポートを開く
typedef int32_t(*CA3_abh3_can_init)(uint8_t,pCANABH3_RESULT);
//CANポートを開く
typedef int32_t(*CA3_abh3_can_port_init)(void);
//指令の初期化
typedef int32_t(*CA3_abh3_cmd_init)(uint8_t,pCANABH3_RESULT);
//現在の受信データ状況を一括取得
typedef void(*CA3_abh3_can_copylastdata)(uint8_t,pCANABH3_LASTDATA);
//現在の受信データ状況の更新フラグを解除する
typedef void(*CA3_abh3_can_resetlastdata)(uint8_t,int32_t);
//受信対象を指定して受信
typedef int32_t(*CA3_abh3_can_read)(uint8_t,pCANABH3_RESULT,PACKETTYPE,uint8_t);
//受信済みデータを受信データ状況に反映させる
typedef int32_t(*CA3_abh3_can_flush)(void);
//指令の送信（軸別）
typedef int32_t(*CA3_abh3_can_cmdAY)(uint8_t,int16_t,pCANABH3_RESULT);
typedef int32_t(*CA3_abh3_can_cmdBX)(uint8_t,int16_t,pCANABH3_RESULT);
//指令の送信（同時）
typedef int32_t(*CA3_abh3_can_cmd)(uint8_t,int16_t,int16_t,pCANABH3_RESULT);
//入力の送信（一括）
typedef int32_t(*CA3_abh3_can_opSet)(uint8_t,int32_t,int32_t,pCANABH3_RESULT);
//入力の送信（ビット）
typedef int32_t(*CA3_abh3_can_opBitSet)(uint8_t,int8_t,int8_t,pCANABH3_RESULT);
//指令と入力の送信（同時）
typedef int32_t(*CA3_abh3_can_cmdAndopSet)(uint8_t,int16_t,int16_t,int32_t,int32_t,pCANABH3_RESULT);
//ブロードキャストパケットのリクエスト
typedef int32_t(*CA3_abh3_can_reqBRD)(uint8_t,uint8_t,uint8_t,pCANABH3_RESULT);
//マルチパケットによるTelABH3パケットの送受信
typedef int32_t(*CA3_abh3_can_trans)(uint8_t,char*,char*,uint32_t*);
//CANを閉じる
typedef int32_t(*CA3_abh3_can_finish)(void);
//速度
typedef int16_t(*CA3_cnvVel2CAN)(float vel);
typedef float(*CA3_cnvCAN2Vel)(int16_t vel);
//電流
typedef int16_t(*CA3_cnvCur2CAN)(float);
typedef float(*CA3_cnvCAN2Cur)(int16_t);
//負荷率
typedef float(*CA3_cnvCAN2Load)(int16_t);
//アナログ入力
typedef float(*CA3_cnvCAN2Analog)(int16_t);
//電源電圧
typedef float(*CA3_cnvCAN2Volt)(int16_t);
//実数
typedef PACK_FLOAT(*CA3_flt_to_pkflt)(float);
typedef float(*CA3_pkflt_to_flt)(PACK_FLOAT);
typedef int(*CA3_str_to_pkfltA)(const char*,PACK_FLOAT*);
typedef int(*CA3_str_to_pkfltW)(const wchar_t*,PACK_FLOAT*);
typedef char*(*CA3_pkflt_to_strA)(PACK_FLOAT,char*);
typedef char*(*CA3_pkflt_to_strW)(PACK_FLOAT,wchar_t*);

//============================================================
//DLLの全関数を扱う為の構造体
//============================================================

//DLLファンクション関数アドレス管理
typedef struct _CA3_FUNCLIST
	{
	//----------------------------------------
	//Windows固有
	//----------------------------------------

	//使用開始時に呼び出す必要が有ります
	CA3_InitInstance		InitInstance;
	//使用終了時に呼び出す必要が有ります
	CA3_ExitInstance		ExitInstance;
	//使用可能なCANインターフェース数を取得
	CA3_GetInterfaceCount	GetInterfaceCount;
	//使用するインターフェースを指定する
	CA3_SetInterface		SetInterface;
	//現在開いているインターフェース番号を取得
	CA3_GetCurrentInterface	GetCurrentInterface;
	//現在インターフェースを開いているか？
	CA3_IsOpenInterface		IsOpenInterface;
	//インターフェースを開くタイムアウト時間を設定
	CA3_SetOpenTimeout		SetOpenTimeout;
	//送信タイムアウト時間を設定
	CA3_SetSendTimeout		SetSendTimeout;
	//受信タイムアウト時間を設定
	CA3_SetRecvTimeout		SetRecvTimeout;
	//通信ホストのアドレスを設定
	CA3_SetHostID			SetHostID;
	//通信ホストのアドレスを取得
	CA3_GetHostID			GetHostID;
	//通信速度を指定
	CA3_SetBaudrate			SetBaudrate;
	//設定した通信速度を取得
	CA3_GetBaudrate			GetBaudrate;
	//PCが起動してからの時間を[ms]単位で取得
	CA3_GetTm				GetTm;
	//バスカウンタの値を取得します
	CA3_GetCounter			GetCounter;
	//送受信カウンタの値を取得します
	CA3_GetTransmitCounter	GetTransmitCounter;
	//CANバスにエラーが発生しているか調べます
	CA3_GetCANerror			GetCANerror;
	//CANバスのエラー解除を行います
	CA3_ResetCANerror		ResetCANerror;

	//----------------------------------------
	//ABH3固有
	//----------------------------------------

	//指令の初期化＋CANポートを開く
	CA3_abh3_can_init			abh3_can_init;
	//CANポートを開く
	CA3_abh3_can_port_init		abh3_can_port_init;
	//指令の初期化
	CA3_abh3_cmd_init			abh3_cmd_init;
	//現在の受信データ状況を一括取得
	CA3_abh3_can_copylastdata	abh3_can_copylastdata;
	//現在の受信データ状況の更新フラグを解除する
	CA3_abh3_can_resetlastdata	abh3_can_resetlastdata;
	//受信対象を指定して受信
	CA3_abh3_can_read			abh3_can_read;
	//受信バッファの内容を受信データ状況に反映
	CA3_abh3_can_flush			abh3_can_flush;
	//指令の送信（軸別）
	CA3_abh3_can_cmdAY			abh3_can_cmdAY;
	CA3_abh3_can_cmdBX			abh3_can_cmdBX;
	//指令の送信（同時）
	CA3_abh3_can_cmd			abh3_can_cmd;
	//入力の送信（一括）
	CA3_abh3_can_opSet			abh3_can_opSet;
	//入力の送信（ビット）
	CA3_abh3_can_opBitSet		abh3_can_opBitSet;
	//指令と入力の送信（同時）
	CA3_abh3_can_cmdAndopSet	abh3_can_cmdAndopSet;

	//ブロードキャストパケットのリクエスト
	CA3_abh3_can_reqBRD			abh3_can_reqBRD;
	//マルチパケットによるTelABH3パケットの送受信
	CA3_abh3_can_trans			abh3_can_trans;
	//CANを閉じる
	CA3_abh3_can_finish			abh3_can_finish;
	//速度
	CA3_cnvVel2CAN				cnvVel2CAN;
	CA3_cnvCAN2Vel				cnvCAN2Vel;
	//電流
	CA3_cnvCAN2Cur				cnvCAN2Cur;
	CA3_cnvCur2CAN				cnvCur2CAN;
	//負荷率
	CA3_cnvCAN2Load				cnvCAN2Load;
	//アナログ入力
	CA3_cnvCAN2Analog			cnvCAN2Analog;
	//電源電圧
	CA3_cnvCAN2Volt				cnvCAN2Volt;
	//実数
	CA3_flt_to_pkflt			flt_to_pkflt;
	CA3_pkflt_to_flt			pkflt_to_flt;
	CA3_str_to_pkfltA			str_to_pkfltA;
	CA3_str_to_pkfltW			str_to_pkfltW;
	CA3_pkflt_to_strA			pkflt_to_strA;
	CA3_pkflt_to_strW			pkflt_to_strW;
	} CA3_FUNCLIST,*pCA3_FUNCLIST;

//DLLの関数を一括で扱う為の関数定義
int32_t GetFunctions(HMODULE hDLL,pCA3_FUNCLIST pFuncList);

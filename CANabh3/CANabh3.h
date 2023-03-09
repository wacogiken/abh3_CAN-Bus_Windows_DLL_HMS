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

#include <Windows.h>
#include <stdint.h>
#include "typedef.h"
#include "ABH3.h"
#include "PackFloat.h"
#include "IxxatV2.h"

//============================================================
//エクスポート？
//============================================================

 #ifndef _NO_CANABH3_EXPORT
  #define CANABH3API __declspec(dllexport)
#else
  #define CANABH3API __declspec(dllimport)
#endif

//============================================================
//以下リンケージはC言語形式とする
//============================================================

#ifdef __cplusplus
 extern "C" {
#endif //__cplusplus

//============================================================
//ユーザー用関数(Windows固有系)
//============================================================

//使用開始時に呼び出す必要が有ります
CANABH3API void InitInstance(int32_t nIFnum);

//使用終了時に呼び出す必要が有ります
CANABH3API void ExitInstance(void);

//使用可能なCANインターフェース数を取得
CANABH3API int32_t GetInterfaceCount(void);

//インターフェースを指定する
CANABH3API int32_t SetInterface(int32_t nDeviceNum);

//現在開いているインターフェース番号を取得
CANABH3API int32_t GetCurrentInterface(void);

//現在インターフェースを開いているか？
CANABH3API int32_t IsOpenInterface(void);

//インターフェースを開くタイムアウト時間を設定
CANABH3API void SetOpenTimeout(uint32_t nTimeoutMS);

//送信タイムアウト時間を設定
CANABH3API void SetSendTimeout(uint32_t nTimeoutMS);

//受信タイムアウト時間を設定
CANABH3API void SetRecvTimeout(uint32_t nTimeoutMS);

//通信ホストのアドレスを設定
CANABH3API void SetHostID(uint8_t nAdrs);

//通信ホストのアドレスを取得
CANABH3API uint8_t GetHostID(void);

//通信速度を指定
CANABH3API void SetBaudrate(uint32_t nBaudrateKbps);

//設定した通信速度を取得
CANABH3API uint32_t GetBaudrate(void);

//PCが起動してからの時間を[ms]単位で取得
CANABH3API uint32_t GetTm(void);

//1秒間に受信したデータ量を取得します
CANABH3API uint32_t GetCounter(void);

//CANバスにエラーが発生しているか調べます
CANABH3API uint32_t GetCANerror(void);

//CANバスのエラー解除を行います
CANABH3API uint32_t ResetCANerror(void);

//================================================================================
//ユーザー用関数(通信系)
//================================================================================

//指令の初期化＋CANポートを開く
CANABH3API int32_t abh3_can_init(uint8_t nTargetID,pCANABH3_RESULT pPtr);

//CANポートを開く
CANABH3API int32_t abh3_can_port_init(void);

//指令の初期化
CANABH3API int32_t abh3_cmd_init(uint8_t nTargetID,pCANABH3_RESULT pPtr);

//指令の送信（軸別）
CANABH3API int32_t abh3_can_cmdAY(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr);
CANABH3API int32_t abh3_can_cmdBX(uint8_t nTargetID,int16_t cmd,pCANABH3_RESULT pPtr);

//指令の送信（同時）
CANABH3API int32_t abh3_can_cmd(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,pCANABH3_RESULT pPtr);

//入力の送信（一括）
CANABH3API int32_t abh3_can_opSet(uint8_t nTargetID,int32_t data,int32_t mask,pCANABH3_RESULT pPtr);

//入力の送信（ビット）
CANABH3API int32_t abh3_can_opBitSet(uint8_t nTargetID,int8_t num,int8_t data,pCANABH3_RESULT pPtr);

//指令と入力の送信（同時）
CANABH3API int32_t abh3_can_cmdAndopSet(uint8_t nTargetID,int16_t cmdAY,int16_t cmdBX,int32_t data,int32_t mask,pCANABH3_RESULT pPtr);

//ブロードキャストパケットのリクエスト
CANABH3API int32_t abh3_can_reqBRD(uint8_t nTargetID,uint8_t num,uint8_t nBroadcast,pCANABH3_RESULT pPtr);

//マルチパケットによるTelABH3パケットの送受信
CANABH3API int32_t abh3_can_trans(uint8_t nTargetID,char* sbuf,char* rbuf,uint32_t* rbuflen);

//現在の受信データ領域の複製を取得
CANABH3API void abh3_can_copylastdata(uint8_t nTargetID,pCANABH3_LASTRECV pPtr);

//受信データ領域の更新フラグを解除
CANABH3API void abh3_can_resetlastdata(uint8_t nTargetID,int32_t nAdrs);

//指定種類のパケットを受信
CANABH3API int32_t abh3_can_read(uint8_t nTargetID,pCANABH3_RESULT pPtr,PACKETTYPE nType,uint8_t nAdrs);

//受信バッファの内容を受信データ状況に反映させる
CANABH3API int32_t abh3_can_flush(void);

//回線を閉じる
CANABH3API int32_t abh3_can_finish(void);

//================================================================================
//ユーザー用関数(単位変換系)
//================================================================================

//速度から内部値へ変換
CANABH3API int16_t cnvVel2CAN(float vel);

//内部値から速度へ変換
CANABH3API float cnvCAN2Vel(int16_t vel);

//電流値から内部値に変換
CANABH3API int16_t cnvCur2CAN(float cur);

//内部値から電流値に変換
CANABH3API float cnvCAN2Cur(int16_t cur);

//内部値から負荷率に変換
CANABH3API float cnvCAN2Load(int16_t load);

//内部値からアナログ入力に変換
CANABH3API float cnvCAN2Analog(int16_t analog);

//内部値から電源電圧に変換
CANABH3API float cnvCAN2Volt(int16_t volt);

//実数からPACK_FLOATへ変換
CANABH3API PACK_FLOAT flt_to_pkflt(float flt);

//PACK_FLOATから実数へ変換
CANABH3API float pkflt_to_flt(PACK_FLOAT okflt);

//ANSI文字列からPACK_FLOATへ変換
CANABH3API int str_to_pkfltA(const char* str,PACK_FLOAT* p_pkflt);

//UNICODE文字列からPACK_FLOATへ変換
CANABH3API int str_to_pkfltW(const wchar_t* str,PACK_FLOAT* p_pkflt);

//PACK_FLOATからANSI文字列に変換
CANABH3API char* pkflt_to_strA(PACK_FLOAT pkflt,char* str);

//PACK_FLOATからUNICODE文字列に変換
CANABH3API wchar_t* pkflt_to_strW(PACK_FLOAT pkflt,wchar_t* str);

//============================================================
//リンケージ解除
//============================================================
#ifdef __cplusplus
 }
#endif //__cplusplus

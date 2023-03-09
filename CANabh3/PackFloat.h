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

#ifndef _PACK_FLOAT_H_INCLUDED_
#define _PACK_FLOAT_H_INCLUDED_

#include <stdint.h>
#include "typedef.h"

/*
// 型定義
typedef struct _PACK_FLOAT
	{
	union
		{
		struct _PACK_FLOAT_INFO
			{
			//LSB側から記載(処理系に注意)
			int32_t kasuu:21;			// 仮数部(-999999～0～99999)
			int32_t shousuutenn:4;	// 少数点(-8～0～7)
			int32_t shisuu:7;			// 指数部(-64～0～63)
			//こちらがMSB側
			} info;
		int32_t	nDirectData;		// 直接アクセス用
		} u;
	} *pPACK_FLOAT,PACK_FLOAT;
*/

//クラス
class CPackFloat : public _PACK_FLOAT
{
public:
	//コンストラクタ
	CPackFloat()
		{SetValue(0.0f);}
	CPackFloat(float nValue)
		{SetValue(nValue);}
	CPackFloat(double nValue)
		{SetValue(nValue);}
	CPackFloat(PACK_FLOAT pack)
		{SetValue(pack);}
	CPackFloat(char* pValue)
		{SetValueA(pValue);}
	CPackFloat(wchar_t* pValue)
		{SetValueW(pValue);}

	//値を設定する
	int	SetValueA(char* pStr);								//文字列を値として設定
	int	SetValueW(wchar_t* pStr);							//文字列を値として設定
	void SetDirectValue(int32_t nValue);					//内部値を直接設定する
	void SetValue(PACK_FLOAT pack);							//他のPACK_FLOAT変数と同じ値を設定
	void SetValue(float nValue);							//floatを値として設定
	void SetValue(double nValue)							//doubleを値として設定、但し精度はfloat
		{SetValue((float)nValue);}

	//値を取得する(文字列)
	void GetValueA(char* pStr,uint32_t nMaxLength);			//値を文字列で取得
	void GetValueW(wchar_t* pStr,uint32_t nMaxLength);		//値を文字列で取得
	void GetValue(pPACK_FLOAT pPack);						//値をPACK_FLOAT形式で取得
	void GetValue(float& nValue);							//値をfloat形式で取得
	void GetValue(double& nValue)							//値をdouble形式で取得、但し精度はfloat
		{float nTmp;GetValue(nTmp);nValue = (double)nTmp;}
	int32_t GetDirectValue(void);							//内部値を直接取得する

private:
	// 10進文字列を整数に変換
	static char* flA(char* str, int32_t* rx, int* rdp);
	static wchar_t* flW(wchar_t* str, int32_t* rx, int* rdp);

	// 文字列に変換
	static char* cvA(int32_t x, char* p);
	static wchar_t* cvW(int32_t x, wchar_t* p);
};
			 
#endif

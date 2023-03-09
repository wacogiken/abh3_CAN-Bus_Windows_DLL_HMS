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

//�W���I�ȃC���N���[�h
#include <Windows.h>
#include <stdint.h>

//�p�����N���X
#include "CanIF.h"

//�{�N���X�Ŏg�p����f�o�C�X�p�̃C���N���[�h
//(USB-to-CAN v2 �̃t�H���_���v���W�F�N�g�ɒǉ����ĉ�����
#define INITGUID
#include "vcisdk.h"
#include "vcitype.h"

//�{�N���X��`
class CIxxatV2 : public CCanIF
{
public:
//================================================================================
//�g�pCAN�C���^�[�t�F�[�X���Ɏ������K�{�̉��z�֐�
//================================================================================
public:

	//���p�\��CAN�C���^�[�t�F�[�X�����擾����ꍇ�ɌĂяo����܂�
	virtual int32_t OnGetInterfaceCount(void);

	//�w��CAN�C���^�[�t�F�[�X���J���ꍇ�ɌĂяo����܂�
	virtual int32_t OnOpenInterface(int32_t nDeviceNum);

	//�J����CAN�C���^�[�t�F�[�X�����ꍇ�ɌĂяo����܂�
	virtual void OnCloseInterface(void);

	//CAN�C���^�[�t�F�[�X�����M����ꍇ�ɌĂяo����܂�
	virtual int32_t OnCanRecv(uint32_t* pCanID,uint8_t* pData8,bool bNoWait = false);

	//CAN�C���^�[�t�F�[�X�ɑ��M����ꍇ�ɌĂяo����܂�
	virtual int32_t OnCanSend(uint32_t nCanID,uint8_t* pData8,uint8_t nLength);

	//CAN�C���^�[�t�F�[�X�ɃG���[���L�邩���ׂ�ꍇ�ɌĂяo����܂�
	virtual int32_t OnCanGetError(void);

	//CAN�C���^�[�t�F�[�X�̃G���[����������ꍇ�ɌĂяo����܂�
	virtual int32_t OnCanClearError(void);

	//��M�o�b�t�@���N���A����ꍇ�ɌĂяo����܂�
	virtual int32_t OnCanRecvClear(void);

//================================================================================
//�����p�i�X���b�h���痘�p�ׂ�public�����j
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
	HANDLE			m_hSema;			//��M�o�b�t�@�r������p�Z�}�t�H
	HANDLE			m_hReadThread;		//��M�X���b�h�n���h��
	PCANMSG			m_pCanMsg;
	uint32_t		m_nCanReadPt;
	uint32_t		m_nCanWritePt;

	//�񓯊���M�X���b�h
	static void ReceiveThread(void* Param);

	//��M�o�b�t�@�r������v��
	bool LockCanMsg(DWORD nTimeoutMS = INFINITE);

	//��M�o�b�t�@�r���������
	void UnlockCanMsg(void);

	//��M�f�[�^�̓o�^����
	uint32_t AddCanMsg(PCANMSG pMsg,int nCount = 1);

	//�o�^���ꂽCAN���b�Z�[�W��1�擾
	uint32_t GetCanMsg(PCANMSG pMsg);

//================================================================================
//�����p
//================================================================================
protected:

public:
	CIxxatV2();
	virtual ~CIxxatV2();
};

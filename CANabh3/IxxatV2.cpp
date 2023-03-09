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
//�T�v
//========================================
//	Ixxat V2�̃f�o�C�X���g���ׂ̃N���X
//
//========================================
//����
//========================================
//2023/02/24	yo0043		OnCanRecv�ɑ҂����ԂȂ��@�\�ǉ�
//
//========================================
//�����ɕK�v�ȑO����
//========================================
//	IxxatV2�p��SDK���C���X�g�[��
//	�v���W�F�N�g�ݒ�ŁA�ǉ��C���N���[�h�t�H���_�Ɉȉ���ǉ�
//		"C:\Program Files\HMS\Ixxat VCI\sdk\vci\inc"
//	�v���W�F�N�g�ݒ�ŁA�ǉ����C�u�����t�H���_�Ɉȉ���ǉ�
//		32bit "C:\Program Files\HMS\Ixxat VCI\sdk\vci\lib\x32\release"
//
//========================================
//�g�p���@
//========================================
//	��{�I�Ȏg�����́A�T���v���\�[�X�����ĉ�����
//
//========================================
///�f�o�C�X�ŗL
//========================================
//�G���[������
//	��{�I�Ɋ֐��ŃG���[������������A�����ؒf���A�ڑ����鏊����
//	��蒼���𐄏����܂��B�i���Z�b�g�őS�v�f��������̂ŁA���ǌq�����������K�{�ƂȂ�ׁj
//
//	�{�[���[�g�ɂ͈ȉ��̒l�݂̂��ݒ�\�ł�
//		�l		�ݒ�			�l		�ݒ�	
//	---------------------------------------------			
//		10	->	10Kbps			20	->	20Kbps
//		50	->	50Kbps			100	->	100Kbps
//		125	->	125Kbps			250	->	250Kbps
//		500	->	500Kbps			800	->	800Kbps
//		1000->	1000Kbps
//	---------------------------------------------			
//
//OnOpenInterface�ɓn���f�o�C�X�̔ԍ�
//	���{�ڂ̃C���^�[�t�F�[�X��(0�J�n)��n���ĉ�����
//
//========================================

//
#include "pch.h"
#include "IxxatV2.h"
#include <process.h>

//
#pragma warning(disable : 6385)

//�{�N���X�ň����f�o�C�X�ɕK�{�̃��C�u����
#pragma comment(lib,"vciapi.lib")


//���S�ȍ폜
template <class T> void SafeDelete(T** ppT)
	{
	if (*ppT)
		{
		delete *ppT;
		*ppT = NULL;
		}
	}

//���S�ȉ��
template <class T> void SafeRelease(T **ppT)
	{
	if (*ppT)
		{
		(*ppT)->Release();
		*ppT = NULL;
		}
	}

//�R���X�g���N�^
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

	//��M�f�[�^�i�[�p�̈�̏�����
	m_pCanMsg = new CANMSG[256]();
	}

//�f�X�g���N�^
CIxxatV2::~CIxxatV2()
	{
	//������J���Ă���ꍇ���L��̂ŕ��鏈�����s��
	OnCloseInterface();

	//
	SafeDelete(&m_pCanMsg);

	}

//================================================================================
//�g�pCAN�C���^�[�t�F�[�X���Ɏ������K�{�̉��z�֐�
//================================================================================

//���p�\��CAN�C���^�[�t�F�[�X�����擾����ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnGetInterfaceCount()
	{
	//�T�v
	//	�w��CAN�C���^�[�t�F�[�X�̗��p�\�����擾����ꍇ�ɌĂяo����܂�
	//�p�����[�^
	//	����
	//�߂�l
	//	0			���p�\�C���^�[�t�F�[�X������܂���
	//	-1			�������ɃG���[���������܂���
	//	��L�ȊO	���p�\��CAN�C���^�[�t�F�[�X��
	//

	//todo:CAN�C���^�[�t�F�[�X�̐���߂��������������ĉ�����

	//�f�o�C�X�}�l�[�W�����J��
	IVciDeviceManager*  pDevMgr = 0;    // device manager
	HRESULT hResult = VciGetDeviceManager(&pDevMgr);
	if (hResult != VCI_OK)
		{
		//�f�o�C�X�}�l�[�W���ŃG���[
		return(-1);
		}

	//�ڑ����ꂽ�C���^�[�t�F�[�X�f�o�C�X��񋓂��Đ��𒲂ׂ�
	VCIDEVICEINFO sInfo;
	int32_t nCount = 0;
	IVciEnumDevice* pEnum = NULL;
	hResult = pDevMgr->EnumDevices(&pEnum);
	while (hResult == VCI_OK)
		{
		//���擾
		hResult = pEnum->Next(1,&sInfo,NULL);
		//�擾�����H
		if (hResult == VCI_OK)
			++nCount;	//�f�o�C�X�����Z
		}
	SafeRelease(&pEnum);

	//����
	return(nCount);
	}

//�w��CAN�C���^�[�t�F�[�X���J���ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnOpenInterface(int32_t nDeviceNum)
	{
	//�T�v
	//	�w��CAN�C���^�[�t�F�[�X���J���ꍇ�ɌĂяo����܂�
	//�p�����[�^
	//	nDeviceNum		0�J�n�̃C���^�[�t�F�[�X�ԍ�
	//�߂�l
	//	0				����I��
	//	0�ȊO			�ُ�I��

	//todo:CAN�C���^�[�t�F�[�X���J���������������ĉ�����

	//
	if(nDeviceNum >= BAL_MAX_SOCKETS)
		return(-2);	//�f�o�C�X�ԍ��̎w�肪�������Ȃ�

	//�ݒ肩�������J���ׂ̋��e���Ԃ��擾
	uint32_t nTimeoutMS = GetTimeout_Open();

	//1�ȏ�̃C���^�[�t�F�[�X���L�邩�H
	if (OnGetInterfaceCount() < 1)
		{
		return(-1);
		}

	//�f�o�C�X�}�l�[�W�����擾����
	IVciDeviceManager*  pDevMgr = 0;
	HRESULT hResult = VciGetDeviceManager(&pDevMgr);

	//�f�o�C�X�}�l�[�W���擾�Ɏ��s�H
	if (hResult != VCI_OK)
		{
		return(-1);
		}

	//�f�o�C�X�}�l�[�W�����g�p���ăf�o�C�X�񋓃C���^�[�t�F�[�X���擾����
	IVciEnumDevice* pEnum = NULL;
	hResult = pDevMgr->EnumDevices(&pEnum);

	//�f�o�C�X�񋓃C���^�[�t�F�[�X���g�p���ăf�o�C�X�����擾����
	int32_t nCount = 0;	//
	VCIDEVICEINFO sInfo;
	while (hResult == VCI_OK)
		{
		//�f�o�C�X�񋓃C���^�[�t�F�[�X���g�p���āA�f�o�C�X����(�擪����)���Ɏ擾����
		hResult = pEnum->Next(1,&sInfo,NULL);
		if (hResult == VCI_OK)
			{
			//�f�o�C�X���𐳏�Ɏ擾�����̂ŁA�f�o�C�X�����X�V
			++nCount;
			}
		}

	//�w��C���^�[�t�F�[�X�ԍ��̃f�o�C�X�͑��݂��ĂȂ��H
	if (nDeviceNum >= nCount)
		{
		SafeRelease(&pEnum);
		SafeRelease(&pDevMgr);
		return(-1);
		}

	//�f�o�C�X�}�l�[�W�����g�p���ăf�o�C�X���擾����
	IVciDevice* pDevice = 0;
	hResult = pDevMgr->OpenDevice(sInfo.VciObjectId,&pDevice);
	if (hResult != VCI_OK)
		{
		//�f�o�C�X�̎擾�Ɏ��s����
		SafeRelease(&pEnum);
		SafeRelease(&pDevMgr);
		return(-1);
		}

	//�f�o�C�X�𗘗p���āA�o�X�A�N�Z�X���C���[���擾����
	hResult = pDevice->OpenComponent(CLSID_VCIBAL,IID_IBalObject,(void**)&m_pBalObject);

	//�s�v�ƂȂ����f�o�C�X���J��
	SafeRelease(&pDevice);

	//�s�v�ƂȂ����f�o�C�X�񋓂��J��
	SafeRelease(&pEnum);

	//�s�v�ƂȂ����f�o�C�X�}�l�[�W�����J��
	SafeRelease(&pDevMgr);

	//================================================================================
	//���̎��_�ŁA���J���̕��̓o�X�A�N�Z�X���C���[�̂�
	//================================================================================

	//�o�X�A�N�Z�X���C���[���������H
	if (m_pBalObject == NULL)
		return(-1);

	//�o�X�A�N�Z�X���C���[�𗘗p���āA�@�\���擾
	BALFEATURES features = { 0 };
	hResult = m_pBalObject->GetFeatures(&features);
	if (hResult != VCI_OK)
		{
		//�@�\���̎擾�Ɏ��s
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//�w�肵���f�o�C�X�ԍ����������Ȃ��H
	if (nDeviceNum >= features.BusSocketCount)
		{
		//�w�肵���f�o�C�X�ԍ��̓f�o�C�X���𒴉߂��Ă���
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//�w�肵���f�o�C�X�ԍ��̃f�o�C�X�����p�\��CAN�f�o�C�X�ł͖����H
	if (VCI_BUS_TYPE(features.BusSocketType[nDeviceNum]) != VCI_BUS_CAN)
		{
		//�w�肵���f�o�C�X�ԍ��̃f�o�C�X�͗��p�s��
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//�o�X�A�N�Z�X���C���[���g�p����CAN�p�\�P�b�g���J��
	ICanSocket* pCanSocket = 0;
	hResult = m_pBalObject->OpenSocket(nDeviceNum,IID_ICanSocket,(void**)&pCanSocket);
	if (hResult == VCI_OK)
		{
		//CAN�p�\�P�b�g���g�p���Č݊������擾
		CANCAPABILITIES capabilities = { 0 };
		hResult = pCanSocket->GetCapabilities(&capabilities);
		if (VCI_OK == hResult)
			{
			//�݊������͎擾�o����
			//����CAN�p�\�P�b�g�ł́A�W��ID(11bits)�Ɗg��ID(29bits)���g���邩�H
			//�i29bits�����̔�����@���������ɒ��Ӂj
			if (capabilities.dwFeatures & CAN_FEATURE_STDANDEXT)
				{
				//������ID���g����i�܂�g��ID(29bits)�����p�\�j

				//���b�Z�[�W�p�`�����l�����\�z
				m_pCanChn = NULL;
				hResult = pCanSocket->CreateChannel(FALSE,&m_pCanChn);
				if(hResult != VCI_OK)
					{
					}
				}
			else
				{
				//�g���Ȃ��i�����W��ID(11bits)�����g���Ȃ��Ǝv����j
				hResult = -1;
				}
			}
		else
			{
			//�݊������̎擾�Ɏ��s����
			}
		}

	//CAN�p�\�P�b�g���J��
	SafeRelease(&pCanSocket);

	//�G���[�L��H
	if (VCI_OK != hResult)
		{
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//================================================================================
	//���̎��_�ŁA���J���̕���
	//	�o�X�A�N�Z�X���C���[(m_pBalObject)
	//	���b�Z�[�W�`�����l��(m_pCanChn)
	//================================================================================

	//���b�Z�[�W�`�����l����������
	UINT16 wRxFifoSize	= 1024;
	UINT16 wRxThreshold	= 1;		//1�ȏ�̎�M�Œʒm
	UINT16 wTxFifoSize	= 128;
	UINT16 wTxThreshold	= 1;		//���M�c�肪1�����Œʒm
	hResult = m_pCanChn->Initialize(wRxFifoSize,wTxFifoSize);
	if (hResult != VCI_OK)
		{
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//���b�Z�[�W�`�����l���𗘗p���āA���[�_�[���擾����
	hResult = m_pCanChn->GetReader(&m_pReader);
	if (hResult == VCI_OK)
		{
		//���[�_�[���擾�o�����̂ŁA�ʒm���x���i�ʒm������M���j��ݒ肷��
		m_pReader->SetThreshold(wRxThreshold);

		//��M�p�C�x���g���\�z
		m_hEventReader = CreateEvent(NULL,FALSE,FALSE,NULL);

		//���[�_�[���g���C�x���g�Ƃ��ēo�^
		m_pReader->AssignEvent(m_hEventReader);
		}
	else
		{
		SafeRelease(&m_pCanChn);
		SafeRelease(&m_pBalObject);
		return(-1);
		}

	//���C�^�[���擾����
	hResult = m_pCanChn->GetWriter(&m_pWriter);
	if (hResult == VCI_OK)
		{
		//���C�^�[���擾�o�����̂ŁA�ʒm���x���i�ʒm����鑗�M���j��ݒ肷��
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

	//���b�Z�[�W�p�`�����l����L��������
	hResult = m_pCanChn->Activate();

	//�G���[�L��H
	if(hResult != VCI_OK)
		{
		//��M�p�C�x���g�J��
		::CloseHandle(m_hEventReader);
		m_hEventReader = NULL;
		//���b�Z�[�W�`�����l���J��
		SafeRelease(&m_pCanChn);
		//�o�X�A�N�Z�X���C���[�J��
		SafeRelease(&m_pBalObject);
		//���s�����Ŗ߂�
		return(-1);
		}

	//================================================================================
	//���̎��_�ŁA���J���̕���
	//	�o�X�A�N�Z�X���C���[(m_pBalObject)
	//	���b�Z�[�W�`�����l��(m_pCanChn)
	//================================================================================

	//CAN�R���g���[���̎擾
	hResult = m_pBalObject->OpenSocket(nDeviceNum,IID_ICanControl,(void**)&m_pCanControl);
	if (hResult == VCI_OK)
		{
		//�{�[���[�g���擾���A�ݒ�p�̒l�ɕϊ�
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

		//CAN�R���g���[���𗘗p���āACAN���C�����w��{�[���[�g�ݒ�ŏ���������
		CANINITLINE init =
			{
			 CAN_OPMODE_STANDARD | CAN_OPMODE_EXTENDED | CAN_OPMODE_ERRFRAME		// opmode
			,0																		// bReserved
			,nB0																	// bt0�̃{�[���[�g
			,nB1																	// bt1�̃{�[���[�g
			};
		hResult = m_pCanControl->InitLine(&init);
		if (hResult != VCI_OK)
			{
			//CAN���C���̏������Ɏ��s
			}

		//�������ɃG���[���������H
		if (hResult == VCI_OK)
			{
			//CAN�R���g���[���𗘗p���āA�A�N�Z�X�^���X�t�B���^�ɕW��CANID(11bits)���w�肵�Ă݂�
			//dwCode = �]��ID���w��AID���w�莞��CAN_ACC_CODE_NONE���w�肷�邩�AdwMask�őS�r�b�g�����ɂ���
			//dwMask = �]��ID�́u�L����bit�v���w��i0..���� 1..�L���j
			//	#define CAN_ACC_MASK_ALL          0x00000000
			//	#define CAN_ACC_CODE_ALL          0x00000000
			//	#define CAN_ACC_MASK_NONE         0xFFFFFFFF
			//	#define CAN_ACC_CODE_NONE         0x80000000

			hResult = m_pCanControl->SetAccFilter(CAN_FILTER_STD,CAN_ACC_CODE_ALL,CAN_ACC_MASK_ALL);
			if (hResult == VCI_OK)
				{
				//CAN�R���g���[���𗘗p���āA�A�N�Z�X�^���X�t�B���^�Ɋg��(29bits)���w�肵�Ă݂�
				hResult = m_pCanControl->SetAccFilter(CAN_FILTER_EXT,CAN_ACC_CODE_ALL,CAN_ACC_MASK_ALL);
				}
			//�ǂ��炩�̃t�B���^�w�肪���s�����H
			if (VCI_OK != hResult)
				{
				}
			}

		//�������ɃG���[���������H
		if (hResult == VCI_OK)
			{
			//CAN���C���̐�����J�n����
			hResult = m_pCanControl->StartLine();
			if (hResult != VCI_OK)
				{
				//CAN���C���̐���J�n�����s����
				}
			}
		}
	else
		{
		//CAN�R���g���[���̎擾�Ɏ��s����
		}
	
	//�������ɃG���[���L�邩�H
	if (hResult != VCI_OK)
		return(-1);

	//�X�e�[�^�X�̏�����
	m_status.err.nRaw = 0;

	//��M�f�[�^�r������p�Z�}�t�H
	m_hSema = ::CreateSemaphore(NULL,1,1,NULL);

	//��M�X���b�h�J�n
	_beginthread(ReceiveThread,0,this);

	//�K�莞�Ԗ���CAN started���b�Z�[�W�����邩���ׂ�
	//�i�ʒm�͔񓯊��j
	bool bReady = false;
	uint32_t nStartTime = GetTm();
	uint32_t nEndTime = nStartTime + nTimeoutMS;
	do
		{
		//CAN�X�^�[�g�i����J�n�j��ԁH
		if(m_status.bCanStart)
			{
			bReady = true;
			break;
			}

		//�҂����ԁi�Œ�j
		Sleep(100);

		} while(GetTm() < nEndTime);
	if(!bReady)
		{
		//���s�ׁ̈A�C���^�[�t�F�[�X����Č�n��������
		OnCloseInterface();
		return(-1);	//���s�����Ŗ߂�
		}

	//��������
	return(0);
	}

//�J����CAN�C���^�[�t�F�[�X�����ꍇ�ɌĂяo����܂�
void CIxxatV2::OnCloseInterface()
	{
	//�T�v
	//	�J����CAN�C���^�[�t�F�[�X�����ꍇ�ɌĂяo����܂�
	//�p�����[�^
	//	����
	//�߂�l
	//	����

	//todo:CAN�C���^�[�t�F�[�X���J���Ă���ꍇ�ɕ��鏈�����������ĉ�����

	//��M�X���b�h���쒆�H
	if (m_hReadThread)
		{
		//��M�X���b�h��~
		m_lMustQuit = 1;
		::WaitForSingleObject(m_hReadThread,INFINITE);
		::CloseHandle(m_hReadThread);
		m_hReadThread = NULL;
		}

	//���[�_�[�J��
	SafeRelease(&m_pReader);

	//���C�^�[�J��
	SafeRelease(&m_pWriter);

	//�`�����l���C���^�[�t�F�[�X�J��
	SafeRelease(&m_pCanChn);

	//CAN�R���g���[���g�p���H
	if (m_pCanControl)
		{
		//CAN�R���g���[�����~������
		HRESULT hResult = m_pCanControl->StopLine();
		if (hResult != VCI_OK)
			{
			//��~�Ɏ��s�i���s�͊��ɒ�~�ς݁H�j
			}

		//CAN�R���g���[���ƃt�B���^��������
		hResult = m_pCanControl->ResetLine();
		if (hResult != VCI_OK)
			{
			//�������Ɏ��s�i���ɒ�~�ς݁H�j
			}

		//CAN�R���g���[�����J��
		SafeRelease(&m_pCanControl);
		}

	//�o�X�A�N�Z�X���C���[���J������
	SafeRelease(&m_pBalObject);

	//��M�o�b�t�@�r������p�Z�}�t�H�J��
	if(m_hSema)
		{
		::CloseHandle(m_hSema);
		m_hSema = NULL;
		}
	}

//CAN�C���^�[�t�F�[�X�����M����ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnCanRecv(uint32_t* pCanID,uint8_t* pData8,bool bNoWait /* false */)
	{
	//�T�v
	//	CAN�C���^�[�t�F�[�X����1���̃p�P�b�g����M���܂�
	//�p�����[�^
	//	pCanID			��M�p�P�b�g��ID���i�[����̈�ւ̃|�C���^
	//	pData8			��M�p�P�b�g���i�[����8[bytes]�̈�ւ̃|�C���^
	//	bNoWait			true�̏ꍇ�A���Ɏ�M�ς݃p�P�b�g�̂ݑΏۂƂ��Ď�M�҂����܂���
	//					false�̏ꍇ�A��M�ς݃p�P�b�g�������ꍇ�́AGetTimeoutRecv�Ŏ擾�������Ԗ���M��҂��܂�
	//�߂�l
	//	0				����I��
	//	0�ȊO			�ُ�I��

	//todo:CAN�C���^�[�t�F�[�X����1�񕪂̎�M���s���������������ĉ�����

	//�߂�l
	int32_t nResult = -1;

	//��M�^�C���A�E�g��Ύ��Ԃ��Z�o
	uint32_t nLimitTime = GetTm() + GetTimeout_Recv();

	//�������ԓ��̂ݏ�������i�A���Œ�1��͏�������j
	do
		{
		CANMSG msg;
		if (GetCanMsg(&msg) == 0)
			{
			//����擾
			*pCanID = uint32_t(msg.dwMsgId);
			::memcpy(pData8,msg.abData,sizeof(char) * 8);
			nResult = 0;
			break;
			}
		//�҂����Ԗ����H
		if(bNoWait)
			break;
		//���ԑ҂�
		Sleep(1);
		} while(GetTm() < nLimitTime);

	//
	return(nResult);
	}

//CAN�C���^�[�t�F�[�X�ɑ��M����ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnCanSend(uint32_t nCanID,uint8_t* pData8,uint8_t nLength)
	{
	//�T�v
	//	CAN�C���^�[�t�F�[�X��1���̃p�P�b�g�𑗐M���܂�
	//�p�����[�^
	//	nCanID			���M�p�P�b�g��ID
	//	pData8			���M�p�P�b�g���i�[����Ă���8[bytes]�̈�ւ̃|�C���^
	//	nTimeoutMS		���M���e����[ms]
	//�߂�l
	//	0				����I��
	//	0�ȊO			�ُ�I��

	//todo:CAN�C���^�[�t�F�[�X�ɑ��M���鏈�����������ĉ�����

	//�f�[�^��
	UINT payloadLen = nLength;

	//���M�pFIFO�ɃA�N�Z�X���āA�f�[�^��ݒ肷��
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
			pMsg->uMsgInfo.Bits.ssm		= 1;					//�V���O���V���[�g���b�Z�[�W
			pMsg->uMsgInfo.Bits.hpm		= 0;					//�n�C�v���C�I���e�B���w�肵�Ȃ�
			pMsg->uMsgInfo.Bits.edl		= 0;					//�g���f�[�^��t���Ȃ�
			pMsg->uMsgInfo.Bits.fdr		= 0;					//High bit rate
			pMsg->uMsgInfo.Bits.esi		= 0;					//(out)Error state indicator
			pMsg->uMsgInfo.Bits.res		= 0;					//�\��G���A(0�w��)
			pMsg->uMsgInfo.Bits.dlc		= payloadLen;			//�p�P�b�g��[byte(s)]
			pMsg->uMsgInfo.Bits.ovr		= 0;					//(out)Data overrun
			pMsg->uMsgInfo.Bits.srr		= 1;					//Self reception request.
			pMsg->uMsgInfo.Bits.rtr		= 0;					//Remote transmission request
			pMsg->uMsgInfo.Bits.ext		= 1;					//�g��ID(29bits)
			pMsg->uMsgInfo.Bits.afc		= 0;					//(out)�A�N�Z�v�^���X�t�B���^�R�[�h

			//�f�[�^�i�[
			for (UINT nLoop = 0;nLoop < payloadLen;nLoop++)
				pMsg->abData[nLoop] = pData8[nLoop];

			//���M���K�v
			written = 1;
			}

		//�X�e�[�^�X������������i���̑��M�Ŕ�������G���[�𔻕ʂ���ׁj
		m_status.err.nRaw = 0;	//@@�b��

		//���M�pFIFO�𓮍삳���A���M������
		hResult = m_pWriter->ReleaseWrite(written);
		if (hResult != VCI_OK)
			{
			//���M���s
			return(-1);
			}

		//�J�E���^�̒l�ɑ��M�T�C�Y(dlc)�����Z
		AddCounter((uint32_t)payloadLen);
		}
	else
		{
		//���M�pFIFO�ւ̃A�N�Z�X�Ɏ��s
		return(-1);
		}

	return(0);
	}

//CAN�C���^�[�t�F�[�X�ɃG���[���L�邩���ׂ�ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnCanGetError()
	{
	//�T�v
	//	CAN�C���^�[�t�F�[�X�ɃG���[���L�邩���ׂ܂�
	//�p�����[�^
	//	����
	//�߂�l
	//	0				����
	//	0�ȊO			�ُ�
	//���ӓ_
	//	���̃f�o�C�X�ł͔��������G���[���S�Ď�M�f�[�^�Ƃ��Ēʒm�����

	//todo:CAN�C���^�[�t�F�[�X�ɃG���[���L�邩���ׂ鏈�����������ĉ�����

	//�����Ɋi�[�����G���[���L�邩�H
	if(m_status.err.nRaw)
		return(1);

	return(0);
	}

//CAN�C���^�[�t�F�[�X�̃G���[����������ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnCanClearError()
	{
	//�T�v
	//	CAN�C���^�[�t�F�[�X�̃G���[���������܂�
	//�p�����[�^
	//	����
	//�߂�l
	//	0				����
	//	0�ȊO			�ُ�

	//todo:CAN�C���^�[�t�F�[�X�̃G���[���������鏈�����������ĉ�����

	//�r�����䐬���H
	if(LockCanMsg())
		{
		//�����Ɋi�[�����G���[�l������
		m_status.err.nRaw = 0;
		//�r���������
		UnlockCanMsg();
		}
	//�f�o�C�X�̃G���[������
	m_pCanControl->ResetLine();

	return(0);
	}

//��M�o�b�t�@���N���A����ꍇ�ɌĂяo����܂�
int32_t CIxxatV2::OnCanRecvClear()
	{
	//�T�v
	//	��M�o�b�t�@���N���A����ꍇ�ɌĂяo����܂�
	//�p�����[�^
	//	����
	//�߂�l
	//	0				����
	//	0�ȊO			�ُ�

	//todo:��M�o�b�t�@���N���A���鏈�����������ĉ�����

	//�r�����䐬���H
	if(LockCanMsg())
		{
		m_nCanReadPt = 0;
		m_nCanWritePt = 0;
		//�r���������
		UnlockCanMsg();
		}
	return(0);
	}

//================================================================================
//�{�N���X��p�̗v�f
//================================================================================

//�񓯊���M�X���b�h
void CIxxatV2::ReceiveThread(void* Param)
	{
	//�X���b�h�\�z���̃N���X
	CIxxatV2* pClass = (CIxxatV2*)Param;

	//
	DWORD		nWaitTimeoutMS = 10;	//��M�҂�����[ms]
	uint32_t	nStage = 0;				//��M�҂��X�e�[�W����J�n
	PCANMSG		pCanMsg = 0;
	UINT16		wCount = 0;

	//
	while (pClass->m_lMustQuit == 0)
		{
		//��M�҂�
		if (nStage == 0)
			{
			//�C�x���g�҂��i��������Ǝ�M�����ŏ�񂪓����Ă���j
			if (WaitForSingleObject(pClass->m_hEventReader,10) == WAIT_OBJECT_0)
				nStage = 100;
			}
		//�f�[�^��M
		else if (nStage == 100)
			{
			//��M����
			pCanMsg = NULL;
			HRESULT hResult = pClass->m_pReader->AcquireRead((PVOID*)&pCanMsg,&wCount);
			if (hResult == VCI_OK)
				{
				//�擾�����u��M�p�P�b�g�v�́A�S�ēo�^�ɉ�
				//�i�o�^���Œ��g�𔻒f����j

				//�o�^
				pClass->AddCanMsg(pCanMsg,int(wCount));
				//�f�[�^��M�ɖ߂�
				nStage = 100;
				}
			else if (hResult == VCI_E_RXQUEUE_EMPTY)
				{
				//��M�o�b�t�@���󂾂����̂ŁA��M�҂��ɖ߂�
				nStage = 0;
				}
			else
				{
				//�����C���M�����[�ȏ�ԂɂȂ����̂ŁA��M�҂��ɖ߂�
				nStage = 0;
				}
			//��M����J��
			pClass->m_pReader->ReleaseRead(wCount);
			}
		}
	//
	_endthread();
	}

//��M�o�b�t�@�r������v��
bool CIxxatV2::LockCanMsg(DWORD nTimeoutMS /* INFINITE */)
	{
	//�����m�ۂɎ��s�H
	if(::WaitForSingleObject(m_hSema,nTimeoutMS) != WAIT_OBJECT_0)
		return(false);	//�m�ێ��s
	//�m�ې���
	return(true);
	}

//��M�o�b�t�@�r���������
void CIxxatV2::UnlockCanMsg()
	{
	//�m�ۂ����������J��
	::ReleaseSemaphore(m_hSema,1,NULL);
	}

//��M�f�[�^�̓o�^����
uint32_t CIxxatV2::AddCanMsg(PCANMSG pMsg,int nCount /* 1 */)
	{
	//������L
	if(!LockCanMsg())
		return(0);	//������L���s���́A�o�^��0�Ƃ��Ė߂�

	//
	uint32_t nResult = 0;
	while(nCount)
		{
		//�J�E���^�̒l�Ɏ�M�T�C�Y(dlc)�����Z
		//�X�e�[�^�X���A�����]�v�ȕ������邪�����܂ł̐��x�͋��߂Ă��Ȃ�
		AddCounter((uint32_t)pMsg->uMsgInfo.Bits.dlc);

		//�o�^�v�f�̓f�[�^�p�P�b�g�H
		if (pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_DATA)
			{
			//���̃p�P�b�g�͖������Ȃ��Ώۂ��H
			if(!IsIgnoreID(pMsg->dwMsgId))
				{
				//�o�b�t�@FULL�ł͖����H
				if(m_nCanWritePt < 255)
					{
					//�i�[
					::CopyMemory(&m_pCanMsg[m_nCanWritePt],pMsg,sizeof(CANMSG));
					//�i�[��X�V
					++m_nCanWritePt;
					//�i�[���X�V
					++nResult;
					}
				else
					{
					//�o�b�t�@����t
					Sleep(0);
					}
				}
			else
				{
				//��������Ώ�
				Sleep(0);
				}
			}

		//�o�^�v�f�͏��p�P�b�g�H
		else if (pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_INFO)
			{
			//���ʏ����̔��ʊJ�n
			switch (pMsg->abData[0])
				{
				//CAN�J�n�H
				case CAN_INFO_START:
					m_status.bCanStart = true;
					break;
				//CAN��~�H
				case CAN_INFO_STOP:
					m_status.bCanStart = false;
					break;
				//CAN���Z�b�g�H
				case CAN_INFO_RESET:
					m_status.bCanStart = false;
					break;
				}
			}

		//�o�^�v�f�̓G���[�p�P�b�g�H
		else if (pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_ERROR)
			{
			//�G���[�ʏ����̔��ʊJ�n
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

		//�o�^�v�f�̓X�e�[�^�X���H
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_STATUS)
			{
			//���������Ă���̂��s��
			Sleep(0);
			}
		//�H
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_WAKEUP)
			{
			//�ڍוs��
			Sleep(0);
			}
		//�o�^�v�f�̓^�C�}�[�I�[�o�[�������H
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_TIMEOVR)
			{
			//���������s��
			Sleep(0);
			}
		//�H
		else if(pMsg->uMsgInfo.Bytes.bType == CAN_MSGTYPE_TIMERST)
			{
			//�ڍוs��
			Sleep(0);
			}

		//�o�^�v�f�̎��ʒu
		++pMsg;

		//�o�^�v�f�̎c�����X�V
		--nCount;
		}
	//�����J��
	UnlockCanMsg();
	//
	return(nResult);
	}

//�o�^���ꂽCAN���b�Z�[�W��1�擾
uint32_t CIxxatV2::GetCanMsg(PCANMSG pMsg)
	{
	//
	uint32_t nResult = 0;
	//������L
	if(!LockCanMsg())
		return(2);	//������L���s���́A���̑��G���[�Ƃ��Ė߂�
	//���ǂݏo���̗v�f���L��H
	else if(m_nCanReadPt < m_nCanWritePt)
		{
		//�Y���ӏ�����擾
		::CopyMemory(pMsg,&m_pCanMsg[m_nCanReadPt],sizeof(CANMSG));
		//�ǂݏo���ʒu�̍X�V
		++m_nCanReadPt;
		//�ǂݏo���c�肪�����H
		if(m_nCanReadPt >= m_nCanWritePt)
			{
			//�������i�S�N���A�����Ńo�b�t�@��擪�ɖ߂����A�r������ς݂Ȃ̂�ClearCanMsg���g��Ȃ����j
			m_nCanReadPt = 0;
			m_nCanWritePt = 0;
			}
		nResult = 0;
		}
	else
		nResult = 1;

	//�����J��
	UnlockCanMsg();
	//
	return(nResult);
	}

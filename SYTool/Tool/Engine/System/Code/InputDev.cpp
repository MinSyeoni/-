#include "InputDev.h"

USING(Engine)
IMPLEMENT_SINGLETON(CInputDev)

Engine::CInputDev::CInputDev(void)
{

}

Engine::CInputDev::~CInputDev(void)
{
	Free();
}

HRESULT Engine::CInputDev::Ready_InputDev(HINSTANCE hInst, HWND hWnd)
{

	// DInput �İ�ü�� �����ϴ� �Լ�
	FAILED_CHECK_RETURN(DirectInput8Create(hInst, 
											DIRECTINPUT_VERSION, 
											IID_IDirectInput8, 
											(void**)&m_pInputSDK, 
											NULL), E_FAIL);

	// Ű���� ��ü ����
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr), E_FAIL);

	// ������ Ű���� ��ü�� ���� ������ �� ��ü���� �����ϴ� �Լ�
	m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard);
	
	// ��ġ�� ���� �������� �������ִ� �Լ�, Ŭ���̾�Ʈ�� ���ִ� ���¿��� Ű �Է��� ������ ������ �����ϴ� �Լ�
	m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// ��ġ�� ���� access ������ �޾ƿ��� �Լ�
	m_pKeyBoard->Acquire();


	// ���콺 ��ü ����
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr), E_FAIL);

	// ������ ���콺 ��ü�� ���� ������ �� ��ü���� �����ϴ� �Լ�
	m_pMouse->SetDataFormat(&c_dfDIMouse);

	// ��ġ�� ���� �������� �������ִ� �Լ�, Ŭ���̾�Ʈ�� ���ִ� ���¿��� Ű �Է��� ������ ������ �����ϴ� �Լ�
	m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// ��ġ�� ���� access ������ �޾ƿ��� �Լ�
	m_pMouse->Acquire();


	return S_OK;
}

void Engine::CInputDev::Set_InputDev(void)
{
	m_pKeyBoard->GetDeviceState(256, m_byKeyState);
	m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);
}

HRESULT CInputDev::Inquire_Input_State()
{
	if (nullptr == m_pKeyBoard ||
		nullptr == m_pMouse)
		return E_FAIL;

	// m_byKeyState�迭 ��, ���� Ű�� �ش��ϴ� �ε����迭�� ������ ä���. 
	// ��, ���������� Ű�鿡���ؼ��� �׻� 0�� ���� ����.
	m_pKeyBoard->GetDeviceState(256, m_byKeyState);

	m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_tMouseState);

	return NOERROR;
}

_bool CInputDev::Get_DIKeyUp(_ubyte byKeyID)
{
	if (m_byKeyState[byKeyID] & 0x80)
	{
		m_bIsKeyPressed[byKeyID] = true;
		return false;
	}
	else if (m_bIsKeyPressed[byKeyID])
	{
		m_bIsKeyPressed[byKeyID] = false;
		return true;
	}

	return false;
}

_bool CInputDev::Get_DIKeyDown(_ubyte byKeyID)
{
	// ������ ������ ���� ���� ������ ��
	if (!(m_bIsKeyDown[byKeyID]) && (m_byKeyState[byKeyID] & 0x80))
	{
		m_bIsKeyDown[byKeyID] = true;
		return true;
	}
	// ���� ������ ������ ������ ������ ��
	else if (!(m_byKeyState[byKeyID] & 0x80) && (m_bIsKeyDown[byKeyID]))
	{
		m_bIsKeyDown[byKeyID] = false;
		return false;
	}
	return false;
}

_bool CInputDev::Get_DIKeyPressing(_ubyte byKeyID)
{
	if (m_byKeyState[byKeyID] & 0x80)
		return true;

	return false;
}

_bool CInputDev::Get_DIKeyCombined(_ubyte byFirst, _ubyte bySecond)
{
	if (Get_DIKeyDown(bySecond) && (m_byKeyState[byFirst] & 0x80))
		return true;

	return false;
}

void Engine::CInputDev::Free(void)
{
	Engine::Safe_Release(m_pKeyBoard);
	Engine::Safe_Release(m_pMouse);
	Engine::Safe_Release(m_pInputSDK);
}


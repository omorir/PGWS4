#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include<vector>
#include<d3dcompiler.h>
#ifdef  _DEBUG
#include <iostream>
#endif //  _DEBUG

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace std;
using namespace DirectX;

XMFLOAT3 vertices[3];

void DebugOutputFormatString(const char* format, ...)
{
#ifdef  _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif //  _DEBUG
}


LRESULT WindowsProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	HRESULT result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	if (!SUCCEEDED(result)) return;

	debugLayer->EnableDebugLayer();
	debugLayer->Release();
}
#endif //_DEBUG

#ifdef _DEBUG
int main()
{
	//#else �A�N�e�B�u�łȂ��v���v���Z�b�T�@�u���b�N
	//#endif
#ifdef _DEBUG
	EnableDebugLayer();
#endif // _DEBUG

	const unsigned int window_width = 1280;
	const unsigned int window_height = 720;
	int FlameTime = 0;

	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* _cmdList = nullptr;
	ID3D12CommandQueue* _cmdQueue = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;

	// �E�B���h�E�N���X�̐���&�o�^
	WNDCLASSEX w = {};

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowsProcedure; // �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample"); // �A�v���P�[�V�����N���X��(�K���ł悢)
	w.hInstance = GetModuleHandle(nullptr); // �n���h���̎擾

	RegisterClassEx(&w); // �A�v���P�[�V�����N���X(�E�B���h�E�N���X�̎w��� OS �ɓ`����)

	RECT wrc = { 0, 0, window_width, window_height }; // �E�B���h�E�T�C�Y�����߂�

	// �֐����g���ăE�B���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	// �E�B���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName, // �N���X���w��
		_T("DX12 �e�X�g"), // �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW, // �^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT, // �\�� x ���W��OS�ɂ��C��
		CW_USEDEFAULT, // �\�� y ���W��OS�ɂ��C��
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

#ifdef _DEBUG
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif // _DEBUG


	// �A�_�v�^�[�񋓗p
	std::vector <IDXGIAdapter*> adapters;
	// �����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); // �A�_�v�^�[�̐����I�u�W�F�N�g�擾
		std::wstring strDesc = adesc.Description;
		// �T�������A�_�v�^�[�̖��O���m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}

	// Direct3D �f�o�C�X�̏�����
	D3D_FEATURE_LEVEL featureLevel;
	for (auto lv : levels) // �A�_�v�^�[���w��
	{
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = lv;
			break; // �����\�ȃo�[�W���������������烋�[�v��ł��؂�
		}
	}

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	// �^�C���A�E�g�Ȃ�
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// �A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł悢
	cmdQueueDesc.NodeMask = 0;
	//�v���C�I���e�B�͎��Ɏw��Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//�R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	//�L���[����
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));

	HRESULT CreateSwapChainForHwnd(
		IUnknown * pDevice,
		HWND hwnd,
		const DXGI_SWAP_CHAIN_DESC1 * pDesc,
		const DXGI_SWAP_CHAIN_FULLSCREEN_DESC * pFullscreenDesc,
		IDXGIOutput * pRestrictToOutput,
		IDXGISwapChain1 * *ppSwapChain
	);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
	swapchainDesc.Width = window_width;
	swapchainDesc.Height = window_height;
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;
	// �o�b�N�o�b�t�@�[�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
	// �t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// ���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	// �E�B���h�E�̃t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, hwnd,
		&swapchainDesc, nullptr, nullptr,
		(IDXGISwapChain1**)&_swapchain); // �{����QueryInterface �Ȃǂ�p����IDXGISChain4* �ւ̕ϊ��`�F�b�N������

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));

	DXGI_SWAP_CHAIN_DESC swcDesc = {};
	result = _swapchain->GetDesc(&swcDesc);

	std::vector<ID3D12Resource*> _backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx)
	{
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));
		D3D12_CPU_DESCRIPTOR_HANDLE handle
			= rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);

	XMFLOAT3 vertices[] = {
		{-1.0f, -1.0f, 0.0f}, //����
		{-1.0f, +1.0f, 0.0f}, //����
		{+1.0f, -1.0f, 0.0f}, //�E��
	};

	D3D12_HEAP_PROPERTIES heapprop = {};
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices); // ���_��񂪓��邾���̃T�C�Y
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	XMFLOAT3* vertMap = nullptr;
	result = vertBuff->Map(0, nullptr, (void**)&vertMap);
	std::copy(std::begin(vertices), std::end(vertices), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress(); // �o�b�t�@�[�̉��z�A�h���X
	vbView.SizeInBytes = sizeof(vertices); // �S�o�C�g��
	vbView.StrideInBytes = sizeof(vertices[0]); // 1���_������̃o�C�g��

	//UnregisterClass(w.lpszClassName, w.hInstance);

	ID3DBlob* _vsBlob = nullptr;
	ID3DBlob* _psBlob = nullptr;

	ID3DBlob* errorBlob = nullptr;
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vsBlob, &errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}    //�f�o�b�O�E�B���h�E�ւ̃G���[�\��
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &errorBlob);
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("�t�@�C������������܂���");
		}
		else {
			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";
			OutputDebugStringA(errstr.c_str());
		}
		exit(1);
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};

	gpipeline.pRootSignature = nullptr;

	gpipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	while (true)
	{
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {}
		{
			//DirectX����
			//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
			auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

			D3D12_RESOURCE_BARRIER BarrierDesc = {};
			BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
			BarrierDesc.Transition.Subresource = 0;
			BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			_cmdList->ResourceBarrier(1, &BarrierDesc);

			//�����_�[�^�[�Q�b�g���w��
			auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
			rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

			// ��ʃN���A
			FlameTime++;
			float Color = 0;
			if (FlameTime % 2 == 1) {
				Color = 1.0f;
			}
			else {
				Color = 0.0f;
			}

			float clearColor[] = { 1.0f, 1.0f, Color, 1.0f };
			_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

			// �O��ɓ���ւ���
			BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			_cmdList->ResourceBarrier(1, &BarrierDesc);

			// ���߂̃N���[�Y
			_cmdList->Close();

			// �R�}���h���X�g�̎��s
			ID3D12CommandList* cmdlists[] = { _cmdList };
			_cmdQueue->ExecuteCommandLists(1, cmdlists);

			/// �҂�
			_cmdQueue->Signal(_fence, ++_fenceVal);

			if (_fence->GetCompletedValue() != _fenceVal) {
				auto event = CreateEvent(nullptr, false, false, nullptr);
				_fence->SetEventOnCompletion(_fenceVal, event);
				WaitForSingleObject(event, INFINITE);
				CloseHandle(event);
			}

			_cmdAllocator->Reset(); // �L���[�N���A
			_cmdList->Reset(_cmdAllocator, nullptr); // �ĂуR�}���h���X�g�����߂鏀��

			// �t���b�v
			_swapchain->Present(1, 0);
		}

		if (msg.message == WM_QUIT)
		{
			break;
		}
	}

	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}
#endif

//struct XMFLOAT3 
//{
//	float x;
//	float y;
//	float z;
//
//	XMFLOAT3() = default;
//
//	XMFLOAT3(const XMFLOAT3&) = default;
//	XMFLOAT3& operator=(const XMFLOAT3&) = default;
//
//	XMFLOAT3(XMFLOAT3&) = default;
//	XMFLOAT3& operator=(XMFLOAT3&) = default;
//
//	constexpr XMFLOAT3(float _x, float _y, float _z) noexcept : x(_x), y(_y), z(_z) {}
//	explicit XMFLOAT3(_In_reads_(3) const float* pArray) noexcept : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}
//};
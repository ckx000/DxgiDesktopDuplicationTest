#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_6.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")


// ==============================
// Helper
// ==============================

std::wstring LuidToString(const LUID& luid)
{
    std::wstringstream ss;

    ss << L"0x"
        << std::hex
        << std::setw(8)
        << std::setfill(L'0')
        << static_cast<unsigned long>(luid.HighPart)
        << L"-"
        << std::setw(8)
        << std::setfill(L'0')
        << luid.LowPart;

    return ss.str();
}


unsigned long long LuidToUInt64(const LUID& luid)
{
    ULARGE_INTEGER value{};

    value.HighPart =
        static_cast<DWORD>(luid.HighPart);

    value.LowPart =
        luid.LowPart;

    return value.QuadPart;
}


std::wstring HrToString(HRESULT hr)
{
    std::wstringstream ss;

    ss << L"0x"
        << std::hex
        << std::setw(8)
        << std::setfill(L'0')
        << static_cast<unsigned long>(hr);

    return ss.str();
}


// ==============================
// Find DXGI Output by Windows DISPLAY name
// ==============================

IDXGIOutput* FindOutputByDeviceName(
    IDXGIAdapter1* adapter,
    const std::wstring& targetDeviceName,
    UINT& foundIndex)
{
    foundIndex = 0;

    for (UINT i = 0; ; i++)
    {
        IDXGIOutput* output = nullptr;

        HRESULT hr =
            adapter->EnumOutputs(
                i,
                &output);

        if (hr == DXGI_ERROR_NOT_FOUND)
            break;

        if (FAILED(hr))
            break;

        DXGI_OUTPUT_DESC desc{};

        hr = output->GetDesc(&desc);

        if (SUCCEEDED(hr))
        {
            if (targetDeviceName == desc.DeviceName)
            {
                foundIndex = i;
                return output;
            }
        }

        output->Release();
    }

    return nullptr;
}


// ==============================
// One complete DuplicateOutput test
// ==============================

void TestAdapter(
    IDXGIAdapter1* adapter,
    UINT adapterIndex,
    const std::wstring& targetDisplay)
{
    DXGI_ADAPTER_DESC1 adapterDesc{};

    HRESULT hr =
        adapter->GetDesc1(&adapterDesc);

    if (FAILED(hr))
    {
        std::wcout
            << L"  GetDesc1 FAILED: "
            << HrToString(hr)
            << L"\n";

        return;
    }


    std::wcout
        << L"\n==============================\n";

    std::wcout
        << L"TEST Adapter "
        << adapterIndex
        << L" -> "
        << targetDisplay
        << L"\n";

    std::wcout
        << L"==============================\n";


    std::wcout
        << L"GPU Name       : "
        << adapterDesc.Description
        << L"\n";

    std::wcout
        << L"Vendor ID      : 0x"
        << std::hex
        << adapterDesc.VendorId
        << L"\n";

    std::wcout
        << L"Device ID      : 0x"
        << adapterDesc.DeviceId
        << L"\n";

    std::wcout
        << L"Adapter LUID   : "
        << LuidToString(adapterDesc.AdapterLuid)
        << L"\n";

    std::wcout
        << L"LUID uint64    : "
        << std::dec
        << LuidToUInt64(adapterDesc.AdapterLuid)
        << L"\n";


    // ----------------------------
    // Step 1: Find DISPLAY1 on this adapter
    // ----------------------------

    UINT outputIndex = 0;

    IDXGIOutput* output =
        FindOutputByDeviceName(
            adapter,
            targetDisplay,
            outputIndex);

    if (!output)
    {
        std::wcout
            << L"\n[1] Find DXGI Output\n"
            << L"    FAILED\n"
            << L"    This adapter does NOT expose "
            << targetDisplay
            << L" as a DXGI Output.\n";

        return;
    }

    std::wcout
        << L"\n[1] Find DXGI Output\n"
        << L"    SUCCESS\n"
        << L"    Output index : "
        << outputIndex
        << L"\n";


    DXGI_OUTPUT_DESC outputDesc{};

    hr =
        output->GetDesc(&outputDesc);

    if (SUCCEEDED(hr))
    {
        std::wcout
            << L"    DeviceName   : "
            << outputDesc.DeviceName
            << L"\n";

        std::wcout
            << L"    Attached     : "
            << (
                outputDesc.AttachedToDesktop
                ? L"TRUE"
                : L"FALSE"
                )
            << L"\n";

        std::wcout
            << L"    Coordinates  : "
            << outputDesc.DesktopCoordinates.left
            << L", "
            << outputDesc.DesktopCoordinates.top
            << L" -> "
            << outputDesc.DesktopCoordinates.right
            << L", "
            << outputDesc.DesktopCoordinates.bottom
            << L"\n";

        std::wcout
            << L"    HMONITOR     : 0x"
            << std::hex
            << reinterpret_cast<uintptr_t>(
                outputDesc.Monitor)
            << std::dec
            << L"\n";
    }


    // ----------------------------
    // Step 2: Create D3D11 Device on this adapter
    // ----------------------------

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;

    D3D_FEATURE_LEVEL featureLevel{};

    UINT creationFlags =
        D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    hr =
        D3D11CreateDevice(
            adapter,
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            creationFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &device,
            &featureLevel,
            &context);

    std::wcout
        << L"\n[2] D3D11CreateDevice\n";

    if (FAILED(hr))
    {
        std::wcout
            << L"    FAILED\n"
            << L"    HRESULT: "
            << HrToString(hr)
            << L"\n";

        output->Release();

        return;
    }

    std::wcout
        << L"    SUCCESS\n";

    std::wcout
        << L"    Feature Level: 0x"
        << std::hex
        << static_cast<unsigned int>(
            featureLevel)
        << std::dec
        << L"\n";


    // ----------------------------
    // Step 3: Query IDXGIOutput1
    // ----------------------------

    IDXGIOutput1* output1 = nullptr;

    hr =
        output->QueryInterface(
            IID_PPV_ARGS(&output1));

    std::wcout
        << L"\n[3] QueryInterface IDXGIOutput1\n";

    if (FAILED(hr))
    {
        std::wcout
            << L"    FAILED\n"
            << L"    HRESULT: "
            << HrToString(hr)
            << L"\n";

        device->Release();
        context->Release();
        output->Release();

        return;
    }

    std::wcout
        << L"    SUCCESS\n";


    // ----------------------------
    // Step 4: DuplicateOutput
    // ----------------------------

    IDXGIOutputDuplication* duplication =
        nullptr;

    hr =
        output1->DuplicateOutput(
            device,
            &duplication);

    std::wcout
        << L"\n[4] DuplicateOutput\n";

    if (SUCCEEDED(hr))
    {
        std::wcout
            << L"    SUCCESS\n";

        std::wcout
            << L"    HRESULT: "
            << HrToString(hr)
            << L"\n";

        // ----------------------------------------------------
        // Additional information
        // ----------------------------------------------------

        DXGI_OUTDUPL_DESC duplicationDesc{};

        duplication->GetDesc(
            &duplicationDesc);

        std::wcout
            << L"\n    Duplication description:\n";

        std::wcout
            << L"      Width       : "
            << duplicationDesc.ModeDesc.Width
            << L"\n";

        std::wcout
            << L"      Height      : "
            << duplicationDesc.ModeDesc.Height
            << L"\n";

        std::wcout
            << L"      Format      : 0x"
            << std::hex
            << static_cast<unsigned int>(
                duplicationDesc.ModeDesc.Format)
            << std::dec
            << L"\n";

        std::wcout
            << L"      RefreshRate : "
            << duplicationDesc.ModeDesc.RefreshRate.Numerator
            << L"/"
            << duplicationDesc.ModeDesc.RefreshRate.Denominator
            << L"\n";

        std::wcout
            << L"      Rotation    : "
            << static_cast<int>(
                duplicationDesc.Rotation)
            << L"\n";

        std::wcout
            << L"\n*** DXGI DESKTOP DUPLICATION IS WORKING ***\n";
    }
    else
    {
        std::wcout
            << L"    FAILED\n";

        std::wcout
            << L"    HRESULT: "
            << HrToString(hr)
            << L"\n";

        if (hr == DXGI_ERROR_UNSUPPORTED)
        {
            std::wcout
                << L"    Meaning: DXGI_ERROR_UNSUPPORTED\n";
        }
        else if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
        {
            std::wcout
                << L"    Meaning: "
                << L"DXGI_ERROR_NOT_CURRENTLY_AVAILABLE\n";
        }
        else if (hr == DXGI_ERROR_ACCESS_LOST)
        {
            std::wcout
                << L"    Meaning: DXGI_ERROR_ACCESS_LOST\n";
        }
        else if (hr == DXGI_ERROR_SESSION_DISCONNECTED)
        {
            std::wcout
                << L"    Meaning: "
                << L"DXGI_ERROR_SESSION_DISCONNECTED\n";
        }
        else
        {
            std::wcout
                << L"    Meaning: unknown / driver-specific HRESULT\n";
        }
    }


    // ----------------------------
    // Cleanup
    // ----------------------------

    if (duplication)
        duplication->Release();

    output1->Release();

    context->Release();
    device->Release();

    output->Release();
}


// ==============================
// Main
// ==============================

int wmain()
{
    std::wcout
        << L"==============================\n"
        << L"DXGI Desktop Duplication Test Tool\n"
        << L"==============================\n\n";


    const std::wstring targetDisplay =
        L"\\\\.\\DISPLAY1";


    std::wcout
        << L"Target Display: "
        << targetDisplay
        << L"\n\n";


    // ----------------------------
    // Create DXGI Factory
    // ----------------------------

    IDXGIFactory6* factory = nullptr;

    HRESULT hr =
        CreateDXGIFactory1(
            IID_PPV_ARGS(&factory));

    if (FAILED(hr))
    {
        std::wcout
            << L"CreateDXGIFactory1 FAILED\n"
            << L"HRESULT: "
            << HrToString(hr)
            << L"\n";

        return 1;
    }


    // ----------------------------
    // Enumerate adapters
    // ----------------------------

    std::vector<IDXGIAdapter1*> adapters;


    std::wcout
        << L"Enumerating DXGI adapters...\n\n";


    for (UINT i = 0; ; i++)
    {
        IDXGIAdapter1* adapter = nullptr;

        hr =
            factory->EnumAdapters1(
                i,
                &adapter);

        if (hr == DXGI_ERROR_NOT_FOUND)
            break;

        if (FAILED(hr))
        {
            std::wcout
                << L"EnumAdapters1 failed at "
                << i
                << L"\n";

            break;
        }

        DXGI_ADAPTER_DESC1 desc{};

        adapter->GetDesc1(&desc);

        std::wcout
            << L"Adapter "
            << i
            << L": "
            << desc.Description
            << L"\n";

        std::wcout
            << L"  Vendor : 0x"
            << std::hex
            << desc.VendorId
            << L"\n";

        std::wcout
            << L"  Device : 0x"
            << desc.DeviceId
            << L"\n";

        std::wcout
            << L"  LUID   : "
            << std::dec
            << LuidToUInt64(
                desc.AdapterLuid)
            << L"\n";

        std::wcout
            << L"\n";

        adapters.push_back(adapter);
    }


    // ----------------------------
    // Test every adapter
    // ----------------------------

    std::wcout
        << L"\n==============================\n"
        << L"STARTING DUPLICATEOUTPUT TESTS\n"
        << L"==============================\n";


    for (UINT i = 0;
        i < adapters.size();
        i++)
    {
        TestAdapter(
            adapters[i],
            i,
            targetDisplay);
    }


    // ----------------------------
    // Cleanup
    // ----------------------------

    for (auto* adapter : adapters)
    {
        adapter->Release();
    }

    factory->Release();


    std::wcout
        << L"\n==============================\n"
        << L"ALL TESTS COMPLETE\n"
        << L"==============================\n";

    std::wcout
        << L"\nPress ENTER to exit...";

    std::wcin.get();

    return 0;
}
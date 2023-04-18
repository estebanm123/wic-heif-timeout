// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <winrt/Windows.Storage.h>
#include <winrt/windows.foundation.h>
#include <wincodec.h>

#include <iostream>
#include <filesystem>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::HeifWicTimeout::implementation
{
    fire_and_forget LoadFromWIC(const void* pSource, std::wstring const& filename)
    {
        com_ptr<IWICImagingFactory> wicImagingFactory;
        winrt::check_hresult(
            CoCreateInstance(
                CLSID_WICImagingFactory2,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&wicImagingFactory)
            )
        );

        // Create input stream for memory
        //com_ptr<IWICStream> stream;
        //HRESULT hr = wicImagingFactory->CreateStream(stream.put());
        //if (FAILED(hr))
        //    return hr;

        //hr = stream->InitializeFromMemory(static_cast<uint8_t*>(const_cast<void*>(pSource)), static_cast<DWORD>(size));
        //if (FAILED(hr))
        //    return hr;

        //// Initialize WIC
        com_ptr<IWICBitmapDecoder> decoder;
        //hr = wicImagingFactory->CreateDecoderFromStream(stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.put());
        //if (FAILED(hr))
        //    return hr;

        check_hresult(wicImagingFactory->CreateDecoderFromFilename(filename.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, decoder.put()));

        co_await resume_background();

        com_ptr<IWICBitmapFrameDecode> frame;
        check_hresult(decoder->GetFrame(0, frame.put()));

        UINT width;
        UINT height;
        check_hresult(frame->GetSize(&width, &height));

        auto stride = 4 * (width * 32 + 31) / 32;
        auto bufferSize = stride * height;

        BYTE* pBuffer = new BYTE[bufferSize];
        OutputDebugString(L"!~ copyingpixels\n");
        auto hr = frame->CopyPixels(nullptr, stride, bufferSize, pBuffer);

        if (hr == 0x80070102)
        {
            OutputDebugString(L"!~ Timeout error\n");
            DebugBreak();
        }

        check_hresult(hr);
        OutputDebugString(L"!~ copy pixels success\n");
    }

    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    constexpr std::wstring_view HEIC_PATH = L"C:\\Users\\emargaron\\OneDrive - Microsoft\\Pictures\\heics\\! (3) - Copy - Copy - Copy.HEIC";

    void MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        WCHAR path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        PathRemoveFileSpecW(path);
        auto heicPath = std::wstring{ path } + L"\\Assets\\test.HEIC";

        int i = 0;
        while (i < 1000)
        {
            i++;
            LoadFromWIC(nullptr, heicPath.c_str());
        }
    }
}

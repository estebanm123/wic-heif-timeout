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
#include <winrt/Windows.Storage.Streams.h>
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
    fire_and_forget LoadFromWIC(Streams::IBuffer fileBuffer)
    {
        co_await resume_background();
        
        com_ptr<IWICImagingFactory> wicImagingFactory;
        check_hresult(
            CoCreateInstance(
                CLSID_WICImagingFactory2,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(&wicImagingFactory)));
        com_ptr<IWICStream> stream;
        check_hresult(wicImagingFactory->CreateStream(stream.put()));

        check_hresult(stream->InitializeFromMemory(fileBuffer.data(), static_cast<DWORD>(fileBuffer.Length())));

        com_ptr<IWICBitmapDecoder> decoder;
        check_hresult(wicImagingFactory->CreateDecoderFromStream(stream.get(), nullptr, WICDecodeMetadataCacheOnDemand, decoder.put()));

        com_ptr<IWICBitmapFrameDecode> frame;
        check_hresult(decoder->GetFrame(0, frame.put()));

        UINT width;
        UINT height;
        check_hresult(frame->GetSize(&width, &height));

        WICPixelFormatGUID pixelFormat;
        check_hresult(frame->GetPixelFormat(&pixelFormat));

        com_ptr<IWICComponentInfo> componentInfo;
        check_hresult(wicImagingFactory->CreateComponentInfo(pixelFormat, componentInfo.put()));

        com_ptr<IWICPixelFormatInfo> pixelFormatInfo;
        check_hresult(componentInfo->QueryInterface<IWICPixelFormatInfo>(pixelFormatInfo.put()));

        UINT bitsPerPixel;
        check_hresult(pixelFormatInfo->GetBitsPerPixel(&bitsPerPixel));

        // Stride calculation from: https://learn.microsoft.com/en-us/windows/win32/api/wincodec/nf-wincodec-iwicbitmapsource-copypixels
        auto stride = (width * bitsPerPixel + 7) / 8;
        auto bufferSize = stride * height;

        auto pBuffer = std::make_unique<BYTE[]>(bufferSize);

        OutputDebugString(L"!~ CopyPixels start\n");
        auto hr = frame->CopyPixels(nullptr, stride, bufferSize, pBuffer.get());

        if (hr == 0x80070102)
        {
            OutputDebugString(L"!~ Timeout error\n");
            __debugbreak();
        }

        check_hresult(hr);
        OutputDebugString(L"!~ CopyPixels success\n");
    }

    MainWindow::MainWindow()
    {
        InitializeComponent();
    }

    fire_and_forget MainWindow::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        WCHAR exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);
        PathRemoveFileSpecW(exePath);
        auto heicPath = std::wstring{ exePath } + L"\\TestHeics";

        auto folder = co_await StorageFolder::GetFolderFromPathAsync(heicPath);
        auto files = co_await folder.GetFilesAsync();

        //auto fileBuffer = co_await FileIO::ReadBufferAsync(files.GetAt(0));
        //LoadFromWIC(fileBuffer);

        for (auto file : files)
        {
            auto fileBuffer = co_await FileIO::ReadBufferAsync(file);
            LoadFromWIC(fileBuffer);
        }
    }
}

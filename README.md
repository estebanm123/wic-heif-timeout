# wic-heif-timeout

This app demonstrates a timeout issue with IWICBitmapSource::CopyPixels with .heic files. It executes a series of WIC decoding calls on background threads across a folder full of HEIC files. Eventually, the CopyPixels call returns a 0x80070102 timeout error. 

This issue does not repro when LoadFromWic is called synchronously on .heics, no offloading to threadpool. On other formats like .avif, .jpg, .gif, concurrent LoadFromWic calls don't repro the issue either and all complete successfully. There's a TestNonHeics and TestAvifs folder included in the project for testing this.

## Repro instructions
1. Clone this repo.
2. Open HeifWicTimeout.sln in Visual Studio 2022. I recommend restarting Visual Studio after this step.
3. Ensure launch configuration is Debug/x64 and run the app with a debugger attached (I use the button with green checkmark labelled Local Machine).  
4. Click the 'Execute test' button on the app window. If the issue repros, you will hit a breakpoint near the end of LoadFromWIC in MainWindow.cpp, after the message "!~ Timeout error" is logged to the console.

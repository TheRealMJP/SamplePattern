# MSAA Sample Pattern Inspector

This is an older tool that I wrote for visually inspecting the sub-pixel sample patterns used for MSAA rendering in D3D11. For more information, see my [blog post](https://mynameismjp.wordpress.com/2010/07/07/msaa-sample-pattern-detector/). I recently updated it to support programmable sample patterns using Nvidia's driver extension API (NVAPI). This is mostly done as a demonstration of the functionality, since the NVAPI documentation omits a few details on how to use it. The programmable sample point functionality is new for Maxwell 2.0 chipsets (GTX 970, 980, Titan X, etc.), which means the option to use custom sampling points won't be available unless you have a compatible GPU. At minimum you'll need a GPU that supports FEATURE_LEVEL_10_0 to run the app.

# How To Use

Press the Up and Down keys to toggle through the available MSAA sample counts, as well as the available quality levels. If your GPU is FEATURE_LEVEL_10_1 or higher, then the D3D standard multisample patterns will be available as quality levels. To enable using custom sample points, press the 'K' key.

# Build instructions

This is an older sample, which means it requires Visual Studio 2010 and the DirectX June 2010 SDK to be installed in order to compile. The project also depends on NVAPI, which isn't included in the repository due to their licensing terms. You'll need to download it from [Nvidia's website](https://developer.nvidia.com/nvapi), and then unzip it into a folder called 'NVAPI'. At that point, you should be able to open the solution and build normally. If you already have NVAPI located somewhere else on your machine, then you can change the header and lib paths at the top of SamplePattern.cpp.

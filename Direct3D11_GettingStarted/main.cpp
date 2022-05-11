#define WIN32_LEAN_AND_MEAN // Excludes rarely used libraries in Windows.h
#include <Windows.h>     // All of the Win32 stuff.
#include <d3d11_1.h>     // Direct3D library
#include <dxgi1_6.h>     // DirectX Graphics Infrastructure 
#include <d3dcompiler.h> // Shader compiler
#include <DirectXMath.h> // SIMD math library utilizing SSE
#include <wrl.h>

#include <fstream>
#include <memory>
#include <string>

#include "TargaLoader.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

HWND gMainWnd = 0;      // Handle to our window
int gXPos     = 0;      // Window X Position
int gYPos     = 0;      // Window Y Position
int gWidth    = 1024;    // Window width
int gHeight   = 768;    // Window height

TargaImage image;

ComPtr<ID3D11Device>                gpDevice            = nullptr;
ComPtr<ID3D11Device1>               gpDevice1           = nullptr;
ComPtr<ID3D11DeviceContext>         gpImmediateContext  = nullptr;
ComPtr<ID3D11DeviceContext1>        gpImmediateContext1 = nullptr;
ComPtr<IDXGISwapChain1>             gpSwapchain         = nullptr;
ComPtr<ID3D11Texture2D>             gpBackBuffer        = nullptr;
ComPtr<ID3D11RenderTargetView>      gpBackBufferTarget  = nullptr;
ComPtr<ID3D11InputLayout>           gpVertexLayout      = nullptr;
ComPtr<ID3D11Buffer>                gpVertexBuffer      = nullptr;
ComPtr<ID3D11Buffer>                gpIndexBuffer       = nullptr;
ComPtr<ID3D11Texture2D>             gpTexture           = nullptr;
ComPtr<ID3D11ShaderResourceView>    gpTexSRV            = nullptr;
ComPtr<ID3D11SamplerState>          gpSampleSate        = nullptr;
ComPtr<ID3D11VertexShader>          gpVertexShader      = nullptr;
ComPtr<ID3D11PixelShader>           gpPixelShader       = nullptr;
ComPtr<IDXGIAdapter1>               gpAdapter           = nullptr;

D3D_FEATURE_LEVEL selectedFeatureLevel;


struct Vertex
{
    XMFLOAT4 position;
    XMFLOAT4 color;
};

struct TextureVertex
{
    XMFLOAT4 position;
    XMFLOAT2 UV;
};

/*
*              Triangle vertex coordinates (2D)
*                       
*                           x (0.0, 1.0)  
*                         /  \
*                        /    \
*                       /      \
*                      /        \
*                     /          \
*                    /            \
*                   /______________\
*     (-1.0, -1.0) x                x (1.0, -1.0) 
* 
*/
//Vertex triangle[] =
//{
//    // pos(x, y z, 1)   color(r,g,b,a) 
//    { XMFLOAT4( 0.5f, -0.5f, 0.0f, 1.0f ),  XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) }, // Bottom right.
//    { XMFLOAT4( -0.5f, -0.5f, 0.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) }, // Bottom left.
//    { XMFLOAT4( -0.5f, 0.5f, 0.0f, 1.0f ),   XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) }, // Top Left.
//    { XMFLOAT4( 0.5f, 0.5f, 0.0f, 1.0f ),   XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) }, // Top Right.
//};

TextureVertex triangle[] =
{
    // pos(x, y z, 1)   color(r,g,b,a) 
    { XMFLOAT4( 0.5f, -0.5f, 0.0f, 1.0f ),  XMFLOAT2( 4.0f, 4.0f) }, // Bottom right.
    { XMFLOAT4( -0.5f, -0.5f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 4.0f) }, // Bottom left.
    { XMFLOAT4( -0.5f, 0.5f, 0.0f, 1.0f ),   XMFLOAT2( 0.0f, 0.0f) }, // Top Left.
    { XMFLOAT4( 0.5f, 0.5f, 0.0f, 1.0f ),   XMFLOAT2( 4.0f, 0.0f ) }, // Top Right.
};

//TextureVertex triangle[] =
//{
//    // pos(x, y z, 1)   color(r,g,b,a) 
//    { XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f ),  XMFLOAT2( 1.0f, 1.0f ) }, // Bottom right.
//    { XMFLOAT4( -1.0f, -1.0f, 0.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) }, // Bottom left.
//    { XMFLOAT4( -1.0f, 1.0f, 0.0f, 1.0f ),   XMFLOAT2( 0.0f, 0.0f ) }, // Top Left.
//    { XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ),   XMFLOAT2( 1.0f, 0.0f ) }, // Top Right.
//};



unsigned short indices[] =
{
    2, 0, 1,
    0, 2, 3
};

bool InitWindow( HINSTANCE instanceHandle, int show );
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void Run();

bool InitDirect3D();
void Draw();


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd )
{
    OutputDebugStringA ( "Starting up!\n" );
    LoadTargaFromFile( "brickTexture.tga", &image );
    std::shared_ptr<char[]> temp = image.getData();
    char* data = temp.get();
    int width = image.getWidth();
    int height = image.getHeight();
    for (int i = 0; i < width; i++)
    {
        OutputDebugStringA(std::to_string((unsigned char)data[i]).c_str());
        OutputDebugStringA( " " );
        if (i == width - 1)
        {
            OutputDebugStringA( "\n" );
        }
    }

    // Try to initialize the application.
    if ( !InitWindow( hInstance, nShowCmd ) ||
         !InitDirect3D() )
    {
        return 1; // Return Error and exit program.
    }
    
    // Begin executing the event and render loop.
    Run();

	return 0;
}

bool InitWindow( HINSTANCE instanceHandle, int show ) 
{
    OutputDebugStringA( "Opening Window!\n" );
    WNDCLASS wc; // Datastructure that holds the details of the windowclass which describes our window.
    ZeroMemory( &wc, sizeof( WNDCLASS ) ); // Initialize the structure.

    wc.style            = CS_HREDRAW | CS_VREDRAW;                  // Class styling. Allows for additional behaviours of the window.
    wc.lpfnWndProc      = WndProc;                                  // A function pointer to the Window Procedure.
    wc.cbClsExtra       = 0;                                        // Extra bytes to allocate to the window class structure.
    wc.cbWndExtra       = 0;                                        // Extra bytes to allocate to the window instance.
    wc.hInstance        = instanceHandle;                           // The module handle of this application.
    wc.hIcon            = LoadIcon( 0, IDI_APPLICATION );           // Icon of the window.
    wc.hCursor          = LoadCursor( 0, IDC_ARROW );               // Cursor used by the window.
    wc.hbrBackground    = ( HBRUSH ) GetStockObject( WHITE_BRUSH ); // Paints the window white.
    wc.lpszMenuName     = 0;                                        // Name of an associated menu.
    wc.lpszClassName    = L"D3DWindowClass";                        // Name of the window class this structure will become

    // Check if the window class failed to register. If it did the function will be a false value.
    // In that case we will print the failure and the error code associated with it.
    if ( !RegisterClass( &wc ) ) 
    {
        MessageBox( 0, L"RegisterClass FAILED. Error code: " + GetLastError(), 0, 0 );
        return false;
    }

    gMainWnd = CreateWindow( 
        L"D3DWindowClass",          // Which window class do we want to instantiate.
        L"Hello, World",            // Title of our window.                         
        WS_OVERLAPPED | WS_SYSMENU, // Window style. We're specifying a window with a title bar and a thin border                     
        gXPos, gYPos,               // Starting position of the window in pixel coordinates.
        gWidth, gHeight,            // Starting size of the window in pixels.
        0,                          // A handle to the parent.
        0,                          // A handle to a menu
        instanceHandle,             // A handle to the instance of this application.
        0 );                        // Extra creation parameters.

    // Check if the CreateWindow function failed. If it did the window handle will be zero.
    // In that case we will print the failure and the error code associated with it.
    if ( gMainWnd == 0 ) 	
    {
        MessageBox( 0, L"CreateWindow FAILED. Error code: " + GetLastError(), 0, 0 );
        return false;
    }

    // Display the window and update that state.
    ShowWindow( gMainWnd, show );
    UpdateWindow( gMainWnd );

    return true;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch ( msg )
    {
    case WM_LBUTTONDOWN: // Left Mouse click
        MessageBox( 0, L"Hello, World", L"Hello", MB_OK );
        break;
    case WM_KEYDOWN: // keypress (down position)
        if ( wParam == VK_ESCAPE ) // Escape Key
        {
            DestroyWindow( hWnd );
        }
        break;
    case WM_DESTROY: // Window was closed by us or by the user
        DestroyWindow( hWnd );
        PostQuitMessage( 0 ); // Send the quit message
        break;
    default:
        return DefWindowProc( hWnd, msg, wParam, lParam ); // Send messages back to the OS.
        break;
    }

    return 0;
}

void Run()
{
    MSG msg = { 0 };

    while ( msg.message != WM_QUIT )
    {
        // Check if messages are sitting in the queue. 
        if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
        {
            // Translate virtual-key messages into character messages.
            TranslateMessage( &msg );

            // Send the message to the WndProc function.
            DispatchMessage( &msg );

            // If we recieve the quit message then its time to break out of the loop 
            // and end the application.
            if ( msg.message == WM_QUIT )
            {
                break;
            }
        } 
        else
        {
            // Here is where we will put our loop logic for rendering.
            Draw();
        }
    }
}

/*
* This function will utilize DXGI methods to enumerate and evaluate the graphics adapters
* that are present on a system.
*/
void GetOptimalAdapter(ComPtr<IDXGIFactory2> pFactory2)
{
    // Query pFactory2 to get a Factory6 interface that has the functionality that we seek.
    ComPtr<IDXGIFactory6> pFactory6;
    if (SUCCEEDED( pFactory2.As( &pFactory6 ) )) 
    {
        /*
        * Here we will use the DXGI 1.6 to get the adapter that fits our performance flag.
        * The highest performing adapter should be at index 0. We still need to double check
        * that the adapter is not software one though as they're extremely unsuitable for
        * real time rendering. Every system since Windows 8 has the "Microsoft Basic Render
        * Driver" by default so we can't skip this check.
        */
        for ( auto adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != pFactory6->EnumAdapterByGpuPreference(
            adapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS( &gpAdapter ) ); // IID_PPV_ARGS gets the UUID of the adapter.
            adapterIndex++ )
        {
            DXGI_ADAPTER_DESC1 desc;
            gpAdapter->GetDesc1( &desc );

            // Ignore the software adapter.
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }
            break;
        }
    }
    else
    {
        /*
        * If for some reason we don't have DXGI 1.6 available we will use the more "legacy" method
        * of enumerating adapters. This method however doesn't provide us with a method for sorting
        * adapters. As such we would have to manually check the card description to determien if its
        * high performance.
        */
        for ( auto adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != pFactory2->EnumAdapters1( adapterIndex, &gpAdapter );
            adapterIndex++ )
        {
            DXGI_ADAPTER_DESC1 desc;
            gpAdapter->GetDesc1( &desc );

            // Ignore the software adapter
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }
            break;
        }
    }

    DXGI_ADAPTER_DESC1 desc;
    gpAdapter->GetDesc1( &desc );
    const std::wstring description( desc.Description );
    const std::wstring output( L"GPU: " + description + L" Video Memory: " + std::to_wstring( desc.DedicatedVideoMemory / 1024 / 1024 ) + L"MB\n" );
    OutputDebugString( output.c_str() );
}

bool CreateSwapchainAndViewport( ComPtr<IDXGIFactory2> pFactory2 )
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
    memset( &swapchainDesc, 0, sizeof( DXGI_SWAP_CHAIN_DESC1 ) );
    swapchainDesc.Width                 = gWidth;                           // Resolution width.
    swapchainDesc.Height                = gHeight;                          // Resolution height.
    swapchainDesc.Format                = DXGI_FORMAT_R8G8B8A8_UNORM;       // Data format of our display buffers.
    swapchainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // How we want to utilize the buffers.
    swapchainDesc.BufferCount           = 2;                                // How many buffers do we want.
    swapchainDesc.SwapEffect            = DXGI_SWAP_EFFECT_DISCARD;         // How we want to treat a buffer after presentation.
    swapchainDesc.Stereo                = false;                            // If we want to setup these buffers for stereo (3D) display
    swapchainDesc.AlphaMode             = DXGI_ALPHA_MODE_UNSPECIFIED;      // Describes transparancy behaviour
    swapchainDesc.SampleDesc.Count      = 1;                                // Sample count per pixel (Used for MSAA)
    swapchainDesc.SampleDesc.Quality    = 0;                                // Quality level for MSAA

    HRESULT hr;
    if (FAILED( hr = pFactory2->CreateSwapChainForHwnd( gpDevice.Get(), gMainWnd, &swapchainDesc, nullptr, nullptr, &gpSwapchain ) ))
    {
        OutputDebugStringA( "Error: Failed to create DXGI Swapchain! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create swapchain!", L"Error!", MB_OK );
        return false;
    }

    D3D11_VIEWPORT viewport;
    memset( &viewport, 0, sizeof( D3D11_VIEWPORT ) );
    viewport.TopLeftX   = 0.0f;                             // left hand bounds of the viewport.
    viewport.TopLeftY   = 0.0f;                             // top bounds of the view port
    viewport.Width      = static_cast< float >( gWidth );   // width of the viewport
    viewport.Height     = static_cast< float >( gHeight );  // height of the view port

    // Bind the viewport.
    gpImmediateContext->RSSetViewports( 1, &viewport );

    // Get the surface of the backbuffer and then create a render target view for that surface.
    gpSwapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), &gpBackBuffer );

    // Get the resource view for our back buffer.
    if (FAILED( hr = gpDevice->CreateRenderTargetView( gpBackBuffer.Get(), nullptr, &gpBackBufferTarget ) ))
    {
        OutputDebugStringA( "Error: Failed to create create render target! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create render target!", L"Error!", MB_OK );
        return false;
    }

    return true;
}

bool CreateShadersAndLayout()
{
    // We are now going to load our compiled vertex and pixel shaders.
    std::ifstream fileReader( "BasicPixelShader.cso", std::ifstream::binary );

    if (!fileReader.is_open())
    {
        OutputDebugStringA( "Failed to open pixel shader file.\n" );
        MessageBox( 0, L"Failed to open pixel shader file.", L"Error!", MB_OK );
        return false;
    }

    /*
    * Need to determine the file size.So we seek to the end of the fileand then use tellg to see
    * our current position. We then seek back to the beggining of the file to read our data.
    */
    fileReader.seekg( 0, std::ios::end );
    auto length = fileReader.tellg();

    fileReader.seekg( 0, std::ios::beg );

    // Allocate a byte array buffer the size of our file.
    auto buffer = std::make_unique<char[]>( length );
    fileReader.read( buffer.get(), length );

    fileReader.close();

    HRESULT hr; // Create our pixel shader.
    if (FAILED( hr = gpDevice->CreatePixelShader( buffer.get(), length, nullptr, &gpPixelShader ) ))
    {
        OutputDebugStringA( "Error: Failed to create pixel shader! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create pixel shader!", L"Error!", MB_OK );
        return false;
    }

    // Repeat process for the vertex shader
    fileReader.open( "BasicVertexShader.cso", std::ifstream::binary );

    if (!fileReader.is_open())
    {
        OutputDebugStringA( "Error: Failed to open vertex shader file!\n" );
        MessageBox( 0, L"Failed to open vertex shader file.", L"Error!", MB_OK );
        return false;
    }

    fileReader.seekg( 0, std::ios::end );
    length = fileReader.tellg();
    fileReader.seekg( 0, std::ios::beg );

    // Allocate a byte array buffer for the size of our file.
    buffer = std::make_unique<char[]>( length );
    fileReader.read( buffer.get(), length );

    // Create our vertex shader.
    if (FAILED( hr = gpDevice->CreateVertexShader( buffer.get(), length, nullptr, &gpVertexShader ) ))
    {
        OutputDebugStringA( "Error: Failed to create vertex shader! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create vertex shader!", L"Error!", MB_OK );
        return false;
    }

    fileReader.close();

    /*
    * We need to describe how data will enter the pipeline, we do this through input layouts.
    * Here we describe the elements
    */
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    unsigned int numElements = ARRAYSIZE( layout );


    if (FAILED( hr = gpDevice->CreateInputLayout( layout, numElements, buffer.get(),
        length, &gpVertexLayout ) ))
    {
        OutputDebugStringA( "Error: Failed to create input layout! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create input layout!", L"Error!", MB_OK );
        return false;
    }

    return true;
}

bool CreateResources()
{
    D3D11_BUFFER_DESC buffDesc;
    memset( &buffDesc, 0, sizeof( D3D11_BUFFER_DESC ) );

    buffDesc.Usage = D3D11_USAGE_DEFAULT;           // Describes how the buffer is to be read and written.
    buffDesc.ByteWidth = sizeof( TextureVertex ) * 4;      // How big the buffer should be.
    buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;  // How we intend to use this buffer.
    buffDesc.CPUAccessFlags = 0;                    // Bitwise flags for CPU access 
    buffDesc.MiscFlags = 0;                         // See D3D11_RESOURCE_MISC_FLAGS for more info.

    D3D11_SUBRESOURCE_DATA initData;
    memset( &initData, 0, sizeof( D3D11_SUBRESOURCE_DATA ) );
    initData.pSysMem = triangle;                    // Data that we wish to store in the buffer
    initData.SysMemPitch = 0;                       // Used for describing how many bytes wide a texture is. (Not used for buffers)
    initData.SysMemSlicePitch = 0;                  // Used for 3D textures for depth. (Not used for buffers.)

    HRESULT hr;
    if (FAILED( hr = gpDevice->CreateBuffer( &buffDesc, &initData, &gpVertexBuffer ) ))
    {
        OutputDebugStringA( "Error: Failed to create vertex buffer! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create vertex buffer!", L"Error!", MB_OK );
        return false;
    }

    buffDesc.ByteWidth = sizeof( unsigned short ) * 6;
    buffDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices;

    if (FAILED( hr = gpDevice->CreateBuffer( &buffDesc, &initData, &gpIndexBuffer ) ))
    {
        OutputDebugStringA( "Error: Failed to create index buffer! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create index buffer!", L"Error!", MB_OK );
        return false;
    }

    // Create a texture resource
    D3D11_TEXTURE2D_DESC texDesc;
    memset( &texDesc, 0, sizeof( D3D11_TEXTURE2D_DESC ) );
    texDesc.Width = image.getWidth();
    texDesc.Height = image.getHeight();
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.MipLevels = 1;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

    // Create the subresource data to upload to the GPU.
    std::shared_ptr<char[]> imageData = image.getData();
    initData.pSysMem = imageData.get();
    initData.SysMemPitch = sizeof(char) * 4 * image.getWidth();
    if (FAILED( hr = gpDevice->CreateTexture2D( &texDesc, &initData, &gpTexture ) ))
    {
        OutputDebugStringA( "Error: Failed to create texture! Line: " + __LINE__ );
        MessageBox( 0, L"Error: Failed to create texture!", L"Error!", MB_OK );
        return false;
    }

    // Create our shader resource view of our texture
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset( &srvDesc, 0, sizeof( D3D11_SHADER_RESOURCE_VIEW_DESC ) );
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = -1;

    if (FAILED( hr = gpDevice->CreateShaderResourceView( gpTexture.Get(), &srvDesc, &gpTexSRV ) ) )
    {
        OutputDebugStringA( "Error: Failed to create SRV! Line: " + __LINE__ );
        MessageBox( 0, L"Error: Failed to create SRV!", L"Error!", MB_OK );
        return false;
    }

    // Create the sampler state
    D3D11_SAMPLER_DESC samplerDesc;
    memset( &samplerDesc, 0, sizeof( D3D11_SAMPLER_DESC ) );
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[ 0 ] = 0;
    samplerDesc.BorderColor[ 1 ] = 0;
    samplerDesc.BorderColor[ 2 ] = 0;
    samplerDesc.BorderColor[ 3 ] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED( hr = gpDevice->CreateSamplerState( &samplerDesc, &gpSampleSate ) ))
    {
        OutputDebugStringA( "Error: Failed to create Sample State! Line: " + __LINE__ );
        MessageBox( 0, L"Error: Failed to create Sample State!", L"Error!", MB_OK );
        return false;
    }

    return true;
}

bool InitDirect3D()
{
    OutputDebugStringA( "Initializing Direct3D\n" );
    ComPtr<IDXGIFactory1> pFactory1;
    ComPtr<IDXGIFactory2> pFactory2;

    HRESULT hr;

    /*
    * Here we are trying to get DXGIFactory objects. These allow us to easily create swapchains and 
    * enumerate graphics adapters. We can only create up to ID3D11Factory1 but we can use COM 
    * queries to get a new version of the object if its available on the system.
    */
    if ( FAILED( hr = CreateDXGIFactory1( __uuidof( IDXGIFactory1 ), ( void** ) ( &pFactory1 ) ) ) )
    {
        OutputDebugStringA( "Error: Failed to create DXGI factory1! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create DXGI factory1!", L"Error!", MB_OK );
        return false;
    } else
    {
        if ( FAILED( hr = pFactory1.As( &pFactory2 ) ) )
        {
            OutputDebugStringA( "Error: Failed to create DXGI factory2! Line: " + __LINE__ );
            MessageBox( 0, L"Failed to create DXGI factory2!", L"Error!", MB_OK );
            return false;
        }
        pFactory1 = nullptr; // Release Factory1 since we no longer need it
    }

    // Enumerate our adapters and select the highest performace adapter.
    GetOptimalAdapter( pFactory2 );

    D3D_FEATURE_LEVEL featureLevels[] = 
    { 
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    int numFeatureLevels = ARRAYSIZE( featureLevels );

    /*
    * The Direct3D device can have optional flags applied to it. One of these flags enables debugging of it.
    * We're simply checking for the _DEBUG environment define to see if we should apply debugging symbols to
    * the device.
    */
    DWORD createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    /*
    * Attempt to create the D3D11 device with feature level 11_1.Older platforms / hardware may not recognize this
    * feature level so we need to check the HRESULT return from D3D11CreateDevice to check for E_INVALIDARG.
    */
    hr = D3D11CreateDevice( gpAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                            D3D11_SDK_VERSION, &gpDevice, &selectedFeatureLevel, &gpImmediateContext );

/*
* If we enter this control block then the system doesnt support 11_1 so create a device with a feature level of 11_0
* or lower. To do this we pass the address of the second index of our feature level array and decrement our feature
* level count.
*/
    if ( hr == E_INVALIDARG )
    {
        hr = D3D11CreateDevice( gpAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, createDeviceFlags, &featureLevels[ 1 ], numFeatureLevels - 1,
                                D3D11_SDK_VERSION, &gpDevice, &selectedFeatureLevel, &gpImmediateContext );

        OutputDebugStringA( "Info: Feature level 11_1 unavailable!" );
    }

    if ( FAILED( hr ) )
    {
        OutputDebugStringA( "Error: Failed to create Device! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create device! Line: ", L"Error!", MB_OK );
        return false;
    }

    /*
    * Here we're trying to query the com object to get the ID3D11Device1 (assuming feature level 11_1)
    * If we're at 11_1 then the QueryInterface() should return a S_OK value. We can then grab the DeviceContext1.
    */
    if ( SUCCEEDED( gpDevice.As( &gpDevice1 ) ) )
    {
        gpImmediateContext.As( &gpImmediateContext1 );
    }

    if (!CreateSwapchainAndViewport( pFactory2 ) 
        || !CreateShadersAndLayout()
        || !CreateResources())
    {
        return false;
    }
   
    // We will now configure the rendering pipeline by setting the resources, shaders, and states for it. 
    unsigned int stride = sizeof( TextureVertex );
    unsigned int offset = 0;
    gpImmediateContext->OMSetRenderTargets( 1, gpBackBufferTarget.GetAddressOf(), nullptr );
    gpImmediateContext->VSSetShader( gpVertexShader.Get(), nullptr, 0 );
    gpImmediateContext->PSSetShader( gpPixelShader.Get(), nullptr, 0 );
    gpImmediateContext->IASetInputLayout( gpVertexLayout.Get() );
    gpImmediateContext->IASetVertexBuffers( 0, 1, gpVertexBuffer.GetAddressOf(), &stride, &offset );
    gpImmediateContext->IASetIndexBuffer( gpIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    gpImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    gpImmediateContext->PSSetSamplers( 0, 1, gpSampleSate.GetAddressOf() );
    gpImmediateContext->PSSetShaderResources( 0, 1, gpTexSRV.GetAddressOf() );
    //gpImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );

    return true;
}

void Draw()
{
    FLOAT clearColor[] = { 0.0f, 0.0f, 0.2f, 1.0f };
    gpImmediateContext->ClearRenderTargetView( gpBackBufferTarget.Get(), clearColor );
    gpImmediateContext->DrawIndexed(6, 0, 0);
    gpSwapchain->Present( 0, 0 );
}




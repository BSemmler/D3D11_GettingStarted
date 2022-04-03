#define WIN32_LEAN_AND_MEAN // Excludes rarely used libraries in Windows.h
#include <Windows.h>     // All of the Win32 stuff.
#include <d3d11_1.h>     // Direct3D library
#include <dxgi1_6.h>     // DirectX Graphics Infrastructure 
#include <d3dcompiler.h> // Shader compiler
#include <DirectXMath.h> // SIMD math library utilizing SSE
#include <wrl.h>

#include <fstream>
#include <memory>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

HWND gMainWnd = 0;      // Handle to our window
int gXPos     = 0;      // Window X Position
int gYPos     = 0;      // Window Y Position
int gWidth    = 800;    // Window width
int gHeight   = 600;    // Window height

ComPtr<ID3D11Device>            gpDevice            = nullptr;
ComPtr<ID3D11Device1>           gpDevice1           = nullptr;
ComPtr<ID3D11DeviceContext>     gpImmediateContext  = nullptr;
ComPtr<ID3D11DeviceContext1>    gpImmediateContext1 = nullptr;
ComPtr<IDXGISwapChain1>         gpSwapchain         = nullptr;
ComPtr<ID3D11Texture2D>         gpBackBuffer        = nullptr;
ComPtr<ID3D11RenderTargetView>  gpBackBufferTarget  = nullptr;
ComPtr<ID3D11InputLayout>       gpVertexLayout      = nullptr;
ComPtr<ID3D11Buffer>            gpVertexBuffer      = nullptr;
ComPtr<ID3D11VertexShader>      gpVertexShader      = nullptr;
ComPtr<ID3D11PixelShader>       gpPixelShader       = nullptr;
ComPtr<IDXGIAdapter1>           gpAdapter           = nullptr;

D3D_FEATURE_LEVEL selectedFeatureLevel;


struct Vertex
{
    XMFLOAT4 position;
    XMFLOAT4 color;
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
Vertex triangle[] =
{
    // pos(x, y z, 1)   color(r,g,b,a) 
    { XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f ),  XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
    { XMFLOAT4( -1.0f, -1.0f, 0.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
    { XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ),   XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
};

bool InitWindow( HINSTANCE instanceHandle, int show );
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void Run();

bool InitDirect3D();
void Draw();


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd )
{
    OutputDebugStringA ( "Starting up!\n" );
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
    case WM_KEYDOWN: // keypress (down position)
        if ( wParam == VK_ESCAPE ) // Escape Key
        {
            DestroyWindow( hWnd );
        }
        break;
    case WM_DESTROY: // Window was closed by us or by the user
        DestroyWindow( hWnd );
        PostQuitMessage( 0 ); // Send the quit message

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

void GetOptimalAdapter(ComPtr<IDXGIFactory2> pFactory2)
{
    // Enumerate the adapters.
    ComPtr<IDXGIFactory6> pFactory6;
    if (SUCCEEDED( pFactory2.As( &pFactory6 ) ))
    {
        for (auto adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != pFactory6->EnumAdapterByGpuPreference(
            adapterIndex,
            DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
            IID_PPV_ARGS( gpAdapter.ReleaseAndGetAddressOf() ) );
            adapterIndex++)
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
    else
    {
        for (auto adapterIndex = 0;
            DXGI_ERROR_NOT_FOUND != pFactory2->EnumAdapters1(
            adapterIndex,
            gpAdapter.ReleaseAndGetAddressOf() );
            adapterIndex++)
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
}

bool CreateSwapchainAndViewport( ComPtr<IDXGIFactory2> pFactory2 )
{
    DXGI_SWAP_CHAIN_DESC1 swapchainDesc;
    memset( &swapchainDesc, 0, sizeof( DXGI_SWAP_CHAIN_DESC1 ) );
    swapchainDesc.Width = gWidth;
    swapchainDesc.Height = gHeight;
    swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapchainDesc.BufferCount = 2;
    swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapchainDesc.Stereo = false;
    swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapchainDesc.SampleDesc.Count = 1;
    swapchainDesc.SampleDesc.Quality = 0;

    HRESULT hr;
    if (FAILED( hr = pFactory2->CreateSwapChainForHwnd( gpDevice.Get(), gMainWnd, &swapchainDesc, nullptr, nullptr, &gpSwapchain ) ))
    {
        OutputDebugStringA( "Error: Failed to create DXGI Swapchain! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create swapchain!", L"Error!", MB_OK );
        return false;
    }

    D3D11_VIEWPORT viewport;
    memset( &viewport, 0, sizeof( D3D11_VIEWPORT ) );
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(gWidth);
    viewport.Height = ( float )gHeight;

    gpImmediateContext->RSSetViewports( 1, &viewport );

    // Get the surface of the backbuffer and then create a render target view for that surface.

    gpSwapchain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), &gpBackBuffer );

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

    HRESULT hr;
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

    buffer = std::make_unique<char[]>( length );
    fileReader.read( buffer.get(), length );

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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

bool CreateBuffers()
{
    D3D11_BUFFER_DESC buffDesc;
    memset( &buffDesc, 0, sizeof( D3D11_BUFFER_DESC ) );

    buffDesc.Usage = D3D11_USAGE_DEFAULT;
    buffDesc.ByteWidth = sizeof( Vertex ) * 3;
    buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    buffDesc.CPUAccessFlags = 0;
    buffDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = triangle;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    HRESULT hr;
    if (FAILED( hr = gpDevice->CreateBuffer( &buffDesc, &initData, &gpVertexBuffer ) ))
    {
        OutputDebugStringA( "Error: Failed to create vertex buffer! Line: " + __LINE__ );
        MessageBox( 0, L"Failed to create vertex buffer!", L"Error!", MB_OK );
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

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1,
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
        || !CreateBuffers())
    {
        return false;
    }
   
    // We will now configure the rendering pipeline by setting the resources, shaders, and states for it. 
    unsigned int stride = sizeof( Vertex );
    unsigned int offset = 0;
    gpImmediateContext->OMSetRenderTargets( 1, gpBackBufferTarget.GetAddressOf(), nullptr );
    gpImmediateContext->VSSetShader( gpVertexShader.Get(), nullptr, 0 );
    gpImmediateContext->PSSetShader( gpPixelShader.Get(), nullptr, 0 );
    gpImmediateContext->IASetInputLayout( gpVertexLayout.Get() );
    gpImmediateContext->IASetVertexBuffers( 0, 1, gpVertexBuffer.GetAddressOf(), &stride, &offset );
    gpImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    return true;
}

void Draw()
{
    FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    gpImmediateContext->ClearRenderTargetView( gpBackBufferTarget.Get(), clearColor );
    gpImmediateContext->Draw( 3, 0 );
    gpSwapchain->Present( 0, 0 );
}




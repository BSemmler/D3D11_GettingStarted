#define WIN32_LEAN_AND_MEAN // Excludes rarely used libraries in Windows.h
#include <Windows.h>     // All of the Win32 stuff.
#include <d3d11_1.h>     // Direct3D library
#include <dxgi.h>        // DirectX Graphics Infrastructure 
#include <d3dcompiler.h> // Shader compiler
#include <DirectXMath.h> // SIMD math library utilizing SSE
#include <wrl.h>         // Library that has utilities for MS COM objects

HWND gMainWnd = 0;      // Handle to our window
int gXPos     = 0;      // Window X Position
int gYPos     = 0;      // Window Y Position
int gWidth    = 800;    // Window width
int gHeight   = 600;    // Window height

bool InitWindow( HINSTANCE instanceHandle, int show );
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
void Run();


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nShowCmd )
{
    // Try to create the window. If it fails exit the program.
    if ( !InitWindow( hInstance, nShowCmd ) )
    {
        return 1; // Return Error and exit program.
    }
    

    // Begin executing the event and render loop.
    Run();

	return 0;
}

bool InitWindow( HINSTANCE instanceHandle, int show ) 
{
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
        L"Hello, World",            // title of our window.                         
        WS_OVERLAPPED | WS_SYSMENU, // window style. We're specifying a window with a title bar and a thin border                     
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
    case WM_LBUTTONDOWN: // Left Mousclick
        MessageBox( 0, L"Hello, World", L"Hello", MB_OK );
        return 0;
    case WM_KEYDOWN: // keypress (down position)
        if ( wParam == VK_ESCAPE ) // Escape Key
        {
            DestroyWindow( hWnd );
        }
        return 0;
    case WM_DESTROY: // Window was closed by us or by the user
        DestroyWindow( hWnd );
        gMainWnd = 0;
        PostQuitMessage( 0 ); // Send the quit message
        return 0;
    default:
        return DefWindowProc( hWnd, msg, wParam, lParam ); // Send messages back to the OS.
        break;
    }
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
        }
    }
}




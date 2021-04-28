
///////////////**************new**************////////////////////

//Include and link appropriate libraries and headers//
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")


#include <windows.h>
#include <D3D11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <xnamath.h>

//Global Declarations//
IDXGISwapChain* SwapChain;
ID3D11Device* d3d11Device;
ID3D11DeviceContext* d3d11DevCon;
ID3D11RenderTargetView* renderTargetView;

float red = 0.0f;
float green = 0.0f;
float blue = 0.0f;
int colormodr = 1;
int colormodg = 1;
int colormodb = 1;

///////////////**************new**************////////////////////
ID3D11Buffer* triangleVertBuffer;
ID3D11VertexShader* VS;
ID3D11PixelShader* PS;
ID3D10Blob* VS_Buffer;
ID3D10Blob* PS_Buffer;
ID3D11InputLayout* vertLayout;
///////////////**************new**************////////////////////

LPCTSTR WndClassName = "firstwindow";
HWND hwnd = NULL;
HRESULT hr;

const int Width  = 500;
const int Height = 500;



//Function Prototypes//
bool InitializeDirect3d11App(HINSTANCE hInstance);
void ReleaseObjects();
bool InitTriangle();
void UpdateScene();
void DrawScene();
void SetViewPort();
void CompileShaders();



bool InitializeWindow(HINSTANCE hInstance,
    int ShowWnd,
    int width, int height,
    bool windowed);
int messageloop();

LRESULT CALLBACK WndProc(HWND hWnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam);


//Vertex Structure and Vertex Layout (Input Layout)//
struct Vertex    //Overloaded Vertex Structure
{
     Vertex(){}
    Vertex(float x, float y, float z,
        float cr, float cg, float cb, float ca)
        : pos(x,y,z), color(cr, cg, cb, ca){}

    XMFLOAT3 pos;
    XMFLOAT4 color;
};

D3D11_INPUT_ELEMENT_DESC layout[] =
{
   // { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },  


};
UINT numElements = ARRAYSIZE(layout);
///////////////**************new**************////////////////////


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)  // main function 
{

    if(!InitializeWindow(hInstance, nShowCmd, Width, Height, true))   // init the window
    {
        MessageBox(0, "Window Initialization - Failed",
            "Error", MB_OK);
        return 0;
    }



    if(!InitializeDirect3d11App(hInstance))    //Initialize Direct3D
    {
        MessageBox(0, "Direct3D Initialization - Failed",
            "Error", MB_OK);
        return 0;
    }

	CompileShaders();  // compile shaders from the file
	SetViewPort();     // setting of view port

    if(!InitTriangle())    //Initialize our scene
    {
        MessageBox(0, "Scene Initialization - Failed",
            "Error", MB_OK);
        return 0;
    }
    
    messageloop();

    ReleaseObjects();
    


    return 0;
}

bool InitializeWindow(HINSTANCE hInstance, int ShowWnd, int width, int height, bool windowed)
{
    typedef struct _WNDCLASS {
        UINT cbSize;
        UINT style;
        WNDPROC lpfnWndProc;
        int cbClsExtra;
        int cbWndExtra;
        HANDLE hInstance;
        HICON hIcon;
        HCURSOR hCursor;
        HBRUSH hbrBackground;
        LPCTSTR lpszMenuName;
        LPCTSTR lpszClassName;
    } WNDCLASS;

    WNDCLASSEX wc;

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WndClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Error registering class",    
            "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    hwnd = CreateWindowEx(
        NULL,
        WndClassName,
        "Window Title",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        NULL,
        NULL,
        hInstance,
        NULL
        );

    if (!hwnd)
    {
        MessageBox(NULL, "Error creating window",
            "Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, ShowWnd);
    UpdateWindow(hwnd);

    return true;
}



bool InitializeDirect3d11App(HINSTANCE hInstance)
{
    //Describe our Buffer
    DXGI_MODE_DESC bufferDesc;

    ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

    bufferDesc.Width = Width;
    bufferDesc.Height = Height;
    bufferDesc.RefreshRate.Numerator = 60;
    bufferDesc.RefreshRate.Denominator = 1;
    bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;   /// tell how to render on raster as we are using double buffer so remain unspecifed. means order dont matter
    bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;   /// means we dont want to specify. It has 2 more options i.e. centered and streched on screen
    
    //Describe our SwapChain
    DXGI_SWAP_CHAIN_DESC swapChainDesc; 
        
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferDesc = bufferDesc;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;  //it is for multisampling
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;   /// not well understood but is is telling that we use target view
    swapChainDesc.BufferCount = 1;   //number of back buffer
    swapChainDesc.OutputWindow = hwnd; 
    swapChainDesc.Windowed = TRUE; 
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;  // it tells the display driver that what to do with front buffer aftr swaping it to back buffer. here we say let driver decide what is most efficient


    //Create our SwapChain
    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL,
		D3D11_SDK_VERSION, &swapChainDesc, &SwapChain, &d3d11Device, NULL, &d3d11DevCon);

    //Create our BackBuffer
    ID3D11Texture2D* BackBuffer;
    SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (void**)&BackBuffer );

    //Create our Render Target
    d3d11Device->CreateRenderTargetView( BackBuffer, NULL, &renderTargetView );
    BackBuffer->Release();

    //Set our Render Target
    d3d11DevCon->OMSetRenderTargets( 1, &renderTargetView, NULL );  // nom of rendertarget  , binding to rendertargetview  , binding to depthstincile which is not present yet

    return true;
}

void ReleaseObjects()   //to remove the resources
{
    //Release the COM Objects we created
    SwapChain->Release();
    d3d11Device->Release();
    d3d11DevCon->Release();
	 triangleVertBuffer->Release();
    VS->Release();
    PS->Release();
    VS_Buffer->Release();
    PS_Buffer->Release();
    vertLayout->Release();
}

void CompileShaders()
{
	 //Compile Shaders from shader file
    hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "VS", "vs_4_0", 0, 0, 0, &VS_Buffer, 0, 0);
    hr = D3DX11CompileFromFile("Effects.fx", 0, 0, "PS", "ps_4_0", 0, 0, 0, &PS_Buffer, 0, 0);

    //Create the Shader Objects
    hr = d3d11Device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), NULL, &VS);
    hr = d3d11Device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), NULL, &PS);

    //Set Vertex and Pixel Shaders
    d3d11DevCon->VSSetShader(VS, 0, 0);   // this 0 in num of class instances
    d3d11DevCon->PSSetShader(PS, 0, 0);   // "
}
bool InitTriangle()
{
	 

    //Create the vertex buffer
    Vertex v[] =
    {
        Vertex( 0.0f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f ),
        Vertex( 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f ),
        Vertex( -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f ), 
       
       
    };

    D3D11_BUFFER_DESC vertexBufferDesc;
    ZeroMemory( &vertexBufferDesc, sizeof(vertexBufferDesc) );

    vertexBufferDesc.Usage          = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth      = sizeof( Vertex ) * 3;
    vertexBufferDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags      = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData; 

    ZeroMemory( &vertexBufferData, sizeof(vertexBufferData) );
    vertexBufferData.pSysMem = v;

	/*1-> ptr to buffer desc
	2-> address of subresource. 
	3-> is the returned buffer*/
    hr = d3d11Device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &triangleVertBuffer);  /// creating buffer   

    //Set the vertex buffer
    UINT stride = sizeof( Vertex );
    UINT offset = 0;

	//Create the Input Layout

	/*1-> layout
	2-> num of elements
	3->ptr to start of VS
	4->size
	5->returned ptr*/

    d3d11Device->CreateInputLayout( layout, numElements, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), &vertLayout );  // creation of input layout


	// binding  vertexbuffer to input assembeler
	/* 1-> input slot
		2-> num of buffer to bind
		3-> ptr to actual buffer.
		4- size of each vertex
		5-> ptr to start*/
    d3d11DevCon->IASetVertexBuffers( 0, 1, &triangleVertBuffer, &stride, &offset );
	// binding of our layout  to input assembeler
    d3d11DevCon->IASetInputLayout( vertLayout );

    //Setting topology Primitive Topology
    d3d11DevCon->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
 
    return true;
}

void SetViewPort()
{
	//Create the Viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = Width ;
    viewport.Height = Height ;

	 //Set the Viewport
    d3d11DevCon->RSSetViewports(1, &viewport);
}

void UpdateScene()
{
   
}
void DrawScene() // main render
{
	 /// Any thing to be called is between clear render target view and present
    //Clear our backbuffer to the updated color
     float bgColor[4] = {(0.0f, 0.0f, 1.0f, 0.0f)};
    d3d11DevCon->ClearRenderTargetView(renderTargetView, bgColor);

    //Draw the triangle
	/*1-> num of vertex
	2->start of vertex*/
    d3d11DevCon->Draw( 3, 0 );
    //Present the backbuffer to the screen
    SwapChain->Present(0, 0);
}
int  messageloop(){
    MSG msg;
    ZeroMemory(&msg, sizeof(MSG));
    while(true)
    {
        BOOL PeekMessageL( 
            LPMSG lpMsg,
            HWND hWnd,
            UINT wMsgFilterMin,
            UINT wMsgFilterMax,
            UINT wRemoveMsg
            );

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);    
            DispatchMessage(&msg);
        }
        else  // if there no interupt 
		{
			// run game code 
			UpdateScene();
			DrawScene();
        }
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg, WPARAM wParam,LPARAM lParam)
{
    switch( msg )
    {
    case WM_KEYDOWN:
        if( wParam == VK_ESCAPE ){
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
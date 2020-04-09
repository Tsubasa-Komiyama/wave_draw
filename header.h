//-------------------------------------------------------------------------
//	main.cppファイルのヘッダーファイル
//	
//	関数，変数宣言用
//-------------------------------------------------------------------------

//関数宣言
BOOL CALLBACK MainDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	//メインダイアログプロシージャ
BOOL WinInitialize( HINSTANCE hInst, HWND hPaWnd, HMENU chID, char *cWinName, HWND PaintArea, WNDPROC WndProc ,HWND *hDC);//子ウィンドウを生成
HRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );		//子ウィンドウプロシージャ

UINT WINAPI TFunc(LPVOID thParam);												//データ読み込み用スレッド

//変数宣言
#define DEF_APP_NAME	TEXT("Waveform Test")
#define DEF_MUTEX_NAME	DEF_APP_NAME			//ミューテックス名
#define DEF_DATAPERS 61.5	//1秒間に何データ入出力するか
#define DATA_MAX 3010 //データの最大個数
#define N 2 //データを読み込みファイルの1行の文字数

static COLORREF	color1, color2, color3;	//色
static HWND hWnd[2];	//子ウィンドウのハンドル
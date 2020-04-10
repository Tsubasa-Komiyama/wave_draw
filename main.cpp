/*-----------------------------------------------------------------------------
	波形描画プログラム

  　「スタート」ボタンが押されるとピクチャーボックス内にウィンドウを作成し，sin波, 矩形波を描画．
  　「ストップ」ボタンを選択することで波形の描画を停止し、「リスタート」ボタンが押されると再び波形を描画する．

-----------------------------------------------------------------------------*/
#include <windows.h>		//Windows環境
#include <windowsx.h>		//Windows環境
#include <stdio.h>			//入出力用
#include <process.h>		//スレッド用
#include <stdlib.h>

#pragma comment(lib,"winmm.lib")//高精度タイマ

#include "header.h"			//ヘッダーファイル
#include "resource.h"		//リソースファイル

//構造体
typedef struct {
	HWND	hwnd;
	HWND	hEdit;
}SEND_POINTER_STRUCT;

typedef struct {
	double sin;
	double square;
}data;

//======================================
//ここからダイアログバージョン

//メイン関数(ダイアログバージョン)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HANDLE hMutex;

	//多重起動判定
	hMutex = CreateMutex(NULL, TRUE, DEF_MUTEX_NAME);		//ミューテックスオブジェクトの生成
	if(GetLastError() == ERROR_ALREADY_EXISTS){				//2重起動の有無を確認
		MessageBox(NULL, TEXT("既に起動されています．"), NULL, MB_OK|MB_ICONERROR);
		return 0;											//終了
	}

	//ダイアログ起動
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainDlgProc);

	return FALSE;			//終了
}


//メインプロシージャ（ダイアログ）
/********************************

システム画面（.rcファイル）に配置したボタン等が押されたときや，
初期化，終了時にどのような処理を行うかをここに記述する．

********************************/
BOOL CALLBACK MainDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hPict1;		//ウィンドウハンドル（PictureBox）
	static HWND hPict2;
	static HANDLE hThread;
	static UINT thID;
	static SEND_POINTER_STRUCT Sps;

	switch(uMsg){
	case WM_INITDIALOG:		//ダイアログ初期化(exeをダブルクリックした時)


		/*******

		システム画面（.rcファイル）に自分で追加したボタンやPicture Control等
		は必ずHWND型でGetDlgItem関数を用いてハンドルを取得します．

		例：hWnd = GetDlgItem(hDlg, ID);
		hWnd：取得したいハンドル
		hDlg：ダイアログのハンドル．ここは基本的にそのまま
		ID：取得したいハンドルの対象となるボタンやPicture Control等
		    のID．.rcファイルから確認できる．詳細は配布資料を参考

	   ********/

		Sps.hwnd = hDlg;

		return TRUE;

	case WM_COMMAND:		//ボタンが押された時
		/*******
		case WM_COMMAND:では各ボタンが押されたときの処理を記載します．
	   ********/
		switch(LOWORD(wParam)){
		case ID_START:			//スタートボタン
			/***********************
			スタートボタンが押されたときに描画を開始します．
			描画を開始するためにPictureControlのハンドルを取得する.
			***********************/

			hPict1 = GetDlgItem(hDlg, ID_PICTBOX1);
			WinInitialize(NULL, hDlg, (HMENU)110, "SINWAVE", hPict1, WndProc, &hWnd[0]); //初期化
			//WinInitialize関数によって子ウィンドウプロシージャにhPictがhWndとしてセットされました．
			hPict2 = GetDlgItem(hDlg, ID_PICTBOX2);
			WinInitialize(NULL, hDlg, (HMENU)110, "SQUAREWAVE", hPict2, WndProc, &hWnd[1]); //初期化
			//WinInitialize関数によって子ウィンドウプロシージャにhPictがhWndとしてセットされました．

			//スレッド起動
			hThread = (HANDLE)_beginthreadex(NULL, 0, TFunc, (PVOID)&Sps, 0, &thID);   //_beginthreadex→スレッドを立ち上げる関数
			EnableWindow(GetDlgItem(hDlg, ID_START), FALSE);						//スタートボタン無効化　　　　//EnableWindowで入力を無効または有効にする。


			return TRUE;

		case ID_STOP:		//ストップボタン
							/*　サスペンドカウンタ　**************************
						　　　実行を許可するまでスレッドを動かさない。
						   　　ResumeThread：　サスペンドカウンタを1減らす
							 　SuspendThread：　サスペンドカウンタを1増やす

							  0のときは実行。それ以外は待機する。
							  **************************************************/


			if (ResumeThread(hThread) == 0) {					//停止中かを調べる(サスペンドカウントを１減らす)
				SetDlgItemText(hDlg, ID_STOP, TEXT("リスタート"));	//リスタートに変更　　　　　　　　　　　　　　　　　　　//SetDlgItemTextでダイアログ内のテキストなどを変更することができる
				SuspendThread(hThread);						//スレッドの実行を停止(サスペンドカウントを１増やす)
			}
			else
				SetDlgItemText(hDlg, ID_STOP, TEXT("ストップ"));	//ストップに変更

			return TRUE;

		}
		break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);			//ダイアログ終了
        return TRUE;
    }

	//オーナー描画後に再描画
	if (uMsg==WM_PAINT){
		InvalidateRect(hWnd[0], NULL, TRUE );	//再描画
		InvalidateRect(hWnd[1], NULL, TRUE);
	}

  return FALSE;
}

/********************************

ここでPictureControlの描画を行います．

子ウィンドウプロシージャ中のhWndはPictureControlのハンドルです．

********************************/
//子ウィンドウプロシージャ
HRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	HDC			hdc;				//デバイスコンテキストのハンドル
	PAINTSTRUCT ps;					//(構造体)クライアント領域描画するための情報
	RECT rect;
	HBRUSH		hBrush, hOldBrush ;	//ブラシ
	HPEN		hPen, hOldPen;		//ペン
	int width, height;

	if (WM_PAINT == uMsg) {

		color1 = RGB(255, 255, 255);	//色指定（白）
		color2 = RGB(0, 0, 0);	//色指定（黒）
		color3 = RGB(0, 0, 255);	//色指定（青）

		/********************************

		PictureControlに描画するためには，HDC型のハンドルを別に取得する
		必要があります．

		例：hdc = BeginPaint(hWnd, &ps);
		hdc:デバイスコンテキストのハンドル
		hWnd:PictureControlのハンドル
		ps：(構造体)クライアント領域描画するための情報

		********************************/

		hdc = BeginPaint(hWnd, &ps);//デバイスコンテキストのハンドル取得

		/********************************

		PictureControlに描画するためには，線を引きたいときはペン，
		塗りつぶす際にはブラシが必要です．

		********************************/

		//ペン，ブラシ生成
		hBrush = CreateSolidBrush(color2);				//ブラシ生成
		hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);	//ブラシ設定
		hPen = CreatePen(PS_SOLID, 2, color1);		//ペン生成
		hOldPen = (HPEN)SelectObject(hdc, hPen);		//ペン設定

		//描画
		/********************************

		図形を描画するためには以下の関数を用います．
		長方形：Rectangle(HDC hdc ,int nLeftRect , int nTopRect ,int nRightRect , int nBottomRect);
		円：Ellipse(HDC hdc ,int nLeftRect , int nTopRect ,int nRightRect , int nBottomRect);

		 nLiftRect：長方形の左上X座標
		  nTopRect：左上Y座標
		  nRightRect：右下X座標
		  nBottomRect：右下のY座標

		線を引くには以下の関数を用います．

		線の始点設定：MoveToEx(HDC hdc , int X , int Y , NULL);
		  X,Y：線の始点の座標
		線；LineTo(HDC hdc , int nXEnd , int nYEnd);
		  nXEnd, nYEnd：線の終点の設定

		********************************/

		//クライアントサイズの取得
		GetClientRect(hWnd, &rect);
		width = rect.left + rect.right;		//幅
		height = rect.top + rect.bottom;	//高さ

		//背景を黒にする
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

		//縦軸
		MoveToEx(hdc, (int)(width * 0.1), 0, NULL);		  //線の始点を決定
		LineTo(hdc, (int)(width * 0.1), height);				    //線の終点を決定し、線を描画
		//横軸
		MoveToEx(hdc, (int)(width * 0.1), (int)(height * 0.5), NULL);		  //線の始点を決定
		LineTo(hdc, width, (int)(height * 0.5));				    //線の終点を決定し、線を描画

		//横軸のラベル
		SetBkColor(hdc, color2);
		SetTextColor(hdc, color1);
		TextOut(hdc, width * 0.45, height * 0.8, TEXT("Time [s]"), 8);		//テキスト描画

		//ペン，ブラシ廃棄
		/********************************

		使い終わったペンとブラシは破棄する必要があります．

		********************************/
		SelectObject(hdc, hOldBrush);
		DeleteObject(hBrush);
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);

		//デバイスコンテキストのハンドル破棄
		EndPaint(hWnd, &ps);
	}	

	return TRUE;
}



//-----------------------------------------------------------------------------
//子ウィンドウ初期化＆生成
//指定したウィンドウハンドルの領域に子ウィンドウを生成する．
//----------------------------------------------------------
// hInst	: 生成用インスタンスハンドル
// hPaWnd	: 親ウィンドウのハンドル
// chID		: 子ウィンドウのID
// cWinName	: 子ウィンドウ名
// PaintArea: 子ウィンドウを生成する領域のデバイスハンドル
// WndProc	: ウィンドウプロシージャ
// *hWnd	: 子ウィンドウのハンドル（ポインタ）
// 戻り値	: 成功時=true
//-----------------------------------------------------------------------------
BOOL WinInitialize( HINSTANCE hInst, HWND hPaWnd, HMENU chID,char *cWinName, HWND PaintArea, WNDPROC WndProc ,HWND *hWnd)
{
	WNDCLASS wc;			//ウィンドウクラス
	WINDOWPLACEMENT	wplace;	//子ウィンドウ生成領域計算用（画面上のウィンドウの配置情報を格納する構造体）
	RECT WinRect;			//子ウィンドウ生成領域
	ATOM atom;				//アトム

	//ウィンドウクラス初期化
	wc.style		=CS_HREDRAW ^ WS_MAXIMIZEBOX | CS_VREDRAW;	//ウインドウスタイル
	wc.lpfnWndProc	=WndProc;									//ウインドウのメッセージを処理するコールバック関数へのポインタ
	wc.cbClsExtra	=0;											//
	wc.cbWndExtra	=0;
	wc.hCursor		=NULL;										//プログラムのハンドル
	wc.hIcon		=NULL;										//アイコンのハンドル
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);		//ウインドウ背景色
	wc.hInstance	=hInst;										//ウインドウプロシージャがあるインスタンスハンドル
	wc.lpszMenuName	=NULL;										//メニュー名
	wc.lpszClassName=(LPCTSTR)cWinName;									//ウインドウクラス名

	if(!(atom=RegisterClass(&wc))){
		MessageBox(hPaWnd,TEXT("ウィンドウクラスの生成に失敗しました．"),NULL,MB_OK|MB_ICONERROR);
		return false;
	}

	GetWindowPlacement(PaintArea,&wplace);	//描画領域ハンドルの情報を取得(ウィンドウの表示状態を取得)
	WinRect=wplace.rcNormalPosition;		//描画領域の設定

	//ウィンドウ生成
	*hWnd=CreateWindow(
		(LPCTSTR)atom,
		(LPCTSTR)cWinName,
		WS_CHILD | WS_VISIBLE,//| WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME |WS_VISIBLE,
		WinRect.left,WinRect.top,
		WinRect.right-WinRect.left,WinRect.bottom-WinRect.top,
		hPaWnd,chID,hInst,NULL
	);

	if( *hWnd==NULL ){
		MessageBox(hPaWnd,TEXT("ウィンドウの生成に失敗しました．"),NULL,MB_OK|MB_ICONERROR);
		return false;
	}

	return true;
}

//データ読み込み用スレッド
UINT WINAPI TFunc(LPVOID thParam)
{
	static SEND_POINTER_STRUCT* FU = (SEND_POINTER_STRUCT*)thParam;        //構造体のポインタ取得

	FILE* fp;			//ファイルポインタ
	BOOL Flag = TRUE;		//ループフラグ
	RECT rect[2];
	HDC			hdc[2];				//デバイスコンテキストのハンドル
	PAINTSTRUCT ps;					//(構造体)クライアント領域描画するための情報
	HBRUSH		hBrush, hOldBrush[2];	//ブラシ
	HPEN		hPen1,hPen2, hOldPen[2];		//ペン

	double data[2];	//データ
	double data_plot[2];		//ウィンドウサイズに合わせたデータ
	int width[2], height[2];	//ウィンドウのサイズ
	int pos_x[2], pos_y[2];	//1週前のプロット点の位置

	DWORD DNum = 0, beforeTime;

	//現在の時刻計算（初期時間）
	beforeTime = timeGetTime();

	//色の指定
	color1 = RGB(255, 255, 255);	//色指定（白）
	color2 = RGB(0, 0, 0);	//色指定（黒）
	color3 = RGB(0, 0, 255);	//色指定（青）

	//ペン・ブラシ生成
	hBrush = CreateSolidBrush(color2);				//ブラシ生成
	hPen1 = CreatePen(PS_SOLID, 2, color1);		//ペン生成
	hPen2 = CreatePen(PS_SOLID, 2, color3);
	for (int i = 0; i < 2; i++) {


	//デバイスコンテキストのハンドル取得
		hdc[i] = BeginPaint(hWnd[i], &ps);

		//クライアントサイズの取得
		GetClientRect(hWnd[i], &rect[i]);
		width[i] = rect[i].right;	//幅
		height[i] = rect[i].bottom;	//高さ

		//位置の初期化
		pos_x[i] = width[i] * 0.1;
		pos_y[i] = height[i] * 0.5;
	}

	for (int i = 0; i < 2; i++) {
		//ペン設定
		hOldBrush[i] = (HBRUSH)SelectObject(hdc[i], hBrush);	//ブラシ設定
		hOldPen[i] = (HPEN)SelectObject(hdc[i], hPen1);

		//背景を黒にする
		Rectangle(hdc[i], 0, 0, width[i] + 1, height[i] + 1);

		//縦軸
		MoveToEx(hdc[i], (int)(width[i] * 0.1), 0, NULL);		  //線の始点を決定
		LineTo(hdc[i], (int)(width[i] * 0.1), height[i]);				    //線の終点を決定し、線を描画

		//横軸
		MoveToEx(hdc[i], (int)(width[i] * 0.1), (int)(height[i] * 0.5), NULL);
		LineTo(hdc[i], width[i], (int)(height[i] * 0.5));

		//横軸のラベル
		SetBkColor(hdc[i], color2);
		SetTextColor(hdc[i], color1);
		TextOut(hdc[i], (int)(width[i] * 0.45), (int)(height[i] * 0.8), TEXT("Time [s]"), 8);		//テキスト描画

	}

	//ファイルオープン
	if ((fp = fopen("data.txt", "r")) == NULL) {
		MessageBox(NULL, TEXT("Input File Open ERROR!"), NULL, MB_OK | MB_ICONERROR);
		return FALSE;
	}

	//データ読み込み・表示
	while (Flag == TRUE) {
		DWORD nowTime, progress, idealTime;

		//時間の調整
		nowTime = timeGetTime();					//現在の時刻計算
		progress = nowTime - beforeTime;				//処理時間を計算
		idealTime = (DWORD)(DNum * (1000.F / (double)DEF_DATAPERS));	//理想時間を計算
		if (idealTime > progress) {
			Sleep(idealTime - progress);			//理想時間になるまで待機
		}
		//データの読み込み
		if (fscanf(fp, "%lf %lf", &data[0], &data[1]) == EOF) {
			MessageBox(NULL, TEXT("終了"), TEXT("INFORMATION"), MB_OK | MB_ICONEXCLAMATION);
			EnableWindow(GetDlgItem(FU->hwnd, ID_START), TRUE);		//スタートボタン有効
			Flag = FALSE;												//ループ終了フラグ
			return FALSE;
		}



		//ファイルから読み取ったデータをウィンドウに合うよう調整
		for (int i = 0; i < 2; i++){
			data_plot[i] = (data[i] * height[i] / 2  * 0.9) + height[i] / 2;
		}

		//波形の描画
		if (pos_x[0] + 1 >= width[0]) {
			for (int i = 0; i < 2; i++) {
				//ペン設定
				hOldBrush[i] = (HBRUSH)SelectObject(hdc[i], hBrush);	//ブラシ設定
				hOldPen[i] = (HPEN)SelectObject(hdc[i], hPen1);

				//背景を黒にする
				Rectangle(hdc[i], 0, 0, width[i] + 1, height[i] + 1);

				//縦軸
				MoveToEx(hdc[i], (int)(width[i] * 0.1), 0, NULL);		  //線の始点を決定
				LineTo(hdc[i], (int)(width[i] * 0.1), height[i]);				    //線の終点を決定し、線を描画

				//横軸
				MoveToEx(hdc[i], (int)(width[i] * 0.1), (int)(height[i] * 0.5), NULL);
				LineTo(hdc[i], width[i], (int)(height[i] * 0.5));

				//横軸のラベル
				SetBkColor(hdc[i], color2);
				SetTextColor(hdc[i], color1);
				TextOut(hdc[i], (int)(width[i] * 0.45), (int)(height[i] * 0.8), TEXT("Time [s]"), 8);		//テキスト描画

				pos_x[i] = width[i] * 0.1;
				pos_y[i] = data_plot[i];
			}
		}
		else {
			for (int i = 0; i < 2; i++) {
				//ペン設定
				hOldPen[i] = (HPEN)SelectObject(hdc[i], hPen2);

				//プロット
				MoveToEx(hdc[i], pos_x[i], pos_y[i], NULL);
				LineTo(hdc[i], pos_x[i], data_plot[i]);

				//位置の更新
				pos_x[i] += 1;
				pos_y[i] = (int)(data_plot[i]);

			}
		}



		DNum++;

		//一秒経過時
		if (progress >= 1000.0) {
			beforeTime = nowTime;
			DNum = 0;
		}
	}

	//ペン，ブラシ廃棄
	SelectObject(hdc[0], hOldBrush);
	DeleteObject(hBrush);
	SelectObject(hdc[1], hOldBrush);
	DeleteObject(hBrush);
	SelectObject(hdc[0], hOldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);
	SelectObject(hdc[1], hOldPen);
	DeleteObject(hPen1);
	DeleteObject(hPen2);


	return 0;
}

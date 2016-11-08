
// MFC_LoginView.cpp : CMFC_LoginView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFC_Login.h"
#endif

#include "MFC_LoginDoc.h"
#include "MFC_LoginView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMFC_LoginView

IMPLEMENT_DYNCREATE(CMFC_LoginView, CView)

BEGIN_MESSAGE_MAP(CMFC_LoginView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFC_LoginView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFC_LoginView 构造/析构

CMFC_LoginView::CMFC_LoginView()
{
	// TODO: 在此处添加构造代码

}

CMFC_LoginView::~CMFC_LoginView()
{
}

BOOL CMFC_LoginView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFC_LoginView 绘制

void CMFC_LoginView::OnDraw(CDC* /*pDC*/)
{
	CMFC_LoginDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CMFC_LoginView 打印


void CMFC_LoginView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFC_LoginView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFC_LoginView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMFC_LoginView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CMFC_LoginView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CMFC_LoginView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CMFC_LoginView 诊断

#ifdef _DEBUG
void CMFC_LoginView::AssertValid() const
{
	CView::AssertValid();
}

void CMFC_LoginView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFC_LoginDoc* CMFC_LoginView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFC_LoginDoc)));
	return (CMFC_LoginDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFC_LoginView 消息处理程序

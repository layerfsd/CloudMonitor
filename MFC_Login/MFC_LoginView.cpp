
// MFC_LoginView.cpp : CMFC_LoginView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
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
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CMFC_LoginView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CMFC_LoginView ����/����

CMFC_LoginView::CMFC_LoginView()
{
	// TODO: �ڴ˴���ӹ������

}

CMFC_LoginView::~CMFC_LoginView()
{
}

BOOL CMFC_LoginView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CMFC_LoginView ����

void CMFC_LoginView::OnDraw(CDC* /*pDC*/)
{
	CMFC_LoginDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CMFC_LoginView ��ӡ


void CMFC_LoginView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CMFC_LoginView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CMFC_LoginView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CMFC_LoginView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
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


// CMFC_LoginView ���

#ifdef _DEBUG
void CMFC_LoginView::AssertValid() const
{
	CView::AssertValid();
}

void CMFC_LoginView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFC_LoginDoc* CMFC_LoginView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFC_LoginDoc)));
	return (CMFC_LoginDoc*)m_pDocument;
}
#endif //_DEBUG


// CMFC_LoginView ��Ϣ�������

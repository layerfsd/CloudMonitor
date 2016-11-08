
// MFC_LoginView.h : CMFC_LoginView ��Ľӿ�
//

#pragma once


class CMFC_LoginView : public CView
{
protected: // �������л�����
	CMFC_LoginView();
	DECLARE_DYNCREATE(CMFC_LoginView)

// ����
public:
	CMFC_LoginDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CMFC_LoginView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // MFC_LoginView.cpp �еĵ��԰汾
inline CMFC_LoginDoc* CMFC_LoginView::GetDocument() const
   { return reinterpret_cast<CMFC_LoginDoc*>(m_pDocument); }
#endif


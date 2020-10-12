
// jsonTestDlg.h: 头文件
//

#pragma once


// CjsonTestDlg 对话框
class CjsonTestDlg : public CDialogEx
{
// 构造
public:
	CjsonTestDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_JSONTEST_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedreadfromstream();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedreadfromstring();
    afx_msg void OnBnClickedstreamwrite();
    afx_msg void OnBnClickedstringwrite();
};

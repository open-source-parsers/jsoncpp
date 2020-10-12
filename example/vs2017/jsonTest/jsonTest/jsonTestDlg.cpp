
// jsonTestDlg.cpp: 实现文件
//

#include "framework.h"
#include "jsonTest.h"
#include "jsonTestDlg.h"
#include "afxdialogex.h"
#include "../../../../include/json/json.h"
#include <fstream>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CjsonTestDlg 对话框



CjsonTestDlg::CjsonTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_JSONTEST_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CjsonTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CjsonTestDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
        ON_BN_CLICKED(IDC_readFromStream,
                      &CjsonTestDlg::OnBnClickedreadfromstream)
    ON_BN_CLICKED(IDC_readFromString, &CjsonTestDlg::OnBnClickedreadfromstring)
    ON_BN_CLICKED(IDC_streamWrite, &CjsonTestDlg::OnBnClickedstreamwrite)
    ON_BN_CLICKED(IDC_stringWrite, &CjsonTestDlg::OnBnClickedstringwrite)
END_MESSAGE_MAP()


// CjsonTestDlg 消息处理程序

BOOL CjsonTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CjsonTestDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CjsonTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CjsonTestDlg::OnBnClickedreadfromstream() 
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    Json::String errs;
    std::ifstream ifs;

    CFileDialog dlg(1);
    if (dlg.DoModal() == IDOK)
    {
        ifs.open(dlg.GetPathName());
        try
        {
            if (!parseFromStream(builder, ifs, &root, &errs))
            {
                MessageBox(errs.c_str());
                return;
            }

            CString strTxt;
            switch (root.type())
            {
            case Json::nullValue:
                strTxt = "null";
                break;
            case Json::intValue:
                strTxt.Format("intValue = %lld", root.asInt64());
                break;
            case Json::uintValue:
                strTxt.Format("uintValue = %llu", root.asUInt64());
                break;
            case Json::realValue:
                strTxt.Format("realValue = %f", root.asDouble());
                break;
            case Json::stringValue:
                strTxt.Format("stringValue = %s", root.asCString());
                break;
            case Json::booleanValue:
                strTxt.Format("booleanValue = %s", root.asBool() ? "True" : "False");
                break;
            case Json::arrayValue:
            {
                strTxt = "It is a arrayValue, types are:\n";
                for (Json::Value::iterator it = root.begin(); it != root.end(); it++)
                {
                    Json::ValueType vvt = it->type();
                    const CString cts[] = { "null","int","uint","real","string","boolean","array","object" };
                    strTxt += cts[vvt] + " = ...\n";
                }
            }
            break;
            case Json::objectValue:
            {
                strTxt = "It is a objectValue, objects are:\n";
                Json::Value::Members mems = root.getMemberNames();
                for (Json::Value::Members::iterator it = mems.begin(); it != mems.end(); it++)
                {
                    strTxt += it->c_str();
                    strTxt += " = ...\n";
                }
            }
            break;
            default:
                strTxt = root.toStyledString().c_str();
                break;
            }
            MessageBox(strTxt);
        }
        catch (Json::Exception e)
        {
            MessageBox(e.what());
        }
        ifs.close();
    }
}


void CjsonTestDlg::OnBnClickedreadfromstring()
{
    const std::string rawJson = R"({"Age": 20, "Name": "colin"})";

    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJson.length(), &root, &err)) 
    {
        MessageBox(err.c_str());
        return;
    }

    std::string name = root["Name"].asString();
    int age = root["Age"].asInt();

    CString strTxt;
    strTxt.Format("%s is %d.", name.c_str(), age);
    MessageBox(strTxt);
}


void CjsonTestDlg::OnBnClickedstreamwrite()
{
    Json::Value root, lang, mail;
    Json::StreamWriterBuilder builder;
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ostringstream os;

    root["Name"] = "Gao zilai";
    root["Age"] = 36;

    lang[0] = "C";
    lang[1] = "C++";
    lang[2] = "Java";
    lang[3] = "Python";
    lang[4] = "Visual C++";
    root["Language"] = lang;

    mail["Netease"] = "utrust@163.com";
    mail["Hotmail"] = "gao_zilao@hotmail.com";
    root["E-mail"] = mail;

    writer->write(root, &os);
    TRACE(os.str().c_str());
    MessageBox(os.str().c_str());
}


void CjsonTestDlg::OnBnClickedstringwrite()
{
    Json::Value root, lang, mail;
    Json::StreamWriterBuilder builder;

    root["Name"] = "Gao zilai";
    root["Age"] = 36;

    lang[0] = "C";
    lang[1] = "C++";
    lang[2] = "Java";
    lang[3] = "Python";
    lang[4] = "Visual C++";
    root["Language"] = lang;

    mail["Netease"] = "utrust@163.com";
    mail["Tencent"] = "121997204@qq.com";
    root["E-mail"] = mail;

    Json::String str = Json::writeString(builder, root);
    MessageBox(str.c_str());
}

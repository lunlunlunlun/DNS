#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <QFile>
#include <QMessageBox>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE 100

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("IP查询程序");
    this->setWindowIcon(QIcon(":///15.jpg"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

#pragma pack(push, 1)
//报文头部结构体
struct DNSHeader
{
    /* 1. 会话标识（2字节）*/
    unsigned short usTransID;        // Transaction ID

    /* 2. 标志（共2字节）*/
    unsigned char RD : 1;            // 表示期望递归，1bit
    unsigned char TC : 1;            // 表示可截断的，1bit
    unsigned char AA : 1;            // 表示授权回答，1bit
    unsigned char opcode : 4;        // 0表示标准查询，1表示反向查询，2表示服务器状态请求，4bit
    unsigned char QR : 1;            // 查询/响应标志位，0为查询，1为响应，1bit

    unsigned char rcode : 4;         // 表示返回码，4bit
    unsigned char zero : 3;          // 必须为0，3bit
    unsigned char RA : 1;            // 表示可用递归，1bit

    /* 3. 数量字段（共8字节） */
    unsigned short Questions;        // 问题数
    unsigned short AnswerRRs;        // 回答资源记录数
    unsigned short AuthorityRRs;     // 授权资源记录数
    unsigned short AdditionalRRs;    // 附加资源记录数
};
#pragma pack(pop)
//使结构体按1字节方式对齐


int num;//域名段数
unsigned short usID;//报文编号
const char* szDomainName;//查询域名
bool fla;

//判断域名合法性
// @Retrun: true表示合法，false表示不合法
bool MainWindow::is_valid_DN()
{
    bool flag=false;

    int DnLen = strlen(szDomainName);

    //域名长度合法性
    if (szDomainName == NULL||DnLen>255||szDomainName[DnLen-1]=='.')
    {
        return flag;
    }
    int i,tempnum;
    //起始字符必须为字母
    if ((szDomainName[0] >= 'a' && szDomainName[0] <= 'z') || (szDomainName[0] >= 'A' && szDomainName[0] <= 'Z'))
        ;
    else
        return flag;

    for (i = 0,tempnum=0; i < DnLen; i++)
    {
        if (szDomainName[i] == '.')
        {
            //连续两个点号，非法
            if (tempnum == 0)
                return flag;
            tempnum = 0;
        }
        else if ((szDomainName[i] >= '0' && szDomainName[i] <= '9')
            || (szDomainName[i] >= 'a' && szDomainName[i] <= 'z')
            || (szDomainName[i] >= 'A' && szDomainName[i] <= 'Z')                                        //不属于字母数字
            || (szDomainName[i] == '-' && i < DnLen && szDomainName[i + 1] != '.' && tempnum != 0))      //-前后符合条件
        {
            tempnum++;
            if (tempnum > 63)
                return flag;
        }
        else
            return flag;
    }

    return true;
}

//组装并发送报文
// @Retrun: true表示发送成功，false表示发送失败
int MainWindow::SendDomainName(IN SOCKET* sock)
{
    fla = false;
    ui->textEdit->clear();
    ui->textEdit_2->clear();
    num=0;

    //接收输入
    QString s=ui->lineEdit->text();
    std::string str = s.toStdString();
    szDomainName = str.c_str();

    // 随机生成一个ID
    srand((unsigned int)time(NULL));
    usID = (unsigned short)rand();

    int an;
    //采用迭代查询
    if(ui->radioButton->isChecked())
    {
        an=0;

    }
    //采用复合查询
    else if(ui->radioButton_2->isChecked())
    {
        an=1;
    }
    bool bRet = false;

    unsigned int uiDnLen = strlen(szDomainName);
    // 判断域名合法性
    if (!is_valid_DN())
    {
        QMessageBox::warning(this,"waring",tr("非法域名!"),QMessageBox::Yes);
        exit(0);
    }

    // 将域名转换为符合查询报文的格式
    // 查询报文的格式：每级域名前面是长度
    //  3 www 5 baidu 3 com 0
    unsigned int uiQueryNameLen = 0;
    // 转换后的查询字段长度为域名长度 +2
    BYTE* pbQueryDomainName = (BYTE*)malloc(uiDnLen + 1 + 1);
    if (pbQueryDomainName == NULL)
    {
        return bRet;
    }
    memset(pbQueryDomainName, 0, uiDnLen + 1 + 1);

    unsigned int uiPos = 0;
    unsigned int i = 0;
    //域名段数
    // 域名的最后是点号，即包含根域名，下面的循环全部转换/设为非法
    for (i = 0; i < uiDnLen; ++i)
    {
        if (szDomainName[i] == '.')
        {
            pbQueryDomainName[uiPos] = i - uiPos;
            if (pbQueryDomainName[uiPos] > 0)
            {
                memcpy(pbQueryDomainName + uiPos + 1, szDomainName + uiPos, i - uiPos);
            }
            uiPos = i + 1;
            num++;
        }
    }
    // 如果域名的最后不是点号，那么上面的循环只转换了一部分
    // 下面的代码继续转换剩余的部分
    if (szDomainName[i - 1] != '.')
    {
        pbQueryDomainName[uiPos] = i - uiPos;
        memcpy(pbQueryDomainName + uiPos + 1, szDomainName + uiPos, i - uiPos);
        uiQueryNameLen = uiDnLen + 1 + 1;
    }
    else
    {
        uiQueryNameLen = uiDnLen + 1;
    }
    num++;


    // 填充内容 头部 + name + type + class
    DNSHeader* PDNSPackage = (DNSHeader*)malloc(sizeof(DNSHeader) + uiQueryNameLen + 4);
    if (PDNSPackage == NULL)
    {
       exit(0);
    }
    memset(PDNSPackage, 0, sizeof(DNSHeader) + uiQueryNameLen + 4);

    // 填充头部内容
    PDNSPackage->usTransID = htons(usID);  // ID
  //  PDNSPackage->RA = 0;   //可用递归

    PDNSPackage->RD = an;
  /*
    printf("选择：0、迭代查询  1、复合查询\n");
    scanf("%d", &an);

    if (an == 0)
    {
        PDNSPackage->RD = 0;   // 期望递归,采用迭代
    }
    else
    {
        PDNSPackage->RD = 0x1;   // 期望递归,采用复合
    }
    */
    PDNSPackage->Questions = htons(0x1);  // 网络字节序，用htons做了转换

    // 填充正文内容  name + type + class
    BYTE* PText = (BYTE*)PDNSPackage + sizeof(DNSHeader);
    memcpy(PText, pbQueryDomainName, uiQueryNameLen);

    unsigned short* usQueryType = (unsigned short*)(PText + uiQueryNameLen);
    *usQueryType = htons(0x1);        // TYPE: A

    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN

    // 需要发送到的DNS服务器的地址
    sockaddr_in dnsServAddr;
    memset(&dnsServAddr, 0, sizeof(dnsServAddr));  //每个字节都用0填充
    dnsServAddr.sin_family = AF_INET;
    dnsServAddr.sin_port = ::htons(53);  // DNS服务端的端口号为53
    dnsServAddr.sin_addr.S_un.S_addr = ::inet_addr("127.0.0.1");

    // 将查询报文发送出去
    sendto(*sock,(char*)PDNSPackage,sizeof(DNSHeader) + uiQueryNameLen + 4,0,(sockaddr*)&dnsServAddr,sizeof(dnsServAddr));
    bRet = true;

    return 0;
}

//接收并解析响应报文
void  MainWindow::RecvDnsPack(IN SOCKET* sock)
{
    QString s1,s2,s3,s4,s5,s6,s7;
    int temp = 0;
    int i;
    for (i = 0; i < num + 1; i++)
    {
        char szBuffer[256] = {};        // 保存接收到的内容
        sockaddr_in servAddr = {};
        int iFromLen = sizeof(sockaddr_in);
        int iRet = ::recvfrom(*sock,szBuffer,256, 0,(sockaddr*)&servAddr, &iFromLen);
        if (SOCKET_ERROR == iRet || 0 == iRet)
        {
            s1="recv-fail!";
            ui->textEdit->append(s1);
        }
        temp++;
        /* 解析收到的内容 */
        DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
        unsigned int uiTotal = iRet;        // 总字节数
        unsigned int uiSurplus = iRet;  // 接受到的总的字节数

        // 确定收到的szBuffer的长度大于sizeof(DNSHeader)
        if (uiTotal <= sizeof(DNSHeader))
        {
            s2="接收到的内容长度不合法";
            ui->textEdit->append(s2);
            return;
        }

        // 确认PDNSPackageRecv中的Flags确实为DNS的响应报文
        if (0x01 != PDNSPackageRecv->QR)
        {
            s3="接收到的报文不是响应报文";
            ui->textEdit->append(s3);
            return;
        }
        // 确认PDNSPackageRecv中的ID是否与发送报文中的是一致的
        if (htons(usID) != PDNSPackageRecv->usTransID)
        {
            s4="接收到的报文ID与查询报文不相符";
            ui->textEdit->append(s4);
            return;
        }

        // 获取Queries中的type和class字段
        unsigned char* pChQueries = (unsigned char*)PDNSPackageRecv + sizeof(DNSHeader);
        uiSurplus -= sizeof(DNSHeader);
        for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // 跳过Queries中的name字段

        ++pChQueries;
        --uiSurplus;
        unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
        pChQueries += 2;
        uiSurplus -= 2;

        unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
        pChQueries += 2;
        uiSurplus -= 2;

        // 解析Answers字段
        unsigned char* pChAnswers = pChQueries;
        while (0 < uiSurplus && uiSurplus <= uiTotal)
        {
            // 跳过name字段（无用）
            if (*pChAnswers == 0xC0)  // 存放的是指针
            {
                if (uiSurplus < 2)
                {
                    s5="接收到的内容长度不合法";
                    ui->textEdit->append(s5);
                    return;
                }
                pChAnswers += 2;       // 跳过指针字段
                uiSurplus -= 2;
            }
            else        // 存放的是域名
            {
                // 跳过域名，因为已经校验了ID，域名就不用了
                for (; *pChAnswers && uiSurplus > 0; ++pChAnswers, --uiSurplus) { ; }
                pChAnswers++;
                uiSurplus--;
            }
            unsigned short usAnswerType = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += 2;
            uiSurplus -= 2;

            unsigned short usAnswerClass = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += 2;
            uiSurplus -= 2;
            printf("F\n");
            if (usAnswerType != usQueryType || usAnswerClass != usQueryClass)
            {
                s6="接收到的内容Type和Class与发送报文不一致";
                ui->textEdit->append(s6);
                return;
            }

            pChAnswers += 4;    // 跳过Time to live字段，对于DNS Client来说，这个字段无用
            uiSurplus -= 4;
            if (htons(0x04) != *(unsigned short*)pChAnswers)
            {
                uiSurplus -= 2;     // 跳过data length字段
                uiSurplus -= ntohs(*(unsigned short*)pChAnswers); // 跳过真正的length
                pChAnswers += 2;
                pChAnswers += ntohs(*(unsigned short*)pChAnswers);
            }
            else
            {
                if (uiSurplus < 6)
                {
                    s7="接收到的内容长度不合法";
                    ui->textEdit->append(s7);
                    return;
                }

                uiSurplus -= 6;
                // Type为A, Class为IN
                 //  if (usAnswerType == 1 && usAnswerClass == 1)
                //      {
                pChAnswers += 2;

                unsigned int uiIP = *(unsigned int*)pChAnswers;
                in_addr in = {};
                in.S_un.S_addr = uiIP;

                if ((temp == num + 1)||(PDNSPackageRecv->AdditionalRRs == 0))
                {
                    ui->textEdit->clear();
                    s1="最终查询域名的IP为:";
                    ui->textEdit->append(s1);
                    s2=inet_ntoa(in);
                    ui->textEdit->append(s2);
                }
                else
                    {
                        ui->textEdit->clear();
                        s1="查询的IP为：";
                        ui->textEdit->append(s1);
                        s2=inet_ntoa(in);
                        ui->textEdit->append(s2);

                        s1="提示";
                        s2="下一步";
                        QMessageBox:: StandardButton result= QMessageBox::information(NULL, s1, s2 ,QMessageBox::Yes);
                        switch (result)
                        {
                        case QMessageBox::Yes:
                            break;
                        default:
                            break;
                        }
                   }
             }

            //表示在本地查询到
            if (PDNSPackageRecv->AdditionalRRs == 0)
               break;

            //表示此时服务器支持递归
            if (ntohs(PDNSPackageRecv->AuthorityRRs) > 0)
               break;

         }
        //表示在本地查询到
        if (PDNSPackageRecv->AdditionalRRs == 0)
           break;

        //表示此时服务器支持递归
        if (ntohs(PDNSPackageRecv->AuthorityRRs) > 0)
           break;

    }
}

//显示查询记录
void MainWindow::displayed()
{
    QString s1="Can't open the file!";
    QFile file("C:\\Users\\DELL\\Desktop\\server\\server\\ed.txt");

    if(!file.open(QIODevice::ReadOnly)) //这里对文件是否打开成功进行判断
    {
        //QDebug()<<"Can't open the file!"<<endl;

        ui->textEdit_2->append(s1);
        //qDebug()<<file.errorString();
    }
    while(!file.atEnd())
    { //atEnd（），判断文件是否到最后一行
        QByteArray line = file.readLine(); //还有readAll（），read（），等 读取文件的类型为QbyteArray
        QString str(line);
        ui->textEdit_2->append(str);
    }
    file.close();
  //      closesocket(sock);
        WSACleanup();
}

void MainWindow::on_pushButton_clicked()
{
    //初始化DLL
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    SendDomainName(&sock);
    RecvDnsPack(&sock);
    displayed();
    closesocket(sock);
    WSACleanup();
}

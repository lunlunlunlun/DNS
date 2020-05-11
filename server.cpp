#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE 100


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

//本地服务器组装并向客户端发送响应报文
// @Param:       sock:套接字
//               clntAddr:客户端地址信息
//               DomainName：域名www.baidu.com
//               IP:查询到的IP地址
//               PDNSPackageRecv:请求报文内容
//               iRet:请求报文长度/字节数
//               len:域名www.baidu.com长度
int locaSendMess(SOCKET* sock, SOCKADDR clntAddr, char* DomainName,
    unsigned char* IP, DNSHeader* PDNSPackageRecv, int iRet, int len)
{
    //域名长度/字节数
    unsigned int DomainLen = iRet - sizeof(DNSHeader) - 4;
    //响应报文
    DNSHeader* sendDNS = (DNSHeader*)malloc(sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);
    memset(sendDNS, 0, sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);

    //域名3www5baidu3com0长度
    len += 2;
    int i;

    //响应报文头部信息
    sendDNS->usTransID = PDNSPackageRecv->usTransID;
    sendDNS->AnswerRRs = htons(0x1);
    sendDNS->AdditionalRRs = 0;//表示为本地搜索结果
    sendDNS->AuthorityRRs = 0;
    sendDNS->QR = 1;

      // 填充正文内容  name + type + class
      //name
    BYTE* PText = (BYTE*)sendDNS + sizeof(DNSHeader);
    BYTE* TEMP = (BYTE*)PDNSPackageRecv + sizeof(DNSHeader);
    strncpy((char*)PText, (char*)TEMP, len);

    //type
    unsigned short* usQueryType = (unsigned short*)(PText + len);
    *usQueryType = htons(0x1);        // TYPE: A
   // printf("\n%u", usQueryType);

    //class
    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN  

    /*answers*/
    //复制域名
    ++usQueryType;
    strncpy((char*)usQueryType, (char*)TEMP, len);

    usQueryType = (unsigned short*)(PText + 2 * DomainLen + 4);
    //  ++usQueryType;
    *usQueryType = htons(0x1);        // TYPE:A 

    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN 

    ++usQueryType;
    *usQueryType = htons(0x1);       //TTL

    ++usQueryType;
    ++usQueryType;
    *usQueryType = htons(0x4);      //Length

    printf("\n%s\n", IP);

    unsigned long netip;
    netip = inet_addr((const char*)IP);
    //  printf("\n%u\n", netip);
    ++usQueryType;
    *usQueryType = (unsigned short)netip;
    ++usQueryType;
    *usQueryType = (unsigned short)(netip >> 16);

    // 将查询报文发送出去
    sendto(*sock, (char*)sendDNS, sizeof(DNSHeader) + 2 * DomainLen + 4 + 14, 0, &clntAddr, sizeof(clntAddr));

    return 0;
}



//交互组装并向客户端发送响应报文
// @Param:       sock:套接字
//               clntAddr:客户端地址信息
//               DomainName：域名www.baidu.com
//               IP:查询到的IP地址
//               PDNSPackageRecv:请求报文内容
//               iRet:请求报文长度/字节数
//               len:域名www.baidu.com长度
int netSendMess(SOCKET* sock,SOCKADDR clntAddr, char* DomainName,
             unsigned char* IP, DNSHeader* PDNSPackageRecv,int iRet,int len)
{
   //域名长度/字节数
    unsigned int DomainLen = iRet - sizeof(DNSHeader) - 4;
    //响应报文
    DNSHeader* sendDNS = (DNSHeader*)malloc(sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);
    memset(sendDNS, 0, sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);
   
    //域名3www5baidu3com0长度
    len += 2;
    int i;

    //响应报文头部信息
    sendDNS->usTransID = PDNSPackageRecv->usTransID;
    sendDNS->AnswerRRs = htons(0x1);
    sendDNS->AdditionalRRs = htons(0x1);//表示为交互搜索结果
    sendDNS->QR = 1;

    // 填充正文内容  name + type + class
    //name
    BYTE* PText = (BYTE*)sendDNS + sizeof(DNSHeader);
    BYTE* TEMP = (BYTE*)PDNSPackageRecv + sizeof(DNSHeader);
    strncpy((char*)PText, (char*)TEMP, len);

    //type
    unsigned short* usQueryType = (unsigned short*)(PText + len);
    *usQueryType = htons(0x1);        // TYPE: A
   // printf("\n%u", usQueryType);

    //class
    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN  

    /*answers*/
    //复制域名
    ++usQueryType;
    strncpy((char*)usQueryType, (char*)TEMP, len);

    usQueryType = (unsigned short*)(PText + 2*DomainLen + 4);
  //  ++usQueryType;
    *usQueryType = htons(0x1);        // TYPE:A 

    ++usQueryType;
    *usQueryType = htons(0x1);        // CLASS: IN 

    ++usQueryType;
    *usQueryType = htons(0x1);       //TTL

    ++usQueryType;
    ++usQueryType;
    *usQueryType = htons(0x4);      //Length

    printf("\nnetSendMess %s\n", IP);
  
    unsigned long netip;
    netip = inet_addr((const char*)IP);
  //  printf("\n%u\n", netip);
    ++usQueryType;
    *usQueryType = (unsigned short)netip;
     ++usQueryType;
    *usQueryType = (unsigned short)(netip>>16);
    
    // 将查询报文发送出去
    sendto(*sock, (char*)sendDNS, sizeof(DNSHeader) + 2 * DomainLen + 4 + 14, 0, &clntAddr, sizeof(clntAddr));
    printf("返回报文len  %d\n", sizeof(DNSHeader) + 2 * DomainLen + 4 + 14);
//    printf("AAAA\n");
    return 0;
}


//迭代查询下一级
//@Param:        socket:客户端套接字
//               clntAddr:客户端地址信息
//               PDNSPackageRecv:客户端请求报文
//               iRet:客户端请求报文长度
//               len:域名（www.baidu.com）长度
//               uiIP:要发送服务器的IP地址、
//               szBuffer:客户端的请求报文char
//               DomainName:域名www.baidu.com
int nextServ(SOCKET* socke, SOCKADDR clntAddr, DNSHeader* PDNSPackageRecv,
             int iRet, int len, unsigned char* uiIP, char* szBuffer, char* DomainName)
{

    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //服务器地址信息
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr((const char*)uiIP);
    servAddr.sin_port = htons(53);

    //发送
    sendto(sock, (char*)PDNSPackageRecv, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    //接收
    char Buffer[9999] = { 0 };        // 保存本次接收到的内容
    sockaddr fromAddr;
    int iFromLen = sizeof(fromAddr);
    int nowRet = ::recvfrom(sock, Buffer, 9999, 0, &fromAddr, &iFromLen);
    if (SOCKET_ERROR == nowRet || 0 == nowRet)
    {
        printf("recv fail \n");
        return 0;
    }
    char str[1024];

    /* 解析收到的内容 */
    DNSHeader* PDNSPackage = (DNSHeader*)Buffer;
    unsigned int uiTotal = nowRet;        // 总字节数
    unsigned int uiSurplus = nowRet;  // 接受到的总的字节数
   // 获取Queries中的type和class字段
    unsigned char* pChQueries = (unsigned char*)PDNSPackage + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // 跳过Queries中的name字段
    ++pChQueries;
    --uiSurplus;//208
    unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;

    unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;
    //解析资源数据段
    int i;
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
    //    printf("a\n");
        // 跳过name字段
        if (*pChAnswers == 0xC0)  // 存放的是指针
        {
      //      printf("c\n");
            if (uiSurplus < 2)
            {
                printf("接收到的内容长度不合法\n");
                return 0;
            }
            pChAnswers += 2;       // 跳过指针字段
            uiSurplus -= 2;
        }
        else        // 存放的是域名
        {
        //    printf("b\n");
            // 跳过域名
            for (; *pChAnswers && uiSurplus > 0; ++pChAnswers, --uiSurplus) { ; }
            pChAnswers++;
            uiSurplus--;
        }

        //type
        unsigned short usAnswerType = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;

        //class
        unsigned short usAnswerClass = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;
     //   printf("%u  %u\n", usAnswerType, usQueryType);
        if (usAnswerType == 1)
        {
            //TTL+length
            pChAnswers += 6;
            uiSurplus -= 6;

            unsigned int uiIP = *(unsigned int*)pChAnswers;
            in_addr in = {};
            in.S_un.S_addr = uiIP;
            printf("IP: %s\n", inet_ntoa(in));
            
            //ip字符串
            char* a = (char*)malloc(100);
            a = inet_ntoa(in);

            //查询到最终结果,发给客户端
            if ((PDNSPackage->AnswerRRs) > 0)
            {
                //发送
                netSendMess(socke, clntAddr, DomainName,(unsigned char*)a, PDNSPackageRecv, iRet, len);
       //         printf("yes\n");
                //写进本地地址表
                FILE* fp = fopen("IP.txt", "a");
                fprintf(fp, "%s\n", DomainName);
                fprintf(fp, "%s\n", a);
                fclose(fp);

                //写进查询记录
                FILE* fped = fopen("ed.txt", "a");
                fprintf(fped, "%s\n", DomainName);
                fprintf(fped, "%s\n", a);
                fclose(fped);
            }
            //向下一级查询
            else
            {
                printf("迭代中\n");
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
                nextServ(socke, clntAddr, PDNSPackageRecv, iRet, len, (unsigned char*)inet_ntoa(in), szBuffer, DomainName);
       //         printf("next\n");
            }

        }
        //该字段返回的不是IP
        else
        {
            pChAnswers += 4;
            uiSurplus -= 4;     //指向length

            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += leng + 2;
            uiSurplus -= leng + 2;
        }
   //     printf("ceshi\n");
    }
    return 0;
}


//迭代查询  RD=0
//根服务器
// @Param:       socket:客户端套接字
//               clntAddr:客户端地址信息
//               PDNSPackageRec:客户端请求报文
//               DomainName:域名www.baidu.com
//               iRet:客户端请求报文长度
//               len:域名（www.baidu.com）长度
int IteraQuery(SOCKET* socke, SOCKADDR clntAddr, DNSHeader* PDNSPackageRec,
                 char* DomainName, int iRet, int len)
{
    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //服务器地址信息
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("198.41.0.4");//根服务器
    servAddr.sin_port = htons(53);
    //发送
    sendto(sock, (char*)PDNSPackageRec, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    // 接收
    char szBuffer[9999] = {};        // 保存接收到的内容
    sockaddr fromAddr;
    int iFromLen = sizeof(fromAddr);
  //  printf("1\n");
    int nowRet = ::recvfrom(sock, szBuffer, 9999, 0, &fromAddr, &iFromLen);
  //  printf("2\n");
    if (SOCKET_ERROR == iRet || 0 == iRet)
    {
        printf("recv fail \n");
        return 0;
    }

    /* 解析收到的内容 */
    DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
    unsigned int uiTotal = nowRet;        // 总字节数
    unsigned int uiSurplus = nowRet;  // 接受到的总的字节数
    printf("从根服务器收到的报文大小：%d\n", nowRet);
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


    //解析资源数据段
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
        // 跳过name字段
        pChAnswers += 2;
        uiSurplus -= 2;
        //TYPE
        unsigned short usAnswerType = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;
        //CLASS
        unsigned short usAnswerClass = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;
   //     printf("%u  %u\n", usAnswerType, usQueryType);
        // 跳过TTL字段
        pChAnswers += 4;
        uiSurplus -= 4;
        //找到了下一级IP
        //type=A
        if (usAnswerType == 1)
        {
            //跳过length
            pChAnswers += 2;
            uiSurplus -= 2;


         //   unsigned char* netip = *(unsigned char*)pChAnswers;
            unsigned int uiIP = *(unsigned int*)pChAnswers;
            in_addr in = {};
            in.S_un.S_addr = uiIP;

      //      printf("\ngenIP: %s\n", inet_ntoa(in));
            char* a = (char*)malloc(100);
            a=inet_ntoa(in);

            //发送给客户端
            netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRec, iRet, len);
            printf("根服务器查到的IP已发给客户端\n");
            //向下一级查询
            nextServ(socke, clntAddr, PDNSPackageRec, iRet, len, (unsigned char*)a, szBuffer, DomainName);

        }
        //该字段返回的不是IP
        else
        {
            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));

            leng += 2;

            pChAnswers += leng;
            uiSurplus -= leng;
        }
    }
    printf("完成一个\n");
    return 0;
}


// 复合查询下一级
//@Param:        socket:客户端套接字
//               clntAddr:客户端地址信息
//               PDNSPackageRecv:客户端请求报文
//               iRet:客户端请求报文长度
//               len:域名（www.baidu.com）长度
//               uiIP:要发送服务器的IP地址、
//               szBuffer:客户端的请求报文char
//               DomainName:域名www.baidu.com
int fhnextServ(SOCKET * socke, SOCKADDR clntAddr, DNSHeader * PDNSPackageRecv,
    int iRet, int len, unsigned char* uiIP, char* szBuffer, char* DomainName)
{

    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //服务器地址信息
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr((const char*)uiIP);
    servAddr.sin_port = htons(53);

    //发送
    sendto(sock, (char*)PDNSPackageRecv, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    //接收
    char Buffer[9999] = { 0 };        // 保存本次接收到的内容
    sockaddr fromAddr;
    int iFromLen = sizeof(fromAddr);
    int nowRet = ::recvfrom(sock, Buffer, 9999, 0, &fromAddr, &iFromLen);
    if (SOCKET_ERROR == nowRet || 0 == nowRet)
    {
        printf("recv fail \n");
        return 0;
    }
    char str[1024];

    /* 解析收到的内容 */
    DNSHeader* PDNSPackage = (DNSHeader*)Buffer;
    unsigned int uiTotal = nowRet;        // 总字节数
    unsigned int uiSurplus = nowRet;  // 接受到的总的字节数
   // 获取Queries中的type和class字段
    unsigned char* pChQueries = (unsigned char*)PDNSPackage + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // 跳过Queries中的name字段
    ++pChQueries;
    --uiSurplus;//208
    unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;

    unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;
    //解析资源数据段
    int i;
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
  //      printf("a\n");
        // 跳过name字段
        if (*pChAnswers == 0xC0)  // 存放的是指针
        {
    //        printf("c\n");
            if (uiSurplus < 2)
            {
                printf("接收到的内容长度不合法\n");
                return 0;
            }
            pChAnswers += 2;       // 跳过指针字段
            uiSurplus -= 2;
        }
        else        // 存放的是域名
        {
      //      printf("b\n");
            // 跳过域名
            for (; *pChAnswers && uiSurplus > 0; ++pChAnswers, --uiSurplus) { ; }
            pChAnswers++;
            uiSurplus--;
        }

        //type
        unsigned short usAnswerType = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;

        //class
        unsigned short usAnswerClass = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;
        //printf("%u  %u\n", usAnswerType, usQueryType);
        if (usAnswerType == 1)
        {
            //TTL+length
            pChAnswers += 6;
            uiSurplus -= 6;

            unsigned int uiIP = *(unsigned int*)pChAnswers;
            in_addr in = {};
            in.S_un.S_addr = uiIP;
            printf("迭代下一级IP: %s\n", inet_ntoa(in));

            //ip字符串
            char* a = (char*)malloc(100);
            a = inet_ntoa(in);

            //查询到最终结果,发给客户端
            if ((PDNSPackage->AnswerRRs) > 0)
            {
                //发送
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
      //          printf("yes\n");
                //写进本地地址表
                FILE* fp = fopen("IP.txt", "a");
                fprintf(fp, "%s\n", DomainName);
                fprintf(fp, "%s\n", a);
                fclose(fp);

                //写进查询记录
                FILE* fped = fopen("ed.txt", "a");
                fprintf(fped, "%s\n", DomainName);
                fprintf(fped, "%s\n", a);
                fclose(fped);
            }
            //向下一级查询
            else
            {
                //服务器支持递归
                if (PDNSPackage->RD == 1 && PDNSPackage->RA == 1)
                {
                    PDNSPackageRecv->AuthorityRRs = htons(0x1);
                    //发送
                    netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
          //          printf("yes\n");
                    //写进本地地址表
                    FILE* fp = fopen("IP.txt", "a");
                    fprintf(fp, "%s\n", DomainName);
                    fprintf(fp, "%s\n", a);
                    fclose(fp);

                    //写进查询记录
                    FILE* fped = fopen("ed.txt", "a");
                    fprintf(fped, "%s\n", DomainName);
                    fprintf(fped, "%s\n", a);
                    fclose(fped);
                }
                else
                {
                    PDNSPackageRecv->AuthorityRRs = 0;
                    netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
                    nextServ(socke, clntAddr, PDNSPackageRecv, iRet, len, (unsigned char*)inet_ntoa(in), szBuffer, DomainName);
            //        printf("next\n");
                }
                
            }

        }
        //该字段返回的不是IP
        else
        {
            pChAnswers += 4;
            uiSurplus -= 4;     //指向length

            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += leng + 2;
            uiSurplus -= leng + 2;
        }
    }
    return 0;
}


//复合查询  RD=1
// @Param:       socket:客户端套接字
//               clntAddr:客户端地址信息
//               PDNSPackageRec:客户端请求报文
//               DomainName:域名www.baidu.com
//               iRet:客户端请求报文长度
//               len:域名（www.baidu.com）长度
int RecurQuery(SOCKET* socke, SOCKADDR clntAddr, DNSHeader* PDNSPackageRec,
                 char* DomainName, int iRet, int len)
{
    //创建套接字
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //服务器地址信息
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("198.41.0.4");//根服务器
    servAddr.sin_port = htons(53);
    //发送
    sendto(sock, (char*)PDNSPackageRec, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    // 接收
    char szBuffer[9999] = {};        // 保存接收到的内容
    sockaddr fromAddr;
    int iFromLen = sizeof(fromAddr);
    //  printf("1\n");
    int nowRet = ::recvfrom(sock, szBuffer, 9999, 0, &fromAddr, &iFromLen);
    //  printf("2\n");
    if (SOCKET_ERROR == iRet || 0 == iRet)
    {
        printf("recv fail \n");
        return 0;
    }

    /* 解析收到的内容 */
    DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
    unsigned int uiTotal = nowRet;        // 总字节数
    unsigned int uiSurplus = nowRet;  // 接受到的总的字节数
    printf("从根服务器收到的报文大小：%d\n", nowRet);

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


    //解析资源数据段
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
        // 跳过name字段
        pChAnswers += 2;
        uiSurplus -= 2;
        //TYPE
        unsigned short usAnswerType = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;
        //CLASS
        unsigned short usAnswerClass = ntohs(*((unsigned short*)pChAnswers));
        pChAnswers += 2;
        uiSurplus -= 2;
        printf("%u  %u\n", usAnswerType, usQueryType);
        // 跳过TTL字段
        pChAnswers += 4;
        uiSurplus -= 4;
        //找到了下一级IP
        //type=A
        if (usAnswerType == 1)
        {
            //跳过length
            pChAnswers += 2;
            uiSurplus -= 2;

            unsigned int uiIP = *(unsigned int*)pChAnswers;
            in_addr in = {};
            in.S_un.S_addr = uiIP;

            printf("\nIP: %s\n", inet_ntoa(in));
            char* a = (char*)malloc(100);
            a = inet_ntoa(in);

            if (PDNSPackageRecv->RD == 1 && PDNSPackageRecv->RA == 1)
            {
                PDNSPackageRec->AuthorityRRs = htons(0x1);//表示服务器支持递归
                // 发送给客户端
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRec, iRet, len);
                return 0;
            }
            else
            {
                PDNSPackageRec->AuthorityRRs = 0;//表示服务器不支持递归
                //发送给客户端
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRec, iRet, len);
                //向下一级查询
                fhnextServ(socke, clntAddr, PDNSPackageRec, iRet, len, (unsigned char*)a, szBuffer, DomainName);
            }
        }
        //该字段返回的不是IP
        else
        {
            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));

            leng += 2;

            pChAnswers += leng;
            uiSurplus -= leng;
        }
    }
    printf("完成一个\n");
    return 0;
}


//查询本地DNS
int receClient(SOCKET* sock)
{
    //客户端地址信息
    SOCKADDR clntAddr;  
    // 保存接收到的内容
    char szBuffer[256] = {};        
    int iFromLen = sizeof(sockaddr_in);

    int iRet = ::recvfrom(*sock, szBuffer, 256, 0, (sockaddr*)&clntAddr, &iFromLen);
    printf("\n%d\n", iRet);



    /* 解析收到的内容 */
    DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
    //选择递归或迭代
    int mode = PDNSPackageRecv->RD;
    unsigned short usTransID = PDNSPackageRecv->usTransID;
    
    // 接受到的总的字节数
    unsigned int uiSurplus = iRet;  
  //  printf("%u\n", htons(PDNSPackageRecv->usTransID));
    // 获取Queries中的域名字段
    unsigned char* pChQueries = (unsigned char*)PDNSPackageRecv + sizeof(DNSHeader);
    //  printf("\n%s\n", pChQueries);
    uiSurplus -= sizeof(DNSHeader);
    uiSurplus -= 4;

    int len = 0;//域名长度

    char* DomainName=(char*)malloc(256*sizeof(char));
    //每级域名长度
    int a = pChQueries[0];
    ++pChQueries;

    for (; uiSurplus > 0; --uiSurplus)
    {
        if (a > 0)
        {
            DomainName[len] = *pChQueries;
            a--;
        }
        else
        {
            if (*pChQueries == 0)
            {
                break;
            }
            a = *pChQueries;
            DomainName[len] = '.';
        }
        pChQueries++;
        len++;
    }
    DomainName[len] = '\0';
  //  printf("\n%s\n", DomainName);

    //查询本地地址表
    char* StrLine=(char*)malloc(256 * sizeof(char));
    unsigned char IP[100];
    int flag = -1;//不存在域名
    int t;
    FILE* fp = fopen("IP.txt", "r");
    while (!feof(fp))
    {
        //读取一行
        fgets(StrLine, 1024, fp);
        t = strlen(StrLine);//注意文件中的换行符
        StrLine[t - 1] = '\0';
        if (strcmp(StrLine, DomainName) == 0)
        {
            flag = 0;
            fgets((char*)IP, 1024, fp);  //读取IP
            t = strlen((char*)IP);
            IP[t - 1] = '\0';
  //          printf("\n%s\n", IP);
                break;
        }
    }
    fclose(fp);
    //本地地址表中找到域名
    //组装相应报文并发送至客户端
    if(flag==0)
    {
        //缓存查询过的域名
        FILE* fped = fopen("ed.txt", "a");//"a" 打开，指向文件尾，在已存在文件中追加
        fprintf(fp, "%s\n", StrLine);
        fprintf(fp, "%s\n", IP);
        fclose(fped);

        //发送响应报文
        locaSendMess(sock,clntAddr, DomainName,IP,PDNSPackageRecv,iRet,len);
    }
    //本地地址表不存在查找的域名
    //需要与其他服务器进行交互
    //复合  or  迭代
    else
    {
        if (mode == 0)//迭代
        {
            printf("迭代\n");
            IteraQuery(sock, clntAddr, PDNSPackageRecv, DomainName, iRet, len);
          }
        else//复合
        {
            printf("复合\n");
            RecurQuery(sock, clntAddr, PDNSPackageRecv, DomainName, iRet, len);
        }
        
    }
    return 0;
    
}


int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //创建套接字
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    //绑定套接字
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //每个字节都用0填充
    servAddr.sin_family = PF_INET;  //使用IPv4地址
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取IP地址
    servAddr.sin_port = htons(53);  //端口
    bind(sock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));

    //接收客户端请求
    int a;
    while (1)
    {
        printf("接收...\n");
        //首先查询本地地址表
        receClient(&sock);
    }
    //结束
    closesocket(sock);
    WSACleanup();
    return 0;
}
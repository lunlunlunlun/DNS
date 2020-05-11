#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
#define BUF_SIZE 100


#pragma pack(push, 1)
//����ͷ���ṹ��
struct DNSHeader
{
    /* 1. �Ự��ʶ��2�ֽڣ�*/
    unsigned short usTransID;        // Transaction ID

    /* 2. ��־����2�ֽڣ�*/
    unsigned char RD : 1;            // ��ʾ�����ݹ飬1bit
    unsigned char TC : 1;            // ��ʾ�ɽضϵģ�1bit
    unsigned char AA : 1;            // ��ʾ��Ȩ�ش�1bit
    unsigned char opcode : 4;        // 0��ʾ��׼��ѯ��1��ʾ�����ѯ��2��ʾ������״̬����4bit
    unsigned char QR : 1;            // ��ѯ/��Ӧ��־λ��0Ϊ��ѯ��1Ϊ��Ӧ��1bit

    unsigned char rcode : 4;         // ��ʾ�����룬4bit
    unsigned char zero : 3;          // ����Ϊ0��3bit
    unsigned char RA : 1;            // ��ʾ���õݹ飬1bit

    /* 3. �����ֶΣ���8�ֽڣ� */
    unsigned short Questions;        // ������
    unsigned short AnswerRRs;        // �ش���Դ��¼��
    unsigned short AuthorityRRs;     // ��Ȩ��Դ��¼��
    unsigned short AdditionalRRs;    // ������Դ��¼��
};
#pragma pack(pop)

//���ط�������װ����ͻ��˷�����Ӧ����
// @Param:       sock:�׽���
//               clntAddr:�ͻ��˵�ַ��Ϣ
//               DomainName������www.baidu.com
//               IP:��ѯ����IP��ַ
//               PDNSPackageRecv:����������
//               iRet:�����ĳ���/�ֽ���
//               len:����www.baidu.com����
int locaSendMess(SOCKET* sock, SOCKADDR clntAddr, char* DomainName,
    unsigned char* IP, DNSHeader* PDNSPackageRecv, int iRet, int len)
{
    //��������/�ֽ���
    unsigned int DomainLen = iRet - sizeof(DNSHeader) - 4;
    //��Ӧ����
    DNSHeader* sendDNS = (DNSHeader*)malloc(sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);
    memset(sendDNS, 0, sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);

    //����3www5baidu3com0����
    len += 2;
    int i;

    //��Ӧ����ͷ����Ϣ
    sendDNS->usTransID = PDNSPackageRecv->usTransID;
    sendDNS->AnswerRRs = htons(0x1);
    sendDNS->AdditionalRRs = 0;//��ʾΪ�����������
    sendDNS->AuthorityRRs = 0;
    sendDNS->QR = 1;

      // �����������  name + type + class
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
    //��������
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

    // ����ѯ���ķ��ͳ�ȥ
    sendto(*sock, (char*)sendDNS, sizeof(DNSHeader) + 2 * DomainLen + 4 + 14, 0, &clntAddr, sizeof(clntAddr));

    return 0;
}



//������װ����ͻ��˷�����Ӧ����
// @Param:       sock:�׽���
//               clntAddr:�ͻ��˵�ַ��Ϣ
//               DomainName������www.baidu.com
//               IP:��ѯ����IP��ַ
//               PDNSPackageRecv:����������
//               iRet:�����ĳ���/�ֽ���
//               len:����www.baidu.com����
int netSendMess(SOCKET* sock,SOCKADDR clntAddr, char* DomainName,
             unsigned char* IP, DNSHeader* PDNSPackageRecv,int iRet,int len)
{
   //��������/�ֽ���
    unsigned int DomainLen = iRet - sizeof(DNSHeader) - 4;
    //��Ӧ����
    DNSHeader* sendDNS = (DNSHeader*)malloc(sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);
    memset(sendDNS, 0, sizeof(DNSHeader) + 2 * DomainLen + 4 + 10);
   
    //����3www5baidu3com0����
    len += 2;
    int i;

    //��Ӧ����ͷ����Ϣ
    sendDNS->usTransID = PDNSPackageRecv->usTransID;
    sendDNS->AnswerRRs = htons(0x1);
    sendDNS->AdditionalRRs = htons(0x1);//��ʾΪ�����������
    sendDNS->QR = 1;

    // �����������  name + type + class
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
    //��������
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
    
    // ����ѯ���ķ��ͳ�ȥ
    sendto(*sock, (char*)sendDNS, sizeof(DNSHeader) + 2 * DomainLen + 4 + 14, 0, &clntAddr, sizeof(clntAddr));
    printf("���ر���len  %d\n", sizeof(DNSHeader) + 2 * DomainLen + 4 + 14);
//    printf("AAAA\n");
    return 0;
}


//������ѯ��һ��
//@Param:        socket:�ͻ����׽���
//               clntAddr:�ͻ��˵�ַ��Ϣ
//               PDNSPackageRecv:�ͻ���������
//               iRet:�ͻ��������ĳ���
//               len:������www.baidu.com������
//               uiIP:Ҫ���ͷ�������IP��ַ��
//               szBuffer:�ͻ��˵�������char
//               DomainName:����www.baidu.com
int nextServ(SOCKET* socke, SOCKADDR clntAddr, DNSHeader* PDNSPackageRecv,
             int iRet, int len, unsigned char* uiIP, char* szBuffer, char* DomainName)
{

    //�����׽���
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //��������ַ��Ϣ
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //ÿ���ֽڶ���0���
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr((const char*)uiIP);
    servAddr.sin_port = htons(53);

    //����
    sendto(sock, (char*)PDNSPackageRecv, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    //����
    char Buffer[9999] = { 0 };        // ���汾�ν��յ�������
    sockaddr fromAddr;
    int iFromLen = sizeof(fromAddr);
    int nowRet = ::recvfrom(sock, Buffer, 9999, 0, &fromAddr, &iFromLen);
    if (SOCKET_ERROR == nowRet || 0 == nowRet)
    {
        printf("recv fail \n");
        return 0;
    }
    char str[1024];

    /* �����յ������� */
    DNSHeader* PDNSPackage = (DNSHeader*)Buffer;
    unsigned int uiTotal = nowRet;        // ���ֽ���
    unsigned int uiSurplus = nowRet;  // ���ܵ����ܵ��ֽ���
   // ��ȡQueries�е�type��class�ֶ�
    unsigned char* pChQueries = (unsigned char*)PDNSPackage + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // ����Queries�е�name�ֶ�
    ++pChQueries;
    --uiSurplus;//208
    unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;

    unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;
    //������Դ���ݶ�
    int i;
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
    //    printf("a\n");
        // ����name�ֶ�
        if (*pChAnswers == 0xC0)  // ��ŵ���ָ��
        {
      //      printf("c\n");
            if (uiSurplus < 2)
            {
                printf("���յ������ݳ��Ȳ��Ϸ�\n");
                return 0;
            }
            pChAnswers += 2;       // ����ָ���ֶ�
            uiSurplus -= 2;
        }
        else        // ��ŵ�������
        {
        //    printf("b\n");
            // ��������
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
            
            //ip�ַ���
            char* a = (char*)malloc(100);
            a = inet_ntoa(in);

            //��ѯ�����ս��,�����ͻ���
            if ((PDNSPackage->AnswerRRs) > 0)
            {
                //����
                netSendMess(socke, clntAddr, DomainName,(unsigned char*)a, PDNSPackageRecv, iRet, len);
       //         printf("yes\n");
                //д�����ص�ַ��
                FILE* fp = fopen("IP.txt", "a");
                fprintf(fp, "%s\n", DomainName);
                fprintf(fp, "%s\n", a);
                fclose(fp);

                //д����ѯ��¼
                FILE* fped = fopen("ed.txt", "a");
                fprintf(fped, "%s\n", DomainName);
                fprintf(fped, "%s\n", a);
                fclose(fped);
            }
            //����һ����ѯ
            else
            {
                printf("������\n");
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
                nextServ(socke, clntAddr, PDNSPackageRecv, iRet, len, (unsigned char*)inet_ntoa(in), szBuffer, DomainName);
       //         printf("next\n");
            }

        }
        //���ֶη��صĲ���IP
        else
        {
            pChAnswers += 4;
            uiSurplus -= 4;     //ָ��length

            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += leng + 2;
            uiSurplus -= leng + 2;
        }
   //     printf("ceshi\n");
    }
    return 0;
}


//������ѯ  RD=0
//��������
// @Param:       socket:�ͻ����׽���
//               clntAddr:�ͻ��˵�ַ��Ϣ
//               PDNSPackageRec:�ͻ���������
//               DomainName:����www.baidu.com
//               iRet:�ͻ��������ĳ���
//               len:������www.baidu.com������
int IteraQuery(SOCKET* socke, SOCKADDR clntAddr, DNSHeader* PDNSPackageRec,
                 char* DomainName, int iRet, int len)
{
    //�����׽���
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //��������ַ��Ϣ
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //ÿ���ֽڶ���0���
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("198.41.0.4");//��������
    servAddr.sin_port = htons(53);
    //����
    sendto(sock, (char*)PDNSPackageRec, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    // ����
    char szBuffer[9999] = {};        // ������յ�������
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

    /* �����յ������� */
    DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
    unsigned int uiTotal = nowRet;        // ���ֽ���
    unsigned int uiSurplus = nowRet;  // ���ܵ����ܵ��ֽ���
    printf("�Ӹ��������յ��ı��Ĵ�С��%d\n", nowRet);
    // ��ȡQueries�е�type��class�ֶ�
    unsigned char* pChQueries = (unsigned char*)PDNSPackageRecv + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // ����Queries�е�name�ֶ�

    ++pChQueries;
    --uiSurplus;
    unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;
    unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;


    //������Դ���ݶ�
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
        // ����name�ֶ�
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
        // ����TTL�ֶ�
        pChAnswers += 4;
        uiSurplus -= 4;
        //�ҵ�����һ��IP
        //type=A
        if (usAnswerType == 1)
        {
            //����length
            pChAnswers += 2;
            uiSurplus -= 2;


         //   unsigned char* netip = *(unsigned char*)pChAnswers;
            unsigned int uiIP = *(unsigned int*)pChAnswers;
            in_addr in = {};
            in.S_un.S_addr = uiIP;

      //      printf("\ngenIP: %s\n", inet_ntoa(in));
            char* a = (char*)malloc(100);
            a=inet_ntoa(in);

            //���͸��ͻ���
            netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRec, iRet, len);
            printf("���������鵽��IP�ѷ����ͻ���\n");
            //����һ����ѯ
            nextServ(socke, clntAddr, PDNSPackageRec, iRet, len, (unsigned char*)a, szBuffer, DomainName);

        }
        //���ֶη��صĲ���IP
        else
        {
            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));

            leng += 2;

            pChAnswers += leng;
            uiSurplus -= leng;
        }
    }
    printf("���һ��\n");
    return 0;
}


// ���ϲ�ѯ��һ��
//@Param:        socket:�ͻ����׽���
//               clntAddr:�ͻ��˵�ַ��Ϣ
//               PDNSPackageRecv:�ͻ���������
//               iRet:�ͻ��������ĳ���
//               len:������www.baidu.com������
//               uiIP:Ҫ���ͷ�������IP��ַ��
//               szBuffer:�ͻ��˵�������char
//               DomainName:����www.baidu.com
int fhnextServ(SOCKET * socke, SOCKADDR clntAddr, DNSHeader * PDNSPackageRecv,
    int iRet, int len, unsigned char* uiIP, char* szBuffer, char* DomainName)
{

    //�����׽���
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //��������ַ��Ϣ
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //ÿ���ֽڶ���0���
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr((const char*)uiIP);
    servAddr.sin_port = htons(53);

    //����
    sendto(sock, (char*)PDNSPackageRecv, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    //����
    char Buffer[9999] = { 0 };        // ���汾�ν��յ�������
    sockaddr fromAddr;
    int iFromLen = sizeof(fromAddr);
    int nowRet = ::recvfrom(sock, Buffer, 9999, 0, &fromAddr, &iFromLen);
    if (SOCKET_ERROR == nowRet || 0 == nowRet)
    {
        printf("recv fail \n");
        return 0;
    }
    char str[1024];

    /* �����յ������� */
    DNSHeader* PDNSPackage = (DNSHeader*)Buffer;
    unsigned int uiTotal = nowRet;        // ���ֽ���
    unsigned int uiSurplus = nowRet;  // ���ܵ����ܵ��ֽ���
   // ��ȡQueries�е�type��class�ֶ�
    unsigned char* pChQueries = (unsigned char*)PDNSPackage + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // ����Queries�е�name�ֶ�
    ++pChQueries;
    --uiSurplus;//208
    unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;

    unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;
    //������Դ���ݶ�
    int i;
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
  //      printf("a\n");
        // ����name�ֶ�
        if (*pChAnswers == 0xC0)  // ��ŵ���ָ��
        {
    //        printf("c\n");
            if (uiSurplus < 2)
            {
                printf("���յ������ݳ��Ȳ��Ϸ�\n");
                return 0;
            }
            pChAnswers += 2;       // ����ָ���ֶ�
            uiSurplus -= 2;
        }
        else        // ��ŵ�������
        {
      //      printf("b\n");
            // ��������
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
            printf("������һ��IP: %s\n", inet_ntoa(in));

            //ip�ַ���
            char* a = (char*)malloc(100);
            a = inet_ntoa(in);

            //��ѯ�����ս��,�����ͻ���
            if ((PDNSPackage->AnswerRRs) > 0)
            {
                //����
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
      //          printf("yes\n");
                //д�����ص�ַ��
                FILE* fp = fopen("IP.txt", "a");
                fprintf(fp, "%s\n", DomainName);
                fprintf(fp, "%s\n", a);
                fclose(fp);

                //д����ѯ��¼
                FILE* fped = fopen("ed.txt", "a");
                fprintf(fped, "%s\n", DomainName);
                fprintf(fped, "%s\n", a);
                fclose(fped);
            }
            //����һ����ѯ
            else
            {
                //������֧�ֵݹ�
                if (PDNSPackage->RD == 1 && PDNSPackage->RA == 1)
                {
                    PDNSPackageRecv->AuthorityRRs = htons(0x1);
                    //����
                    netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRecv, iRet, len);
          //          printf("yes\n");
                    //д�����ص�ַ��
                    FILE* fp = fopen("IP.txt", "a");
                    fprintf(fp, "%s\n", DomainName);
                    fprintf(fp, "%s\n", a);
                    fclose(fp);

                    //д����ѯ��¼
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
        //���ֶη��صĲ���IP
        else
        {
            pChAnswers += 4;
            uiSurplus -= 4;     //ָ��length

            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));
            pChAnswers += leng + 2;
            uiSurplus -= leng + 2;
        }
    }
    return 0;
}


//���ϲ�ѯ  RD=1
// @Param:       socket:�ͻ����׽���
//               clntAddr:�ͻ��˵�ַ��Ϣ
//               PDNSPackageRec:�ͻ���������
//               DomainName:����www.baidu.com
//               iRet:�ͻ��������ĳ���
//               len:������www.baidu.com������
int RecurQuery(SOCKET* socke, SOCKADDR clntAddr, DNSHeader* PDNSPackageRec,
                 char* DomainName, int iRet, int len)
{
    //�����׽���
    SOCKET sock = socket(PF_INET, SOCK_DGRAM, 0);
    //��������ַ��Ϣ
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //ÿ���ֽڶ���0���
    servAddr.sin_family = PF_INET;
    servAddr.sin_addr.s_addr = inet_addr("198.41.0.4");//��������
    servAddr.sin_port = htons(53);
    //����
    sendto(sock, (char*)PDNSPackageRec, iRet, 0, (struct sockaddr*) & servAddr, sizeof(servAddr));
    // ����
    char szBuffer[9999] = {};        // ������յ�������
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

    /* �����յ������� */
    DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
    unsigned int uiTotal = nowRet;        // ���ֽ���
    unsigned int uiSurplus = nowRet;  // ���ܵ����ܵ��ֽ���
    printf("�Ӹ��������յ��ı��Ĵ�С��%d\n", nowRet);

    // ��ȡQueries�е�type��class�ֶ�
    unsigned char* pChQueries = (unsigned char*)PDNSPackageRecv + sizeof(DNSHeader);
    uiSurplus -= sizeof(DNSHeader);
    for (; *pChQueries && uiSurplus > 0; ++pChQueries, --uiSurplus) { ; } // ����Queries�е�name�ֶ�

    ++pChQueries;
    --uiSurplus;
    unsigned short usQueryType = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;
    unsigned short usQueryClass = ntohs(*((unsigned short*)pChQueries));
    pChQueries += 2;
    uiSurplus -= 2;


    //������Դ���ݶ�
    unsigned char* pChAnswers = pChQueries;
    while (0 < uiSurplus && uiSurplus <= uiTotal)
    {
        // ����name�ֶ�
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
        // ����TTL�ֶ�
        pChAnswers += 4;
        uiSurplus -= 4;
        //�ҵ�����һ��IP
        //type=A
        if (usAnswerType == 1)
        {
            //����length
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
                PDNSPackageRec->AuthorityRRs = htons(0x1);//��ʾ������֧�ֵݹ�
                // ���͸��ͻ���
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRec, iRet, len);
                return 0;
            }
            else
            {
                PDNSPackageRec->AuthorityRRs = 0;//��ʾ��������֧�ֵݹ�
                //���͸��ͻ���
                netSendMess(socke, clntAddr, DomainName, (unsigned char*)a, PDNSPackageRec, iRet, len);
                //����һ����ѯ
                fhnextServ(socke, clntAddr, PDNSPackageRec, iRet, len, (unsigned char*)a, szBuffer, DomainName);
            }
        }
        //���ֶη��صĲ���IP
        else
        {
            unsigned short leng = ntohs(*((unsigned short*)pChAnswers));

            leng += 2;

            pChAnswers += leng;
            uiSurplus -= leng;
        }
    }
    printf("���һ��\n");
    return 0;
}


//��ѯ����DNS
int receClient(SOCKET* sock)
{
    //�ͻ��˵�ַ��Ϣ
    SOCKADDR clntAddr;  
    // ������յ�������
    char szBuffer[256] = {};        
    int iFromLen = sizeof(sockaddr_in);

    int iRet = ::recvfrom(*sock, szBuffer, 256, 0, (sockaddr*)&clntAddr, &iFromLen);
    printf("\n%d\n", iRet);



    /* �����յ������� */
    DNSHeader* PDNSPackageRecv = (DNSHeader*)szBuffer;
    //ѡ��ݹ�����
    int mode = PDNSPackageRecv->RD;
    unsigned short usTransID = PDNSPackageRecv->usTransID;
    
    // ���ܵ����ܵ��ֽ���
    unsigned int uiSurplus = iRet;  
  //  printf("%u\n", htons(PDNSPackageRecv->usTransID));
    // ��ȡQueries�е������ֶ�
    unsigned char* pChQueries = (unsigned char*)PDNSPackageRecv + sizeof(DNSHeader);
    //  printf("\n%s\n", pChQueries);
    uiSurplus -= sizeof(DNSHeader);
    uiSurplus -= 4;

    int len = 0;//��������

    char* DomainName=(char*)malloc(256*sizeof(char));
    //ÿ����������
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

    //��ѯ���ص�ַ��
    char* StrLine=(char*)malloc(256 * sizeof(char));
    unsigned char IP[100];
    int flag = -1;//����������
    int t;
    FILE* fp = fopen("IP.txt", "r");
    while (!feof(fp))
    {
        //��ȡһ��
        fgets(StrLine, 1024, fp);
        t = strlen(StrLine);//ע���ļ��еĻ��з�
        StrLine[t - 1] = '\0';
        if (strcmp(StrLine, DomainName) == 0)
        {
            flag = 0;
            fgets((char*)IP, 1024, fp);  //��ȡIP
            t = strlen((char*)IP);
            IP[t - 1] = '\0';
  //          printf("\n%s\n", IP);
                break;
        }
    }
    fclose(fp);
    //���ص�ַ�����ҵ�����
    //��װ��Ӧ���Ĳ��������ͻ���
    if(flag==0)
    {
        //�����ѯ��������
        FILE* fped = fopen("ed.txt", "a");//"a" �򿪣�ָ���ļ�β�����Ѵ����ļ���׷��
        fprintf(fp, "%s\n", StrLine);
        fprintf(fp, "%s\n", IP);
        fclose(fped);

        //������Ӧ����
        locaSendMess(sock,clntAddr, DomainName,IP,PDNSPackageRecv,iRet,len);
    }
    //���ص�ַ�������ڲ��ҵ�����
    //��Ҫ���������������н���
    //����  or  ����
    else
    {
        if (mode == 0)//����
        {
            printf("����\n");
            IteraQuery(sock, clntAddr, PDNSPackageRecv, DomainName, iRet, len);
          }
        else//����
        {
            printf("����\n");
            RecurQuery(sock, clntAddr, PDNSPackageRecv, DomainName, iRet, len);
        }
        
    }
    return 0;
    
}


int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    //�����׽���
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    //���׽���
    sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));  //ÿ���ֽڶ���0���
    servAddr.sin_family = PF_INET;  //ʹ��IPv4��ַ
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); //�Զ���ȡIP��ַ
    servAddr.sin_port = htons(53);  //�˿�
    bind(sock, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));

    //���տͻ�������
    int a;
    while (1)
    {
        printf("����...\n");
        //���Ȳ�ѯ���ص�ַ��
        receClient(&sock);
    }
    //����
    closesocket(sock);
    WSACleanup();
    return 0;
}